/******************************************************************************
 * Copyright (c) 2023-2025, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Buffer for thread processes with LT/AT support.
 * 
 * Used as base channel in Loosely timed mode for Target, Initiator and Buffer.
 * Not intended to be used in user code.
 * 
 * Author: Mikhail Moiseev
 */   

#ifndef SCT_PRIM_BUFFER_H
#define SCT_PRIM_BUFFER_H

#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {

/// Buffer works in TLM in Loosely timed (LT) or Approximately timed (AT) modes
/// Buffer supports multiple put/get in one delta cycle (DC) to use with LT mode
template <class T>
class sct_prim_buffer : 
    public sct_fifo_if<T>,
    public sc_prim_channel
{
   public:
    /// \param multi_put -- multiple put in one DC allowed
    /// \param multi_get -- multiple get in one DC allowed
    explicit sct_prim_buffer(const char* name, unsigned size,
                             bool multi_put, bool multi_get) :
        sc_prim_channel(name),
        bufSize(size), 
        buffer(size, T{}),      
        MULTI_PUT(multi_put), MULTI_GET(multi_get),
        put_event(std::string(std::string(name)+"_put_event").c_str()),
        get_event(std::string(std::string(name)+"_get_event").c_str())
    {
        assert (bufSize > 1);
    }

    sct_prim_buffer(const sct_prim_buffer<T>&) = delete;
    sct_prim_buffer& operator = (const sct_prim_buffer<T>&) = delete;
        
  public:
    /// No @event notified after exit from reset, the processes should be
    /// waken up by another channel  
      
    inline bool ready() const override {
        return (bufSize-element_num > (MULTI_PUT ? put_num : 0));
    }
    
    inline bool request() const override {
        return (element_num > (MULTI_GET ? get_num : 0));
    }
    
    /// Call in METHOD everywhere and CTHREAD reset sections
    inline void reset_get() override {
        pop_indx = 0;
        get_num = 0;
        request_update();
    }
    
    /// Call in METHOD everywhere and CTHREAD reset sections
    inline void reset_put() override {
        push_indx = 0;
        element_num = 0;
        put_num = 0; 
        buffer[0] = T{};
        request_update();
    }
    
    /// Call both put and get resets if used from the same process
    inline void reset() {
        reset_get();
        reset_put();
    }
    
    inline void clear_get() override {
        if (!MULTI_GET) {
            get_num = 0;
        } else {
            cout << "\nNo clear allowed for sct_lt_buffer" << endl;
            sc_assert(false);
        }
    }
    
    inline void clear_put() override {
        if (!MULTI_PUT) {
            put_num = 0;
        } else {
            cout << "\nNo clear allowed for sct_lt_buffer" << endl;
            sc_assert(false);
        }
    }
    
    inline T peek() const override {
        return buffer[pop_indx];
    }
    
  protected:    
    inline void doGet() {
        if (MULTI_GET) {
            pop_indx = (pop_indx == bufSize-1) ? 0 : pop_indx+1;
        }
        // Notify put process if no more elements in buffer
        // Plus 1 to initiate first put at same cycle as last get
        if (MULTI_PUT && element_num <= get_num+2) { 
            put_event.notify(clk_period);
            //cout << sc_time_stamp() << " " << sc_delta_count() << " put_event 1" << endl;
        }
        // First call of @doGet in this cycle
        if (get_num == 0) {
            // Notify put process if buffer was full
            if (!MULTI_PUT && element_num == bufSize) {
                put_event.notify(clk_period);
                //cout << sc_time_stamp() << " " << sc_delta_count() << " put_event 2" << endl;
            }
            // Notify get process after each get
            if (!MULTI_GET) {
                get_event.notify(clk_period);
                //cout << sc_time_stamp() << " " << sc_delta_count() << " get_event 3" << endl;
            }
            // If both sides use multi requests, need to wake up them every cycle
            if (MULTI_PUT && MULTI_GET) {
                get_event.notify(clk_period);
            }
            
            request_update();
        }
        get_num = MULTI_GET ? get_num+1 : 1;
    }
    
    inline void doPut(const T& data) {
        //cout << "doPut at " << push_indx << " val " << data << endl;
        buffer[push_indx] = data; 
        if (MULTI_PUT) {
            push_indx = (push_indx == bufSize-1) ? 0 : push_indx+1;
        }
        // Notify get process if buffer is full
        // Minus 1 to initiate first get at same cycle as last put
        if (MULTI_GET && element_num >= bufSize-put_num-2) {
            get_event.notify(clk_period);
            //cout << sc_time_stamp() << " " << sc_delta_count() << " get_event 1" << endl;
        }
        // First call of @doPut in this cycle
        if (put_num == 0) {
            // Notify get process if buffer was empty
            if (!MULTI_GET && element_num == 0) {
                get_event.notify(clk_period);
                //cout << sc_time_stamp() << " " << sc_delta_count() << " get_event 2" << endl;
            }
            // Notify put process after each put
            if (!MULTI_PUT) {
                put_event.notify(clk_period);
                //cout << sc_time_stamp() << " " << sc_delta_count() << " put_event 3" << endl;
            }
            // If both sides use multi requests, need to wake up them every cycle
            if (MULTI_PUT && MULTI_GET) {
                put_event.notify(clk_period);
            }

            request_update();
        }
        put_num = MULTI_PUT ? put_num+1 : 1;
    }

  public:     
    /// \return current request data, if no request last data returned
    inline T get() override {
        T res = buffer[pop_indx];
        //cout << sc_time_stamp() << " " << sc_delta_count() << " put " << name() 
        //     << " get_num " << get_num << " data " << res << endl;
        if (element_num > (MULTI_GET ? get_num : 0)) { doGet(); }
        return res;
    }
    
    /// \return true if request is valid and enable is true
    inline bool get(T& data, bool enable = true) override {
        data = buffer[pop_indx];
        if (enable && element_num > (MULTI_GET ? get_num : 0)) {
            doGet();
            return true;
        } else {
            return false;
        }
    }

    inline T b_get() override {
        if (!MULTI_GET) {
            while (element_num == 0) wait();
            doGet();
            return buffer[pop_indx];
        } else {
            cout << "/nBlocking get not allowed for sct_lt_buffer" << endl;
            sc_assert(false);
            return T{};
        }
    }
    
    inline bool put(const T& data) override {
        //cout << sc_time_stamp() << " " << sc_delta_count() << " put " << name() 
        //     << " put_num " << put_num << " data " << data << endl;
        if (bufSize-element_num > (MULTI_PUT ? put_num : 0)) {
            doPut(data);
            return true;
        } else {
            return false;
        }
    }

    inline bool put(const T& data, sc_uint<1> mask) override {
        if (mask && bufSize-element_num > (MULTI_PUT ? put_num : 0)) {
            doPut(data);
            return true;
        } else {
            return false;
        }
    }
    
    inline void b_put(const T& data) override {
        if (!MULTI_PUT) {
            while (element_num == bufSize) wait();
            doPut(data);
        } else {
            cout << "/nBlocking put not allowed for sct_lt_buffer" << endl;
            sc_assert(false);
        }
    }
    
    /// Maximal number of elements
    inline unsigned size() const override {
        return bufSize;
    }
    
    /// Resize buffer
    inline void resize(unsigned new_size) {
        sc_assert (new_size > 1);
        bufSize = new_size;
        buffer.resize(new_size, T{});
    }
    
    /// Number of elements in FIFO after last clock edge
    inline unsigned elem_num() const override {
        return element_num;
    }
    
    /// Buffer has (size()-N) elements or more
    inline bool almost_full(const unsigned& N = 0) const override {
        return (element_num >= bufSize-N);
    }
    
    /// Buffer has N elements or less
    inline bool almost_empty(const unsigned& N = 0) const override {
        return (element_num <= N);
    }
    
  protected:
    /// Multiple put/get i.e. LT mode for buffer sides
    const bool MULTI_PUT;
    const bool MULTI_GET;
    
    unsigned short  pop_indx    : 16 = 0;
    unsigned short  get_num     : 16 = 0;
    unsigned short  push_indx   : 16 = 0;
    unsigned short  put_num     : 16 = 0;
    unsigned short  bufSize     : 16 = 0; 
    unsigned short  element_num : 16 = 0;

    sc_in_clk*      clk_in = nullptr;
    sc_time         clk_period = SC_ZERO_TIME;
    sc_in<bool>     nrst{"nrst"};

    std::vector<T>  buffer;
    
    /// Event for put, get and peek thread processes notification
    sc_event        put_event;
    sc_event        get_event;

    void update() override {
        if (!MULTI_GET && get_num) {
            pop_indx = (pop_indx == bufSize-1) ? 0 : pop_indx+1;
        }
        element_num -= get_num;
        get_num  = 0;
        
        if (!MULTI_PUT && put_num) {
            push_indx = (push_indx == bufSize-1) ? 0 : (push_indx+1);
        }
        element_num += put_num;
        put_num = 0;

        //cout << sc_time_stamp() << " " << sc_delta_count() << " update " << name() 
        //     << " element_num " << element_num << endl;
        //cout << name() << " element_num " << element_num << endl;
    }
    
    /// Use end_of_elaboration as @get_clk_period used inside
    void end_of_elaboration() override {
        if (clk_in) {
            clk_period = get_clk_period(clk_in);
        } else {
            cout << "\nBuffer " << name() << " clock input is not bound" << endl;
            assert (false);
        }
    }
    
  public:
    void clk_nrst(sc_in_clk& clk_in_, sc_in<bool>& nrst_in) override {
        clk_in = &clk_in_; 
        nrst(nrst_in);
    }
    
    void add_to(sc_sensitive& s, bool attachedPut, bool attachedGet) 
    {
        bool cthread;
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread = true;
        } else {
            auto procKind = sc_get_current_process_handle().proc_kind();
            assert (procKind != SC_CTHREAD_PROC_);
            cthread = procKind == SC_THREAD_PROC_;
        }
        if (!cthread) {
            cout << "\nBuffer cannot be used in combinational method process" << endl;
            assert (false);
        }
        
        s << nrst;
        if (attachedPut) { s << put_event; }
        if (attachedGet) { s << get_event; }
    }
    
    void add_to(sc_sensitive* s, sc_process_handle* p, bool attachedPut, 
                bool attachedGet) 
    {
        bool cthread;
        if (sct_seq_proc_handle == *p) {
            // Sequential method
            cthread = true;
        } else {
            auto procKind = p->proc_kind();
            assert (procKind != SC_CTHREAD_PROC_);
            cthread = procKind == SC_THREAD_PROC_;
        }
        if (!cthread) {
            cout << "\nBuffer cannot be used in combinational method process" << endl;
            assert (false);
        }

        *s << *p << nrst;
        if (attachedPut) { *s << put_event; }
        if (attachedGet) { *s << get_event; }
    }
        
    void addTo(sc_sensitive& s) { 
        add_to(s, true, true); 
    }
    void addTo(sc_sensitive* s, sc_process_handle* p) override { 
        add_to(s, p, true, true); 
    }
    
    void addToPut(sc_sensitive& s) { 
        add_to(s, true, false); 
    }
    void addToPut(sc_sensitive* s, sc_process_handle* p) override {
        add_to(s, p, true, false); 
    }
    
    void addToGet(sc_sensitive& s) { 
        add_to(s, false, true); 
    }
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {
        add_to(s, p, false, true); 
    }
    
    void addPeekTo(sc_sensitive& s) override {
        if (MULTI_GET) {
            cout <<  "\nNo addPeekTo() allowed for multi-get Buffer: " << name() << endl;
            assert (false);
        }
        s << get_event; 
    }
    
    /// Get get/put process events
    const sc_event& event_get() const { 
        return get_event; 
    }
    
    const sc_event& event_put() const { 
        return put_event; 
    }
    
    const sc_event& default_event() const override {
        cout << "No default event for sct_lt_buffer " << name() << endl;
        assert (false); 
        return put_event;
    }
    
    inline void print(::std::ostream& os) const override
    {
        os << "sct_lt_buffer " << name();
        
        if (element_num != 0) {
            os << " ( ";
            unsigned popIndx = pop_indx;
            for (unsigned i = 0; i != element_num; ++i) {
                os << buffer[popIndx] << " ";
                popIndx = (popIndx == bufSize-1) ? 0 : popIndx+1;
            }
            os << ")";
        } else {
            os << " is empty";
        }
        os << ::std::endl;
    }
    
    const char* kind() const override { 
        return "sct_prim_buffer"; 
    }
};

} // namespace sct

//==============================================================================

#ifndef __SC_TOOL__

namespace sc_core {


template<class T>
inline ::std::ostream& operator << (::std::ostream& os, 
                                    const sct::sct_prim_buffer<T>& buffer) 
{
    buffer.print(os);
    return os;
}

} // namespace sc_core
#endif

#endif /* SCT_PRIM_BUFFER_H */

