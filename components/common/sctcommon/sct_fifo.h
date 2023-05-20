/******************************************************************************
 * Copyright (c) 2021-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. FIFO channel.
 * 
 * The FIFO can be used for inter-process communication between processes 
 * in the same module and for storing requests inside one process. 
 * Also the FIFO could be used inside of Target as an extended buffer.
 * 
 * Author: Mikhail Moiseev, Leonid Azarenkov
 */   

#ifndef SCT_FIFO_H
#define SCT_FIFO_H

#include "sct_prim_fifo.h"
#include "sct_static_log.h"
#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
/// Cycle accurate implementation    
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS            /// Clock edge and reset level traits
>
class sct_fifo<T, LENGTH, TRAITS, 0> : 
    public sc_module,
    public sct_fifo_if<T>
{
   public:
    /// Assert @out_valid combinationally if false
    const bool SYNC_VALID;  
    /// Assert @ready_to_push combinationally if false
    const bool SYNC_READY;  
    /// Initialize FIFO slots in reset with zeros
    const bool INIT_BUFFER;  
       
    /// Number of bits in variables store index and length of FIFO
    static const unsigned INDX_WIDTH = sct_addrbits1<LENGTH>;
    using Indx_t = sc_uint<INDX_WIDTH>;
    static const unsigned ELEM_NUM_WIDTH = sct_nbits<LENGTH>; 
    using ElemNum_t = sc_uint<ELEM_NUM_WIDTH>;  
    
    sc_in_clk       clk{"clk"};
    sc_in<bool>     nrst{"nrst"};

    SC_HAS_PROCESS(sct_fifo);

    /// \param sync_val     -- request path has synchronous register 
    /// \param sync_ready   -- Response path has synchronous register  
    /// \param use_elem_num -- Element number/Almost full or empty used 
    /// \param init_buffer  -- Initialize all buffer elements with zeros in reset
    ///                        First element to get is always initialized to zero id 
    explicit sct_fifo(const sc_module_name& name, 
                      bool sync_valid = 0, bool sync_ready = 0,
                      bool use_elem_num = 0,
                      bool init_buffer = 0) :
        sc_module(name), 
        SYNC_VALID(sync_valid), SYNC_READY(sync_ready), 
        INIT_BUFFER(init_buffer)
    {
        static_assert (LENGTH > 0);
        //cout << "RTL FIFO " << name << " SYNC " << SYNC_VALID << SYNC_READY << endl;
        
        SC_METHOD(asyncProc);
        sensitive << data_in << put_req << put_req_d << get_req << get_req_d 
                  << pop_indx << element_num_d;
        for (auto& i : buffer) sensitive << i;

        SCT_CTHREAD(syncProc, clk, TRAITS::CLOCK);
        async_reset_signal_is(nrst, TRAITS::RESET);
    }

public:
    bool ready() const override {
        return ready_push;
    }
    
    bool request() const override {
        return out_valid;
    }
    
    /// Call in METHOD everywhere and CTHREAD reset sections
    void reset_get() override {
        get_req = 0;
    }
    
    /// Call in METHOD everywhere and CTHREAD reset sections
    void reset_put() override {
        put_req = 0;
        data_in = T{};
    }
    
    /// Call both put and get resets if used from the same process
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
        } else {
            put_req = 0;
        }
        data_in = T{};
    }
    
    T peek() const override {
        return data_out.read();
    }
    
    /// \return current request data, if no request last data returned
    T get() override {
        if (out_valid) {
            get_req = cthread_get ? !get_req : 1;
        } else {
            if (!cthread_get) get_req = 0;
        }
        return data_out.read();
    }
    
    /// \return true if request is valid and enable is true
    bool get(T& data, bool enable = true) override {
        data = data_out.read();
        if (out_valid) {
            get_req = cthread_get ? (enable ? !get_req : get_req) : enable;
            return enable;
        } else {
            if (!cthread_get) get_req = 0;
            return false;
        }
    }

    T b_get() override {
        if (cthread_get) {
            while (!out_valid) wait();
            get_req = !get_req;
            return data_out.read();
            
        } else {
            cout << "No blocking get allowed in METHOD process" << endl;
            assert (false);
            return T{};
        } 
    }
    
    bool put(const T& data) override {
        // Assign input data as it does not store any value
        data_in = data;
        if (ready_push) {
            put_req = cthread_put ? !put_req : true;
            return true;
        } else {
            if (!cthread_put) put_req = 0;
            return false;
        }
    }
    bool put(const T& data, sc_uint<1> mask) override {
        // Assign input data as it does not store any value
        data_in = data;
        if (ready_push) {
            put_req = cthread_put ? (mask ? !put_req : put_req) : bool(mask);
            return mask;
        } else {
            if (!cthread_put) put_req = 0;
            return false;
        }
    }
    
    void b_put(const T& data) override {
        if (cthread_put) {
            // Assign input data as it does not store any value 
            data_in = data;
            while (!ready_push) wait();
            put_req = !put_req;
            
        } else {
            cout << "No blocking put allowed in METHOD process" << endl;
            assert (false);
        }
    }
    
    /// Maximal number of elements
    unsigned size() const override {
        return LENGTH;
    }
    
    /// Number of elements in FIFO after last/current clock edge
    unsigned elem_num() const override {
        return (sct_is_method_proc() ? element_num_d.read() : element_num.read());
    }
    
    /// FIFO has (size()-N) elements or more
    bool almost_full(const unsigned& N = 0) const override {
        assert (N <= LENGTH);
        
        if (sct_is_method_proc()) {
            return (element_num_d.read() >= LENGTH-N);
        } else {
            return (element_num.read() >= LENGTH-N);
        }
    }
    
    /// FIFO has N elements or less
    bool almost_empty(const unsigned& N = 0) const override {
        assert (N <= LENGTH);
        
        if (sct_is_method_proc()) {
            return (element_num_d.read() <= N);
        } else {
            return (element_num.read() <= N);
        }
    }
    
  protected:
    bool cthread_put = false;
    bool cthread_get = false;
    
    /// This FIFO attached to a processes
    bool attached_put = false;
    bool attached_get = false;
    
    /// FIFO buffer
    sc_vector<sc_signal<T>> buffer{"buffer", LENGTH};

    /// Push operation is performed without @enable checking for burst on core clock,
    /// @push may be asserted when @ready_to_push is high only
    sc_signal<bool>         put_req{"put_req"};
    sc_signal<bool>         put_req_d{"put_req_d"};
    /// @pop may be asserted whenever, pop operation is done when @pop && @out_valid
    sc_signal<bool>         get_req{"get_req"};
    sc_signal<bool>         get_req_d{"get_req_d"};
    /// Push/pop data 
    sc_signal<T>            data_in{"data_in"};
    sc_signal<T>            data_out{"data_out"};
    /// FIFO is ready to @push assert signal
    sc_signal<bool>         ready_push{"ready_push"};
    /// Output data is valid signal
    sc_signal<bool>         out_valid{"out_valid"};
    /// Index of element that will be poped
    sc_signal<Indx_t>       pop_indx{"pop_indx"};
    /// Index where pushed element will be stored
    sc_signal<Indx_t>       push_indx{"push_indx"};
    
    /// Number of elements
    sc_signal<ElemNum_t>    element_num{"element_num"};
    sc_signal<ElemNum_t>    element_num_d{"element_num_d"};
    sc_signal<bool>         not_empty_d{"not_empty_d"};

    void asyncProc()
    {
        bool outValid; 
        bool readyPush;
        Indx_t popIndx;
        const bool notEmpty   = element_num_d.read() != 0;
        const bool notOne     = element_num_d.read() != 1;
        const bool notFullOne = element_num_d.read() != LENGTH-1;
        const bool notFull    = element_num_d.read() != LENGTH;
        const bool push = cthread_put ? put_req != put_req_d : put_req;
        const bool pop  = cthread_get ? get_req != get_req_d : get_req;
        
        // Consider pop in CTHREAD performed at next clock edge 
        if (cthread_get && pop) {
            outValid = notEmpty && notOne;
            popIndx  = (pop_indx.read() == LENGTH-1) ? 0 : pop_indx.read()+1;
        } else {
            outValid = notEmpty;
            popIndx  = pop_indx;
        }
        not_empty_d = notEmpty;
        
        if (cthread_put && push) {
            readyPush = notFull && notFullOne;
        } else {
            readyPush = notFull;
        }
        
        if (!SYNC_VALID) {
            // Empty FIFO with push, input data taken by pop
            out_valid = push || outValid;
            if (outValid) data_out = buffer[popIndx]; 
            else data_out = data_in;
        } else {
            out_valid = outValid;
            data_out  = buffer[popIndx];
        }

        if (!SYNC_READY) {
            ready_push = pop || readyPush;
        } else {
            ready_push = readyPush;
        }
        
        element_num = element_num_d.read();
        if (pop && !push) {
            element_num = element_num_d.read()-1;
        } else 
        if (!pop && push) {
            element_num = element_num_d.read()+1;
        }
    }
    
    void syncProc()
    {
        pop_indx  = 0;
        push_indx = 0;
        
        if (cthread_put) put_req_d = 0;
        if (cthread_get) get_req_d = 0;
        element_num_d = 0;

        // Initialize zero cell to provide zero data before first push
        buffer[0] = T{};
        if (INIT_BUFFER) {
            for (unsigned i = 1; i < LENGTH; i++) {
                buffer[i] = T{};
            }
        }
        wait();

        while (true) {
            const bool push = cthread_put ? put_req != put_req_d : put_req;
            const bool pop  = cthread_get ? get_req != get_req_d : get_req;

            // No pop from empty buffer
            if (pop && not_empty_d) {
                pop_indx = (pop_indx.read() == LENGTH-1) ? 
                            0 : pop_indx.read()+1;
                //cout << sc_time_stamp() << " " << sc_delta_count() << " : pop done" << endl;
            }

            // No push to empty buffer if there is @pop in not @SYNC_VALID mode
            bool A = !SYNC_VALID && !not_empty_d && pop;
            if (push && !A) {
                buffer[push_indx.read()] = data_in;

                push_indx = (push_indx.read() == LENGTH-1) ? 
                            0 : push_indx.read()+1;
                //cout << sc_time_stamp() << " " << sc_delta_count() << " : push done" << endl;
            }

            if (cthread_put) put_req_d = put_req;
            if (cthread_get) get_req_d = get_req;
            element_num_d = element_num;
            //cout << sc_time_stamp() << " " << sc_delta_count() << " : FIFO elem_num " << element_num << endl;
            
            wait();
        }
    }
    
    void before_end_of_elaboration() override {
        if (!attached_put || !attached_get) {
            cout << "\nFIFO " << name() 
                 << " is not fully attached to process(es)" << endl;
            assert (false);
        }
        if (!cthread_put && !cthread_get && !SYNC_VALID && !SYNC_READY) {
            cout << "\nFIFO " << name() 
                 << " attached to method should have sync valid or sync ready" << endl;
            assert (false);
        }
        PUT.fifo = nullptr;
        GET.fifo = nullptr;
        PEEK.fifo = nullptr;
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
        auto procKind = sc_get_current_process_handle().proc_kind();
        cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        if (cthread_put) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << ready_push << element_num_d;
        }
        
        if (attached_put) {
            cout <<  "Double addToPut() for FIFO: " << name() << endl; 
            assert (false);
        }
        attached_put = true;
    }

    void addToPut(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        
        if (cthread_put) {
            if (TRAITS::CLOCK == 2) *s << *p << clk; 
            else *s << *p << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            *s << *p << ready_push << element_num_d;
        }

        if (attached_put) {
            cout <<  "Double addToPut() for FIFO: " << name() << endl; 
            assert (false);
        }
        attached_put = true;
    }  
    
    void addToGet(sc_sensitive& s) override {
        auto procKind = sc_get_current_process_handle().proc_kind();
        cthread_get = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        
        if (cthread_get) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << out_valid << data_out << element_num_d;
        }
        
        if (attached_get) {
            cout <<  "Double addToGet() for FIFO: " << name() << endl; 
            assert (false);
        }
        attached_get = true;
    }
    
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        cthread_get = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        
        if (cthread_get) {
            if (TRAITS::CLOCK == 2) *s << *p << clk; 
            else *s << *p << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            *s << *p << out_valid << data_out << element_num_d;
        }
        
        if (attached_get) {
            cout <<  "Double addToGet() for FIFO: " << name() << endl; 
            assert (false);
        }
        attached_get = true;
    }

    void addPeekTo(sc_sensitive& s) override {
        auto procKind = sc_get_current_process_handle().proc_kind();
        bool cthread_peek = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        if (cthread_peek) {
            if (TRAITS::CLOCK == 2) s << clk; 
            else s << (TRAITS::CLOCK ? clk.pos() : clk.neg());
        } else {
            s << out_valid << data_out << element_num_d;
        }
    }
    
    sct_fifo_put<T, LENGTH, TRAITS, false> PUT{this};
    sct_fifo_get<T, LENGTH, TRAITS, false> GET{this};
    sct_fifo_peek<T, LENGTH, TRAITS, false> PEEK{this};
};

