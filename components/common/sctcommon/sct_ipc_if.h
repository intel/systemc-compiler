/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Interfaces, general templates and defines.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_IPC_IF_H
#define SCT_IPC_IF_H

#include <systemc.h>

namespace sct {
    
/// Simulation mode (could be specified in CMakeLists.txt): 
///  0 -- cycle accurate, used for debug and SystemVerilog RTL synthesis 
///  1 -- approximate time, used for fast simulation and integration into VP
#ifdef SCT_TLM_MODE
static const bool SCT_CMN_TLM_MODE = 1;
#else
static const bool SCT_CMN_TLM_MODE = 0;
#endif

/// Intel Compiler for SystemC requires cycle accurate mode
#ifdef SCT_TLM_MODE
   #ifdef __SC_TOOL__
   static_assert (false, "DO NOT run Intel Compiler for SystemC in TLM mode");
   #endif  
#endif

/// Clock and reset traits:
///  clock 0 -- negedge, 1 -- posedge, 2 -- both edges
///  reset 0 -- active low, 1 -- active high
struct SCT_POSEDGE_NEGRESET {
    static constexpr unsigned CLOCK = 1;
    static constexpr bool RESET = 0;
};
struct SCT_POSEDGE_POSRESET {
    static constexpr unsigned CLOCK = 1;
    static constexpr bool RESET = 1;
};
struct SCT_NEGEDGE_NEGRESET {
    static constexpr unsigned CLOCK = 0;
    static constexpr bool RESET = 0;
};
struct SCT_NEGEDGE_POSRESET {
    static constexpr unsigned CLOCK = 0;
    static constexpr bool RESET = 1;
};
struct SCT_BOTHEDGES_POSRESET {
    static constexpr unsigned CLOCK = 2;
    static constexpr bool RESET = 1;
};
struct SCT_BOTHEDGES_NEGRESET {
    static constexpr unsigned CLOCK = 2;
    static constexpr bool RESET = 0;
};

/// Default traits for project modules (could be specified in CMakeLists.txt)
/// It is possible to redefine for individual modules
#ifndef SCT_CMN_TRAITS
  #define SCT_CMN_TRAITS SCT_POSEDGE_NEGRESET
#endif

//=============================================================================

/// Put interface for Initiator and FIFO
template<class T, unsigned N = 1>
struct sct_put_if : virtual public sc_interface 
{
    static_assert (N <= 64, "N should be not great than 64");
    static const unsigned long long ALL_ENABLED = ~0ULL;
    
    /// Ready to put request, can be used in SCT_ASSERT 
    virtual bool ready() const = 0;
    /// Reset put request in THREAD reset section and METHOD everywhere
    virtual void reset_put() = 0;
    /// Clear put request in METHOD and THREAD after reset
    virtual void clear_put() = 0;
    /// Non-blocking put request into FIFO/Initiator if it is ready 
    /// \param data -- request data 
    /// \param mask -- enable put or specify targets in multi-cast put
    /// \return ready to request
    virtual bool put(const T& data) = 0;
    virtual bool put(const T& data, sc_uint<N> mask) = 0;
    /// May-blocking put, can be used in THREAD only
    /// \param data -- request data 
    virtual void b_put(const T& data) = 0;
    
    /// Add put related signal to process sensitivity, use operator << instead
    virtual void addTo(sc_sensitive& s) = 0;
    virtual void addTo(sc_sensitive* s, sc_process_handle* p) = 0;
};

/// Get interface for Target and FIFO
template<class T>
struct sct_get_if : virtual public sc_interface 
{
    /// Request can be taken, can be used in SCT_ASSERT 
    virtual bool request() const = 0;
    /// Reset get ready in THREAD reset section and METHOD everywhere
    virtual void reset_get() = 0;
    /// Clear get ready in METHOD or THREAD after reset
    virtual void clear_get() = 0;
    /// Return current request data, no change of the request 
    /// \return current request data, if no request returns last request data 
    virtual T peek() const = 0;
    /// Non-blocking get request data and remove the request 
    /// \return current request data, if no request returns last request data 
    virtual T get() = 0;
    /// Non-blocking get request data and remove the request if @enable is true
    /// \param data   -- current request data returned
    /// \param enable -- remove request 
    /// \return true if there is a request
    virtual bool get(T& data, bool enable = true) = 0;
    /// May-blocking get, can be used in THREAD only
    virtual T b_get() = 0;

