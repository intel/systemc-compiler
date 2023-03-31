/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/**
 * Static logarithm implementation.
 *
 * Author: Leonid Azarenkov
 */

#ifndef SCT_STATIC_LOG_H
#define SCT_STATIC_LOG_H

#include <cstdint>

namespace sct {
    /// Compile-time floor(log2(X)) function
    constexpr unsigned sct_log2_func(uint64_t n, unsigned p = 0) noexcept 
    {
        return (n < 2) ? p : sct_log2_func(n / 2, p + 1);
    }
    
    /// Compile-time floor(log2(X)) value
    template <uint64_t X>
    constexpr unsigned sct_log2 = sct_log2_func(X);

    /// Compile-time floor(log2(X)) value
    template <uint64_t X>
    constexpr unsigned sct_floor_log2 = sct_log2_func(X);

    /// Compile-time ceil(log2(X)) value
    template <uint64_t X>
    constexpr unsigned sct_ceil_log2 = (X < 2) ? 0 : sct_log2<X - 1> + 1;
    
    /// Number of bits needed to store the value X
    /// Zero if X is zero; otherwise, one plus the base-2 logarithm of X, with any fractional part discarded.
    template <uint64_t X>
    constexpr unsigned sct_nbits = (X == 0) ? 0 : sct_log2<X> + 1;
    
    /// Address/index width for N elements, same as ceil(log2(N))
    /// Zero if N < 2; otherwise, one plus the base-2 logarithm of N-1, with any fractional part discarded.
    template <uint64_t N> 
    constexpr unsigned sct_addrbits = (N < 2) ? 0 : sct_log2<N - 1> + 1;

    /// Address/index width for N elements, same as sct_addrbits<N> but not less than one
    /// One if N < 2; otherwise, one plus the base-2 logarithm of N-1, with any fractional part discarded.
    template <uint64_t N> 
    constexpr unsigned sct_addrbits1 = (N < 2) ? 1 : sct_log2<N - 1> + 1;
    
    /// Checks if a number X is power of 2, numbers 0 and 1 considered as power of 2
    /// Returns 1 if given number is power of 2, 0 otherwise
    constexpr bool sct_is_pow2_func(uint64_t x) noexcept 
    {
        if (x <= 2) return 1;
        return (x & 1u) ? 0 : sct_is_pow2_func(x >> 1);
    }
    
    /// Checks if a number X is power of 2, numbers 0 and 1 considered as power of 2
    /// Value is 1 if given number is power of 2, 0 otherwise
    template <uint64_t X>
    constexpr bool sct_is_pow2 = sct_is_pow2_func(X);

    /// Next power of 2 number greater than X
    template <uint64_t X>
    constexpr uint64_t sct_next_pow2 = (sct_is_pow2<X> && X) ? (2u * X) : (1u << sct_ceil_log2<X>);

} // namespace sct

#endif /* SCT_STATIC_LOG_H */
