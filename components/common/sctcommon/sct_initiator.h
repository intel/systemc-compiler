/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source communication library. Initiator channel.
 * 
 * Target and Initiator are channels intended to connect two user defined modules. 
 * Initiator implements sct_put_if interface and could be used in one METHOD or 
 * THREAD process to put requests. 
 * 
 * Target and Initiator should be connected to clock and reset with clk_nrst(). 
 * Target and Initiator are connected to each other with method bind().
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_INITIATOR_H
#define SCT_INITIATOR_H

#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
/// Cycle accurate implementation    
template<class T, class TRAITS>
class sct_initiator<T, TRAITS, 0> : 
    public sc_module,
    public sct_put_if<T>
{
    friend class sct_target<T, TRAITS, 0>;
    
  public:
    static const unsigned long long ALL_ENABLED = ~0ULL;
    
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};

    sc_in<bool>             core_ready{"core_ready"};
    sc_out<bool>            core_req{"core_req"};
    sc_out<T>               core_data{"core_data"};
    
  public:
    SC_HAS_PROCESS(sct_initiator);

    /// \param sync -- add register in initiator or target
    explicit sct_initiator(const sc_module_name& name, bool sync_ = 0) : 
        sc_module(name), sync(sync_)
    {
        //cout << "Initiator " << name << endl;
        SCT_CTHREAD(sync_thread, clk, TRAITS::CLOCK);
        async_reset_signal_is(nrst, TRAITS::RESET);

        SC_METHOD(req_control);
        sensitive << put_req << put_req_d << core_ready_d << core_req_d;
        
        SCT_CTHREAD(core_thread, clk, TRAITS::CLOCK);
        async_reset_signal_is(nrst, TRAITS::RESET);
    }
    
    /// Return true if it is ready to put request
    bool ready() const override {
        return (always_ready ? true : core_ready);
    }

    /// Reset put request in THREAD reset section and METHOD everywhere
    void reset_put() override {
        if (sync) {
            sync_req  = 0;
            sync_data = T{};
        } else {
            put_req   = 0;
            core_data = T{};
        }
    }
    
    /// Clear put request in METHOD and THREAD after reset
    /// Do not clear @core_data in CTHREAD as it may contain current request data
    void clear_put() override {
        if (sync) {
            if (cthread) {
                sync_req  = sync_req.read();
            } else {
                sync_req  = 0;
                sync_data = T{};
            }
        } else {
            if (cthread) {
                put_req = put_req.read();
                if (always_ready) core_data = T{};
            } else {
                put_req   = 0;
                core_data = T{};
            }
        }
    }
    
    /// Put request if it is ready 
    /// \return true if request is pushed 
    bool put(const T& data) override {
         if (sync) {
            if (core_ready) {
                sync_req = cthread ? !sync_req : true;
                // Assign input data only under request as it is register, value stored
                sync_data = data;
                return true;
            } else {
                if (!cthread) {
                    sync_req = 0;
                    sync_data = T{};  // Clear to avoid latch in METHOD
                }
                return false;
            }
        } else {
            if (always_ready || core_ready) {
                put_req = cthread ? !put_req : true;
                // Assign input data only under request as it is register, value stored
                core_data = data;
                return true;
            } else {
                if (!cthread) {
                    put_req = 0;
                    core_data = T{};  // Clear to avoid latch in METHOD
                }
                return false;
            }
        }
    }
    bool put(const T& data, sc_uint<1> mask) override {
         if (sync) {
            if (core_ready) {
                sync_req = cthread ? (mask ? !sync_req : sync_req) : bool(mask);
                // Assign input data only under request as it is register, value stored
                sync_data = data;
                return mask;
            } else {
                if (!cthread) {
                    sync_req = 0;
                    sync_data = T{};  // Clear to avoid latch in METHOD
                }
                return false;
            }
        } else {
            if (always_ready || core_ready) {
                put_req = cthread ? (mask ? !put_req : put_req) : bool(mask);
                // Assign input data only under request as it is register, value stored
                core_data = data;
                return mask;
            } else {
                if (!cthread) {
                    put_req = 0;
                    core_data = T{};  // Clear to avoid latch in METHOD
                }
                return false;
            }
        }
    }
    
    /// May-block put, can be used in THREAD only
    void b_put(const T& data) override {
        if (cthread) {
            if (!always_ready) {
                while (!core_ready) wait();
                // Assign input data only under request as it is register, value stored
                if (sync) {
                    sync_req = !sync_req;
                    sync_data = data;
                } else {
                    put_req = !put_req;
                    core_data = data;
                }
            } else {
                cout << "No blocking put allowed with always ready target" << endl;
                assert (false);
            }
        } else {
            cout << "No blocking put allowed in METHOD process" << endl;
            assert (false);
        }
    }
    
  public:  
    bool sync;
    bool cthread = false;
    bool always_ready;       

    /// This initiator attached to a process
    bool attached = false;
    /// This initiator bound to target
    bool bound = false;

  protected:
    // Put new request call this cycle/DC
    sc_signal<bool>         put_req{"put_req"};
    sc_signal<bool>         put_req_d{"put_req_d"};
    sc_signal<bool>         core_req_d{"core_req_d"};
    sc_signal<bool>         core_ready_d{"core_ready_d"};
    // Sync mode related signals
    sc_signal<bool>         sync_req{"sync_req"};
    sc_signal<bool>         sync_req_d{"sync_req_d"};
    sc_signal<T>            sync_data{"sync_data"};
    
    void sync_thread() {
        if (sync) {
            if (cthread) sync_req_d = 0;
            put_req   = 0;
            core_data = T{};
            wait();
            
            while (true) {
                bool A = cthread ? sync_req != sync_req_d : sync_req;

                if (A && core_ready) {
                    put_req   = !put_req;
                    core_data = sync_data;
                    if (cthread) sync_req_d = sync_req;
                }

                wait();
            }
        }
    }

    void req_control() {
        const bool A = (sync || cthread) ? put_req != put_req_d : put_req;
        if (always_ready) {
            core_req = A;
        } else {
            if (A) {
                core_req = 1;
            } else 
            if (core_ready_d) {
                core_req = 0;
            } else {
                core_req = core_req_d;
            }
        }
    }
    
    void core_thread() {
        if (sync || cthread) put_req_d = 0;
        if (!always_ready) {
            core_req_d   = 0;
            core_ready_d = 0;
        }
        wait();
        
        while (true) {
            if (sync || cthread) put_req_d = put_req;
            if (!always_ready) {
                core_req_d   = core_req;
                core_ready_d = core_ready;
            }
            wait();
        }
    }
    
    void before_end_of_elaboration() override {
        if (!attached) {
            cout << "\nInitiator " << name() 
                 << " is not attached to any process" << endl;
            assert (false);
        }
        if (!bound) {
            cout << "\nInitiator " << name() 
                 << " is not bound to target" << endl;
            assert (false);
        }
        if (always_ready && sync) {
            cout << "\nInitiator " << name() 
                 << " bound to always ready target cannot have sync option" << endl;
            assert (false);
        }
    }

  public:
    /// Get initiator instance, used for sc_port of initiator
    sct_initiator<T, TRAITS, 0>& get_instance() {
        return *this;
    }

    /// Bind two given ports to each other via a new allocated signal
    template<class DutPort, class TbPort> 
    void bind_rtl(DutPort& dutPort, TbPort& tbPort) {
        using PT = typename TbPort::data_type;
        std::string sigName = std::string(tbPort.basename()) + "_s";
        auto* sig = new sc_signal<PT>(sigName.c_str());
        dutPort(*sig);
        tbPort(*sig);
        
        bound = true;
    }
    
    /// Bind to target
    template<class Module> 
    void bind(Module& module, unsigned indx = 0) {
        module.do_bind(*this, indx);
        do_bind(module, indx);
    }
    
    template<class Module> 
    void do_bind(Module& module, unsigned indx) {
        // Avoid name duplicating warning
        std::string reqName = "core_req_s";
        std::string dataName = "core_data_s";
        #ifndef __SC_TOOL__
            reqName = std::string(basename()) + "_" + reqName;
            dataName = std::string(basename()) + "_" + dataName;
        #endif

        if constexpr (std::is_base_of<sct_multi_target_base, Module>::value) {
            sc_signal<T>* core_data_s = new sc_signal<T>(dataName.c_str());
            core_data(*core_data_s);
            module.core_data[indx](*core_data_s);

            sc_signal<bool>* core_req_s = new sc_signal<bool>(reqName.c_str());
            core_req(*core_req_s);
            module.core_req[indx](*core_req_s);
            
        } else {
            sc_signal<T>* core_data_s = new sc_signal<T>(dataName.c_str());
            core_data(*core_data_s);
            module.core_data(*core_data_s);
            
            sc_signal<bool>* core_req_s = new sc_signal<bool>(reqName.c_str());
            core_req(*core_req_s);
            module.core_req(*core_req_s);
        }
    }
    
    template<class Module>
    void operator() (Module&) {
        cout << "Initiator " << name() <<  " operator() is prohibited, use bind() instead" << endl;
        assert (false);
    }

    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in)
    {
        clk(clk_in);
        nrst(nrst_in);
    }
    
    /// Add signals to sensitivity list of METHOD where @put() is called
    void addTo(sc_sensitive& s) override
    {
        auto procKind = sc_get_current_process_handle().proc_kind();
        cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        if (cthread) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << core_ready;    // No @nrst required here
        }
        
        if (attached) {
            cout <<  "\nDouble attach initiator " << name() << " to a process " << endl;
            assert (false);
        }
        attached = true;
        //cout << "Set " << this->name() << " CTHREAD " << cthread << endl; 
    }
    
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        if (cthread) {
            if (TRAITS::CLOCK == 2) *s << *p << clk; 
            else *s << *p << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            *s << *p << core_ready;    // No @nrst required here
        }
        
        if (attached) {
            cout <<  "\nDouble attach initiator " << name() << " to a process " << endl;
            assert (false);
        }
        attached = true;
    }
};

