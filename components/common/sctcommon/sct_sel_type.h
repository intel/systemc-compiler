/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Wrapper over SystemC integral types which selects sc_uint/sc_biguint or
 * sc_int/sc_bigint depends on bit width.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_SEL_TYPE_H
#define	SCT_SEL_TYPE_H

#include "systemc.h"

// Unsigned type which is @sc_uint or @sc_biguint depends on bit width
template <unsigned N, bool SmallSize = (N <= 64)> 
struct sc_suint;

template <unsigned N> 
struct sc_suint<N, true> {
    typedef sc_uint<N> T;
};

template <unsigned N> 
struct sc_suint<N, false> {
    typedef sc_biguint<N> T;
};

// Signed type which is @sc_uint or @sc_biguint depends on bit width
template <unsigned N, bool SmallSize = (N <= 64)> 
struct sc_sint;

template <unsigned N> 
struct sc_sint<N, true> {
    typedef sc_int<N> T;
};

template <unsigned N> 
struct sc_sint<N, false> {
    typedef sc_bigint<N> T;
};

#endif /* SCT_SEL_TYPE_H */