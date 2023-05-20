/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source communication library. Target channel.
 * 
 * Target and Initiator are channels intended to connect two user defined modules. 
 * Target implements sct_get_if interface and could be used in one METHOD or 
 * THREAD process to get requests which put by the connected Initiator.
 * 
 * Target and Initiator should be connected to clock and reset with clk_nrst(). 
 * Target and Initiator are connected to each other with method bind().
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_TARGET_H
#define SCT_TARGET_H

#include "sct_ipc_if.h"
#include "sct_prim_fifo.h"

namespace sct {

/// Cycle accurate implementation    
template<class T, class TRAITS>
class sct_target<T, TRAITS, 0> : 
    public sc_module, 
    public sct_get_if<T>
{
    friend class sct_initiator<T, TRAITS, 0>;
    
  public:
    sc_in<bool>             clk{"clk"};
    sc_in<bool>             nrst{"nrst"};

    sc_out<bool>            core_ready{"core_ready"};
    sc_in<bool>             core_req{"core_req"};
    sc_in<T>                core_data{"core_data"};
    
    SC_HAS_PROCESS(sct_target);
    
    /// \param sync -- add register in initiator or target
    /// \param always_ready -- target process is always ready to get request
    explicit sct_target(const sc_module_name& name, 
                        bool sync_ = 0,
                        bool always_ready_ = 0) : 
        sc_module(name), sync(sync_), always_ready(always_ready_)
    {
        if (always_ready) {
            SCT_CTHREAD(always_ready_thread, clk, TRAITS::CLOCK);
            async_reset_signal_is(nrst, TRAITS::RESET);
            
            SC_METHOD(core_ready_meth);
            
        } else {
            SC_METHOD(ready_control);
            sensitive << get_req << get_req_d << reg_full;

            SC_METHOD(full_control);
            sensitive << get_req << get_req_d << core_req_d << reg_full_d;

            SCT_CTHREAD(core_thread, clk, TRAITS::CLOCK);
            async_reset_signal_is(nrst, TRAITS::RESET);
            
            SC_METHOD(put_to_fifo);
            sensitive << core_req << reg_full << core_data << core_data_d;
            put_fifo_handle = new sc_process_handle(sc_get_current_process_handle());
        }
    }
    
  public:
    /// Request can be taken
    bool request() const override {
        return (fifo ? fifo->request() : 
                always_ready ? (sync ? core_req_d : core_req) :
                               (core_req || reg_full));        
    }
    
    /// Reset get ready in THREAD reset section and METHOD everywhere
    void reset_get() override {
        if (fifo) {
            fifo->reset_get();
        } else {
            get_req = 0;
        }
    }
    
    /// Clear get ready in METHOD or THREAD after reset
    void clear_get() override {
        if (fifo) {
            fifo->clear_get();
        } else {
            if (cthread) {
                get_req = get_req.read();
            } else {
                get_req = 0;
            }
        }
    }
    
    /// \return current request data, no change of the request 
    T peek() const override {
        if (fifo) {
            return fifo->peek();
        } else {
            if (always_ready) {
                if (sync) return core_data_d.read(); 
                else return core_data.read();
            } else {
                if (sync || reg_full) return core_data_d.read();
                else return core_data.read();
            }
        }
    }

    /// \return current request data, if no request last data returned
    T get() override {
        if (fifo) {
            return fifo->get();
        } else {
            bool A = always_ready ? (sync ? core_req_d : core_req) : 
                                    (core_req || reg_full);
            if (A) {
                get_req = cthread ? !get_req : 1;
            } else {
                if (!cthread) get_req = 0;
            }
            
            if (always_ready) {
                if (sync) return core_data_d.read(); 
                else return core_data.read();
            } else {
                if (sync || reg_full) return core_data_d.read(); 
                else return core_data.read();
            }
        }
    }
    
    /// \return true if request is valid and enable is true
    bool get(T& data, bool enable = true) override {
        if (fifo) {
            return fifo->get(data, enable);
            
        } else {
            if (always_ready) {
                if (sync) data = core_data_d.read(); 
                else data = core_data.read(); 
            } else {
                if (sync || reg_full) data = core_data_d.read(); 
                else data = core_data.read();
            }

            bool A = always_ready ? (sync ? core_req_d : core_req) : 
                                    (core_req || reg_full);
            if (A) {
                get_req = cthread ? (enable ? !get_req : get_req) : enable;
                return enable;
            } else {
                if (!cthread) get_req = 0;
                return false;
            }
        }
    }
    
    T b_get() override {
        if (fifo) {
            return fifo->b_get();

        } else {
            if (cthread) {
                if (!always_ready) {
                    while (!core_req && !reg_full) wait();
                } else {
                    if (sync) {
                        while (!core_req_d) wait();
                    } else {
                        while (!core_req) wait();
                    }
                }

                get_req = !get_req;

                if (always_ready) {
                    if (sync) return core_data_d.read(); 
                    else return core_data.read();
                } else {
                    if (sync || reg_full) return core_data_d.read(); 
                    else return core_data.read();
                }
            } else {
                cout << "\nNo blocking get allowed in METHOD process" << endl;
                assert (false);
                return T{};
            }
        }
    }

 public:
    bool sync;
    bool cthread = false;
    const bool always_ready;

    /// This initiator attached to a process
    bool attached = false;
    /// This target bound to initiator
    bool bound = false;

  protected:
    /// FIFO bound
    sct_fifo_if<T>*         fifo = nullptr;
    /// Handle of @put_to_fifo method to add FIFO bound to sensitivity list
    /// Use pointer to avoid parsing @sc_process_handle in ICSC
    sc_process_handle*      put_fifo_handle;
    
    /// Get new request call this cycle/DC
    sc_signal<bool>         get_req{"get_req"};
    sc_signal<bool>         get_req_d{"get_req_d"};
    /// Register contains request to process
    sc_signal<bool>         reg_full{"reg_full"};
    sc_signal<bool>         reg_full_d{"reg_full_d"};
    sc_signal<bool>         core_req_d{"core_req_d"};
    sc_signal<T>            core_data_d{"core_data_d"};
    
    /// Get request and put it to FIFO if bound
    void put_to_fifo() {
        if (fifo) {
            get_req = 0;
            fifo->reset_put();
            
            if (reg_full) {
                get_req = fifo->put(core_data_d.read());
            } else 
            if (core_req) {
                get_req = fifo->put(core_data.read());
            }    
        }
    }
    
    void ready_control() {
        const bool A = cthread ? get_req != get_req_d : get_req_d;
        core_ready = A || !reg_full;
    }
    
    void full_control() {
        const bool A = cthread ? get_req != get_req_d : get_req_d;
        if (A) {
            reg_full = 0;
        } else 
        if (core_req_d) {
            reg_full = 1;
        } else {
            reg_full = reg_full_d;
        }
    }
    
    void core_thread() {
        get_req_d   = 0;
        core_req_d  = 0;
        reg_full_d  = 0;
        core_data_d = T{};
        wait();
        
        while (true) 
        {
            get_req_d = get_req;
            core_req_d = core_req;
            reg_full_d = reg_full;

            if (core_req && !reg_full) {
                core_data_d = core_data;
            } 
            wait();
        }
    }
    
    void always_ready_thread() {
        if (cthread) get_req_d = 0;
        if (sync) {
            core_req_d  = 0;
            core_data_d = T{};
        }
        wait();
        
        while (true) 
        {
            if (cthread) get_req_d = get_req;
            if (sync) {
                core_req_d  = core_req;
                core_data_d = core_data;
            } 
            wait();
        }
    }
    
    // To avoid X at module interface
    void core_ready_meth() {
        core_ready = 1;
    } 
    
    void before_end_of_elaboration() override {
        if (!attached) {
            cout << "\nTarget " << name() 
                 << " is not attached to any process" << endl;
            assert (false);
        }
        if (!bound) {
            cout << "\nTarget " << name() 
                 << " is not bound to initiator" << endl;
            assert (false);
        }
        if (always_ready && fifo) {
            cout << "\nAlways ready target " << name() 
                 << " cannot have FIFO bound" << endl;
            assert (false);
        }
        put_fifo_handle = nullptr;
        PEEK.target = nullptr;
    }
    
  public:
    template<unsigned LENGTH>
    void add_fifo(bool sync_valid = 0, bool sync_ready = 0,
                  bool init_buffer = 0) {
        if (always_ready) {
            cout << "\nWarning : FIFO added to always ready target " 
                 << this->name() << endl;
        }
        
        fifo = new sct_fifo<T, LENGTH, TRAITS, 0>(
                        std::string(std::string(this->basename()) + "_fifo").c_str(), 
                        sync_valid, sync_ready, init_buffer);
        fifo->clk_nrst(clk, nrst);
        
        // Put to FIFO is performed in METHOD process
        fifo->addToPut(&this->sensitive, put_fifo_handle);

        // This target get is taken in METHOD process
        cthread = false;
        // Attach to put_to_fifo() process
        if (attached) {
            cout << "\nCreate target FIFO should be done before adding"
                 << " to sensitivity list : " << name() << endl; 
            assert (false);
        }
        attached = true;
    }
      
    /// Get target instance, used for sc_port of target
    sct_target<T, TRAITS, 0>& get_instance() {
        return *this;
    }
      
    /// Bind two given ports to each other via a new allocated signal
    template<class DutPort, class TbPort> 
    void bind_rtl(DutPort& dutPort, TbPort& tbPort) {
        using PT = typename DutPort::data_type;
        std::string sigName = std::string(dutPort.basename()) + "_s";
        auto* sig = new sc_signal<PT>(sigName.c_str());
        dutPort(*sig);
        tbPort(*sig);
        
        bound = true;
    }
    
    /// Bind to initiator 
    template<class Module> 
    void bind(Module& module, unsigned indx = 0) {
        do_bind(module, indx);
        module.do_bind(*this, indx);
    }
    
    template<class Module> 
    void do_bind(Module& module, unsigned indx) {
        if (bound) {
            cout << "\nDouble bound of target: " << name() << endl;
            assert (false);
        }
        bound = true; 

        if (always_ready) {
            // If always ready, register added in target
            if (module.sync) sync = true;
            module.sync = false;
        } else {
            // If not always ready, register added in initiator
            if (sync) module.sync = true;
            sync = false;
        }
        module.always_ready = always_ready;

        // Avoid name duplicating warning
        std::string readyName = "core_ready_s";
        #ifndef __SC_TOOL__
            readyName = std::string(basename()) + "_" + readyName;
        #endif
                
        if constexpr (std::is_base_of<sct_multi_initiator_base, Module>::value) {
            // Bind multi target initiator
            if (!always_ready) {
                cout << "\nNot-always ready target cannot be bound to multi initiator: " 
                     << module.name() << endl;
                assert (false);
            }
            module.bound_cntr++;
            
            sc_signal<bool>* core_ready_s = new sc_signal<bool>(readyName.c_str());
            core_ready(*core_ready_s);
            module.core_ready[indx](*core_ready_s);

        } else {
            // Bind initiator port to this target
            if (module.bound) {
                cout << "\nDouble bound of initiator: " << module.name() << endl;
                assert (false);
            }
            module.bound = true;

            sc_signal<bool>* core_ready_s = new sc_signal<bool>(readyName.c_str());
            core_ready(*core_ready_s);
            module.core_ready(*core_ready_s);
        }
    }

    template<class Module>
    void operator() (Module&) {
        cout << "Target " << name() <<  " operator() is prohibited, use bind() instead" << endl;
        assert (false);
    }

    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in)
    {
        clk(clk_in);
        nrst(nrst_in);
    }
 
