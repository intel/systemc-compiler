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
 * Target and Initiator are bound to each other with method bind().
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_TARGET_H
#define SCT_TARGET_H

#include "sct_ipc_if.h"
#include "sct_prim_fifo.h"
#include "sct_prim_buffer.h"

namespace sct {

/// Cycle accurate implementation    
template<class T, class TRAITS>
class sct_target<T, TRAITS, 0> : 
    public sc_module, public sct_get_if<T>
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
        sc_module(name), chan_sync(sync_), orig_sync(sync_), always_ready(always_ready_)
    {
        if (always_ready) {
            SCT_CTHREAD(always_ready_thread, clk, TRAITS::CLOCK);
            async_reset_signal_is(nrst, TRAITS::RESET);
            
            SC_METHOD(core_ready_meth);
            
        } else {
            SC_METHOD(ready_control);
            sensitive << get_req << get_req_d << core_req_d;

        #ifdef SCT_SEQ_METH
            SC_METHOD(core_thread);
            sensitive << (TRAITS::CLOCK ? clk.pos() : clk.neg()) 
                      << (TRAITS::RESET ? nrst.pos() : nrst.neg()); 
        #else
            SCT_CTHREAD(core_thread, clk, TRAITS::CLOCK);
            async_reset_signal_is(nrst, TRAITS::RESET);
        #endif
            
            SC_METHOD(put_to_fifo);
            sensitive << core_req << core_ready << core_data << core_data_d;
            put_fifo_handle = new sc_process_handle(sc_get_current_process_handle());
        }
        
        SC_METHOD(core_data_mux);
        sensitive << core_ready << core_data << core_data_d;
        
    #ifdef SCT_DEBUG
        SC_METHOD(debugProc); 
        debug_handle = new sc_process_handle(sc_get_current_process_handle());
    #endif
    }
    
