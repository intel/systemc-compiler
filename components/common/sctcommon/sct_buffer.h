/******************************************************************************
 * Copyright (c) 2023-2026, Intel Corporation. All rights reserved.
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

#include "sct_prim_buffer.h"
#include "sct_static_log.h"
#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
#ifndef __SC_TOOL__

/// Approximately timed and cycle accurate implementation for simulation 
/// Buffer should use the same reset as thread(s) operates with it
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS = SCT_CMN_TRAITS /// Clock edge and reset level traits
>
class sct_buffer_impl : 
    public sc_prim_channel,  
    public sct_fifo_if<T>
{
  public:
    static const unsigned ELEM_NUM_WIDTH = sct_nbits<LENGTH>; 
      
    sc_in_clk       clk;
    sc_in<bool>     nrst;
       
    explicit sct_buffer_impl(const char* name, 
                             bool sync_valid = 0, bool sync_ready = 0,
                             bool use_elem_num = 0, bool init_buffer = 0) :
        clk(std::string(std::string(name)+"_clk").c_str()),   // To avoid warning
        nrst(std::string(std::string(name)+"_nrst").c_str()), // To avoid warning
    #ifdef SCT_DEBUG
        sc_prim_channel(std::string(std::string(name)+"_chan").c_str()), 
        attached_put(false), attached_get(false), 
    #else
        sc_prim_channel(name), 
    #endif
        event(std::string(std::string(name)+"_event").c_str())
    {
        assert (LENGTH > 0);
        // This implementation equals to async valid/ready FIFO
        assert (!sync_valid && !sync_ready && 
                "No sync valid/ready allowed for Buffer");
        
    #ifdef SCT_DEBUG
        dbg_mod = new DbgMod(sc_module_name(name));
        dbg_mod->clk(clk);
        dbg_mod->nrst(nrst);
    #endif
    }

  public:
    bool ready() const override {
        return (element_num != LENGTH);
    }
    
    bool request() const override {
        return (element_num != 0);
    }
    
    /// Call in THREAD reset section
    void reset_get() override {
    #ifdef SCT_DEBUG
        dbg_mod->get_req = 0;
    #endif
        pop_indx = 0;
        get_req = 0; 
        request_update();
    }
    
    /// Call in THREAD reset section
    void reset_put() override {
    #ifdef SCT_DEBUG
        dbg_mod->put_req = 0;
        dbg_mod->data_in = T{};
    #endif
        push_indx = 0;
        put_req = 0; 
        element_num = 0;
        buffer[0] = T{};
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
        return buffer[pop_indx];
    }
    
  protected:    
    inline void doGet() {
        get_req = 1;
        request_update();
    }
    
    inline void doPut(const T& data) {
        put_req = 1;
        buffer[push_indx] = data; 
        request_update();
    }

  public:     
    /// \return current request data, if no request last data returned
    T get() override {
        if (element_num != 0) { 
        #ifdef SCT_DEBUG
            dbg_mod->get_req = !dbg_mod->get_req;
        #endif
            doGet(); 
        }
        return buffer[pop_indx];
    }
    
    /// \return true if request is valid and enable is true
    bool get(T& data, bool enable = true) override {
        data = buffer[pop_indx];
        if (enable && element_num != 0) { 
        #ifdef SCT_DEBUG
            dbg_mod->get_req = !dbg_mod->get_req;
        #endif
            doGet(); 
            return true;
        } else {
            return false;
        }
    }

    T b_get() override {
        while (element_num == 0) sc_prim_channel::wait();
    #ifdef SCT_DEBUG
        dbg_mod->get_req = !dbg_mod->get_req;
    #endif
        doGet();
        return buffer[pop_indx];
    }
    
    bool put(const T& data) override {
    #ifdef SCT_DEBUG
        dbg_mod->data_in = data;
    #endif
        if (element_num != LENGTH) {
        #ifdef SCT_DEBUG
            dbg_mod->put_req = !dbg_mod->put_req;
        #endif
            doPut(data);
            return true;
        } else {
            return false;
        }
    }

    bool put(const T& data, sc_uint<1> mask) override {
    #ifdef SCT_DEBUG
        dbg_mod->data_in = data;
    #endif
        if (mask && element_num != LENGTH) {
        #ifdef SCT_DEBUG
            dbg_mod->put_req = !dbg_mod->put_req;
        #endif
            doPut(data);
            return true;
        } else {
            return false;
        }
    }
    
    void b_put(const T& data) override {
    #ifdef SCT_DEBUG
        dbg_mod->data_in = data;
    #endif
        while (element_num == LENGTH) sc_prim_channel::wait();
    #ifdef SCT_DEBUG
        dbg_mod->put_req = !dbg_mod->put_req;
    #endif
        doPut(data);
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
    
#ifdef SCT_DEBUG
  public:    
    // Debugging module with signals to be visible in third party simulation tools
    struct DbgMod : public sc_module {
        using ElemNum_t = sc_uint<ELEM_NUM_WIDTH>;
        
        sc_in_clk       clk{"clk"};
        sc_in<bool>     nrst{"nrst"};
        
        sc_signal<bool>         put_req{"put_req"};
        sc_signal<bool>         get_req{"get_req"};
        sc_signal<bool>         ready_push{"ready_push"};
        sc_signal<bool>         out_valid{"out_valid"};
        sc_signal<T>            data_out{"data_out"};
        sc_signal<T>            data_in{"data_in"};
        sc_signal<ElemNum_t>    element_num{"element_num"};
        
        explicit DbgMod(const sc_module_name& name) : sc_module(name) 
        {}
    };
    
    DbgMod* dbg_mod = nullptr;
#endif  
    
  protected:
    /// Index of element that will be poped
    unsigned short  pop_indx    : 16 = 0;
    /// Index where pushed element will be stored
    unsigned short  push_indx   : 16 = 0;
    /// Number of elements
    unsigned short  element_num : 16 = 0;
    
    bool    put_req   : 8 = false;
    bool    get_req   : 8 = false;
    
#ifdef SCT_DEBUG
    bool    attached_put : 8;
    bool    attached_get : 8;
#endif
    
    sc_time         clk_period = SC_ZERO_TIME;

    T               buffer[LENGTH] = {};
    
    /// Event for put, get and peek thread processes notification
    sc_event        event;

    void update() {
        if (get_req) {
            pop_indx = (pop_indx == LENGTH-1) ? 0 : (pop_indx+1);
            element_num -= 1;
            get_req  = 0;
        }
        if (put_req) {
            push_indx = (push_indx == LENGTH-1) ? 0 : (push_indx+1);
            element_num += 1;
            put_req = 0;
        }

    #ifdef SCT_DEBUG
        dbg_mod->element_num = element_num;
        dbg_mod->ready_push  = element_num != LENGTH;
        dbg_mod->out_valid   = element_num != 0;
        dbg_mod->data_out    = buffer[pop_indx];
    #endif

        if constexpr (SCT_CMN_TLM_MODE) event.notify(clk_period);
    }
    
    void before_end_of_elaboration() override {
        if (clk.bind_count() != 1 || nrst.bind_count() != 1) {
            cout << "\nBuffer " << sc_prim_channel::name() 
                 << " clock/reset inputs are not bound or multiple bound" << endl;
            assert (false);
        }
    }
    
    /// Use end_of_elaboration as @get_clk_period used inside
    void end_of_elaboration() override {
    #ifdef SCT_DEBUG
        if (!attached_put || !attached_get) {
            cout << "\nBuffer " << name() 
                 << " is not fully attached to process(es)" << endl;
            assert (false);
        }
    #endif
        
        if constexpr (SCT_CMN_TLM_MODE) { clk_period = get_clk_period(&clk); }
        
        // That is OK as this buffer is for simulation only (not for synthesis)
        PUT.buf = nullptr;
        GET.buf = nullptr;
        PEEK.buf = nullptr;
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
    
    // Reset is added to sensitivity in TLM mode as soon as there is no resetProc()
    // as in @sct_fifo
    void add_to(sc_sensitive& s, bool attachedPut, bool attachedGet) {
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            if constexpr (SCT_CMN_TLM_MODE) { 
                s << event << nrst; 
            } else {
                if (TRAITS::CLOCK == 2) s << clk; 
                else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            }
        } else {
            auto procKind = sc_get_current_process_handle().proc_kind();
            if (procKind != SC_THREAD_PROC_ && procKind != SC_CTHREAD_PROC_) {
                cout << "Buffer cannot be used in combinational method process" << endl;
                assert (false);
            }
            if constexpr (SCT_CMN_TLM_MODE) {
                if (procKind != SC_CTHREAD_PROC_) { s << event << nrst; }
            } else {
                if (TRAITS::CLOCK == 2) s << clk; 
                else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            }
        }
    #ifdef SCT_DEBUG    
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
    #endif
    }
    
    void add_to(sc_sensitive* s, sc_process_handle* p, bool attachedPut, 
                bool attachedGet) {
        if (sct_seq_proc_handle == *p) {
            // Sequential method
            if constexpr (SCT_CMN_TLM_MODE) { 
                *s << *p << event << nrst; 
            } else {
                if (TRAITS::CLOCK == 2) *s << *p << clk; 
                else *s << *p << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            }
        } else {
            auto procKind = p->proc_kind();
            if (procKind != SC_THREAD_PROC_ && procKind != SC_CTHREAD_PROC_) {
                cout << "Buffer cannot be used in combinational method process" << endl;
                assert (false);
            }
            if constexpr (SCT_CMN_TLM_MODE) { 
                if (procKind != SC_CTHREAD_PROC_) { *s << *p << event << nrst; }
            } else {
                if (TRAITS::CLOCK == 2) *s << *p << clk; 
                else *s << *p << (TRAITS::CLOCK ? clk.pos() : clk.neg());
            }
        }
    #ifdef SCT_DEBUG    
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
    #endif
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
    
    
    void trace(sc_trace_file* tf) const override {
    #ifdef SCT_DEBUG
        const std::string& bufName = name();
        sc_trace(tf, dbg_mod->ready_push, bufName + "_ready");
        sc_trace(tf, dbg_mod->put_req, bufName + "_put");
        sc_trace(tf, dbg_mod->data_in, bufName + "_data_in");
        sc_trace(tf, dbg_mod->out_valid, bufName + "_request");
        sc_trace(tf, dbg_mod->get_req, bufName + "_get");
        sc_trace(tf, dbg_mod->data_out, bufName + "_data_out");
        sc_trace(tf, dbg_mod->element_num, bufName + "_element_num");
    #endif
    }
   
    inline void print(::std::ostream& os) const override
    {
        os << "sct_buffer [ " << element_num << " of " << LENGTH << " ] : " << name();
        
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

//==============================================================================

/// Cycle accurate implementation for simulation
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS            /// Clock edge and reset level traits
>
class sct_buffer<T, LENGTH, TRAITS, 0> : public sct_buffer_impl<T, LENGTH, TRAITS>
{
  public:
    using base_class = sct_buffer_impl<T, LENGTH, TRAITS>;
    
    explicit sct_buffer(const char* name, 
               bool sync_valid = 0, bool sync_ready = 0,
               bool use_elem_num = 0, bool init_buffer = 0) :
    base_class(name, sync_valid, sync_ready, use_elem_num, init_buffer)
    {}
};

//==============================================================================

/// Approximately timed implementation
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS            /// Clock edge and reset level traits
>
class sct_buffer<T, LENGTH, TRAITS, 1> : public sct_buffer_impl<T, LENGTH, TRAITS>
{
  public:
    using base_class = sct_buffer_impl<T, LENGTH, TRAITS>;
    
    explicit sct_buffer(const char* name,
               bool sync_valid = 0, bool sync_ready = 0,
               bool use_elem_num = 0, bool init_buffer = 0) :
    base_class(name, sync_valid, sync_ready, use_elem_num, init_buffer)
    {}
};

//==============================================================================

/// Loosely timed implementation
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS            /// Clock edge and reset level traits
>
class sct_buffer<T, LENGTH, TRAITS, 2> : public sct_prim_buffer<T>
{
  public:
    using base_class = sct_prim_buffer<T>;
    
    /// \param multi_put -- multiple put in one DC allowed
    /// \param multi_get -- multiple get in one DC allowed
    explicit sct_buffer(const char* name, 
                        bool sync_valid = 0, bool sync_ready = 0,
                        bool use_elem_num = 0, bool init_buffer = 0,
                        bool multi_put = 0, bool multi_get = 0) :
    base_class(name, LENGTH, multi_put, multi_get)
    {}

  protected:    
    void end_of_elaboration() override {
        base_class::end_of_elaboration();

        PUT.buf = nullptr;
        GET.buf = nullptr;
        PEEK.buf = nullptr;
    }

  public:    
    sct_buffer_put<T, LENGTH, TRAITS> PUT{this};
    sct_buffer_get<T, LENGTH, TRAITS> GET{this};
    sct_buffer_peek<T, LENGTH, TRAITS> PEEK{this};
};

#else // __SC_TOOL__

//==============================================================================

/// Cycle accurate implementation for synthesis
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS = SCT_CMN_TRAITS  /// Clock edge and reset level traits
>
using sct_buffer = sct_fifo<T, LENGTH, TRAITS, 0>;

#endif // __SC_TOOL__


} // namespace sct

//==============================================================================

#ifndef __SC_TOOL__

namespace sc_core {

template<class T, unsigned LENGTH, class TRAITS, unsigned TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_buffer<T, LENGTH, TRAITS, TLM_MODE>& buffer )
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

template<class T, unsigned LENGTH, class TRAITS, unsigned TLM_MODE>
inline ::std::ostream& operator << (::std::ostream& os, 
            const sct::sct_buffer<T, LENGTH, TRAITS, TLM_MODE>& buffer) 
{
    buffer.print(os);
    return os;
}

} // namespace sc_core
#endif

#endif /* SCT_BUFFER_H */

