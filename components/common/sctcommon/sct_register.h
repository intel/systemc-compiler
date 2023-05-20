/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Register to store METHOD state.
 * 
 * Register can be written in one METHOD process and read in the same or
 * another METHOD process(es). 
 * Register is intended to introduce state for METHOD process.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_REGISTER_H
#define SCT_REGISTER_H

#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
/// Cycle accurate implementation    
template<class T, class TRAITS>
class sct_register<T, TRAITS, 0> : 
    public sc_module,
    public sct_inout_if<T>
{
 public:
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};
    
    const T RESET_VAL;

    SC_HAS_PROCESS(sct_register);
    
    explicit sct_register(const sc_module_name& name, 
                          const T& RESET_VAL_ = T{}) : 
        sc_module(name), RESET_VAL(RESET_VAL_)
    {
        SCT_CTHREAD(reg_thread, clk, TRAITS::CLOCK);
        async_reset_signal_is(nrst, TRAITS::RESET);
    }
    
    /// Call in writer METHOD process only
    void reset() {
        reg_data = reg_data_d;
    }
    
    /// Call in writer METHOD process only
    void write(const T& data) override {
        reg_data = data;
    }
    
    void operator = (const T& data) {
        reg_data = data;
    }
    
    T read() const override {
        return reg_data_d.read();
    }
    
    operator T () {
        return reg_data_d.read();
    }
    
  protected:  
    /// This register attached to a process
    bool attached = false;

    sc_signal<T>    reg_data{"reg_data"};
    sc_signal<T>    reg_data_d{"reg_data_d"};

    void reg_thread() 
    {
        reg_data_d = RESET_VAL;
        wait();
        
        while (true) {
            reg_data_d = reg_data;
            wait();
        }
    }
    
    void before_end_of_elaboration() override {
        if (!attached) {
            // No assert violation here
            cout << "\nRegister " << name() 
                 << " is not attached to any process" << endl;
        }
    }
    
  public:
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in) {
        clk(clk_in);
        nrst(nrst_in);
    }
    
    void addTo(sc_sensitive& s) {
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            
            // No assert violation here
            cout << "\nRegister " << name() 
                 << " attached to THREAD process" << endl; 
        } else {
            s << reg_data_d;    // No @nrst required here
        }

        attached = true;
    }

    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override {
        std::cout << "\nMethod not supported for register " << name() << "\n";
        assert (false);
    }
};

//==============================================================================

/// Register primitive channel. 
/// Used as base for register approximate time implementation.
template<class T>
struct sct_prim_register : 
    public sc_module, 
    public sc_interface
{
  public:
    const T RESET_VAL;
    
    SC_HAS_PROCESS(sct_prim_register);
    
    explicit sct_prim_register(const sc_module_name& name,
                               const T& RESET_VAL_ = T{}) : 
        sc_module(name), 
        RESET_VAL(RESET_VAL_),
        update_event(std::string(std::string(name)+"_update_event").c_str()),
        thrd_event(std::string(std::string(name)+"_thrd_event").c_str())                               
    {
        SC_METHOD(updateProc); 
        dont_initialize(); 
        sensitive << update_event;
    }
        
  protected:    
    sc_in_clk*  clk_in = nullptr;
    sc_time     clk_period = SC_ZERO_TIME;
    
    bool        hasReset = 0;       // Has reset last DC    
    T           nextVal  = T{};     // Put data, next data to store 
    
    sc_signal<T> curr_val{"curr_val"};   // Current data

    sc_event    update_event;   
    sc_event    thrd_event;
    
    /// Thread events used by a process
    bool        thrdEvent = false;
    
    /// Channel update 
    void updateProc() 
    {
        if (hasReset) {
            curr_val = RESET_VAL;
            hasReset = 0;
            // Notify thread processes
            if (thrdEvent && !(curr_val.read() == RESET_VAL)) {
                thrd_event.notify(clk_period);
            }
        } else
        if (!(curr_val.read() == nextVal)) {
            curr_val = nextVal;
            // Notify thread processes
            if (thrdEvent) thrd_event.notify(clk_period);
        }
    }
    
    void end_of_elaboration() override {
        assert (clk_in && "clk_in is nullptr");
        clk_period = get_clk_period(clk_in);
    }    
    
  public:  
    /// Can write to @hasReset/@nextVal from 2 processes as one is updateProc
    /// notified at ONE_SEC and another is METHOD

    /// Reset register core, called by reset process in register 
    void reset_core(bool reset) {
        if (reset) {
            hasReset = 1;
            update_event.notify(clk_period);
        } else {
            // Notify thread processes
            if (thrdEvent) thrd_event.notify(clk_period);
        }
    }
    
    void reset() {
        nextVal = curr_val;
    }
    
    void write(const T& data) {
        nextVal = data;
        if (!(curr_val.read() == data)) {
            update_event.notify(clk_period);
        }
    }
    
    T read() const {
        return curr_val;
    }
    
  public: 
    void clk_nrst(sc_in_clk& clk_in_, sc_in<bool>& nrst_in) {
        clk_in = &clk_in_;
    }

    /// Get default event for METHOD
    const sc_event& default_event() const override {
        return curr_val.default_event(); 
    }

    /// Get event for THREAD which read this register
    const sc_event& thread_event() {
        thrdEvent = true;
        return thrd_event;
    }
};

//==============================================================================

/// Approximate time implementation
template<class T, class TRAITS>
class sct_register<T, TRAITS, 1> : 
    public sc_module, 
    public sct_inout_if<T>
{
  public:
    sc_in<bool>     nrst{"nrst"};
    
    SC_HAS_PROCESS(sct_register);
    
    explicit sct_register(const sc_module_name& name,
                          const T& RESET_VAL_ = T{}) : 
        sc_module(name), reg("reg", RESET_VAL_)
    {
        SC_METHOD(resetProc);
        sensitive << nrst;
    }
    
  protected:  
    sct_prim_register<T> reg;
    
    /// Reset register 
    void resetProc() {
        // Reset is active
        bool reset = TRAITS::RESET ? nrst : !nrst;
        reg.reset_core(reset);
    }
    
  public:
    /// Call CTHREAD reset sections and METHOD everywhere
    void reset() {
        reg.reset();
    }
    
    void write(const T& data) override {
        reg.write(data);
    }
    
    void operator = (const T& data) {
        reg.write(data);
    }
    
    T read() const override {
        return reg.read();
    }
    
    operator T () {
        return reg.read();
    }
    
  public:
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in) {
        reg.clk_nrst(clk_in, nrst_in);
        nrst(nrst_in);
    }
    
    void addTo(sc_sensitive& s) {
        auto procKind = sc_get_current_process_handle().proc_kind();
        
        if (procKind == SC_METHOD_PROC_) {
            s << reg.default_event();   // No @nrst required here
        } else 
        if (procKind == SC_THREAD_PROC_) {
            s << reg.thread_event();    // No @nrst required here
        }
    }
    
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override {
        std::cout << "\nMethod not supported for register " << name() << "\n";
        assert (false);
    }
};

} // namespace sct

//==============================================================================

namespace sc_core {

template<class T, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, sct::sct_register<T, TRAITS, TLM_MODE>& reg )
{
    reg.addTo(s);
    return s;
}

} // namespace sc_core

#endif /* SCT_REGISTER_H */

