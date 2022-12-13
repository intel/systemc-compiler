/*************************************************************************

  This file is part of the ParaNut project.

  Copyright (C) 2010-2022 Alexander Bahle <alexander.bahle@hs-augsburg.de>
                          Gundolf Kiefer <gundolf.kiefer@hs-augsburg.de>
                          Christian H. Meyer <christian.meyer@hs-augsburg.de>
      Hochschule Augsburg, University of Applied Sciences

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *************************************************************************/


#include "base.h"

#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <systemc.h>

// *********** Dynamic Configuration ************


int pn_cfg_vcd_level = 0;
int pn_cfg_insn_trace = 0;
bool pn_cfg_disable_cache = 0;
bool pn_cfg_debug_mode = 0;


// **************** Tracing *********************


bool pn_trace_verbose = false;


std::string pn_GetTraceName (sc_object *obj, const char *name, int dim, int arg1, int arg2) {
    std::string ret;

    if (dim < 0 || dim > 2)
        PN_ERRORF (("pn_GetTraceName: Parameter dim outside of range (0-2): %d", dim));

    // Read full object name and get the base module name
    ret = obj->name ();
    ret = ret.substr (0, ret.find_last_of ('.') + 1);

    // Add name...
    ret += name;

    // Add first dimension...
    if (dim > 0)
        ret += "(" + std::to_string (arg1) + ")";

    // Add second dimension...
    if (dim == 2)
        ret += "(" + std::to_string (arg2) + ")";

    return ret;
}


// **************** Testbench helpers ***********


sc_trace_file *pn_trace_file = NULL;


char *pn_TbPrintf (const char *format, ...) {
    static char buf[200];

    va_list ap;
    va_start (ap, format);
    vsprintf (buf, format, ap);
    return buf;
}


void pn_TbAssert (bool cond, const char *msg, const char *filename, const int line) {
    if (!cond) {
        fprintf (stderr, "ASSERTION FAILURE: %s, %s:%i", sc_time_stamp ().to_string ().c_str (), filename, line);
        if (msg)
            fprintf (stderr, ": %s\n", msg);
        else
            fprintf (stderr, "\n");
        if (pn_trace_file) sc_close_vcd_trace_file (pn_trace_file);
        abort ();
    }
}


void pn_TbInfo (const char *msg, const char *filename, const int line) {
    int time_size = sc_time_stamp ().to_double () == 0.0 ? 1 : 15;
    fprintf (stderr, "(INFO): %*s, %s:%i:   %s\n", time_size, sc_time_stamp ().to_string ().c_str (), filename, line, msg);
}


void pn_TbWarning (const char *msg, const char *filename, const int line) {
    int time_size = sc_time_stamp ().to_double () == 0.0 ? 1 : 12;
    fprintf (stderr, "(WARNING): %*s, %s:%i: %s\n", time_size, sc_time_stamp ().to_string ().c_str (), filename, line, msg);
}


void pn_TbError (const char *msg, const char *filename, const int line) {
    int time_size = sc_time_stamp ().to_double () == 0.0 ? 1 : 14;
    fprintf (stderr, "(ERROR): %*s, %s:%i: %s\n", time_size, sc_time_stamp ().to_string ().c_str (), filename, line, msg);
    if (pn_trace_file) sc_close_vcd_trace_file (pn_trace_file);
    exit (3);
}


// **************** DisAss **********************


