/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/* 
 * Single Source library. Signal channel with multiple drivers.
 * 
 * Used in sct_signal cycle accurate implementation and in sct_prim_fifo.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_PRIM_SIGNAL_H
#define SCT_PRIM_SIGNAL_H

#include "sct_ipc_if.h"
#include <systemc.h>

namespace sct {
    
template <class T>
class sct_prim_signal : 
    public sc_prim_channel, 
    public sct_inout_if<T>
{
  public:
    using this_type = sct_prim_signal<T>; 
    
    SC_HAS_PROCESS(sct_prim_signal);
    
    explicit sct_prim_signal(const sc_module_name& name) :
        sc_prim_channel(name),
        event(std::string(std::string(name)+"_event").c_str())
    {}

  protected:  
    T           curr_val = T{};
    T           next_val = T{};
    
    sc_event    event;

    /// Channel update, run at DC 0 
    void update() override
    {
        if (!(curr_val == next_val)) {
            curr_val = next_val;
            event.notify(SC_ZERO_TIME);
        }
    }
    
  public:    
    void write(const T& val) override {
        if (!(next_val == val)) {
            request_update();
        }
        next_val = val;
    }
    
    this_type& operator = (const this_type& other) {
        write(other.read());
        return *this;
    }

    this_type& operator = (const T& val) {
        write(val);
        return *this;
    }
    
    T read() const override {
        return curr_val;
    }
    
    operator T () {
        return curr_val;
    }
    
  public:
    const sc_event& default_event() const override {
        return event; 
    }
    
    inline void print(::std::ostream& os) const override {
        os << "sct_prim_signal " << name() << " = " << curr_val << ::std::endl;
    }

    const char* kind() const override { 
        return "sct_prim_signal"; 
    }
};

template<class T>
bool operator == (const sct_prim_signal<T>& a, const sct_prim_signal<T>& b) {
    return (a.read() == b.read());
}
    
template<class T>
bool operator == (const sct_prim_signal<T>& a, const sc_signal<T>& b) {
    return (a.read() == b.read());
}

template<class T>
bool operator != (const sct_prim_signal<T>& a, const sct_prim_signal<T>& b) {
    return (!(a.read() == b.read()));
}

template<class T>
bool operator != (const sct_prim_signal<T>& a, const sc_signal<T>& b) {
    return (!(a.read() == b.read()));
}

} // namespace sct

#endif /* SCT_PRIM_SIGNAL_H */