  public:
    /// Request can be taken
    bool request() const override {
        return (fifo ? fifo->request() : 
                always_ready ? (chan_sync ? core_req_d : core_req) :
                               (!core_ready || core_req));
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
            return core_data_out.read();
        }
    }
    
    /// \return current request data, if no request last data returned
    T get() override {
        if (fifo) {
            return fifo->get();
        } else {
            bool A = always_ready ? (chan_sync ? core_req_d : core_req) : 
                                    (!core_ready || core_req);
            if (A) {
                get_req = cthread ? !get_req : 1;
            } else {
                if (!cthread) get_req = 0;
            }
            return core_data_out.read();
        }
    }
    
    /// \return true if request is valid and enable is true
    bool get(T& data, bool enable = true) override {
        if (fifo) {
            return fifo->get(data, enable);
            
        } else {
            data = core_data_out.read();
        
            bool A = always_ready ? (chan_sync ? core_req_d : core_req) : 
                                    (!core_ready || core_req);
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
            if (!cthread) {
                cout << "\nNo blocking get allowed in METHOD process" << endl;
                sc_assert (false);
            }
            
            if (!always_ready) {
                while (core_ready && !core_req) wait();
            } else {
                if (chan_sync) {
                    while (!core_req_d) wait();
                } else {
                    while (!core_req) wait();
                }
            }
            get_req = !get_req;
            return core_data_out.read();
        }
    }

 public:
    bool chan_sync;
    bool orig_sync;
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
    sc_signal<bool>         core_req_d{"core_req_d"};
    sc_signal<T>            core_data_d{"core_data_d"};
    sc_signal<T>            core_data_out{"core_data_out"};
    
    /// Output data multiplexer to support return constant reference
    void core_data_mux() {
        if (always_ready) {
            if (chan_sync) core_data_out = core_data_d.read();
            else core_data_out = core_data.read();
        } else {
            if (core_ready) core_data_out = core_data.read();
            else core_data_out = core_data_d.read();
        }
    }
    
    /// Get request and put it to FIFO if bound
    void put_to_fifo() {
        if (fifo) {
            get_req = 0;
            fifo->reset_put();
            
            if (!core_ready) {
                get_req = fifo->put(core_data_d.read());
            } else 
            if (core_req) {
                get_req = fifo->put(core_data.read());
            }    
        }
    }
    
    void ready_control() {
        const bool A = cthread ? get_req != get_req_d : get_req_d;
        if (A) {
            core_ready = 1;
        } else {
            core_ready = !core_req_d;
        }
    }
    
    void core_thread() {
    #ifdef SCT_SEQ_METH
        if (TRAITS::RESET ? nrst : !nrst) {
    #endif
            get_req_d   = 0;
            core_req_d  = 0;
            core_data_d = T{};
    #ifdef SCT_SEQ_METH
        } else {
    #else
        wait();
        
        while (true) {
    #endif
            get_req_d  = get_req;
            core_req_d = !core_ready || core_req;

            if (core_req && core_ready) {
                core_data_d = core_data;
            } 
    #ifndef SCT_SEQ_METH
            wait();
    #endif
        }
    }
    
    void always_ready_thread() {
    #ifdef SCT_DEBUG   
        if (cthread) get_req_d = 0;
    #endif
        if (chan_sync) {
            core_req_d  = 0;
            core_data_d = T{};
        }
        wait();
        
        while (true) 
        {
        #ifdef SCT_DEBUG   
            if (cthread) get_req_d = get_req;
        #endif
            if (chan_sync) {
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
        if (clk.bind_count() != 1 || nrst.bind_count() != 1) {
            cout << "\nTarget " << name() 
                 << " clock/reset inputs are not bound or multiple bound" << endl;
            assert (false);
        }

        put_fifo_handle = nullptr;
        PEEK.target = nullptr;
        
    #ifdef SCT_DEBUG
        if (fifo) {
            this->sensitive << *debug_handle;
            fifo->addPeekTo(this->sensitive);
        } else {
            this->sensitive << *debug_handle << get_req << get_req_d
                            << core_req_d << core_req << core_ready
                            << core_data << core_data_d << core_data_out;
        }
        debug_handle = nullptr;
    #endif
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
        
        if (!always_ready && chan_sync) {
            cout << "\nNo sync register allowed in TB target: " << name() << endl;
            assert (false);
        }
        
        bound = true;
    }
    
    /// Bind to initiator 
    template<class Module> 
    void bind(Module& module, unsigned indx = 0) {
        do_bind(module, indx);
        module.do_bind(*this, indx);    // Do nothing in CDC FIFO
    }
    
    template<class Module> 
    void do_bind(Module& module, unsigned indx) {
        if (bound) {
            cout << "\nDouble bound of target: " << name() << endl;
            assert (false);
        }
        bound = true; 

        if constexpr (std::is_base_of<sct_cdc_fifo_base, Module>::value) {
            if (always_ready) {
                cout << "\nNo always ready target " << name() 
                     << " supported as it bound to CDC FIFO" << endl;
                assert (false);
            }
            if (chan_sync) {
                cout << "\nNo sync register in target " << name() 
                     << " supported as it bound to CDC FIFO" << endl;
                chan_sync = false;
                assert (false);
            }

            // Do not change @chan_sync and @always_ready
            // Bind CDC FIFO pop side to this target
            std::string readyName = "pop_req_s";
            
            sc_signal<bool>* pop_req_s = new sc_signal<bool>(readyName.c_str());
            core_ready(*pop_req_s);
            module.pop_req(*pop_req_s);

            // Bind this target to CDC FIFO pop side
            module.do_pop_bind(*this);

        } else {
            if (always_ready) {
                // If always ready, register added in target
                if (module.chan_sync) chan_sync = true;
                module.chan_sync = false;
            } else {
                // If not always ready, register added in initiator
                if (chan_sync) module.chan_sync = true;
                chan_sync = false;
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
                sc_signal<bool>* core_ready_s = new sc_signal<bool>(readyName.c_str());
                core_ready(*core_ready_s);
                module.core_ready(*core_ready_s);
            }
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
                s << core_req << core_req_d << core_ready << core_data_out;  // No @nrst required here
            }
            
            if (attached) {
                cout << "\nTarget " << name() 
                     << " is already attached to a process" << endl;
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
                *s << *p << core_req << core_req_d << core_ready << core_data_out;  // No @nrst required here
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
                s << core_req << core_req_d << core_ready << core_data_out;  // No @nrst required here
            }
        }
    }
    
#ifdef SCT_DEBUG
    sc_process_handle*  debug_handle = nullptr;

    sc_signal<bool>     out_valid{"out_valid"};
    sc_signal<bool>     debug_get{"get"};
    sc_signal<T>        data_out{"data_out"};
    sc_signal<unsigned> element_num_d{"element_num_d"};
    
    void debugProc() {
        out_valid = request();
        debug_get = cthread ? get_req != get_req_d : get_req;
        data_out  = peek();
        element_num_d = fifo ? fifo->elem_num() : core_req_d.read();
    }
#endif
    
    void trace(sc_trace_file* tf) const override {
    #ifdef SCT_DEBUG
        std::string targName = name();
        // Functional interface signals
        sc_trace(tf, out_valid, targName + "_req_if");
        sc_trace(tf, debug_get, targName + "_get_if");
        sc_trace(tf, data_out, targName + "_data_if");

        // Core interface to initiator or other
        sc_trace(tf, core_ready, targName + "_core_ready");
        sc_trace(tf, core_req, targName + "_core_req");
        sc_trace(tf, core_data, targName + "_core_data");
        sc_trace(tf, get_req, targName + "_get_req");
        sc_trace(tf, get_req_d, targName + "_get_req_d");
        sc_trace(tf, core_req_d, targName + "_core_req_d");
        sc_trace(tf, core_data_d, targName + "_core_data_d");
        sc_trace(tf, element_num_d, targName + "_element_num_d");
    #endif
    }
    
    inline void print(::std::ostream& os) const override
    {
        os << "sct_target " << name();
        
        if ( request() ) {
            os << " ( " << peek() << " )";
        } else {
            os << " is empty";
        }
        os << ::std::endl;
    }
    
    sct_target_peek<T, TRAITS, false> PEEK{this};
};

//==============================================================================

/// Approximately timed implementation
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
    /// Handle of put process attached to the initiator bound 
    sc_process_handle** put_handle_ptr = nullptr;
      
    /// Attached FIFO length
    unsigned att_fifo_length = 0;
    
    /// Clear FIFO buffer
    void resetProc() {
        // Reset is active
        bool reset = TRAITS::RESET ? nrst : !nrst;
        fifo.reset_core(reset);
    }
    
    /// Use before_end_of_elaboration as there is end_of_elaboration in @fifo
    void before_end_of_elaboration() override {
        // Set FIFO simulation mode for Target and Initiator    
        fifo.setTargInit(sync);

        bool cthread_put = false;
        if (put_handle_ptr) {
            if (*put_handle_ptr) {
                fifo.addToPut(&this->sensitive, *put_handle_ptr);
                // Set sync ready if get processes is method
                auto procKind = (*put_handle_ptr)->proc_kind();
                cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
                fifo.setSync(sync, !cthread);
            } else {
                // Do not check initiator attached to a process as it could
                // be used in non-process context
            }
        } else {
             cout << "No initiator bound for target "  << name() << endl;
             assert (false);
        }

        // Update FIFO size
        unsigned fifoSize = cthread_put ? 2 : 1; 
        // If sync valid, one more FIFO slot required
        if (sync) fifoSize += 1;
        
        // Consider attached FIFO length
        fifoSize += att_fifo_length;
        fifo.resize(fifoSize);

        //cout << "FIFO " << fifo.name() << " " << fifoSize << ", sync valid " 
        //     << sync << ", sync ready " << (!cthread) << endl;
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
        if constexpr (std::is_base_of<sct_cdc_fifo_base, Module>::value) {
            module.do_pop_bind(*this);
        } else {
            if (module.sync) sync = true;

            if constexpr (std::is_base_of<sct_multi_initiator_base, Module>::value) {
                module.put_port[indx].bind(fifo);
            } else {
                module.put_port.bind(fifo);
            }
            put_handle_ptr = &module.put_handle;
        }
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
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            auto procKind = sc_get_current_process_handle().proc_kind();
            cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
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
     
    void trace(sc_trace_file* tf) const override {
    #ifdef SCT_DEBUG
        fifo.trace(tf);
    #endif
    }

    inline void print(::std::ostream& os) const override {
        fifo.print(os);
    }
    
    sct_target_peek<T, TRAITS, 1> PEEK{this};
};

//==============================================================================

/// Loosely timed implementation
template<class T, class TRAITS>
class sct_target<T, TRAITS, 2> : 
    public sct_prim_buffer<T>
{
    friend class sct_initiator<T, TRAITS, 2>;

  public:
    using base_class = sct_prim_buffer<T>;
    
    SC_HAS_PROCESS(sct_target);
    
    /// \param multi_put -- multiple put in one DC allowed
    /// \param multi_get -- multiple get in one DC allowed
    explicit sct_target(const char* name, bool sync_ = 0, 
                        bool always_ready_ = 0,
                        bool multi_put = 0, bool multi_get = 0) : 
        base_class(name, 2, multi_put, multi_get),
        always_ready(always_ready_)
    {}
    
    sct_target(const sct_target<T>&) = delete;
    sct_target& operator = (const sct_target<T>&) = delete;
    
    // Get the default event
    const sc_event& default_event() const override {
        return base_class::event_put();
    }
  
    const bool always_ready;

  protected:
    /// Handle of put process attached to the initiator bound 
    sc_process_handle** put_handle_ptr = nullptr;
    sc_sensitive** put_sens_ptr = nullptr;
      
    /// Attached FIFO length
    unsigned att_fifo_length = 0;
    
    /// Use before_end_of_elaboration as there is end_of_elaboration in @base_class
    void before_end_of_elaboration() override {
        if (put_handle_ptr && put_sens_ptr) {
            if (*put_handle_ptr && *put_sens_ptr) {
                base_class::addToPut(*put_sens_ptr, *put_handle_ptr);
                // Set sync ready if get processes is method
                auto procKind = (*put_handle_ptr)->proc_kind();
                bool cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
                if (!cthread) {
                    cout << "Initiator in LT mode cannot be used in combinational method process" << endl;
                    assert (false);
                }
            } else {
                // Do not check initiator attached to a process as it could
                // be used in non-process context
            }
        } else {
             cout << "No initiator bound for target " << this->name() << endl;
             assert (false);
        }

        // Consider attached FIFO length
        if (att_fifo_length) {
            base_class::resize(att_fifo_length+2);
        }
    }
    
  public:
    /// Add internal FIFO 
    template<unsigned LENGTH>
    void add_fifo(bool sync_valid = 0, bool sync_ready = 0,
                  bool init_buffer = 0) {
        att_fifo_length = LENGTH;
    }

    /// Get target instance, used for sc_port of target
    sct_target<T, TRAITS, 2>& get_instance() {
        return *this;
    }
    
    /// Bind to initiator 
    template<class Module> 
    void bind(Module& module) {
        module.put_port.bind(*this);
        put_handle_ptr = &module.put_handle;
        put_sens_ptr = &module.put_sens;
    }

    template<class MOD>
    void operator() (MOD&) {
        cout << "Target " << this->name() <<  " operator() is prohibited, use bind() instead" << endl;
        assert (false);
    }
    
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in, RSTN_t& nrst_in) {
        base_class::clk_nrst(clk_in, nrst_in);
    }
    
    /// Add this target to sensitivity list
    void addTo(sc_sensitive& s) override {
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method, do nothing
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            auto procKind = sc_get_current_process_handle().proc_kind();
            bool cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
            if (!cthread) {
                cout << "Target in LT mode cannot be used in combinational method process" << endl;
                assert (false);
            }
        }
        base_class::addToGet(s); 
    }
    
    /// Used in @sc_port of @sct_signal
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        bool cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        if (!cthread) {
            cout << "Target in LT mode cannot be used in combinational method process" << endl;
            assert (false);
        }
        base_class::addToGet(s, p);
        
        if (procKind != SC_CTHREAD_PROC_) {
            *s << *p;       // No @nrst required here
        }
    }
    
    void trace(sc_trace_file* tf) const override {
    #ifdef SCT_DEBUG
        // this->trace(tf); -- Not implemented yet
    #endif
    }

    sct_target_peek<T, TRAITS, 2> PEEK{this};
};

