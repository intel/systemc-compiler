/******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Pipe module intended to pipeline arbitrary logic with register re-timing 
 * in logic synthesis tool. 
 * 
 * The module can be used at input or output of the logic to pipeline.
 * The generated code instantiates SystemVerilog pipeline register component
 * provided by a logic synthesis tool or an external library.
 * The SystemVerilog pipeline register name specified by constructor parameter.
 * 
 * Pipe module can be used in THREAD and METHOD process, put and get can be
 * done in the same process as well as in two processes. 
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_PIPE_H
#define SCT_PIPE_H

#include "sct_prim_fifo.h"
#include "sct_ipc_if.h"
#include "sct_utils.h"
#include <systemc.h>

namespace sct {

/// Cycle accurate implementation
template <
    class T, 
    unsigned N,         /// Number of pipeline registers, one or more
    class TRAITS
>
class sct_pl_reg : public sc_module
{
  public:    
    /// Don't generate RTL by ICSC tool (black-box mode)
    const std::string __SC_TOOL_VERILOG_MOD__{""};
    /// Module name override
    std::string __SC_TOOL_MODULE_NAME__;
    
    sc_in<bool>             SC_NAMED(clk);
    sc_in<bool>             SC_NAMED(rst_n);
    sc_in<sct_uint<N>>      SC_NAMED(enable);
    sc_in<T>                SC_NAMED(data_in);
    sc_out<T>               SC_NAMED(data_out);
    
    SC_HAS_PROCESS(sct_pl_reg);
    
    /// \param addInReg  -- add input register not moved by re-timing 
    /// \param addOutReg -- add output register not moved by re-timing
    /// \param rtlName   -- pipeline register instantiated component name
    explicit sct_pl_reg(const sc_module_name& name,
                        bool addInReg, bool addOutReg,
                        const std::string& rtlName) : 
        sc_module(name)
    {
        assert (N > 0 && "Number of registers should be one or more");
        assert (TRAITS::RESET == 0 && "Only active low reset supported");
        assert ((!addInReg || !addOutReg) && 
                 "Only input or output register can be added, not both");
        // Stage number          
        unsigned stageNum = (addInReg || addOutReg) ? N : (N + 1); 
        // Parameters: (width, in_reg, stages, out_reg, rst_mode)
        __SC_TOOL_MODULE_NAME__ = rtlName + " #(" +
                                  std::to_string(sct_width<T>()) + ", " +
                                  std::to_string(addInReg) + ", " + 
                                  std::to_string(stageNum) + ", " + 
                                  std::to_string(addOutReg) + ", 0)";

        #ifdef SCT_SEQ_METH
            SC_METHOD(plProc);
            sensitive << (TRAITS::CLOCK ? clk.pos() : clk.neg()) 
                      << (TRAITS::RESET ? rst_n.pos() : rst_n.neg()); 
        #else
            SCT_CTHREAD(plProc, clk, TRAITS::CLOCK);
            async_reset_signal_is(rst_n, TRAITS::RESET);
        #endif
    }
    
  protected:
    T regs[N];

    void plProc() 
    {
    #ifdef SCT_SEQ_METH
        if (TRAITS::RESET ? rst_n : !rst_n) {
    #endif
            for (unsigned i = 0; i < N; ++i) regs[i] = 0;
            data_out = 0;
    #ifdef SCT_SEQ_METH
        } else {
    #else
        wait();
        
        while (true) {
    #endif
            for (unsigned i = N-1; i != 0; --i) {
                if (enable.read().bit(i)) regs[i] = regs[i-1];
            }
            if (enable.read().bit(0)) regs[0] = data_in;
            data_out = regs[N-1];
    #ifndef SCT_SEQ_METH
            wait();
    #endif
        }
    }

  public:
    template<class RSTN_t, class Enable, class DataIn, class DataOut>
    void bind(sc_in_clk& clk_, RSTN_t& nrst_, Enable& enable_, 
              DataIn& data_in_, DataOut& data_out_) 
    {
        clk(clk_);
        rst_n(nrst_);
        enable(enable_);
        data_in(data_in_);
        data_out(data_out_);
    }
};


template <
    class T, 
    unsigned N,             /// Number of pipeline registers, one or more
    class TRAITS
>
class sct_pipe<T, N, TRAITS, 0> : 
    public sc_module, 
    public sct_fifo_if<T>
{
  public:
    sc_in<bool>         SC_NAMED(clk);
    sc_in<bool>         SC_NAMED(nrst);
    
    SC_HAS_PROCESS(sct_pipe);
    
    /// \param addInReg  -- add input register not moved by re-timing 
    /// \param addOutReg -- add output register not moved by re-timing
    /// \param rtlName   -- pipeline register instantiated component name,
    ///                     the default value is not related to any existing 
    ///                     component and should be considered as example only
    explicit sct_pipe(const sc_module_name& name, 
                      bool addInReg = 0, bool addOutReg = 0,
                      const std::string& rtlName = "DW_pl_reg") : 
        sc_module(name), reg{"reg", addInReg, addOutReg, rtlName}
    {
        //cout << "PIPE " << name << " DELAY " << DELAY << endl;
        reg.bind(clk, nrst, enable, data_in, data_out);
        
        SCT_METHOD(asyncProc);
        sensitive << put_req << put_req_d << get_req << get_req_d
                  << reg_full_d << data_reg_d << data_out;
        for (unsigned i = 0; i != N; ++i) {
            sensitive << busy_reg_d[i];
        }

        #ifdef SCT_SEQ_METH
            SC_METHOD(syncProc);
            sensitive << (TRAITS::CLOCK ? clk.pos() : clk.neg()) 
                      << (TRAITS::RESET ? nrst.pos() : nrst.neg()); 
        #else
            SCT_CTHREAD(syncProc, clk, TRAITS::CLOCK);
            async_reset_signal_is(nrst, TRAITS::RESET);
        #endif
    }

  public:
    bool ready() const override {
        return ready_push;
    }
    
    bool request() const override {
        return out_valid;
    }
    
    void reset_get() override {
        get_req = 0;
    }
    
    void reset_put() override {
        put_req = 0;
        data_in = T{};
    }
    
    void reset() {
        reset_get();
        reset_put();
    }
    
    void clear_get() override {
        if (cthread_get) {
            get_req = get_req.read();
        } else {
            get_req = 0;
        }
    }
    
    void clear_put() override {
        if (cthread_put) {
            put_req = put_req.read();
            // Do not clear input data
        } else {
            put_req = 0;
            data_in = T{};
        }
    }
    
    T peek() const override {
        return data_reg.read();
    }
    
    /// \return current request data, if no request last data returned
    T get() override {
        if (out_valid) {
            get_req = cthread_get ? !get_req : 1;
        } else {
            if (!cthread_get) get_req = 0;
        }
        return data_reg.read();
    }
    
    /// \return true if request is valid and enable is true
    bool get(T& data, bool enable = true) override {
        data = data_reg.read();
        if (out_valid) {
            get_req = cthread_get ? (enable ? !get_req : get_req) : enable;
            return enable;
        } else {
            if (!cthread_get) get_req = 0;
            return false;
        }
    }

    bool put(const T& data) override {
        if (ready_push) {
            put_req = cthread_put ? !put_req : 1;
            // Assign input data only under request as it is register, value stored
            data_in = data;
            return true;
        } else {
            if (!cthread_put) {
                put_req = 0;
                data_in = T{};  // Clear to avoid latch in METHOD
            }
            return false;
        }
    }
    
    bool put(const T& data, sc_uint<1> mask) override {
        if (ready_push) {
            put_req = cthread_put ? (mask ? !put_req : put_req) : bool(mask);
            // Assign input data only under request as it is register, value stored
            data_in = data;
            return mask;
        } else {
            if (!cthread_put) {
                put_req = 0;
                data_in = T{};  // Clear to avoid latch in METHOD
            }
            return false;
        }
    }
    
    T b_get() override {
        if (cthread_get) {
            while (!out_valid) wait();
            get_req = !get_req;
            return data_reg.read();
        } else {
            cout << "No blocking get allowed in METHOD process" << endl;
            sc_assert (false);
            return T{};
        }
    }
    
    void b_put(const T& data) override {
        if (cthread_put) {
            while (!ready_push) wait();
            put_req = !put_req;
            // Assign input data only under request as it is register, value stored
            data_in = data;
        } else {
            cout << "No blocking put allowed in METHOD process" << endl;
            sc_assert (false);
        }
    }
    
    /// \return number of register in pipeline
    unsigned size() const override {
        return N;
    }
    
    unsigned elem_num() const override {
        cout << "No elem_num() supported in sct_pipe" << endl;
        sc_assert (false);
        return 0;
    }
    
    bool almost_full(const unsigned&) const override {
        cout << "No almost_full() supported in sct_pipe" << endl;
        sc_assert (false);
        return false;
    }
    
    bool almost_empty(const unsigned&) const override {
        cout << "No almost_empty() supported in sct_pipe" << endl;
        sc_assert (false);
        return false;
    }    
    
  public:
    bool cthread_put = false;
    bool cthread_get = false;

    /// This pipe attached to processes
    bool attached_put = false;
    bool attached_get = false;

  protected:
    sct_pl_reg<T, N, TRAITS>    reg;

    /// Input put/get requests
    sc_signal<bool>                     SC_NAMED(put_req);
    sc_signal<bool>                     SC_NAMED(put_req_d);
    sc_signal<bool>                     SC_NAMED(get_req);
    sc_signal<bool>                     SC_NAMED(get_req_d);

    /// Pipe is ready to put/get
    sc_signal<bool>                     SC_NAMED(ready_push);
    sc_signal<bool>                     SC_NAMED(out_valid);
    
    /// Use comb signals to assign them in thread process
    sc_signal<sct_uint<N>>              SC_NAMED(enable);
    sc_signal<T>                        SC_NAMED(data_in);
    sc_signal<T>                        SC_NAMED(data_out);

    /// Output register to perform one shift if there is no pop in THREAD 
    /// Also it decouples combinational @ready_push from @pop in METHOD
    sc_signal<bool>                     SC_NAMED(reg_full);
    sc_signal<bool>                     SC_NAMED(reg_full_d);
    sc_signal<T>                        SC_NAMED(data_reg);
    sc_signal<T>                        SC_NAMED(data_reg_d);
    
    sc_vector<sc_signal<bool>>          busy_reg{"busy_reg", N};
    sc_vector<sc_signal<bool>>          busy_reg_d{"busy_reg_d", N};
    
    void asyncProc()
    {
        const bool push = cthread_put ? put_req != put_req_d : put_req;
        const bool pop  = cthread_get ? get_req != get_req_d : get_req;

        bool busyRegs[N];
        for (unsigned i = 0; i != N; ++i) { busyRegs[i] = busy_reg_d[i]; }
        bool regFull = reg_full_d;
        T dataReg    = data_reg_d;
        
        // Consider last get done in THREAD process in this cycle
        if (cthread_get && pop) { regFull = 0; }
        
        if (!regFull && busyRegs[N-1]) {
            busyRegs[N-1] = 0;
            regFull = 1;
            dataReg = data_out;
        }

        sct_uint<N> enableRegs = 0;
        for (unsigned i = N-1; i != 0; --i) {
            if (!busyRegs[i] && busyRegs[i-1]) {
                busyRegs[i] = 1; busyRegs[i-1] = 0;
                enableRegs.bit(i) = 1;
            }
        }

        ready_push = !busyRegs[0];
        out_valid  = regFull;

        if (push && !busyRegs[0]) {
            busyRegs[0] = 1;
            enableRegs.bit(0) = 1;
        } 

        // Consider last get done in METHOD process in next cycle
        if (!cthread_get && pop) { regFull = 0; }
        
        for (unsigned i = 0; i != N; ++i) { busy_reg[i] = busyRegs[i]; }
        enable   = enableRegs;
        reg_full = regFull;
        data_reg = dataReg;
    }
    
    void syncProc()
    {
    #ifdef SCT_SEQ_METH
        if (TRAITS::RESET ? nrst : !nrst) {
    #endif
            // Busy registers, bit 0 -- input, bit N-1 -- output 
            for (unsigned i = 0; i != N; ++i) {
                busy_reg_d[i] = 0;
            }
            reg_full_d = 0;
            data_reg_d = 0;

            if (cthread_put) put_req_d = 0;
            if (cthread_get) get_req_d = 0;
    #ifdef SCT_SEQ_METH
        } else {
    #else
        wait();

        while (true) {
    #endif
            for (unsigned i = 0; i != N; ++i) {
                busy_reg_d[i] = busy_reg[i];
            }
            reg_full_d = reg_full;
            data_reg_d = data_reg;

            // Check @ready_push to keep input request active if it is not taken
            if (cthread_put && ready_push) put_req_d = put_req;
            if (cthread_get) get_req_d = get_req;
            
            //cout << sc_time_stamp() << " " << sc_delta_count() << " : enableRegs " << hex << enable.read() << dec << endl;
    #ifndef SCT_SEQ_METH
            wait();
    #endif
        }
    }
    
    void before_end_of_elaboration() override {
        if (!attached_put || !attached_get) {
            cout << "\nPipe " << name() 
                 << " is not fully attached to process(es)" << endl;
            assert (false);
        }
        if (clk.bind_count() != 1 || nrst.bind_count() != 1) {
            cout << "\nPipe " << name() 
                 << " clock/reset inputs are not bound or multiple bound" << endl;
            assert (false);
        }
        PUT.pipe = nullptr;
        GET.pipe = nullptr;
        PEEK.pipe = nullptr;
    }

  public:
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in) {
        clk(clk_in);
        nrst(nrst_in);
    }
    
    void clk_nrst(sc_in_clk& clk_in, sc_in<bool>& nrst_in) override {
        clk(clk_in);
        nrst(nrst_in);
    }

    void addTo(sc_sensitive& s) override {
        addToPut(s);
        addToGet(s);
    }
    
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        assert (false);
    }
    
    void addToPut(sc_sensitive& s) override {
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread_put = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            auto procKind = sc_get_current_process_handle().proc_kind();
            cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (cthread_put) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << ready_push;
        }

        if (attached_put) {
            cout <<  "\nDouble addToPut() for pipe: " << name() << endl; 
            assert (false);
        }
        attached_put = true;
    }

    void addToPut(sc_sensitive* s, sc_process_handle* p) override {
        assert (false);
    }  
    
    void addToGet(sc_sensitive& s) override {
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread_get = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            auto procKind = sc_get_current_process_handle().proc_kind();
            cthread_get = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (cthread_get) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << out_valid << data_reg;  // No @nrst required here
        }

        if (attached_get) {
            cout <<  "\nDouble addToGet() for pipe: " << name() << endl; 
            assert (false);
        }
        attached_get = true;
    }
    
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {
        assert (false);
    }

    void addPeekTo(sc_sensitive& s) override {
        bool cthread_peek;
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread_peek = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            auto procKind = sc_get_current_process_handle().proc_kind();
            cthread_peek = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (cthread_peek) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << data_reg;
        }
    }

    sct_pipe_put<T, N, TRAITS, false> PUT{this};
    sct_pipe_get<T, N, TRAITS, false> GET{this};
    sct_pipe_peek<T, N, TRAITS, false> PEEK{this};
};

//==============================================================================

/// Fast simulation implementation
/// Primitive FIFO used instead of accurate implementation for faster simulation
template <
    class T, 
    unsigned N,             /// Number of pipeline registers, one or more
    class TRAITS
>
class sct_pipe<T, N, TRAITS, 1> : 
    public sc_module, 
    public sct_fifo_if<T>
{
  public:
    sc_in<bool>     nrst{"nrst"};

    SC_HAS_PROCESS(sct_pipe);

    /// \param addInReg -- add input register not moved by re-timing 
    /// \param addInReg -- add output register not moved by re-timing
    /// \param rtlName  -- pipeline register instantiated component name,
    ///                    the default value is not related to any existing 
    ///                    component and should be considered as example only
    explicit sct_pipe(const sc_module_name& name, 
                      bool addInReg = 0, bool addOutReg = 0,
                      const std::string& rtlName = "DW_pl_reg") : 
        sc_module(name), 
        fifo("fifo", N+1, 1, 1, 0)  // sync valid and ready to avoid comb loop
    {
        SC_METHOD(resetProc);
        sensitive << nrst;
    }
    
  protected:
    /// Minimum 2 slots required to have put&get at the same DC
    sct_prim_fifo<T>   fifo;
    
    /// Clear FIFO buffer
    void resetProc() {
        // Reset is active
        bool reset = TRAITS::RESET ? nrst : !nrst;
        fifo.reset_core(reset);
    }
    
  public:
    void reset() {
        reset_get();
        reset_put();
    }

    bool ready() const override {
        return fifo.ready();
    }

    void reset_put() override {
        fifo.reset_put();
    }
    
    void clear_put() override {
        fifo.clear_put();
    }
    
    bool put(const T& data) override {
        return fifo.put(data);
    }
    bool put(const T& data, sc_uint<1> mask) override {
        return fifo.put(data, mask);
    }
    
    void b_put(const T& data) override {
        while (!fifo.ready()) wait();
        fifo.put(data, true);
    }

    bool request() const override {
        return fifo.request();
    }

    void reset_get() override {
        fifo.reset_get();
    }

    void clear_get() override {
        fifo.clear_get();
    }
    
    T peek() const override {
        return fifo.peek();
    }

    T get() override {
        return fifo.get();
    }
    
    bool get(T& data, bool enable = true) override {
        return fifo.get(data, enable);
    }
    
    T b_get() override {
        while (!fifo.request()) wait();
        return fifo.get();
    }
      
    unsigned size() const override {
        return N;
    }

   unsigned elem_num() const override {
        cout << "No elem_num() supported in sct_pipe" << endl;
        sc_assert (false);
        return 0;
    }
    
    bool almost_full(const unsigned&) const override {
        cout << "No almost_full() supported in sct_pipe" << endl;
        sc_assert (false);
        return false;
    }
    
    bool almost_empty(const unsigned&) const override {
        cout << "No almost_empty() supported in sct_pipe" << endl;
        sc_assert (false);
        return false;
    }    
    
  public:
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in) {
        fifo.clk_nrst(clk_in, nrst_in);
        nrst(nrst_in);
    }
    
    void clk_nrst(sc_in_clk& clk_in, sc_in<bool>& nrst_in) override {
        fifo.clk_nrst(clk_in, nrst_in);
        nrst(nrst_in);
    }
    
    /// Add this target to sensitivity list
    void addTo(sc_sensitive& s) override {
        fifo.addTo(s);
    }
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        assert (false);
    }
    
    void addToPut(sc_sensitive& s) override {
        fifo.addToPut(s);
    }
    
    void addToPut(sc_sensitive* s, sc_process_handle* p) override {
        assert (false);
    }

    void addToGet(sc_sensitive& s) override {
        fifo.addToGet(s);
    }
    
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {    
        assert (false);
    }
    
    void addPeekTo(sc_sensitive& s) override {
        fifo.addPeekTo(s);
    }

    sct_pipe_put<T, N, TRAITS, true> PUT{this};
    sct_pipe_get<T, N, TRAITS, true> GET{this};
    sct_pipe_peek<T, N, TRAITS, true> PEEK{this};
};

} // namespace sct

//==============================================================================

namespace sc_core {

template<class T, unsigned N, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, sct::sct_pipe<T, N, TRAITS, TLM_MODE>& pipe )
{
    pipe.addTo(s);
    return s;
}

template<class T, unsigned N, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_pipe_put<T, N, TRAITS, TLM_MODE>& put )
{
    put.pipe->addToPut(s);
    return s;
}

template<class T, unsigned N, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_pipe_get<T, N, TRAITS, TLM_MODE>& get )
{
    get.pipe->addToGet(s);
    return s;
}

template<class T, unsigned N, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_pipe_peek<T, N, TRAITS, TLM_MODE>& get )
{
    get.pipe->addPeekTo(s);
    return s;
}

} // namespace sc_core



#endif /* SCT_PIPE_H */

