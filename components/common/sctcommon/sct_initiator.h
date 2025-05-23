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
        sc_module(name), chan_sync(sync_), orig_sync(sync_)
    {
        //cout << "Initiator " << name << endl;
    #ifdef SCT_SEQ_METH
        SC_METHOD(sync_thread);
        sensitive << (TRAITS::CLOCK ? clk.pos() : clk.neg()) 
                  << (TRAITS::RESET ? nrst.pos() : nrst.neg()); 

        SC_METHOD(core_thread);
        sensitive << (TRAITS::CLOCK ? clk.pos() : clk.neg()) 
                  << (TRAITS::RESET ? nrst.pos() : nrst.neg()); 
    #else
        SCT_CTHREAD(sync_thread, clk, TRAITS::CLOCK);
        async_reset_signal_is(nrst, TRAITS::RESET);
        
        SCT_CTHREAD(core_thread, clk, TRAITS::CLOCK);
        async_reset_signal_is(nrst, TRAITS::RESET);
    #endif

        SC_METHOD(req_control);
        sensitive << put_req << put_req_d << core_req_d;
        
    #ifdef DEBUG_SYSTEMC
        SC_METHOD(debugProc);
        debug_handle = new sc_process_handle(sc_get_current_process_handle());
    #endif
    }
    
    /// Return true if it is ready to put request
    bool ready() const override {
        return (always_ready ? true : core_ready);
    }

    /// Reset put request in THREAD reset section and METHOD everywhere
    void reset_put() override {
        if (chan_sync) {
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
        if (chan_sync) {
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
         if (chan_sync) {
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
         if (chan_sync) {
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
                if (chan_sync) {
                    sync_req = !sync_req;
                    sync_data = data;
                } else {
                    put_req = !put_req;
                    core_data = data;
                }
            } else {
                cout << "No blocking put allowed with always ready target" << endl;
                sc_assert (false);
            }
        } else {
            cout << "No blocking put allowed in METHOD process" << endl;
            sc_assert (false);
        }
    }
    
  public:  
    bool chan_sync;
    bool orig_sync;
    bool cthread = false;
    bool always_ready;       

    /// This initiator attached to a process
    bool attached = false;
    /// This initiator bound to target
    bool bound = false;

  protected:
    /// Put new request call this cycle/DC
    sc_signal<bool>         put_req{"put_req"};
    sc_signal<bool>         put_req_d{"put_req_d"};
    /// Request is stored at the initiator part in @core_data output (put thread only)
    sc_signal<bool>         core_req_d{"core_req_d"};
    /// Sync mode related signals
    sc_signal<bool>         sync_req{"sync_req"};
    sc_signal<bool>         sync_req_d{"sync_req_d"};
    sc_signal<T>            sync_data{"sync_data"};
    
    void sync_thread() {
        if (chan_sync) {
        #ifdef SCT_SEQ_METH
            if (TRAITS::RESET ? nrst : !nrst) {
        #endif
                if (cthread) sync_req_d = 0;
                put_req   = 0;
                core_data = T{};
        #ifdef SCT_SEQ_METH
            } else {
        #else
            wait();

            while (true) {
        #endif            
                bool A = cthread ? sync_req != sync_req_d : sync_req;

                if (A && core_ready) {
                    put_req   = !put_req;
                    core_data = sync_data;
                    if (cthread) sync_req_d = sync_req;
                }

        #ifndef SCT_SEQ_METH
                wait();
        #endif
            }
        }
    }
    
    void req_control() {
        const bool A = (chan_sync || cthread) ? put_req != put_req_d : put_req;
        if (always_ready) {
            core_req = A;
        } else {
            if (A) {
                core_req = 1;
            } else {
                core_req = (chan_sync || cthread) ? core_req_d : 0;
            }
        }
    }
    
    void core_thread() {
        if (chan_sync || cthread) {
        #ifdef SCT_SEQ_METH
            if (TRAITS::RESET ? nrst : !nrst) {
        #endif
                put_req_d = 0;
                if (!always_ready) { 
                    core_req_d = 0; 
                }
        #ifdef SCT_SEQ_METH
            } else {
        #else
            wait();

            while (true) {
        #endif
                put_req_d = put_req;
                if (!always_ready) {
                    core_req_d = !core_ready && core_req;
                }
        #ifndef SCT_SEQ_METH
                wait();
        #endif
            }
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
        // @bind() call clears @chan_sync in Initiator if Target is always ready
        if (always_ready && chan_sync) {
            cout << "\nInitiator " << name() 
                 << " bound to always ready target cannot have sync register" << endl;
            assert (false);
        }
        if (clk.bind_count() != 1 || nrst.bind_count() != 1) {
            cout << "\nInitiator " << name() 
                 << " clock/reset inputs are not bound or multiple bound" << endl;
            assert (false);
        }        

    #ifdef DEBUG_SYSTEMC
        if (chan_sync) {
            this->sensitive << *debug_handle << sync_req << sync_req_d
                            << core_ready << sync_data;
        } else {
            this->sensitive << *debug_handle << put_req << put_req_d
                            << core_ready << core_data;
        }
        debug_handle = nullptr;
    #endif
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
        
        if (always_ready && chan_sync) {
            cout << "\nNo sync register allowed in TB initiator: " << name() << endl;
            assert (false);
        }
        
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

        if constexpr (std::is_base_of<sct_multi_target_base, Module>::value ||
                      std::is_base_of<sct_arbiter_target_base, Module>::value) 
        {
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
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            auto procKind = sc_get_current_process_handle().proc_kind();
            cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }

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
        if (sct_seq_proc_handle == *p) {
            // Sequential method
            cthread = true;
            //cout << "SEQ METHOD " << p->name() << endl;
        } else {
            auto procKind = p->proc_kind();
            cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }

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
    
#ifdef DEBUG_SYSTEMC
    sc_process_handle*  debug_handle = nullptr;

    sc_signal<bool>     ready_push{"ready_push"};
    sc_signal<bool>     debug_put{"put"};
    sc_signal<T>        data_in{"data_in"};
    
    void debugProc() {
        ready_push = ready();
        debug_put  = chan_sync ? (cthread ? sync_req != sync_req_d : sync_req) :
                            (cthread ? put_req != put_req_d : put_req);
        data_in    = chan_sync ? sync_data.read() : core_data.read();
    }
#endif
    
    void trace(sc_trace_file* tf) const override {
    #ifdef DEBUG_SYSTEMC
        std::string initName = name();
        sc_trace(tf, ready_push, initName + "_ready");
        sc_trace(tf, debug_put, initName + "_put");
        sc_trace(tf, data_in, initName + "_data_in");
    #endif
    }
    
    inline void print(::std::ostream& os) const override
    {
        os << "sct_initiator " << name();
        
        if ( ready() ) {
            os << " is ready";
        } else {
            os << " is not ready";
        }
        os << ::std::endl;
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
    
    void trace(sc_trace_file* tf) const override {
        // Do nothing, all traces stored in the target bound
    }
    
    inline void print(::std::ostream& os) const override {
        // Do nothing
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

template<class T, class TRAITS, bool TLM_MODE>
inline ::std::ostream& operator << (::std::ostream& os, 
                    const sct::sct_initiator<T, TRAITS, TLM_MODE>& initiator ) 
{
    initiator.print(os);
    return os;
}

} // namespace sc_core

#endif /* SCT_INITIATOR_H */