    /// Add get/request related signal to process sensitivity, use operator << instead
    virtual void addTo(sc_sensitive& s) = 0;
    virtual void addTo(sc_sensitive* s, sc_process_handle* p) = 0;
    /// Add peek/request related signal to process sensitivity
    virtual void addPeekTo(sc_sensitive& s) = 0;
};

/// FIFO specific interface 
template<class T>
struct sct_fifo_if : public sct_put_if<T>, public sct_get_if<T>
{
    /// FIFO size (number of slots), can be used in SCT_ASSERT
    virtual unsigned size() const = 0;
    /// Number of elements in FIFO (less or equal to size), can be used in SCT_ASSERT 
    /// \return value updated last clock edge for METHOD, last DC for THREAD
    virtual unsigned elem_num() const = 0;
    /// FIFO has (size()-N) elements or more
    /// \return value updated last clock edge for METHOD, last DC for THREAD
    virtual bool almost_full(const unsigned& N = 0) const = 0;
    /// FIFO has N elements or less
    /// \return value updated last clock edge for METHOD, last DC for THREAD
    virtual bool almost_empty(const unsigned& N = 0) const = 0;
    
    /// Bind clock and reset to FIFO, required in sct_target
    virtual void clk_nrst(sc_in_clk& clk_in, sc_in<bool>& nrst_in) = 0;
    /// Add FIFO signals to process sensitivity
    virtual void addTo(sc_sensitive& s) = 0;
    virtual void addToPut(sc_sensitive& s) = 0;
    virtual void addToPut(sc_sensitive* s, sc_process_handle* p) = 0;
    virtual void addToGet(sc_sensitive& s) = 0;
    virtual void addToGet(sc_sensitive* s, sc_process_handle* p) = 0;
};

/// Input port interface
template <class T> 
struct sct_in_if : virtual public sc_interface
{
    virtual T read() const = 0;
    virtual void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) {}
};

/// Output port interface
template <class T> 
struct sct_inout_if : public sct_in_if<T>
{
    virtual void write(const T& val) = 0;
};

/// Clock interface to get period
struct sct_clock_if : virtual public sc_interface 
{
    virtual const sc_time& period() const = 0;
};

//==============================================================================

/// Clock input which current process is sensitive to, set by @SCT_THREAD macro
static sc_in_clk* sct_curr_clock = nullptr;

/// Get period of @sc_clock from @sc_clk_in bound to it
inline const sc_time& get_clk_period(sc_in_clk* clk_in) 
{
    if (auto* i = clk_in->get_interface()) {
        if (auto* clk = dynamic_cast<sct_clock_if*>(i)) {
            //cout << " get_clk_period (sct_clock) " << clk_in->name() << " " << clk->period() << "\n";
            return clk->period();
        } else 
        if (auto* clk = dynamic_cast<sc_clock*>(i)) {
            //cout << " get_clk_period (sc_clock) "  << clk_in->name() << " " << clk->period() << "\n";
            return clk->period();
        }
    }
    std::cout << "No clock period taken from sc_in_clk: " << clk_in->name() << "\n";
    assert (false);
    return SC_ZERO_TIME;
}

#ifdef SCT_TLM_MODE 

/// Reset callback for @SCT_THREAD macro, used in approximate time mode only
struct sct_reset_callback {
    sc_event* m_event;
    sc_in_clk* m_clk;
    sc_time m_period{SC_ZERO_TIME};
    
    inline sct_reset_callback(sc_event* event, sc_in_clk* clk) : 
            m_event(event), m_clk(clk) {}
    
    inline void operator () () {
        if (m_period == SC_ZERO_TIME) {
            m_period = get_clk_period(m_clk);
        }
        m_event->notify(m_period);
        //cout << sc_time_stamp() << " " << sc_delta_count() << " callback " << m_period << " ("<< m_clk->name() << ")" << endl;
    }
};