char *pn_DisAss (TWord insn) {
    static char ret[80] = "";
    TWord opcode, funct3, funct7, rs1, rs2, rd, bit30, bit20, bit21, bit25, bit28, bit29, itype, utype, btype, jtype, stype;
    sc_uint<32> inst = insn;

    opcode = insn & 0x7f;
    funct3 = (insn >> 12) & 0x7;
    funct7 = (insn >> 25);

    rd = (insn >> 7) & 0x1f;
    rs1 = (insn >> 15) & 0x1f;
    rs2 = (insn >> 20) & 0x1f;

    bit20 = (insn >> 20) & 0x1;
    bit21 = (insn >> 21) & 0x1;
    bit25 = (insn >> 25) & 0x1;
    bit28 = (insn >> 28) & 0x1;
    bit29 = (insn >> 28) & 0x1;
    bit30 = (insn >> 30) & 0x1;

    itype = ((insn >> 20) ^ 0x800) - 0x800;
    utype = insn & 0xFFFFF000;
    btype = ((sc_uint<32>)(inst[31], inst[7], inst (30, 25), inst (11, 8), 0) ^ 0x1000) - 0x1000;
    jtype = ((sc_uint<32>)(inst[31], inst (19, 12), inst[20], inst (30, 25), inst (24, 21), 0) ^ 0x100000) - 0x100000;
    stype = ((((sc_uint<32>)(inst (31, 25), 0, 0, 0, 0, 0)).value () + rd) ^ 0x800) - 0x800;

    strcpy (ret, "  ");

    // ALU instructions...
    if (opcode == 0x13) { // OP_IMM
        const char *table[] = { "addi", "slli", "slti", "sltiu", "xori", "srli", "ori", "andi" };
        if (funct3 == 5 || funct3 == 1) {
            if (bit30) table[funct3] = "srai";
            itype &= 0x1F; // shamt
        }
        sprintf (ret + 2, "%s r%i, r%i, 0x%x", table[funct3], rd, rs1, itype);
    } else if (opcode == 0x33 && !bit25) { // OP (Bit 25 is on for M-Extension (DIV/MUL...)
        const char *table[] = { "add", "sll", "slt", "sltu", "xor", "srl", "or", "and" };
        if (funct3 == 0) {
            if (bit30) table[funct3] = "sub";
        } else if (funct3 == 5) {
            if (bit30) table[funct3] = "sra";
        }
        sprintf (ret + 2, "%s r%i, r%i, r%i", table[funct3], rd, rs1, rs2);
    } else if ((opcode == 0x33 && bit25)) { // OP - M-Extension
        static const char *table[] = { "mul", "mulh", "mulhsu", "mulhu",
                                       "div", "divu", "rem",    "remu" };
        sprintf (ret + 2, "%s r%i, r%i, r%i", table[funct3], rd, rs1, rs2);
    } else if (opcode == 0x17) { // AUIPC
        sprintf (ret + 2, "auipc r%i, 0x%x", rd, utype);
    } else if (opcode == 0x37) { // LUI
        sprintf (ret + 2, "lui r%i, 0x%x", rd, utype);
    } else if (opcode == 0x63) { // BRANCH
        const char *table[] = { "beq", "bneq", "INV_SUB", "INV_SUB", "blt", "bge", "bltu", "bgeu" };
        sprintf (ret + 2, "%s r%i, r%i, 0x%x", table[funct3], rs1, rs2, btype);
    } else if (opcode == 0x6F) { // JAL
        sprintf (ret + 2, "jal r%i, 0x%x", rd, jtype);
    } else if (opcode == 0x67) { // JALR
        sprintf (ret + 2, "jalr r%i, r%i, 0x%x", rd, rs1, itype);
    } else if (opcode == 0x03) { // LOAD
        static const char *table[] = { "lb", "lh", "lw", "INV_SUB", "lbu", "lhu" };
        sprintf (ret + 2, "%s r%i, 0x%x(r%i)", table[funct3], rd, itype, rs1);
    } else if (opcode == 0x23) { // STORE
        static const char *table[] = { "sb", "sh", "sw" };
        sprintf (ret + 2, "%s r%i, 0x%x(r%i)", table[funct3], rs2, stype, rs1);
    } else if (opcode == 0x73) { // SYSTEM
        const char *table[] = { "ecall",   "csrrw",  "csrrs",  "csrrc",
                                "INV_SUB", "csrrwi", "csrrsi", "csrrci" };
        if (bit25 && bit28) table[0] = "sfence.vma";
        if (bit21) table[0] = bit28 ? bit29 ? "mret" : "sret" : "uret";
        if (bit20) table[0] = "ebreak";
        if (funct3 == 0)
            sprintf (ret + 2, "%s", table[funct3]);
        else if (funct3 < 4)
            sprintf (ret + 2, "%s r%i, 0x%x, r%i", table[funct3], rd, itype, rs1);
        else
            sprintf (ret + 2, "%s r%i, 0x%x, 0x%x", table[funct3], rd, itype, rs1);
    } else if (opcode == 0x0F) {
        sprintf (ret + 2, "fence");
    } else if (opcode == 0xB) { // PARA
        static const char *table[] = { "halt", "cinvalidate", "cwriteback", "cflush" };
        if (funct3 == 0)
            sprintf (ret + 2, "%s", table[funct3]);
        else
            sprintf (ret + 2, "%s 0x%x(r%i) ", table[funct3], itype, rs1);
    } else if (opcode == 0x2F) { // AMO
        if (funct3 == 2 && (funct7 >> 2) == 2) // LR.W
            sprintf (ret + 2, "lr.w r%i, (r%i)", rd, rs1);
        else if (funct3 == 2 && (funct7 >> 2) == 3) // SC.W
            sprintf (ret + 2, "sc.w r%i, r%i, (r%i) ", rd, rs2, rs1);
        else
            sprintf (ret + 2, "AMO_INVALID r%i, r%i, (r%i) ", rd, rs2, rs1);
    } else
        sprintf (ret, "? 0x%08x ?", insn);

    return ret;
}


