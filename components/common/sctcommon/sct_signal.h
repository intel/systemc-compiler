/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Signal channel.
 * To use as normal SystemC signal for inter-process communications.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_SIGNAL_H
#define SCT_SIGNAL_H

#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {

/// Cycle accurate implementation
template <class T, class ENABLE_EVENT>
class sct_signal<T, ENABLE_EVENT, 0> : public sc_signal<T>
{
  public:
    using base_type = sc_signal<T>;
    using this_type = sct_signal<T, ENABLE_EVENT, 0>; 
      
    explicit sct_signal(const sc_module_name& name) : base_type(name)
    {}
    
    explicit sct_signal() : base_type("sct_signal")
    {}
        
    this_type& operator = (const T& val) {
        this->write(val);
        return *this;
    }
    
    this_type& operator = (const this_type& other) { 
        this->write(other.read()); 
        return *this; 
    }

    void addTo(sc_sensitive& s) {
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (procKind == SC_METHOD_PROC_) {
            s << this->default_event();
        } else {
            // Add nothing
        }
    }
    
    void addTo(sc_sensitive* s, sc_process_handle* p) {
        auto procKind = p->proc_kind();
        if (procKind == SC_METHOD_PROC_) {
            *s << *p << this->default_event();
        } else {
            // Add nothing
        }
    }
};

//==============================================================================

/// Approximate time implementation
template <class T, class ENABLE_EVENT>
class sct_signal<T, ENABLE_EVENT, 1> : 
    public sc_prim_channel,
    public sct_inout_if<T>
{
  public:
    using this_type = sct_signal<T, ENABLE_EVENT, 1>; 

    /// Generate events for thread process every clock period if the signal
    /// level is as specified by @ENABLE_EVENT::LEVEL
    static const bool USE_ENABLE_EVENT = !std::is_void_v<ENABLE_EVENT>;

    SC_HAS_PROCESS(sct_signal);
    
    explicit sct_signal(const sc_module_name& name) :
        sc_prim_channel(name),
        meth_event(std::string(std::string(name)+"_meth_event").c_str()),
        thrd_event(std::string(std::string(name)+"_thrd_event").c_str())
    {
        if constexpr (USE_ENABLE_EVENT) {
            sc_spawn_options edge_options;
            edge_options.spawn_method(); 
            edge_options.dont_initialize();
            edge_options.set_sensitivity(&thrd_event); 
            sc_spawn([this]() {
                        if (this->curr_val == ENABLE_EVENT::LEVEL) 
                            this->thrd_event.notify(this->clk_period);
                    }, sc_gen_unique_name("updateProc"), &edge_options);
        }
    }

    explicit sct_signal() {
        sct_signal("sct_signal");
    }

  protected:  
    sc_in_clk*  clk_in = nullptr;
    sc_time     clk_period = SC_ZERO_TIME;
    
    T           curr_val = T{};
    T           next_val = T{};
    
    /// Method/thread events used by a process
    bool        methEvent = false;
    bool        thrdEvent = false;
      
    sc_event    meth_event;
    sc_event    thrd_event;

    /// Channel update, run at DC 0 
    void update() override
    {
        if (!(curr_val == next_val)) {
            curr_val = next_val;
            
            // Notify thread and method processes
            if (methEvent) meth_event.notify(SC_ZERO_TIME);
            if (thrdEvent) thrd_event.notify(clk_period);
        }
    }
    
    void end_of_elaboration() override {
        if (thrdEvent) {
            if (clk_in) {
                clk_period = get_clk_period(clk_in);
            } else {
                std::cout << "Signal " << name() << " in process sensitivity"
                          << " list requires SCT_THREAD macro" << std::endl;
                assert (false);
            }
        }
    }
    
  public:    
    void write(const T& data) override 
    {
        if (!(next_val == data)) {
            request_update();
        }
        next_val = data;
    }
    
    this_type& operator = (const T& data) {
        write(data);
        return *this;
    }
    
    this_type& operator = (const this_type& other) {
        write(other.read());
        return *this;
    }

    const T& read() const override {
        return curr_val;
    }
    
    operator const T& () const {
        return curr_val;
    }
    
  public:
    void addTo(sc_sensitive& s) {
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_) {
            if (procKind != SC_CTHREAD_PROC_) {
                s << thrd_event;
            }
            clk_in = sct_curr_clock;
            thrdEvent = true;
        } else {
            if constexpr (USE_ENABLE_EVENT) {
                std::cout << "Signal " << name() << " with event enable added"
                          << " to method process" << std::endl;
                assert(false);
            }
            s << meth_event;
            methEvent = true;
        }
    }
    
    /// Used in @sc_port of @sct_signal
    void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) override {
        auto procKind = p->proc_kind();
        if (procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_) {
            if (procKind != SC_CTHREAD_PROC_) {
                *s << *p << thrd_event;
            }
            if (clk_in && clk_in->get_interface() != c->get_interface()) {
                std::cout << "Signal " << name() << " added to process sensitivity"
                          << " lists which have different clock inputs" << std::endl;
                assert(false);
            }
            clk_in = c;
            thrdEvent = true;
        } else {
            if constexpr (USE_ENABLE_EVENT) {
                std::cout << "Signal " << name() << " with event enable added"
                          << " to method process" << std::endl;
                assert(false);
            }
            *s << *p << meth_event;
            methEvent = true;
        }
    }
    
    /// Get get/put process events
    const sc_event& thread_event() const { 
        return thrd_event; 
    }
    
    const sc_event& method_event() const {
        return meth_event; 
    }
    
    inline void print(::std::ostream& os) const override {
        os << "sct_signal " << name() << " = " << curr_val << ::std::endl;
    }

    const char* kind() const override { 
        return "sct_signal"; 
    }
};

} // namespace sct

//==============================================================================

namespace sc_core {
    
    template<class T, class ENABLE_EVENT, bool TLM_MODE>
    sc_sensitive& 
    operator << ( sc_sensitive& s, sct::sct_signal<T, ENABLE_EVENT, TLM_MODE>& signal )
    {
        signal.addTo(s);
        return s;
    }
} // namespace sc_core


template <class T>
inline ::std::ostream& operator << (::std::ostream& os, 
                                    const sct::sct_signal<T>& sig) 
{
    os << sig.read();
    return os;
}

#endif /* SCT_SIGNAL_H */

