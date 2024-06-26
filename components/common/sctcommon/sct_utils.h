/******************************************************************************
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/**
 * Various utility functions for SystemC
 * 
 * Author: Leonid Azarenkov
 */


#ifndef SCT_UTILS_H
#define SCT_UTILS_H

#include <systemc.h>
#include <type_traits>
#include "sct_sel_type.h"
#include "sct_static_log.h"

namespace sct {

/// Returns: b < a ? b : a;
template<typename _Tp>
constexpr const _Tp&
sct_min(const _Tp& __a, const _Tp& __b)
{
    return __b < __a ? __b : __a;
}

/// Returns: a < b ? b : a;
template<typename _Tp>
constexpr const _Tp&
sct_max(const _Tp& __a, const _Tp& __b)
{
    return  __a < __b ? __b : __a;
}

/// Returns sum of two unsigned numbers a+b, saturate result to sct_ones<Aw>
/// Requires Aw >= Bw
template<int Aw, int Bw>
sc_uint<Aw> sct_sum_saturated(sc_uint<Aw> a, sc_uint<Bw> b)
{
    static_assert(Aw >= Bw);
    sc_uint<Aw + 1> res = a + b;
    return res.bit(Aw) ? sct_ones<Aw> : sc_uint<Aw>(res);
}


/// Returns number of bits set to 1
constexpr unsigned sct_popcount(uint64_t val)
{
    unsigned popcnt = 0;
    for (unsigned i = 0; i < 64; ++i) {
        popcnt += val & 1;
        val = val >> 1;
    }
    return popcnt;
}

/// Returns number of bits set to 1
template<template<int> typename T, int MskLen, unsigned CntLen = sct_addrbits1<MskLen>>
constexpr sc_uint<CntLen>
sct_popcount(const T<MskLen> &mask) {
    static_assert(std::is_same<decltype(mask), const sct_uint<MskLen>&>::value,
     "mask must be sct_uint<>");
    sc_uint<CntLen> popcnt = 0;
    for (unsigned i = 0; i < MskLen; ++i) {
        popcnt += sc_uint<1>(mask[i]);
    }
    return popcnt;
}


/// Converts binary value to one-hot value
template <int BinLen, unsigned OneHotLen = (1 << BinLen)>
constexpr sct_uint<OneHotLen>
sct_bin_to_1hot(const sc_uint<BinLen> &bin_val)
{
    return sct_uint<OneHotLen>(1) << bin_val;
}


/// Helper struct to get width W of sct_int<W>/sct_uint<W> type
template <typename T>
struct sct_width_helper;

template <template <int> typename T, int W_>
struct sct_width_helper<T<W_>> { enum { W = W_ }; };

/// Returns width W of sct_int<W>/sct_uint<W> type and number of bits of C++ integers
template <typename T>
constexpr unsigned sct_width()
{
    if constexpr (std::is_integral<T>::value) {
        if constexpr (std::is_same<T, bool>::value) {
            return 1;
        } else {
            return (8*sizeof(T));
        }
    } else {
        return sct_width_helper<T>::W;
    }
}


/// Returns width W of sct_int<W>/sct_uint<W> value and number of bits of C++ integers
template <typename T>
constexpr unsigned sct_width(const T & val)
{
   if constexpr (std::is_integral<T>::value) {
        if constexpr (std::is_same<T, bool>::value) {
            return 1;
        } else {
            return (8*sizeof(T));
        }
    } else {
         return sct_width_helper<T>::W;
    }
}


/// Convert one-hot value to binary value
/// based in MatchLib one_hot_to_bin.h
template<template<int> typename T, int OneHotLen, unsigned BinLen = sct_addrbits1<OneHotLen>>
constexpr sct_uint<BinLen>
sct_1hot_to_bin(const T<OneHotLen> &ohot_val) {
    static_assert(std::is_same<decltype(ohot_val), const sct_uint<OneHotLen>&>::value,
     "ohot_val must be sct_uint<>");
    sct_uint<BinLen> bin_val = 0;

    for (unsigned bin = 0; bin < BinLen; bin++) {
        // constructs mask which matches one-hot to binary outputs
        sct_uint<OneHotLen>  tmp = 0;
        for (unsigned bit = 0; bit < OneHotLen; bit++) {
            sct_uint<OneHotLen> ind = bit;
            tmp[bit] = ind[bin];
        }
        tmp = (tmp & ohot_val);

        // Update output
        bin_val[bin] = tmp.or_reduce();
    }
    return bin_val;
}


/// Helper class for passing arrays in template parameters
template <class T, T... args> 
struct sct_tarray
{
    static constexpr unsigned size = sizeof...(args);
    static constexpr T data[sizeof...(args)] = { args... };
};


/// Replaces all occurrences of substring @from with substring @to
inline void sct_str_replace_all(std::string& str, const std::string& from,
                    const std::string& to)
{
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

} // namespace sct

#endif /* SCT_UTILS_H */
