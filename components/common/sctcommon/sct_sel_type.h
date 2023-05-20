/******************************************************************************
 * Copyright (c) 2020-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/**
 * Wrapper over SystemC integral types which selects sc_uint/sc_biguint or
 * sc_int/sc_bigint depends on bit width.
 *
 * Author: Leonid Azarenkov
 */

#ifndef SCT_SEL_TYPE_H
#define SCT_SEL_TYPE_H

#include "sct_zero_width.h"
#include "systemc.h"

namespace sct {

/// Unsigned type which is @sc_uint or @sc_biguint depends on bit width
template <unsigned N, bool SmallSize = (N <= 64)>
struct sct_suint;

template <unsigned N>
struct sct_suint<N, true> {
    typedef sc_uint<N> T;
};

template <>
struct sct_suint<0, true> {
    typedef sc_dt::sct_zero_width T;
};

template <unsigned N>
struct sct_suint<N, false> {
    typedef sc_biguint<N> T;
};

/// Unsigned type which is @sc_uint or @sc_biguint depends on bit width
template <unsigned N>
using sct_uint = typename sct_suint<N>::T;


/// Signed type which is @sc_uint or @sc_biguint depends on bit width
template <unsigned N, bool SmallSize = (N <= 64)>
struct sct_sint;

template <unsigned N>
struct sct_sint<N, true> {
    typedef sc_int<N> T;
};

template <>
struct sct_sint<0, true> {
    typedef sc_dt::sct_zero_width T;
};

template <unsigned N>
struct sct_sint<N, false> {
    typedef sc_bigint<N> T;
};

/// Signed type which is @sc_uint or @sc_biguint depends on bit width
template <unsigned N>
using sct_int = typename sct_sint<N>::T;


/// N-bit zeros sct_uint<N>
template <unsigned N>
const sct_uint<N> sct_zeros = sct_uint<N>(0);

/// N-bit ones sct_uint<N>
template <unsigned N>
const sct_uint<N> sct_ones = ~sct_uint<N>(0);

} // namespace sct

#endif /* SCT_SEL_TYPE_H */