/// Reset callback storage <reset input port, callback>
static std::unordered_map<size_t, sct_reset_callback*> sct_reset_stor;

/// Thread process macro if it is sensitive to signals only
#define SCT_THREAD3(proc, clk, rst) \
    { \
        sc_spawn_options edge_options; \
        edge_options.spawn_method(); \
        edge_options.dont_initialize(); \
        edge_options.set_sensitivity(&rst.value_changed()); \
        sct_reset_callback* callback; \
        auto i = sct_reset_stor.find((size_t)&rst); \
        if (i != sct_reset_stor.end()) { \
            callback = i->second; \
            if (callback->m_clk != &clk) { \
                std::cout << "Different clock inputs for same reset " \
                          << rst.name() << " not supported\n"; \
                assert (false); \
            } \
        } else { \
            callback = new sct_reset_callback(new sc_event("e"), &clk); \
            sc_spawn(*callback, sc_gen_unique_name("reset_callback"), &edge_options); \
            sct_reset_stor.emplace((size_t)&rst, callback); \
        } \
        SC_THREAD(proc); \
        this->sensitive << *(callback->m_event); \
        sct_curr_clock = &clk; \
    }

/// Thread process macro if it is sensitive to signals and channels
#define SCT_THREAD2(proc, clk) \
        SC_THREAD(proc); \
        sct_curr_clock = &clk; 
        
#else

#define SCT_THREAD3(proc, clk, rst) \
    SC_THREAD(proc); \
    if (SCT_CMN_TRAITS::CLOCK == 1) { \
        this->sensitive << clk.pos(); \
    } else \
    if (SCT_CMN_TRAITS::CLOCK == 0) { \
        this->sensitive << clk.neg(); \
    } else { \
        this->sensitive << clk; \
    } 

#define SCT_THREAD2(proc, clk) SCT_THREAD3(proc, clk, )

#endif

/// Use default clock input name
#define SCT_THREAD1(proc) SCT_THREAD2(proc, clk);

/// Macro argument number overload
#ifndef SCT_GET_MACRO
#define SCT_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME
#endif

#define SCT_THREAD(...) SCT_GET_MACRO(__VA_ARGS__, , SCT_THREAD3,\
                                      SCT_THREAD2, SCT_THREAD1)(__VA_ARGS__)

/// Clocked thread process macro with clock traits, used in cycle accurate mode
#define SCT_CTHREAD3(proc, clk, clock_mode) \
    if (clock_mode == 1) { \
        SC_CTHREAD(proc, clk.pos()); \
    } else \
    if (clock_mode == 0) { \
        SC_CTHREAD(proc, clk.neg()); \
    } else { \
        SC_THREAD(proc); sensitive << clk; \
    }

#define SCT_CTHREAD2(proc, clk) SCT_CTHREAD3(proc, clk, SCT_CMN_TRAITS::CLOCK);

#define SCT_CTHREAD(...) SCT_GET_MACRO(__VA_ARGS__, , SCT_CTHREAD3,\
                                       SCT_CTHREAD2, )(__VA_ARGS__)

/// Method process macro, same as SC_METHOD(...)
#define SCT_METHOD(proc) SC_METHOD(proc)

/// Bind SV DUT target/initiator channel to SystemC TB initiator/target channel 
/// for multi-language simulation
#ifdef SCT_TLM_MODE
    #define SCT_BIND_CHANNEL3(dut_module, dut_channel, tb_channel) \
        static_assert (false, "DO NOT use SCT_BIND_CHANNEL macro in TLM mode");
    #define SCT_BIND_CHANNEL4(dut_module, dut_channel, tb_channel, size) \
        static_assert (false, "DO NOT use SCT_BIND_CHANNEL macro in TLM mode");