//==============================================================================

template<class T, class TRAITS>
class sct_comb_target<T, TRAITS, 0> : public sct_target<T, TRAITS, 0>
{
  public:
    explicit sct_comb_target(const sc_module_name& name, bool SYNC_ = 0) : 
        sct_target<T, TRAITS, 0>(name, SYNC_, true)
    {}
};

template<class T, class TRAITS>
class sct_comb_target<T, TRAITS, 1> : public sct_target<T, TRAITS, 1>
{
  public:
    explicit sct_comb_target(const sc_module_name& name, bool SYNC_ = 0) : 
        sct_target<T, TRAITS, 1>(name, SYNC_, true)
    {}
};

template<class T, class TRAITS>
class sct_comb_target<T, TRAITS, 2> : public sct_target<T, TRAITS, 2>
{
  public:
    explicit sct_comb_target(const sc_module_name& name) : 
        sct_target<T, TRAITS, 2>(name, false, true)
    {}
};

} // namespace sct

//==============================================================================

namespace sc_core {

template<class T, class TRAITS, unsigned TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s,  
              sct::sct_target<T, TRAITS, TLM_MODE>& target )
{
    target.addTo(s);
    return s;
}

template<class T, class TRAITS, unsigned TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_target_peek<T, TRAITS, TLM_MODE>& peek )
{
    peek.target->addPeekTo(s);
    return s;
}

template<class T, class TRAITS, unsigned TLM_MODE>
inline ::std::ostream& operator << (::std::ostream& os, 
                    const sct::sct_target<T, TRAITS, TLM_MODE>& target ) 
{
    target.print(os);
    return os;
}

} // namespace sc_core

#endif /* SCT_TARGET_H */

