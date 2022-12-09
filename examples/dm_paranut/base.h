/************************************************************************
*
  This file is part of the ParaNut project.

  Copyright (C) 2010-2022 Alexander Bahle <alexander.bahle@hs-augsburg.de>
                          Gundolf Kiefer <gundolf.kiefer@hs-augsburg.de>
                          Christian H. Meyer <christian.meyer@hs-augsburg.de>
      Hochschule Augsburg, University of Applied Sciences

  Description:
    This module contains various types, constants and helper functions
    for the SystemC model of ParaNut.

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

////////////////////////////////////////////////////////////////////////////////////////////////
/// @mainpage
/// This is the documentation for the **SystemC model** of the <em>ParaNut</em> processor. It
/// is meant as a reference for developers interested in
/// - creating ParaNut comaptible Wishbone bus hardware,
/// - improving the SystemC simulation model capabilities and
/// - adding functionality to the ParaNut processor.
///
/// The main docoumentation containing the definition of the ParaNut architecture and design rules
/// can be found in the [*ParaNut Manual*](../../doc/paranut-manual.pdf). If you only want to
/// develop software for a system featuring ParaNut processor take a look at the libparanut
/// documentation [*libparanut Manual*](../../doc/libparanut_manual.pdf).
///
/// To get started, navigate to the [Modules](modules.html) page.
///
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_
#define _BASE_

/// @file
/// @brief Helpers, Makros and performance measuring Classes used in most *ParaNut* files.
/// @defgroup helpers Helpers
/// @brief Helpers, Makros and performance measuring Classes used in most *ParaNut* files.
/// @{


#include <systemc.h>
#include <sstream>

// Print useful information during HLS. The tool analyses the source code once without the 
// __SYNTHESIS__ define set and once with the define set. This printed info helps to discern the two.  
//  Uses #warning, because Vivado HLS does not print #info messages 
#ifndef SIMBUILD
#ifdef __SYNTHESIS__
#warning "INFO: __SYNTHESIS__ is set! This is just a ParaNut debug info."
#else
#warning "INFO: __SYNTHESIS__ is not set! This is just a ParaNut debug info."
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
/// @name Static Configuration...
/// @{

/// @brief System endianess configuration
///
/// Can be used to switch between 0 - little endian and 1 - big endian for testing purposes.
///
/// __CAUTION:__ For RISC-V this should always be 0.
#define PN_BIG_ENDIAN 0

/// @} // @name Static Configuration...
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @name Dynamic Configuration....
/// @{

/// @brief VCD trace level.
///
/// Holds the VCD trace level (0 = no VCD file) set by the "-t" command line option supplied to the
/// MParaNutSystem constructor.
extern int pn_cfg_vcd_level;

/// @brief Internal simulation instruction trace level.
///
/// Holds the internal simulation instruction trace level set by the "-i" command line option supplied to the
/// MParaNutSystem constructor.
/// \arg 0 - no trace
/// \arg 1 - instruction trace (shows register changes)
/// \arg >2 - same as 1 + prints complete register file after each instruction
extern int pn_cfg_insn_trace;

/// @brief Cache enable override.
///
/// Holds the disable cache bit set by the "-c" command line option supplied to the MParaNutSystem constructor.
/// If set, disables instruction and data caching irrespective of the value in the pncache CSR.
extern bool pn_cfg_disable_cache;

/// @brief Interactive debug mode enable.
///
/// Holds the debug mode enable bit set by the "-d" command line option supplied to the MParaNutSystem
/// constructor. If set, the MParaNutSystem will set up a remote bitbang interface on port 9824 and waits
/// for you to connect to it. Use OpenOCD and GDB to debug software running in the SystemC model.
///
/// See doc/paranut_manual.pdf for more information.
extern bool pn_cfg_debug_mode;

///@} // @name Dynamic Configuration....
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @name Basic types and constants...
/// @{

/// @brief Number of Bytes in a Kilobyte.
#define KB 1024
/// @brief Number of Bytes in a Megabyte.
#define MB (1024 * 1024)
/// @brief Number of Bytes in a Gigabyte.
#define GB (1024 * 1024 * 1024)

/// @brief Byte type (8 Bit).
typedef unsigned char TByte;
/// @brief Half word type (16 Bit).
typedef unsigned short THalfWord;
/// @brief Word type (32 Bit).
typedef unsigned TWord;
/// @brief Double word type (64 Bit).
typedef unsigned long long TDWord;

/// @brief Minimum of A and B.
#define MIN(A, B) ((A) < (B) ? (A) : (B))
/// @brief Maximum of A and B.
#define MAX(A, B) ((A) > (B) ? (A) : (B))
/// @brief Number of bits necessary to encode A.
#define NUM_BITS(A) ((int)(ceil(log2 (A))))
/// @brief Convenient macro to compare a member variable with the member of input t
#define C(MEMBER) (MEMBER == t.MEMBER)
/// @brief Same as C(MEMBER), but for arrays
#define C_ARR(MEMBER, NUM) ({ bool ret = 1; for (int n = 0; n < NUM; ++n) if (!C(MEMBER[n])) ret = 0; ret; })


/// @brief Number of instruction/register Bits.
#define XLEN 32

///@} // @name Basic types and constants...
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @name SystemC tracing...
/// @{

/// @brief Output verbose tracing information.
///
/// If set, the system modules will output verbose information when they trace their internal
/// signals.
extern bool pn_trace_verbose;


/// @brief Generates and returns a trace file compatible string.
///
/// Generates and returns a trace file compatible string using the supplied information to
/// parse multidimensional inputs.
/// \param obj is the sc_object (port/signal) pointer.
/// \param name is the base name appearing in the trace file.
/// \param dim is the number of dimensions the obj has (only 0 to 2 are supported).
/// \param arg1 is the number to add to the base name to represent the first dimension
/// (should be 0 for dim < 1).
/// \param arg2 is the number to add to the base name + first dimension to represent the second
/// dimension (should be 0  for dim < 2).
/// \return A std::string formatted for the sc_trace() function (e.g. MODULE.name(arg1)(arg2)).
#ifndef __SYNTHESIS__
std::string pn_GetTraceName (sc_object *obj, const char *name, int dim, int arg1, int arg2);
#else
// std::string pn_GetTraceName (...) {
//     return "\0";
// }
#endif

/// @brief Add sc_object OBJ to the trace file TF.
///
/// Macro for easily adding the sc_object (port/signal) to a trace file. Uses pn_GetTraceName to
/// generate the correct object name.
/// \param TF is the trace file.
/// \param OBJ is the sc_object (port/signal).
#define PN_TRACE(TF, OBJ)                                                                \
    {                                                                                    \
        if (TF) sc_trace (TF, OBJ, pn_GetTraceName (&(OBJ), #OBJ, 0, 0, 0));             \
        if (!TF || pn_trace_verbose) cout << "  " #OBJ " = '" << (OBJ).name () << "'\n"; \
    }

/// @brief Add each sc_object of OBJ array/bus to the trace file TF.
///
/// Macro for easily adding the array/bus of sc_objects (port/signal) to a trace file. Uses
/// pn_GetTraceName to generate the correct object name.
/// \param TF is the trace file.
/// \param OBJ is the array/bus of sc_objects (port/signal).
/// \param N_MAX is number of array/bus elements.
#define PN_TRACE_BUS(TF, OBJ, N_MAX)                                                     \
    {                                                                                    \
        for (int n = 0; n < N_MAX; n++) {                                                \
            if (TF) sc_trace (TF, (OBJ)[n], pn_GetTraceName (&(OBJ)[n], #OBJ, 1, n, 0)); \
            if (!TF || pn_trace_verbose)                                                 \
                cout << "  " #OBJ "[" << n << "] = '" << (OBJ)[n].name () << "'\n";      \
        }                                                                                \
    }

/// @brief Add each sc_object of 2D OBJ array/bus to the trace file TF.
///
/// Macro for easily adding the two dimensional array/bus of sc_objects (port/signal) to a trace file.
/// Uses pn_GetTraceName to generate the correct object name.
/// \param TF is the trace file.
/// \param OBJ is the array/bus of sc_objects (port/signal).
/// \param N_MAX is number of array/bus elements in the first dimension.
/// \param K_MAX is number of array/bus elements in the second dimension.
#define PN_TRACE_BUS_BUS(TF, OBJ, N_MAX, K_MAX)                                                         \
    {                                                                                                   \
        for (int n = 0; n < N_MAX; n++)                                                                 \
            for (int k = 0; k < K_MAX; k++) {                                                           \
                if (TF) sc_trace (TF, (OBJ)[n][k], pn_GetTraceName (&(OBJ)[n][k], #OBJ, 2, n, k));      \
                if (!TF || pn_trace_verbose)                                                            \
                    cout << "  " #OBJ "[" << n << "][" << k << "] = '" << (OBJ)[n][k].name () << "'\n"; \
            }                                                                                           \
    }

/// @brief Helper macro for recursively calling sc_trace in own types/structs.
///
/// Macro for easily adding a sc_object member (port/signal) of an sc_object to a trace file.
/// Use PN_TRACE_R when overloading sc_trace for own types/structs to easily trace its members.
/// \param TF is the trace file.
/// \param OBJ is the sc_objects having the member MEMBER.
/// \param MEMBER is the member sc_object (port/signal).
/// \param STR is the calling sc_trace string to which the member name will be appended.
#define PN_TRACE_R(TF, OBJ, MEMBER, STR)                        \
    {                                                           \
        if (TF) sc_trace (TF, (OBJ).MEMBER, STR + "."#MEMBER);  \
    }

/// @brief Helper macro for recursively calling sc_trace in own types/structs.
///
/// Macro for easily adding each sc_object of an array member (ports/signals) of an sc_object to a
/// trace file.
/// Use PN_TRACE_R_BUS when overloading sc_trace for own types/structs to easily trace its members.
/// \param TF is the trace file.
/// \param OBJ is the sc_objects having the member MEMBER.
/// \param MEMBER is the member sc_object (port/signal).
/// \param STR is the calling sc_trace string to which the member name + index will be appended.
/// \param N_MAX is number of MEMBER array/bus elements.
#define PN_TRACE_R_BUS(TF, OBJ, MEMBER, STR, N_MAX)                                                           \
    {                                                                                                         \
        std::stringstream ss;                                                                                 \
        for (int n = 0; n < N_MAX; n++) {                                                                     \
            ss << n;                                                                                          \
            if (TF) sc_trace (TF, (OBJ).MEMBER[n], STR + "."#MEMBER"(" + ss.str().c_str() + ")");             \
        }                                                                                                     \
    }

/// @brief Helper macro for recursively calling sc_trace in own types/structs.
///
/// Macro for easily adding each sc_object of an two-dimensional array member (ports/signals) of an sc_object to a
/// trace file.
/// Use PN_TRACE_R_BUS_BUS when overloading sc_trace for own types/structs to easily trace its members.
/// \param TF is the trace file.
/// \param OBJ is the sc_objects having the member MEMBER.
/// \param MEMBER is the member sc_object (port/signal).
/// \param STR is the calling sc_trace string to which the member name + index will be appended.
/// \param N_MAX is number of array/bus elements in the first dimension.
/// \param K_MAX is number of array/bus elements in the second dimension.
#define PN_TRACE_R_BUS_BUS(TF, OBJ, MEMBER, STR, N_MAX, K_MAX)                                                \
    {                                                                                                         \
        std::stringstream ss_n, ss_k;                                                                         \
        for (int n = 0; n < N_MAX; n++) {                                                                     \
            for (int k = 0; k < K_MAX; k++) {                                                                 \
                ss_n << n;                                                                                    \
                ss_k << k;                                                                                    \
                if (TF) sc_trace (TF, (OBJ).MEMBER[n][k], STR + "."#MEMBER"(" + ss_n.str().c_str() + ")(" + ss_k.str().c_str() + ")");             \
            }                                                                                                 \
        }                                                                                                     \
    }

///@} // @name SystemC tracing...
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @name Testbench helpers...
/// @{

/// @brief ParaNut trace file pointer.
///
/// If set the trace file is closed using sc_close_vcd_trace_file() after errors and asserts.
extern sc_trace_file *pn_trace_file;

/// @brief Testbench printf helper.
///
/// Returns a C string formatted according to supplied format and addistional arguments.
///
/// \param format is a C string that contains the text to be formatted.
/// \param ... (additional arguments) are the parameters according to the format string.
/// \return formatted C string according to supplied format and additional arguments.
char *pn_TbPrintf (const char *format, ...);
/// @brief Testbench assert helper.
///
/// Asserts condition cond and if it fails prints the supplied msg, filename and line to stderr, closes
/// the pn_trace_file and aborts program execution.
///
/// Should be called through the PN_ASSERT(), PN_ASSERTF() and PN_ASSERTM() macros.
///
/// \param cond is the condition to assert.
/// \param msg is a C string to print if the assertion fails.
/// \param filename is a C string to print if the assertion fails containig the source filename.
/// \param line is the line number in the source file to print if the assertion fails.
void pn_TbAssert (bool cond, const char *msg, const char *filename, const int line);
/// @brief Testbench information helper.
///
/// Prints the supplied msg to stderr with a defined header containing the current SytemC simulation
/// time stamp, filename and line and the "(INFO):" keyword.
///
/// Should be called through the PN_INFO() and PN_INFOF() macros.
///
/// \param msg is a C string to print.
/// \param filename is a C string to print containig the source filename.
/// \param line is the line number in the source file to print.
void pn_TbInfo (const char *msg, const char *filename, const int line);
/// @brief Testbench warning helper.
///
/// Prints the supplied msg to stderr with a defined header containing the current SytemC simulation
/// time stamp, filename and line and the "(WARNING):" keyword.
///
/// Should be called through the PN_WARNING() and PN_WARNINGF() macros.
///
/// \param msg is a C string to print.
/// \param filename is a C string to print containig the source filename.
/// \param line is the line number in the source file to print.
void pn_TbWarning (const char *msg, const char *filename, const int line);
/// @brief Testbench error helper.
///
/// Prints the supplied msg to stderr with a defined header containing the current SytemC simulation
/// time stamp, filename and line and the "(ERROR):" keyword. Also closes the pn_trace_file and
/// exits the program execution with an error.
///
/// Should be called through the PN_ERROR() and PN_ERRORF() macros.
///
/// \param msg is a C string to print.
/// \param filename is a C string to print containig the source filename.
/// \param line is the line number in the source file to print
void pn_TbError (const char *msg, const char *filename, const int line);


#ifndef __SYNTHESIS__
/// @brief Testbench assert without message.
///
/// Calls pn_TbAssert() with the condition COND, an empty message and the filename and line macros.
///
/// \param COND is the condition to assert.
#define PN_ASSERT(COND) pn_TbAssert (COND, NULL, __FILE__, __LINE__)
/// @brief Testbench assert with formatted message.
///
/// Calls pn_TbAssert() with the condition COND, the formatted message according to FMT and the
/// filename and line macros.
///
/// __NOTE__: You need to put the FMT argument in brackets (e.g. PN_ASSERTF(1, ("E: %d", 5));)
///
/// \param COND is the condition to assert.
/// \param FMT is the format C string and all additional arguments needed to print if the assertion fails.
#define PN_ASSERTF(COND, FMT) pn_TbAssert (COND, pn_TbPrintf FMT, __FILE__, __LINE__)
/// @brief Testbench assert with message.
///
/// Calls pn_TbAssert() with the condition COND, the supplied message MSG and the
/// filename and line macros.
///
///
/// \param COND is the condition to assert.
/// \param MSG is the C string to print if the assertion fails.
#define PN_ASSERTM(COND, MSG) pn_TbAssert (COND, MSG, __FILE__, __LINE__)

/// @brief Testbench info with message.
///
/// Calls pn_TbInfo() with the supplied message MSG and the filename and line macros.
///
/// \param MSG is the C string to print.
#define PN_INFO(MSG) pn_TbInfo (MSG, __FILE__, __LINE__)
/// @brief Testbench info with formatted message.
///
/// Calls pn_TbInfo() with the formatted message according to FMT and the filename and line macros.
///
/// __NOTE__: You need to put the FMT argument in brackets (e.g. PN_INFO(("Val: %d", 5));)
///
/// \param FMT is the format C string and all additional arguments needed to print.
#define PN_INFOF(FMT) pn_TbInfo (pn_TbPrintf FMT, __FILE__, __LINE__)

/// @brief Testbench warning with message.
///
/// Calls pn_TbWarning() with the supplied message MSG and the filename and line macros.
///
/// \param MSG is the C string to print.
#define PN_WARNING(MSG) pn_TbWarning (MSG, __FILE__, __LINE__)
/// @brief Testbench warning with formatted message.
///
/// Calls pn_TbWarning() with the formatted message according to FMT and the filename and line macros.
///
/// __NOTE__: You need to put the FMT argument in brackets (e.g. PN_WARNINGF(("Val: %d", 5));)
///
/// \param FMT is the format C string and all additional arguments needed to print.
#define PN_WARNINGF(FMT) pn_TbWarning (pn_TbPrintf FMT, __FILE__, __LINE__)

/// @brief Testbench error with message.
///
/// Calls pn_TbError() with the supplied message MSG and the filename and line macros.
///
/// \param MSG is the C string to print.
#define PN_ERROR(MSG) pn_TbError (MSG, __FILE__, __LINE__)
/// @brief Testbench error with formatted message.
///
/// Calls pn_TbError() with the formatted message according to FMT and the filename and line macros.
///
/// __NOTE__: You need to put the FMT argument in brackets (e.g. PN_ERRORF(("Val: %d", 5));)
///
/// \param FMT is the format C string and all additional arguments needed to print.
#define PN_ERRORF(FMT) pn_TbError (pn_TbPrintf FMT, __FILE__, __LINE__)


/// @brief vh_open struct (vhdl open equivalent)
///
/// This static constant struct can be used to connect unused ports to "nothing" to avoid elaboration
/// warnings and errors.
static struct {
    template <typename T> operator sc_core::sc_signal_inout_if<T> & () const {
        return *(new sc_core::sc_signal<T> (sc_core::sc_gen_unique_name ("vh_open")));
    }

} const vh_open = {};

/// @brief vvh_const type/struct (vhdl constant value equivalent)
///
/// This type can be used to easily connect ports or signals to a "constant" value.
///
/// @tparam typename is the type the constant value will be applied to.
/// @param v is the constant value.
/// @todo The current implementation has a memory leak.
template <typename T>
sc_core::sc_signal_in_if<T> const &vh_const (T const &v) // keep the name consistent with vh_open
{
    // Yes, this is an (elaboration-time) memory leak.  You can avoid it with some extra effort
    sc_core::sc_signal<T> *sig_p =
    new sc_core::sc_signal<T> (sc_core::sc_gen_unique_name ("vh_const"));
    sig_p->write (v);
    return *sig_p;
}

#else // #ifdef __SYNTHESIS__
#define PN_ASSERT(COND)
#define PN_ASSERTF(COND, FMT)
#define PN_ASSERTM(COND, MSG)

#define PN_INFO(MSG)
#define PN_INFOF(FMT)

#define PN_WARNING(MSG)
#define PN_WARNINGF(FMT)

#define PN_ERROR(MSG)
#define PN_ERRORF(FMT)
#endif // #ifndef __SYNTHESIS__


/// @brief Dissassemble RISC-V instructions to C string.
///
/// Takes the 32 Bit RISC-V instruction insn and disassembles it into a human readable C string.
/// Disassembles all instructions the ParaNut processor can execute (RV32IMA currently).
/// Returns "? 0xHEX_VALUE_OF_INSN" if the instruction is unknown or invalid.
///
/// __NOTE:__ The returned string is only valid until the next call to this function.
/// @param insn is the instruction to disassemble.
/// @return A human readable C string representing the disassemble of insn.
char *pn_DisAss (TWord insn);

///@} // @name Testbench helpers...
////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
/// @name Performance measuring...
/// @{

/// @brief Event definition class.
class CEventDef {
public:
    /// @brief Event name for displaying.
    const char *name;
    /// @brief 0 - event is just counted, 1 - event is timed.
    bool is_timed;
};


/// @brief Performance monitor class.
class CPerfMon {
public:
    /// @brief Default constructor, 0 events and no event definitions.
    CPerfMon () { Init (0, NULL); }
    /// @brief Constructor for supplying number of events and event definition(s).
    CPerfMon (int events, CEventDef *ev_tab) { Init (events, ev_tab); }
    /// @brief Destructor.
    ~CPerfMon () { Done (); }

    /// @brief Init performance monitor for supplied number of events and event definition(s).
    void Init (int events, CEventDef *ev_tab);
    /// @brief Free reserved memory for performance monitoring.
    void Done ();

    /// @brief Reset performance monitor (counted events/times, not number of events and event definition(s)).
    void Reset ();
    /// @brief Count an event at current simulation time.
    void Count (int ev_no);

    /// @brief Display the collected performance information.
    void Display (const char *name = NULL);

protected:
    /// @brief Number of events this performance monitor monitors.
    int events_;
    /// @brief Pointer to event definition(s).
    CEventDef *ev_tab_;
    /// @brief Event counter table.
    int *count_tab_;
    /// @brief Event total time table.
    double *time_tab_,
    /// @brief Event minimal time table.
           *min_tab_,
    /// @brief Event maximum time table.
           *max_tab_;

    /// @brief Time stamp of last event.
    double last_stamp_;
    /// @brief Event number of last event.
    int last_no_;
};


// ***** CPerfMonCPU *****
/// @brief CPU performance monitor events enum.
typedef enum { EV_ALU = 0, EV_LOAD, EV_STORE, EV_JUMP, EV_OTHER, EV_IFETCH} EEventsCPU;

/// @brief CPU performance monitor class.
#ifndef __SYNTHESIS__
class CPerfMonCPU : public CPerfMon {
public:
    /// @brief Default constructor for @ref EEventsCPU events and event definitions.
    CPerfMonCPU () { Init (); }
    /// @brief Call CPerfMon::Init() for @ref EEventsCPU events and event definitions.
    void Init ();

    /// @brief Call CPerfMon::Count() after casting @ref EEventsCPU to int.
    void Count (EEventsCPU ev_no) { CPerfMon::Count ((int)ev_no); }
};
#else
class CPerfMonCPU  : public CPerfMon{
public:
    CPerfMonCPU () { Init(); }

    void Display () { /* nothing */
    }
    void Count (EEventsCPU ev_no) { /* nothing */
    }
    void Init ();
};
#endif

///@} // @name Performance measuring...
////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __SC_TOOL__
#define PN_CLOCK_TRIGGERED(method_name) SC_CTHREAD(method_name, clk.pos());
#else
#define PN_CLOCK_TRIGGERED(method_name) SC_METHOD(method_name) sensitive << clk.pos();
#endif
/// @}  // @file
#endif
