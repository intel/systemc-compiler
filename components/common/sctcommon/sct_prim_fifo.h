/******************************************************************************
 * Copyright (c) 2021-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Primitive FIFO channel.
 * 
 * Used as base channel in approximate time mode for Target, Initiator and FIFO.
 * Not intended to be used in user code.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_PRIM_FIFO_H
#define SCT_PRIM_FIFO_H

#include "sct_prim_signal.h"
#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
//#define NOTIFY_DEBUG 1
    
/// Primitive FIFO module
template <class T>
class sct_prim_fifo : 
    public sct_fifo_if<T>,
    public sc_module
{
  public:
    SC_HAS_PROCESS(sct_prim_fifo);
    
    explicit sct_prim_fifo(const sc_module_name& name, unsigned size,
                           bool sync_valid_ = 0, bool sync_ready_ = 0,
                           bool use_elem_num = 0) :
        sc_module(name), 
        sync_valid(sync_valid_), sync_ready(sync_ready_),
        USE_ELEM_NUM(use_elem_num),
        fifoSize(size), 
        buffer(size, T{}),
        put_event(std::string(std::string(name)+"_put_event").c_str()),
        get_event(std::string(std::string(name)+"_get_event").c_str()),
        update_event(std::string(std::string(name)+"_update_event").c_str())
    {
        assert (size != 0 && "sct_prim_fifo size zero not supported");
        //cout << "sct_prim_fifo " << this->name() << " " << sync_valid << sync_ready << endl;
        
        has_reset = 0;
        get_req   = 0;
        put_req   = 0;
        put_req_d = 0;
        get_req_d = 0;
        put_data  = T{};
        element_num_d = 0;
        
        SC_METHOD(updateProc); 
        dont_initialize();
        sensitive << update_event;
        
    #if defined(DEBUG_SYSTEMC) || defined(EXTR_SIM_DEBUG)
        SC_METHOD(debugProc); 
        sensitive << get_req << get_req_d << put_req << put_req_d 
                  << put_data << get_data << get_data_next << element_num_d;
    #endif
    }
        
    explicit sct_prim_fifo() {
        sct_prim_fifo("sct_prim_fifo", 2);
    }

    sct_prim_fifo(const sct_prim_fifo<T>&) = delete;
    sct_prim_fifo& operator = (const sct_prim_fifo<T>&) = delete;
    
  protected:  
    bool sync_valid;  
    bool sync_ready; 
    const bool USE_ELEM_NUM;
      
    bool cthread_put  = false;
    bool cthread_get  = false;
    bool cthread_peek = false;
    
    /// FIFO used for Target/Initiator simulation 
    bool targ_init      = false;
    bool targ_init_sync = false;
    
    sc_in_clk*  clk_in  = nullptr;
    sc_time     clk_period = SC_ZERO_TIME;

    sc_time GET_TIME  = SC_ZERO_TIME;
    sc_time PUT_TIME  = SC_ZERO_TIME;
    sc_time PEEK_TIME = SC_ZERO_TIME;
    
    sct_prim_signal<bool> has_reset{"has_reset"};   
    sct_prim_signal<bool> put_req{"put_req"};  
    sct_prim_signal<bool> get_req{"get_req"};        
    sct_prim_signal<T>    put_data{"put_data"};      

    sc_signal<bool> put_req_d{"put_req_d"};     // used if get done in THREAD
    sc_signal<bool> get_req_d{"get_req_d"};     // used if get done in THREAD
    sc_signal<unsigned> element_num_d{"element_num_d"};
    sc_signal<T>    get_data{"get_data"};       
    sc_signal<T>    get_data_next{"get_data_next"};       

    sc_signal<bool> put_done{"put_done"}; // Auxiliary signal to meet RTL timing
    
    bool            hasReset= 0;        
    unsigned        getIndx = 0;        // Index of element that will be get
    unsigned        putIndx = 0;        // Index where element will be put
    unsigned        elemNum = 0;        // Number of elements in buffer
    unsigned        fifoSize;           // FIFO size

#ifdef SCT_TLM_DEBUG
    sc_time         putTime;            // for debug purpose to avoid multiple put
    sc_time         getTime;            // for debug purpose to avoid multiple get
#endif
    
    std::vector<T>  buffer;
    
    sc_event        put_event;
    sc_event        get_event;
    sc_event*       peek_event = nullptr;
    sc_event        update_event;

    /// Channel update, run at DC 0 
    void updateProc()
    {
        bool doPut = put_req != put_req_d;
        bool doGet = get_req != get_req_d;
        
        if (has_reset != hasReset) {
            #ifdef NOTIFY_DEBUG 
            cout << sc_time_stamp() << " " << sc_delta_count() << " updateProc "<< name() << " reset " << endl;
            #endif
            getIndx = 0;
            putIndx = 0;
            elemNum = 0;
            // Only zero element is visible, so do not need to clear others
            buffer[0] = T{};
            
        } else if (doPut || doGet) {
            if (doPut) {
                buffer[putIndx] = put_data;
                putIndx = putIndx == fifoSize-1 ? 0 : putIndx+1;
                sc_assert (elemNum < fifoSize || doGet);
                elemNum++;
            }
            if (doGet) {
                getIndx = getIndx == fifoSize-1 ? 0 : getIndx+1;
                sc_assert (elemNum > 0);
                elemNum--;
            }
            
            // Notify put/get processes
            // If async ready/valid process notified in directly get()/put()
            if (!cthread_put || (doGet && sync_ready)) {
                put_event.notify(PUT_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED update put " << name() << endl; 
                #endif
            }
            if (!cthread_get || (doPut && sync_valid)) {
                get_event.notify(GET_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED update get " << name() << endl; 
                #endif
            }
            if (peek_event && (!cthread_peek || (doPut && sync_valid))) {
                peek_event->notify(PEEK_TIME);
            }
            
            #ifdef NOTIFY_DEBUG 
            //if (std::string(name()).find("tresp") != std::string::npos) 
            cout << sc_time_stamp() << " " << sc_delta_count() << " updateProc "
                 << name() << " doPut|doGet " << doPut << doGet << " elemNum" << elemNum << endl;
            #endif
        }
        
        if (targ_init_sync) {
            // To provide de-assert for @put_done
            if (!cthread_put && (doPut || doGet)) update_event.notify(clk_period);
            put_done = doPut || doGet;
        }
        
        element_num_d = elemNum;
        get_data = buffer[getIndx];
        get_data_next = buffer[getIndx == fifoSize-1 ? 0 : getIndx+1];
        
        hasReset = has_reset;
        put_req_d = put_req;
        get_req_d = get_req;        
    }
    
    /// Internal ready/valid/data functions equivalent to RTL logic
    inline bool putReady() const {
        // For Target/Initiator mode get required at almost full FIFO to allow put
//        cout << sc_time_stamp() << " " << sc_delta_count()
//             << " targ_init_sync " << targ_init_sync
//             << " put_done " << put_done
//             << " get_req == get_req_d " << (get_req == get_req_d)
//             << " put_req == put_req_d " << (put_req == put_req_d)
//             << " element_num_d " << element_num_d.read() << endl;
        
        if (targ_init_sync && !cthread_put) {
            if (cthread_get && get_req != get_req_d) { return true; }
            if (!put_done && element_num_d.read() != 0) { return false; }
        }
        
        return ((element_num_d.read() != fifoSize &&
                 (!cthread_put || 
                  (targ_init ? get_req != get_req_d : put_req == put_req_d) ||
                  element_num_d.read() != fifoSize-1)) || 
                (!sync_ready && get_req != get_req_d));
    }

    inline bool outValid() const {
        return ((element_num_d.read() != 0 && 
                 (!cthread_get || get_req == get_req_d || 
                  element_num_d.read() != 1)) || 
                (!sync_valid && put_req != put_req_d));
    }
    
    inline T getData() const {
        return ((cthread_get && get_req != get_req_d) ?
                (element_num_d.read() > 1 ? get_data_next.read() : put_data.read()) :
                (element_num_d.read() > 0 ? get_data.read() : put_data.read()));
    }
    
    void end_of_elaboration() override {
        assert (clk_in && "clk_in is nullptr");
        clk_period = get_clk_period(clk_in);
        
        GET_TIME  = cthread_get ? clk_period : SC_ZERO_TIME;
        PUT_TIME  = cthread_put ? clk_period : SC_ZERO_TIME;
        PEEK_TIME = cthread_peek ? clk_period : SC_ZERO_TIME;
                
        //cout << "PUT_TIME " << PUT_TIME << " GET_TIME " << GET_TIME << " PEEK_TIME " << PEEK_TIME << endl;
            
        // Do not check @sync_valid and @sync_ready as prim_fifo could be used
        // as part of target/initiator in non-process context
    }
    
  public:    
    /// Reset FIFO core, called by reset process in target/initiator/fifo 
    void reset_core(bool reset) {
        //cout << sc_time_stamp() << " " << sc_delta_count() << " reset_core " << name()
        //     << " reset " << reset << " cthread " << cthread_put << cthread_get 
        //     << " sync " << sync_valid << sync_ready << endl;
        if (reset) {
            // Reset is active (reset entry), clear FIFO
            has_reset = !has_reset;
            update_event.notify(clk_period);
        } else {
            // Notify put/get processes at reset exit 
            put_event.notify(PUT_TIME);
            get_event.notify(GET_TIME);
            if (peek_event) peek_event->notify(PEEK_TIME);
        }
    }
    
    void reset_put() override {
        // Can be called in method process or in reset section of thread process
        // In method process that does initialization of @put_req/@put_data
        if (!cthread_put) {
            put_req  = put_req_d;
            put_data = T{};
        }
        
        if (!cthread_put && !sync_valid && put_req != put_req_d) {
            if (!cthread_get) {
                get_event.notify(GET_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED reset_put() " << name() << endl; 
                #endif
            }
            if (peek_event && !cthread_peek) peek_event->notify(PEEK_TIME);
        }
    }
    
    void clear_put() override {
        put_req  = cthread_put ? put_req : put_req_d;
        put_data = T{};

        // Clear put notifies get process if both are methods
        if (!cthread_put && !sync_valid && put_req != put_req_d) {
            if (!cthread_get) {
                get_event.notify(GET_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED clear_put() " << name() << endl; 
                #endif
            }
            if (peek_event && !cthread_peek) peek_event->notify(PEEK_TIME);
        }
    }

    void reset_get() override {
        // Can be called in method process or in reset section of thread process
        // In method process that does initialization of @get_req
        if (!cthread_get) get_req = get_req_d;
        
        if (!cthread_get && !cthread_put && !sync_ready && get_req != get_req_d) {
            put_event.notify(PUT_TIME);
            #ifdef NOTIFY_DEBUG 
            cout << "NOTIFIED reset_get() " << name() << endl; 
            #endif
        }
    }
    
    void clear_get() override {
        get_req = cthread_get ? get_req : get_req_d;
        
        // Clear get notifies put process if both are methods
        if (!cthread_get && !cthread_put && !sync_ready && get_req != get_req_d) {
            put_event.notify(PUT_TIME);
            #ifdef NOTIFY_DEBUG 
            cout << "NOTIFIED clear_get() " << name() << endl; 
            #endif
        }
    }

    bool ready() const override {
        return putReady();
    }
    
    bool request() const override {
//        cout << sc_time_stamp() << " " << sc_delta_count() << " request " << name()  
//             << " elemNum " << element_num_d << endl;
//        cout << sc_time_stamp() << " " << sc_delta_count() << " request " << name()  
//             << " " << outValid() << " element_num_d " << element_num_d
//             << " put_req " << (put_req != put_req_d) << endl;
        return outValid();
    }
    
    bool put(const T& data) override 
    {
        if (putReady()) {
            put_req = cthread_put ? !put_req : !put_req_d;
            put_data = data;
            update_event.notify(clk_period);
            
            // If @USE_ELEM_NUM every put notifies get process
            // If put() in thread and get makes FIFO empty, notifies get process
            // as this put() allows one more get
            if (!sync_valid && (USE_ELEM_NUM || element_num_d == 0 ||
                (cthread_put && get_req != get_req_d && element_num_d == 1)))
            {
                get_event.notify(GET_TIME);
                if (peek_event) peek_event->notify(PEEK_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED put() " << name() << endl; 
                #endif
            }
            // Notify thread itself to allow next put
            if (cthread_put) put_event.notify(PUT_TIME);

        #ifdef SCT_TLM_DEBUG
            if (cthread_put && putTime == sc_time_stamp()) {
                cout << sc_time_stamp() << " " << name() << ", ERROR: multiple put\n" << endl;
                sc_assert (false);
            }
            putTime = sc_time_stamp();
        #endif
            return true;
        }
        put_req = cthread_put ? put_req : put_req_d;
        return false;
    }

    bool put(const T& data, sc_uint<1> mask) override 
    {
        if (mask && putReady()) {
            put_req = cthread_put ? !put_req : !put_req_d;
            put_data = data;
            update_event.notify(clk_period);
            
            if (!sync_valid && (USE_ELEM_NUM || element_num_d == 0 ||
                (cthread_put && get_req != get_req_d && element_num_d == 1))) 
            {
                get_event.notify(GET_TIME);
                if (peek_event) peek_event->notify(PEEK_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED put() " << name() << endl; 
                #endif
            }
            // Notify thread itself to allow next put
            if (cthread_put) put_event.notify(PUT_TIME);
            
        #ifdef SCT_TLM_DEBUG
            if (cthread_put && putTime == sc_time_stamp()) {
                cout << sc_time_stamp() << " " << name() << ", ERROR: multiple put\n" << endl;
                sc_assert (false);
            }
            putTime = sc_time_stamp();
        #endif
            return true;
        }
        put_req = cthread_put ? put_req : put_req_d;
        return false;
    }
    
    T peek() const override {
        return getData();
    }
    
    T get() override 
    {
        if (outValid()) {
            get_req = cthread_get ? !get_req : !get_req_d;
            update_event.notify(clk_period);
            //cout << "   get() " << name() << " element_num_d " << element_num_d << endl;
            
            // If @USE_ELEM_NUM every get notifies put process
            // If get() in thread and put makes FIFO full, notifies put process
            // as this get() allows one more put
            // For Target/Initiator mode notify at almost full FIFO to allow put
            
            if (!sync_ready && (USE_ELEM_NUM || element_num_d == fifoSize ||
                (cthread_get && put_req != put_req_d && element_num_d == fifoSize-1) ||
                (targ_init && cthread_put && element_num_d == fifoSize-1) ||
                (targ_init_sync && !cthread_put && cthread_get) ))
            {
                put_event.notify(PUT_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED get() " << name() << endl; 
                #endif
            }
            // Notify thread itself to allow next get
            if (cthread_get) get_event.notify(GET_TIME);
            if (peek_event && cthread_peek) peek_event->notify(PEEK_TIME);

        #ifdef SCT_TLM_DEBUG
            if (cthread_get && getTime == sc_time_stamp()) {
                cout << sc_time_stamp() << " " << name() << ", ERROR: multiple get\n" << endl;
                sc_assert (false);
            }
            getTime = sc_time_stamp();
        #endif
        } else {
            get_req = cthread_get ? get_req : get_req_d;
        }
        return getData();
    }
      
    bool get(T& data, bool enable = true) override 
    {
        data = getData();
        
        if (enable && outValid()) {
            get_req = cthread_get ? !get_req : !get_req_d;
            update_event.notify(clk_period);
            
            if (!sync_ready && (USE_ELEM_NUM || element_num_d == fifoSize ||
                (cthread_get && put_req != put_req_d && element_num_d == fifoSize-1))) 
            {
                put_event.notify(PUT_TIME);
                #ifdef NOTIFY_DEBUG 
                cout << "NOTIFIED get() " << name() << endl; 
                #endif
            }
            // Notify thread itself to allow next get
            if (cthread_get) get_event.notify(GET_TIME);
            if (peek_event && cthread_peek) peek_event->notify(PEEK_TIME);

        #ifdef SCT_TLM_DEBUG
            if (cthread_get && getTime == sc_time_stamp()) {
                cout << sc_time_stamp() << " " << name() << ", ERROR: multiple get\n" << endl;
                sc_assert (false);
            }
            getTime = sc_time_stamp();
        #endif
            return true;
        }
        
        get_req = cthread_get ? get_req : get_req_d;
        return false;
    }
    
    void b_put(const T& data) override {
        sc_assert (false);
    }
    
    T b_get() override {
        sc_assert (false);
        return T{};
    }
    
    /// Number of elements in FIFO after last/current edge
    unsigned elem_num() const override {
        if (!USE_ELEM_NUM) {
            cout << "\nFIFO " << name() << " should have use_elem_num enabled" << endl;
            sc_assert (false);
        }
        
        if (sct_is_method_proc()) {
            return element_num_d.read();
        } else {
            bool doPut = put_req != put_req_d;
            bool doGet = get_req != get_req_d;
            
            if (doGet && !doPut) {
                return (element_num_d.read()-1);
            } else 
            if (!doGet && doPut) {
                return (element_num_d.read()+1);
            } else {
                return element_num_d.read();
            }
        }
    }
    
    /// FIFO has (size()-N) elements or more
    bool almost_full(const unsigned& N = 0) const override {
        sc_assert (N <= fifoSize && 
                "almost_full() parameter cannot be great than FIFO size");
        return (elem_num() >= fifoSize-N);
    }
    /// FIFO has N elements or less
    bool almost_empty(const unsigned& N = 0) const override {
        sc_assert (N <= fifoSize && 
                "almost_empty() parameter cannot be great than FIFO size");
        return (elem_num() <= N);
    }

    /// Get FIFO current size
    unsigned size() const override {
        return fifoSize;
    }
    
    /// Resize FIFO
    void resize(unsigned new_size) {
        sc_assert (new_size > 0);
        fifoSize = new_size;
        buffer.resize(new_size, T{});
    }
    
    /// Set synchronous/asynchronous for valid and ready
    void setSync(bool syncValid, bool syncReady) {
        sync_valid = syncValid;
        sync_ready = syncReady;
    }
    
    /// Set the FIFO used for Target/Initiator simulation
    void setTargInit(bool sync = false) {
        if (sync) targ_init_sync = true; 
        else targ_init = true;
    }
    
  public:
    template <typename RSTN_t>
    void clk_nrst(sc_in_clk& clk_in_, RSTN_t& nrst_in)  {
        clk_in = &clk_in_;
    }
    
    void clk_nrst(sc_in_clk& clk_in_, sc_in<bool>& nrst_in) override {
        clk_in = &clk_in_;
    }

    void addTo(sc_sensitive& s) override {
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread_put = true;
            cthread_get = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
            cthread_get = cthread_put;
        }
        
        if (procKind != SC_CTHREAD_PROC_) {
            s << put_event << get_event;
        }
    }
    
    void addTo(sc_sensitive* s, sc_process_handle* p) override {
        assert (false);
    }
    
    void addToPut(sc_sensitive& s) override {
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread_put = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (procKind != SC_CTHREAD_PROC_) {
            s << put_event;
        }
    }
    
    void addToPut(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        if (sct_seq_proc_handle == *p) {
            // Sequential method
            cthread_put = true;
            //cout << "SEQ METHOD " << name() << " " << p->name() << endl;
        } else {
            cthread_put = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (procKind != SC_CTHREAD_PROC_) {
            *s << *p << put_event;
        }
    }
    
    void addToGet(sc_sensitive& s) override {
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread_get = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            cthread_get = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (procKind != SC_CTHREAD_PROC_) {
            s << get_event;
        }
    }
    
    void addToGet(sc_sensitive* s, sc_process_handle* p) override {
        auto procKind = p->proc_kind();
        if (sct_seq_proc_handle == *p) {
            // Sequential method
            cthread_get = true;
            //cout << "SEQ METHOD " << name() << " " << p->name() << endl;
        } else {
            cthread_get = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (procKind != SC_CTHREAD_PROC_) {
            *s << *p << get_event;
        }
    }

    void addPeekTo(sc_sensitive& s) override {
        bool cthread;
        auto procKind = sc_get_current_process_handle().proc_kind();
        if (sct_seq_proc_handle == sc_get_current_process_handle()) {
            // Sequential method
            cthread = true;
            //cout << "SEQ METHOD " << sct_seq_proc_handle.name() << endl;
        } else {
            // Other processes
            cthread = procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_;
        }
        
        if (peek_event) {
            if (cthread_peek != cthread) {
                cout << "\nDouble addPeekTo() with different process kinds for : "
                     << name() << endl;
                assert (false);
            }
        } else {
            auto eventName = std::string(std::string(basename()))+"_peek_event";
            peek_event = new sc_event( eventName.c_str() );
            cthread_peek = cthread;
        }
        
        if (procKind != SC_CTHREAD_PROC_) {
            s << *peek_event;
        }
    }

    /// Get get/put process events
    const sc_event& event_get() const { 
        return get_event; 
    }
    
    const sc_event& event_put() const { 
        return put_event; 
    }
    
    const sc_event& default_event() const override {
        cout << "No default event for sct_prim_fifo " << name() << endl;
        assert (false); 
        return put_event;
    }
    
    inline void print(::std::ostream& os) const override
    {
        os << "sct_prim_fifo " << name();
        if (element_num_d.read() != 0) {
            os << " ( ";
            for (unsigned i = 0; i != element_num_d.read(); ++i) {
                os << buffer[i] << " ";
            }
            os << ")";
        } else {
            os << " is empty";
        }
        os << ::std::endl;
    }

    const char* kind() const override { 
        return "sct_prim_fifo"; 
    }
    
  public:
      
#if defined(DEBUG_SYSTEMC) || defined(EXTR_SIM_DEBUG)
    sc_signal<bool>    ready_push{"ready_push"};
    sc_signal<bool>    out_valid{"out_valid"};
    sc_signal<bool>    debug_put{"put"};
    sc_signal<bool>    debug_get{"get"};
    sc_signal<T>       data_out{"data_out"};
    sc_signal<T>       data_in{"data_in"};
    
    void trace(sc_trace_file* tf) const override {
        std::string fifoName = name();
        sc_trace(tf, ready_push, fifoName + "_ready");
        sc_trace(tf, debug_put, fifoName + "_put");
        sc_trace(tf, data_in, fifoName + "_data_in");
        sc_trace(tf, out_valid, fifoName + "_request");
        sc_trace(tf, debug_get, fifoName + "_get");
        sc_trace(tf, data_out, fifoName + "_data_out");
        sc_trace(tf, element_num_d, fifoName + "_element_num_d");
    }

    void debugProc() {
        ready_push  = putReady();
        out_valid   = outValid();
        debug_put   = put_req != put_req_d;
        debug_get   = get_req != get_req_d;
        data_out    = getData();
        data_in     = put_data.read();
    }
#endif
      
};

} // namespace sct

//==============================================================================

template <class T>
inline ::std::ostream& operator << (::std::ostream& os, 
                                    const sct::sct_prim_signal<T>& sig) 
{
    sig.print(os);
    return os;
}

template <class T>
inline ::std::ostream& operator << (::std::ostream& os, 
                                    const sct::sct_prim_fifo<T>& fifo) 
{
    fifo.print(os);
    return os;
}


#endif /* SCT_PRIM_FIFO_H */