//==============================================================================

/// Approximate time implementation
template<class T, class TRAITS>
class sct_initiator<T, TRAITS, 1> : 
    public sc_module,
    public sct_put_if<T>
{
    friend class sct_target<T, TRAITS, 1>;
    
  public:
    static const unsigned long long ALL_ENABLED = ~0ULL;
      
    explicit sct_initiator(const sc_module_name& name, bool sync_ = 0) : 
        sc_module(name), sync(sync_)
    {}
    
    sct_initiator(const sct_initiator<T>&) = delete;
    sct_initiator& operator = (const sct_initiator<T>&) = delete;
    
    /// Get event finder for event in target
    sc_event_finder& event_finder() const {
        return *new sc_event_finder_t< sct_put_if<T> >(
                    put_port, &sct_put_if<T>::default_event);
    }
    
    /// Do not call before this initiator is bound to target
    const sc_event& default_event() const override {
        return put_port->default_event();
    }
    
    bool sync;
    
    /// Port to connect target
    sc_port< sct_put_if<T> >    put_port{"put_port"};
    /// Handle of put process attached to this initiator
    sc_process_handle*  put_handle = nullptr;
    
  public:
    /// Check @ready() for request can be pushed
    bool ready() const override {
        return put_port->ready();
    }

    /// Call in METHOD everywhere and in CTHREAD reset section
    void reset_put() override {
        put_port->reset_put();
    }
    
    /// Call in METHOD everywhere and in CTHREAD body
    /// Clear put request which pushed in this cycle/DC
    void clear_put() override {
        put_port->clear_put();
    }
    
    /// \return true means request is pushed 
    bool put(const T& data) override {
        return put_port->put(data);
    }
    bool put(const T& data, sc_uint<1> mask) override {
        return put_port->put(data, mask);
    }
    
    /// May-block put, call in THREAD only
    void b_put(const T& data) override {
        // Do not use fifo @b_put() as it is not provided
        while (!put_port->ready()) wait();
        put_port->put(data);
    }

  public:
    /// Get initiator instance, used for sc_port of initiator
    sct_initiator<T, TRAITS, 1>& get_instance() {
        return *this;
    }
      
    /// Bind put interface to target
    template<class Module> 
    void bind(Module& module, unsigned indx = 0) {
        module.bind(*this, indx);
    }

    template<class MOD>
    void operator() (MOD&) {
        cout << "Initiator " << name() <<  " operator() is prohibited, use bind() instead" << endl;
        assert (false);
    }    

    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in)
    {}
    
    void addTo(sc_sensitive& s) override {
        if (put_handle) {
            cout <<  "\nDouble attach initiator " << name() << " to a process " << endl; 
            assert (false);
        }
        put_handle = new sc_process_handle(sc_get_current_process_handle());
        // No @nrst required here
    }
    
    /// Used in @sc_port of @sct_signal
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        if (put_handle) {
            cout <<  "\nDouble attach initiator " << name() << " to a process " << endl; 
            assert (false);
        }
        put_handle = p;
        
        auto procKind = p->proc_kind();
        if (procKind != SC_CTHREAD_PROC_) {
            *s << *p;
        }
    }
};

} // namespace sct

//=============================================================================

namespace sc_core {

template<class T, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s,  
              sct::sct_initiator<T, TRAITS, TLM_MODE>& initiator )
{
    initiator.addTo(s);
    return s;
}

} // namespace sc_core

#endif /* SCT_INITIATOR_H */

