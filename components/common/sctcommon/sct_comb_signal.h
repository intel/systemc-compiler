/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Combinational signal, used instead of sc_signal where it needs to read 
 * its value without delay in 1DC
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SC_COMB_SIGNAL_H
#define SC_COMB_SIGNAL_H

#include <systemc.h>

namespace sc_core {

/// CLEAR -- clear signal value in clocked thread process before each cycle
/// Ignored in method process
template <typename T, bool CLEAR = true>
class sct_comb_signal : public sc_signal<T>
{
   public:
    typedef sc_signal<T>              base_type;
    typedef sct_comb_signal<T, CLEAR> this_type;

    sct_comb_signal() : base_type(sc_gen_unique_name("signal"), T()) {}

    sct_comb_signal(const char* name_) : base_type(name_) {}

    sct_comb_signal(const sct_comb_signal&) = delete;

    virtual ~sct_comb_signal(){};

    this_type& operator=(const int& a)
    {
        base_type::operator=(a);
        return *this;
    }

    this_type& operator=(const unsigned& a)
    {
        base_type::operator=(a);
        return *this;
    }

    this_type& operator=(const T& a)
    {
        base_type::operator=(a);
        return *this;
    }

    this_type& operator=(const sc_signal_in_if<T>& a)
    {
        base_type::operator=(a);
        return *this;
    }

    this_type& operator=(const this_type& a)
    {
        base_type::operator=(a);
        return *this;
    }
};

}  // namespace sc_core

#endif /* SC_COMB_SIGNAL_H */