#else 
    #define SCT_BIND_CHANNEL3(dut_module, dut_channel, tb_channel) \
        tb_channel.bind_rtl(dut_module.dut_channel##_core_req, tb_channel.core_req); \
        tb_channel.bind_rtl(dut_module.dut_channel##_core_data, tb_channel.core_data); \
        tb_channel.bind_rtl(dut_module.dut_channel##_core_ready, tb_channel.core_ready);
    // Reverse order in SV port array 
    #define SCT_BIND_CHANNEL4(dut_module, dut_channel, tb_channel, size) \
        for (unsigned i = 0; i != size; ++i) { \
            tb_channel[i].bind_rtl(*dut_module.dut_channel##_core_req[size-1-i], tb_channel[i].core_req); \
            tb_channel[i].bind_rtl(*dut_module.dut_channel##_core_data[size-1-i], tb_channel[i].core_data); \
            tb_channel[i].bind_rtl(*dut_module.dut_channel##_core_ready[size-1-i], tb_channel[i].core_ready); \
        }
#endif

#define SCT_BIND_CHANNEL(...) SCT_GET_MACRO(__VA_ARGS__, SCT_BIND_CHANNEL4, \
                                    SCT_BIND_CHANNEL3, , )(__VA_ARGS__)

//==============================================================================

/// General initiator and target templates
template<
    class T, class TRAITS = SCT_CMN_TRAITS, 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_initiator 
{};

template<
    class T, class TRAITS = SCT_CMN_TRAITS, 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_target 
{};

/// General template for combinational target, ALWAYS_READY = 1 
template<
    class T, class TRAITS = SCT_CMN_TRAITS, 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_comb_target 
{};

/// General template for multi-initiator/multi-target
/// Implementation is not available in open-source yet
template<
    class T, unsigned N, class TRAITS = SCT_CMN_TRAITS, 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_multi_target
{};

template<
    class T, unsigned N, class TRAITS = SCT_CMN_TRAITS, 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_multi_initiator 
{};

/// Empty base class for multi target/initiator
/// Implementation is not available in open-source yet
class sct_multi_target_base 
{};
class sct_multi_initiator_base 
{};

/// Target peek helper
template<class T, class TRAITS, bool TLM_MODE>
struct sct_target_peek{
    sct_target<T, TRAITS, TLM_MODE>* target;
};
/// Multi-target peek helper
template<class T, unsigned N, class TRAITS, bool TLM_MODE>
struct sct_multi_target_peek{
    sct_multi_target<T, N, TRAITS, TLM_MODE>* target;
};

/// FIFO general template
template<
    class T, unsigned LENGTH, class TRAITS = SCT_CMN_TRAITS,
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_fifo 
{};

/// FIFO put/get helpers
template <typename T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
struct sct_fifo_put{
    sct_fifo<T, LENGTH, TRAITS, TLM_MODE>* fifo;
};
template <typename T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
struct sct_fifo_get{
    sct_fifo<T, LENGTH, TRAITS, TLM_MODE>* fifo;
};
template <typename T, unsigned LENGTH, class TRAITS, bool TLM_MODE>
struct sct_fifo_peek{
    sct_fifo<T, LENGTH, TRAITS, TLM_MODE>* fifo;
};

/// Register general template 
template<
    class T, class TRAITS = SCT_CMN_TRAITS, 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_register {};

/// Input and output port general templates
template<
    class T, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_in {};
template<
    class T, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_out {};

/// Signal general template 
template<
    class T, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_signal {};

/// Flip-Flop Synchronizer Cell Wrapper
/// SyncType: 1 - single msff, 2 - double msff, 3 - triple msff
/// RstVal: reset value
/// Implementation is not available in open-source yet
template <unsigned SyncType = 2, bool RstVal = 0, 
          class TRAITS = SCT_CMN_TRAITS, bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_ff_synchronizer
{};

/// Clock general template 
template< 
    bool TLM_MODE = SCT_CMN_TLM_MODE>
class sct_clock {};

}  // namespace sct

//=============================================================================

namespace sc_core {

/// Check if process is THREAD or CTHREAD
inline bool sct_is_thread_proc() {
#ifdef __SC_TOOL__
    return false;
#else
    auto procKind = sc_get_current_process_handle().proc_kind();
    return (procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_);
#endif
}

/// Check if process is METHOD
inline bool sct_is_method_proc() {
#ifdef __SC_TOOL__
    return false;
#else
    auto procKind = sc_get_current_process_handle().proc_kind();
    return (procKind == SC_METHOD_PROC_);
#endif
}

}  // namespace sc_core

#endif /* SCT_IPC_IF_H */

