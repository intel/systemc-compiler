/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Interfaces, general templates and defines.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_IPC_IF_H
#define SCT_IPC_IF_H

#include <systemc.h>

namespace sct {
    
/// Simulation mode: 0 -- RTL, 1 -- TLM
static const bool SCT_DEFAULT_MODE = 0;    

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

#ifndef SCT_DEFAULT_TRAITS
  #define SCT_DEFAULT_TRAITS SCT_POSEDGE_NEGRESET
#endif

//=============================================================================

/// Put interface for initiator port and FIFO
template<class T, unsigned N = 1>
struct sct_put_if : virtual public sc_interface 
{
    static_assert (N <= 64, "N should be not great than 64");
    static const unsigned long long ALL_ENABLED = ~0ULL;
    
    /// Ready to request, can be used in SCT_ASSERT 
    virtual bool ready() const = 0;
    /// Reset put request in THREAD reset section and METHOD everywhere
    virtual void reset_put() = 0;
    /// Clear put request in METHOD or THREAD after reset
    virtual void clear_put() = 0;
    /// Put request into FIFO/target if it is ready 
    /// \param mask -- enable put or specify targets in multi-cast put
    /// \return ready to request
    virtual bool put(const T& data) = 0;
    virtual bool put(const T& data, sc_uint<N> mask) = 0;
    /// May-block put, call in THREAD only
    virtual void b_put(const T& data) = 0;
    
    /// Add put related signal to process sensitivity
    virtual void addTo(sc_sensitive& s) = 0;
    virtual void addTo(sc_sensitive* s, sc_process_handle* p) = 0;
};

/// Get interface for target port and FIFO
template<class T>
struct sct_get_if : virtual public sc_interface 
{
    /// Request is active, can be used in SCT_ASSERT 
    virtual bool request() const = 0;
    /// Reset get ready in THREAD reset section and METHOD everywhere
    virtual void reset_get() = 0;
    /// Clear get ready in METHOD or THREAD after reset
    virtual void clear_get() = 0;
    /// Return current request data, no change of the request 
    /// \return current request data, if no request last request data 
    virtual T peek() const = 0;
    /// Return request data and remove it from FIFO/target
    /// \return current request data, if no request last request data 
    virtual T get() = 0;
    /// Get request and remove it from FIFO/target if @enable is true
    /// \return true if there is a request
    virtual bool get(T& data, bool enable = true) = 0;
    /// May-block get, call in THREAD only
    virtual T b_get() = 0;

    /// Add get/request related signal to process sensitivity
    virtual void addTo(sc_sensitive& s) = 0;
    virtual void addTo(sc_sensitive* s, sc_process_handle* p) = 0;
    /// Add peek/request related signal to process sensitivity
    virtual void addPeekTo(sc_sensitive& s) = 0;
};

template<class T>
struct sct_fifo_if : public sct_put_if<T>, public sct_get_if<T>
{
    /// FIFO size, can be used in SCT_ASSERT
    virtual unsigned size() const = 0;
    /// Number of elements in FIFO, can be used in SCT_ASSERT 
    /// \return value updated last clock edge for METHOD, last DC for THREAD
    virtual unsigned elem_num() const = 0;
    /// FIFO has (LENGTH-N) elements or more
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

template <class T> 
struct sct_in_if : virtual public sc_interface
{
    virtual T read() const = 0;
    virtual void addTo(sc_sensitive* s, sc_process_handle* p, sc_in_clk* c) {}
};

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

/// Macro argument number overload
#ifndef SCT_GET_MACRO
#define SCT_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME
#endif

/// Clocked thread process macro with clock traits, used in RTL mode
#define SCT_CTHREAD3(proc, clk, clock_mode) \
    if (clock_mode == 1) { \
        SC_CTHREAD(proc, clk.pos()); \
    } else \
    if (clock_mode == 0) { \
        SC_CTHREAD(proc, clk.neg()); \
    } else { \
        SC_THREAD(proc); sensitive << clk; \
    }

#define SCT_CTHREAD2(proc, clk) SCT_CTHREAD3(proc, clk, SCT_DEFAULT_TRAITS::CLOCK);

#define SCT_CTHREAD(...) SCT_GET_MACRO(__VA_ARGS__, , SCT_CTHREAD3,\
                                       SCT_CTHREAD2, )(__VA_ARGS__)

//==============================================================================

/// General initiator and target templates
template<
    class T, class TRAITS = SCT_DEFAULT_TRAITS, 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_initiator 
{};

template<
    class T, class TRAITS = SCT_DEFAULT_TRAITS, 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_target 
{};

/// General template for combinational target, ALWAYS_READY = 1 
template<
    class T, class TRAITS = SCT_DEFAULT_TRAITS, 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_comb_target 
{};

/// General template for multi-initiator/multi-target
template<
    class T, unsigned N, class TRAITS = SCT_DEFAULT_TRAITS, 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_multi_target
{};

template<
    class T, unsigned N, class TRAITS = SCT_DEFAULT_TRAITS, 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_multi_initiator 
{};

/// Empty base class for multi target/initiator
class sct_multi_target_base 
{};
class sct_multi_initiator_base 
{};

/// FIFO general template
template<
    class T, unsigned LENGTH, class TRAITS = SCT_DEFAULT_TRAITS,
    bool TLM_MODE = SCT_DEFAULT_MODE>
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

/// Register general template 
template<
    class T, class TRAITS = SCT_DEFAULT_TRAITS, 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_register {};

/// Input and output port general templates
template<
    class T, bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_in {};
template<
    class T, bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_out {};

/// Signal general template 
template<
    class T, bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_signal {};

/// Flip-Flop Synchronizer Cell Wrapper
/// SyncType: 1 - single msff, 2 - double msff, 3 - triple msff
/// RstVal: reset value
template <unsigned SyncType = 2, bool RstVal = 0, 
          class TRAITS = SCT_DEFAULT_TRAITS, bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_ff_synchronizer
{};

/// Clock general template 
template< 
    bool TLM_MODE = SCT_DEFAULT_MODE>
class sct_clock {};

}  // namespace sct

//=============================================================================

namespace sc_core {

/// Check if process is SC_THREAD or SC_CTHREAD
inline bool sct_is_thread_proc() {
#ifdef __SC_TOOL__
    return false;
#else
    auto procKind = sc_get_current_process_handle().proc_kind();
    return (procKind == SC_THREAD_PROC_ || procKind == SC_CTHREAD_PROC_);
#endif
}

/// Check if process is SC_METHOD
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