// **************** Performance measuring *****************


void CPerfMon::Init (int events, CEventDef *ev_tab) {
    events_ = events;
    ev_tab_ = ev_tab;
    count_tab_ = new int[events];
    time_tab_ = new double[events];
    min_tab_ = new double[events];
    max_tab_ = new double[events];
    Reset ();
}


void CPerfMon::Done () {
    if (events_ > 0) {
        delete[] count_tab_;
        delete[] time_tab_;
        delete[] min_tab_;
        delete[] max_tab_;
    }
}


void CPerfMon::Reset () {
    last_no_ = -1;
    for (int n = 0; n < events_; n++) {
        count_tab_[n] = 0;
        time_tab_[n] = max_tab_[n] = 0.0;
        min_tab_[n] = DBL_MAX;
    }
}


void CPerfMon::Count (int ev_no) {
    double curStamp = sc_time_stamp ().to_double ();

    if (last_no_ >= 0) {
        if (ev_tab_[last_no_].is_timed) {
            double t = curStamp - last_stamp_;
            if (t == 0) // Time of 0 between two events is not plausible
                return;
            time_tab_[last_no_] += t;
            if (t < min_tab_[last_no_])
                min_tab_[last_no_] = t;
            if (t > max_tab_[last_no_])
                max_tab_[last_no_] = t;
        }
    }

    count_tab_[ev_no]++;
    if (ev_tab_[ev_no].is_timed) last_stamp_ = curStamp;
    last_no_ = ev_no;
}


static void DisplayLine (const char *name, int count, int avg_count, double total, double min, double max, bool is_timed) {
    if (avg_count > 0 && is_timed)
        fprintf (stderr, "(PERF):   %-10s %7i   %8.1lf %8.1lf %8.1lf %11.1lf\n", name, count, min,
                total / avg_count, max, total);
    else
        fprintf (stderr, "(PERF):   %-10s %7i\n", name, count);
}


void CPerfMon::Display (const char *name) {
    double time_total, min_total, max_total;
    int count_total, avg_count_total;

    fprintf (stderr, "(PERF):\n"
            "(PERF): ********** Performance statics ");
    if (name) fprintf (stderr, "of unit '%s'", name);
    fprintf (stderr, "\n"
            "(PERF):\n"
            "(PERF):                          Time [ns]\n"
            "(PERF):   Event        Count        min      avg      max       Total\n"
            "(PERF):   -----------------------------------------------------------\n");
    count_total = avg_count_total = 0;
    time_total = max_total = 0.0;
    min_total = DBL_MAX;
    for (int n = 0; n < events_; n++) {
        DisplayLine (ev_tab_[n].name, count_tab_[n], count_tab_[n], time_tab_[n], min_tab_[n],
                     max_tab_[n], ev_tab_[n].is_timed);
        count_total += count_tab_[n];
        if (ev_tab_[n].is_timed) avg_count_total += count_tab_[n];
        time_total += time_tab_[n];
        if (min_tab_[n] < min_total) min_total = min_tab_[n];
        if (max_tab_[n] > max_total) max_total = max_tab_[n];
    }
    fprintf (stderr, "(PERF):   -----------------------------------------------------------\n");
    DisplayLine ("Total", count_total, avg_count_total, time_total, min_total, max_total, true);
    fprintf (stderr, "(PERF):\n");
}


// ***** CPerfMonCPU *****


void CPerfMonCPU::Init () {
    static CEventDef CPU_events[] = {
        { "ALU", true }, { "Load", true }, { "Store", true }, { "Jump", true }, { "Other", true }, { "IFetch", true }
    };
    CPerfMon::Init (6, CPU_events);
}