//==============================================================================

/// Approximate time implementation
template <
    typename T,             /// Data type
    unsigned LENGTH,        /// Size (maximal number of elements)
    class TRAITS            /// Clock edge and reset level traits
>
class sct_fifo<T, LENGTH, TRAITS, 1> : 
    public sc_module,
    public sct_fifo_if<T>
{
  public:
    /// Assert @out_valid combinationally
    const bool SYNC_VALID;  
    /// Assert @ready_to_push combinationally
    const bool SYNC_READY;  

    sc_in<bool>     nrst{"nrst"};

    SC_HAS_PROCESS(sct_fifo);

    /// \param sync_val     -- request path has synchronous register 
    /// \param sync_ready   -- Response path has synchronous register  
    /// \param use_elem_num -- Element number/Almost full or empty used 
    /// \param init_buffer  -- Initialize all buffer elements with zeros in reset
    ///                        First element to get is always initialized to zero id 
    explicit sct_fifo(const sc_module_name& name, 
                      bool sync_valid = 0, bool sync_ready = 0,
                      bool use_elem_num = 0,
                      bool init_buffer = 0) :
        sc_module(name),
        SYNC_VALID(sync_valid), SYNC_READY(sync_ready),
        fifo("fifo", LENGTH > 1 ? LENGTH : 2, sync_valid, sync_ready, use_elem_num)
    {
        //cout << "TLM FIFO " << name << " LENGTH " << LENGTH << endl;
        static_assert (LENGTH > 0);

        SC_METHOD(resetProc);
        sensitive << nrst;
    }
    
  protected:
    // Minimum 2 slots required to have put&get at the same DC
    sct_prim_fifo<T>   fifo;
    
    // Clear FIFO buffer
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
        return fifo.size();
    }

    unsigned elem_num() const override {
        return fifo.elem_num();
    }
    bool almost_full(const unsigned& N = 0) const override {
        return fifo.almost_full(N);
    }
    bool almost_empty(const unsigned& N = 0) const override {
        return fifo.almost_empty(N);
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
        fifo.addToPut(s, p);
    }

    void addToGet(sc_sensitive& s) override {
        fifo.addToGet(s);
    }
    
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {    
        fifo.addToGet(s, p);
    }
    
    void addPeekTo(sc_sensitive& s) override {
        fifo.addPeekTo(s);
    }

    sct_fifo_put<T, LENGTH, TRAITS, true> PUT{this};
    sct_fifo_get<T, LENGTH, TRAITS, true> GET{this};
    sct_fifo_peek<T, LENGTH, TRAITS, true> PEEK{this};
};

} // namespace sct

//==============================================================================

namespace sc_core {

template<class T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_fifo<T, LENGTH, TRAITS, TLM_MODE>& fifo )
{
    fifo.addTo(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_fifo_put<T, LENGTH, TRAITS, TLM_MODE>& put )
{
    put.fifo->addToPut(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_fifo_get<T, LENGTH, TRAITS, TLM_MODE>& get )
{
    get.fifo->addToGet(s);
    return s;
}

template<class T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
sc_sensitive& 
operator << ( sc_sensitive& s, 
              sct::sct_fifo_peek<T, LENGTH, TRAITS, TLM_MODE>& get )
{
    get.fifo->addPeekTo(s);
    return s;
}

} // namespace sc_core


#endif  /* SCT_FIFO_H */