    /// Add signals to sensitivity list of METHOD where @get() is called
    void addTo(sc_sensitive& s) override
    {
        if (fifo) {
            fifo->addToGet(s);
            
        } else {
            auto procKind = sc_get_current_process_handle().proc_kind();
            cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
            if (cthread) {
                if (TRAITS::CLOCK == 2) s << clk; 
                else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            } else {
                s << core_req << core_req_d << reg_full 
                  << core_data << core_data_d;  // No @nrst required here
            }
            
            if (attached) {
                cout <<  "\nDouble addTo() for target: " << name() << endl; 
                assert (false);
            }
            attached = true;
            //cout << "Set " << this->name() << " CTHREAD " << cthread << endl; 
        }
    }
    
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        if (fifo) {
            fifo->addToGet(s, p);
            
        } else {
            auto procKind = p->proc_kind();
            cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
            if (cthread) {
                if (TRAITS::CLOCK == 2) *s << *p << clk; 
                else *s << *p << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            } else {
                *s << *p << core_req << core_req_d << reg_full 
                   << core_data << core_data_d;  // No @nrst required here
            }
            
            if (attached) {
                cout <<  "\nDouble addTo() for target: " << name() << endl; 
                assert (false);
            }
            attached = true;
            //cout << "Set " << this->name() << " CTHREAD " << cthread << endl; 
        }
    }
    
    void addPeekTo(sc_sensitive& s) override
    {
        if (fifo) {
            fifo->addPeekTo(s);
            
        } else {
            auto procKind = sc_get_current_process_handle().proc_kind();
            bool cthread_peek = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
            if (cthread_peek) {
                if (TRAITS::CLOCK == 2) s << clk; 
                else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            } else {
                s << core_req << core_req_d << reg_full 
                  << core_data << core_data_d;  // No @nrst required here
            }
        }
    }
    
    sct_target_peek<T, TRAITS, false> PEEK{this};
};

