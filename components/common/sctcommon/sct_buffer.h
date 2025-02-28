/******************************************************************************
 * Copyright (c) 2023-2024, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Buffer channel.
 * 
 * Buffer is a kind of FIFO to use in single or two clocked processes. 
 * Put, get and peek processes should be sequential processes.
 * It is implemented as @sc_prim_channel to speed up simulation.
 * Buffer does not have separate implementation for TLM mode.
 * 
 * Author: Mikhail Moiseev
 */   

#ifndef SCT_BUFFER_H
#define SCT_BUFFER_H

#include "sct_static_log.h"
#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
#ifndef __SC_TOOL__

/// Fast implementation for cycle accurate simulation and TLM mode     
/// Buffer should use the same reset as thread(s) operates with it
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS = SCT_CMN_TRAITS /// Clock edge and reset level traits
>
class sct_buffer : 
    public sc_prim_channel,  
    public sct_fifo_if<T>
{
   public:
    /// Initialize FIFO slots in reset with zeros
    const bool INIT_BUFFER;  
       
    /// Number of bits in variables store index and length of FIFO
    static const unsigned INDX_WIDTH = sct_addrbits1<LENGTH>;
    using Indx_t = sc_uint<INDX_WIDTH>;
    static const unsigned ELEM_NUM_WIDTH = sct_nbits<LENGTH>;
    using ElemNum_t = sc_uint<ELEM_NUM_WIDTH>;

    /// \param init_buffer  -- Initialize all buffer elements with zeros in reset
    ///                        First element to get is always initialized to zero id 
    explicit sct_buffer(const char* name, 
                        bool sync_valid = 0, bool sync_ready = 0,
                        bool use_elem_num = 0, bool init_buffer = 0) :
        sc_prim_channel(name), INIT_BUFFER(init_buffer),
        event(std::string(std::string(name)+"_event").c_str())
    {
        static_assert (LENGTH > 0);
        // This implementation equals to async valid/ready FIFO
        assert (!sync_valid && !sync_ready && 
                "No sync valid/ready allowed for Buffer");
    }

  public:
    // No @event notified after exit from reset, the processes should be
    // waken up by another channel  
      
    bool ready() const override {
        return (element_num != LENGTH);
    }
    
    bool request() const override {
        return (element_num != 0);
    }
    
    /// Call in METHOD everywhere and CTHREAD reset sections
    void reset_get() override {
        pop_indx = 0;
        get_req = 0; data_out = T{};
        request_update();
    }
    
    /// Call in METHOD everywhere and CTHREAD reset sections
    void reset_put() override {
        push_indx = 0;
        put_req = 0; data_in = T{};
        element_num = 0;
        
        // Initialize zero cell to provide zero data before first push
        buffer[0] = T{};
        if (INIT_BUFFER) {
            for (unsigned i = 1; i != LENGTH; i++) {
                buffer[i] = T{};
            }
        }
        request_update();
    }
    
    /// Call both put and get resets if used from the same process
    void reset() {
        reset_get();
        reset_put();
    }
    
    void clear_get() override {
        get_req = 0;
        request_update();
    }
    
    void clear_put() override {
        put_req = 0;
        request_update();
    }
    
    T peek() const override {
        return data_out;
    }
    
    /// \return current request data, if no request last data returned
    T get() override {
        if (element_num != 0) {
            get_req = 1;
            request_update();
            if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
        }
        return data_out;
    }
    
    /// \return true if request is valid and enable is true
    bool get(T& data, bool enable = true) override {
        data = data_out;
        if (element_num != 0) {
            get_req = enable;
            request_update();
            if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
            return enable;
        } else {
            return false;
        }
    }

    T b_get() override {
        while (element_num == 0) wait();
        get_req = 1;
        request_update();
        if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
        return data_out;
    }
    
    bool put(const T& data) override {
        data_in = data; 
        if (element_num != LENGTH) {
            put_req = 1;
            request_update();
            if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
            return true;
        } else {
            return false;
        }
    }

    bool put(const T& data, sc_uint<1> mask) override {
        data_in = data;
        if (element_num != LENGTH) {
            put_req = bool(mask);
            request_update();
            if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
            return mask;
        } else {
            return false;
        }
    }
    
    void b_put(const T& data) override {
        data_in = data;
        while (element_num == LENGTH) wait();
        put_req = 1;
        request_update();
        if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
    }
    
    /// Maximal number of elements
    unsigned size() const override {
        return LENGTH;
    }
    
    /// Number of elements in FIFO after last clock edge
    unsigned elem_num() const override {
        return element_num;
    }
    
    /// Buffer has (size()-N) elements or more
    bool almost_full(const unsigned& N = 0) const override {
        return (element_num >= LENGTH-N);
    }
    
    /// Buffer has N elements or less
    bool almost_empty(const unsigned& N = 0) const override {
        return (element_num <= N);
    }
    
  protected:
    /// This buffer attached to a processes
    bool attached_put = false;
    bool attached_get = false;
    
    sc_in_clk*      clk_in = nullptr;
    sc_time         clk_period = SC_ZERO_TIME;

    T               buffer[LENGTH];
    
    /// Index of element that will be poped
    Indx_t          pop_indx;
    /// Index where pushed element will be stored
    Indx_t          push_indx;
    /// Number of elements
    ElemNum_t       element_num;
    
    bool            put_req;
    T               data_in;
    
    bool            get_req;
    T               data_out;

    /// Event for put, get and peek thread processes notification
    sc_event        event;

    void update() {
        if (get_req) {
            pop_indx = (pop_indx == LENGTH-1) ? 0 : (pop_indx+1);
            element_num -= 1;
            get_req  = 0;
        }
        if (put_req) {
            buffer[push_indx] = data_in;
            push_indx = (push_indx == LENGTH-1) ? 0 : (push_indx+1);
            element_num += 1;
            put_req = 0;
        }
        data_out = buffer[pop_indx];
    }
    
    void end_of_elaboration() override {
        if (!attached_put || !attached_get) {
            cout << "\nBuffer " << name() 
                 << " is not fully attached to process(es)" << endl;
            assert (false);
        }
        if constexpr (SCT_CMN_TLM_MODE) {
            if (clk_in) {
                clk_period = get_clk_period(clk_in);
            } else {
                cout << "\nBuffer " << name() << " clock input is not bound" << endl;
                assert (false);
            }
        }
    }
    
  public:
    void clk_nrst(sc_in_clk& clk_in_, sc_in<bool>& nrst_in) {
        if constexpr (SCT_CMN_TLM_MODE) { clk_in = &clk_in_; }
    }
    
    void add_to(sc_sensitive& s, bool attachedPut, bool attachedGet) {
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            if constexpr (SCT_CMN_TLM_MODE) { s << event; }
        } else {
            auto procKind = sc_get_current_process_handle().proc_kind();
            if (procKind != SC_THREAD_PROC_ && procKind != SC_CTHREAD_PROC_) {
                cout << "Buffer cannot be used in combinational method process" << endl;
                assert (false);
            }
            if constexpr (SCT_CMN_TLM_MODE) { 
                if (procKind != SC_CTHREAD_PROC_) { s << event; }
            }                
        }
        if (attachedPut) {
            if (attached_put) {
                cout <<  "Double addToPut() for Buffer: " << name() << endl; 
                assert (false);
            }
            attached_put = true;
        }
        if (attachedGet) {
            if (attached_get) {
                cout <<  "Double addToGet() for Buffer: " << name() << endl;
                assert (false);
            }
            attached_get = true;
        }
    }
    
    void add_to(sc_sensitive* s, sc_process_handle* p, bool attachedPut, 
                bool attachedGet) {
        if (sct_seq_proc_handle == *p) {
            // Sequential method
            if constexpr (SCT_CMN_TLM_MODE) { *s << *p << event; }
        } else {
            auto procKind = p->proc_kind();
            if (procKind != SC_THREAD_PROC_ && procKind != SC_CTHREAD_PROC_) {
                cout << "Buffer cannot be used in combinational method process" << endl;
                assert (false);
            }
            if constexpr (SCT_CMN_TLM_MODE) { 
                if (procKind != SC_CTHREAD_PROC_) { *s << *p << event; }
            }                
        }
        if (attachedPut) {
            if (attached_put) {
                cout <<  "Double addToPut() for Buffer: " << name() << endl; 
                assert (false);
            }
            attached_put = true;
        }
        if (attachedGet) {
            if (attached_get) {
                cout <<  "Double addToGet() for Buffer: " << name() << endl;
                assert (false);
            }
            attached_get = true;
        }
    }
        
    void addTo(sc_sensitive& s) override { add_to(s, true, true); }
    void addTo(sc_sensitive* s, sc_process_handle* p) override { 
            add_to(s, p, true, true); }
    void addToPut(sc_sensitive& s) override { add_to(s, true, false); }
    void addToPut(sc_sensitive* s, sc_process_handle* p) override {
            add_to(s, p, true, false); }
    void addToGet(sc_sensitive& s) override { add_to(s, false, true); }
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {
            add_to(s, p, false, true); }
    void addPeekTo(sc_sensitive& s) override {add_to(s, false, false); }
    
    inline void print(::std::ostream& os) const override
    {
        os << "sct_buffer " << name();
        
        if (element_num != 0) {
            os << " ( ";
            unsigned popIndx = pop_indx;
            for (unsigned i = 0; i != element_num; ++i) {
                os << buffer[popIndx] << " ";
                popIndx = (popIndx == LENGTH-1) ? 0 : popIndx+1;
            }
            os << ")";
        } else {
            os << " is empty";
        }
        os << ::std::endl;
    }

    sct_buffer_put<T, LENGTH, TRAITS> PUT{this};
    sct_buffer_get<T, LENGTH, TRAITS> GET{this};
    sct_buffer_peek<T, LENGTH, TRAITS> PEEK{this};
};

#else

/// Cycle accurate implementation for synthesis
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS = SCT_CMN_TRAITS  /// Clock edge and reset level traits
>
using sct_buffer = sct_fifo<T, LENGTH, TRAITS, 0>;

#endif


} // namespace sct

//==============================================================================

#ifndef __SC_TOOL__

namespace sc_core {

template<class T, unsigned LENGTH, class TRAITS>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_buffer<T, LENGTH, TRAITS>& buffer )
{
    buffer.addTo(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS>
sc_sensitive& 
operator << ( sc_sensitive& s, sct::sct_buffer_put<T, LENGTH, TRAITS>& put )
{
    put.buf->addToPut(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS>
sc_sensitive& 
operator << ( sc_sensitive& s, sct::sct_buffer_get<T, LENGTH, TRAITS>& get )
{
    get.buf->addToGet(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_buffer_peek<T, LENGTH, TRAITS>& peek )
{
    peek.buf->addPeekTo(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS>
inline ::std::ostream& operator << (::std::ostream& os, 
                    const sct::sct_buffer<T, LENGTH, TRAITS>& buffer) 
{
    buffer.print(os);
    return os;
}

} // namespace sc_core
#endif

#endif /* SCT_BUFFER_H */