//==============================================================================

/// Approximate time implementation
template<class T, class TRAITS>
class sct_target<T, TRAITS, 1> : 
    public sc_module,
    public sct_get_if<T>
{
    friend class sct_initiator<T, TRAITS, 1>;

  public:
    static const unsigned long long ALL_ENABLED = ~0ULL;
    
    sc_in<bool>     nrst{"nrst"};

    SC_HAS_PROCESS(sct_target);
    
    explicit sct_target(const sc_module_name& name, 
                        bool sync_ = 0, 
                        bool always_ready_ = 0) : 
        sc_module(name), sync(sync_), always_ready(always_ready_),
        fifo(std::string(std::string(name)+"_fifo").c_str(), 2)
    {
        SC_METHOD(resetProc);
        sensitive << nrst;
    }
    
    sct_target(const sct_target<T>&) = delete;
    sct_target& operator = (const sct_target<T>&) = delete;
    
    // Get the default event
    const sc_event& default_event() const override {
        return fifo.event_put();
    }
  
    bool sync;
    bool cthread = false;   
    const bool always_ready;

  protected:
    /// FIFO channel to store requests
    sct_prim_fifo<T>    fifo;
    /// Handle of put processes attached to the initiator bound 
    sc_process_handle** put_handle_ptr = nullptr;
      
    /// Attached FIFO length
    unsigned att_fifo_length = 0;
    
    /// Clear FIFO buffer
    void resetProc() {
        // Reset is active
        bool reset = TRAITS::RESET ? nrst : !nrst;
        fifo.reset_core(reset);
    }
    
    void before_end_of_elaboration() override {
        unsigned A;
        if (always_ready) {
            A = 2;
        } else {
            A = fifo.size();
        }
        // If sync valid, one more FIFO slot required
        if (sync) A += 1;
        
        // Consider attached FIFO length
        A += att_fifo_length;

        // Minimum 2 slots required to have put&get at the same DC
        assert (A > 1 && "Primitive FIFO size should be at least 2");
        fifo.resize(A);

        if (put_handle_ptr) {
            if (*put_handle_ptr) {
                fifo.addToPut(&this->sensitive, *put_handle_ptr);
                
                // Set sync ready if both put and get processes are methods
                auto procKind = (*put_handle_ptr)->proc_kind();
                bool A = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
                fifo.setSync(sync, !cthread && !A);
                
            } else {
                // Do not check initiator attached to a process as it could
                // be used in non-process context
            }
        } else {
             cout << "No initiator bound for target "  << name() << endl;
             assert (false);
        }
    }
    
  public:
    /// Check @request() for request data is available to take
    bool request() const override {
        return fifo.request();
    }
    
    /// Call in METHOD initial section and CTHREAD reset section
    void reset_get() override {
        fifo.reset_get();
    }
    
    /// Call in METHOD and CTHREAD everywhere
    /// Clear get request which taken in this cycle/DC
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
    
  public:
    /// Add internal FIFO 
    template<unsigned LENGTH>
    void add_fifo(bool sync_valid = 0, bool sync_ready = 0,
                  bool init_buffer = 0) {
        att_fifo_length = LENGTH;
    }

    /// Get target instance, used for sc_port of target
    sct_target<T, TRAITS, 1>& get_instance() {
        return *this;
    }
    
    /// Bind to initiator 
    template<class Module> 
    void bind(Module& module, unsigned indx = 0) {
        if (module.sync) sync = true;

        if constexpr (std::is_base_of<sct_multi_initiator_base, Module>::value) {
            module.put_port[indx].bind(fifo);
        } else {
            module.put_port.bind(fifo);
        }
        put_handle_ptr  = &module.put_handle;
    }

    template<class MOD>
    void operator() (MOD&) {
        cout << "Target " << name() <<  " operator() is prohibited, use bind() instead" << endl;
        assert (false);
    }
    
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in) {
        fifo.clk_nrst(clk_in, nrst_in);
        nrst(nrst_in);
    }
    
    /// Add this target to sensitivity list
    void addTo(sc_sensitive& s) override {
        auto procKind = sc_get_current_process_handle().proc_kind();
        cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        fifo.addToGet(s);   // No @nrst required here
    }
    
    /// Used in @sc_port of @sct_signal
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        fifo.addToGet(s, p);
        if (procKind != SC_CTHREAD_PROC_) {
            *s << *p;       // No @nrst required here
        }
    }    
    
    void addPeekTo(sc_sensitive& s) override {
        fifo.addPeekTo(s);  // No @nrst required here
    }
     
    sct_target_peek<T, TRAITS, true> PEEK{this};
};

//==============================================================================

template<class T, class TRAITS>
class sct_comb_target<T, TRAITS, false> : public sct_target<T, TRAITS, 0>
{
  public:
    explicit sct_comb_target(const sc_module_name& name, bool SYNC_ = 0) : 
        sct_target<T, TRAITS, 0>(name, SYNC_, 1)
    {}
};

template<class T, class TRAITS>
class sct_comb_target<T, TRAITS, true> : public sct_target<T, TRAITS, 1>
{
  public:
    explicit sct_comb_target(const sc_module_name& name, bool SYNC_ = 0) : 
        sct_target<T, TRAITS, 1>(name, SYNC_, 1)
    {}
};

} // namespace sct

//==============================================================================

namespace sc_core {

template<class T, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s,  
              sct::sct_target<T, TRAITS, TLM_MODE>& target )
{
    target.addTo(s);
    return s;
}

template<class T, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_target_peek<T, TRAITS, TLM_MODE>& peek )
{
    peek.target->addPeekTo(s);
    return s;
}

} // namespace sc_core

#endif /* SCT_TARGET_H */

