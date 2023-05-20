/******************************************************************************
 * Copyright (c) 2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 *
 *****************************************************************************/

/**
 * Zero width integer implementation
 *
 * Author: Mikhail Moiseev, Leonid Azarenkov
 */

#ifndef SCT_ZERO_WIDTH_H
#define SCT_ZERO_WIDTH_H

#include "sysc/datatypes/int/sc_int.h"
#include "sysc/datatypes/int/sc_uint.h"
#include "sysc/datatypes/int/sc_signed.h"
#include "sysc/datatypes/int/sc_unsigned.h"
#include "sysc/datatypes/bit/sc_bv_base.h"
#include "sysc/datatypes/bit/sc_bit.h"

/// Zero width integer type, hidden in namespace to avoid implicit cast to it
namespace sc_dt {

struct sct_zero_width;    
    
struct sct_zero_width : public sc_dt::sc_value_base 
{
    sct_zero_width() {}
    sct_zero_width(int_type v) {}
    sct_zero_width(uint_type v) {}
    sct_zero_width(const char* a) {}
    sct_zero_width(unsigned long a) {}
    sct_zero_width(long a) {}
    sct_zero_width(unsigned int a) {}
    sct_zero_width(int a) {}
    sct_zero_width(double a) {}

    sct_zero_width(const sc_uint_base& a) {}
    sct_zero_width(const sc_unsigned& a) {}
    sct_zero_width(const sc_int_base& a) {}
    sct_zero_width(const sc_signed& a) {}
    sct_zero_width(const sc_bv_base& a) {}
    sct_zero_width(const sc_bit& a) {}

    sct_zero_width(const sc_concatref& a) {}

    sct_zero_width(const sct_zero_width& a) {}
    
    template<class T>
    sct_zero_width& operator = (const T& v) {return *this;}

    template<class T>
    sct_zero_width& operator += (T v) {return *this;}
    template<class T>
    sct_zero_width& operator -= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator *= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator /= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator %= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator &= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator |= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator ^= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator <<= (T v) {return *this;}
    template<class T>
    sct_zero_width& operator >>= (T v) {return *this;}

    sct_zero_width& operator ++ () {return *this;}
    sct_zero_width& operator ++ (int) {return *this;}
    sct_zero_width& operator -- () {return *this;}
    sct_zero_width& operator -- (int) {return *this;}

    bool operator == (const sct_zero_width&) {return true;}
    bool operator == (int_type v) {return v == 0;}
    bool operator == (uint_type v) {return v == 0;}
    bool operator == (int v) {return v == 0;}
    bool operator == (unsigned int v) {return v == 0;}

    bool operator ! () {return true;}
    bool operator ~ () {return true;}
    
    sct_zero_width& operator [] ( int i ) {return *this;}
    const sct_zero_width& operator [] ( int i ) const {return *this;}
    sct_zero_width& bit( int i ) {return *this;}
    const sct_zero_width& bit( int i ) const {return *this;}

    sct_zero_width& operator () ( int left, int right ) {return *this;}
    const sct_zero_width& operator () ( int left, int right ) const {return *this;}
    sct_zero_width& range( int left, int right ) {return *this;}
    const sct_zero_width& range( int left, int right ) const {return *this;}

    bool test( int i ) const {return false;}
    void set( int i ) {}
    void set( int i, bool v ) {}

    int length() const {return 0;}
#ifdef SC_DT_DEPRECATED
    int bitwidth() const {return 0;}
#endif    
    
    // Concatenation methods
    virtual int concat_length(bool* xz_present_p) const
        { if ( xz_present_p ) *xz_present_p = false; return 0; }
    virtual bool concat_get_ctrl( sc_digit* dst_p, int low_i ) const 
        { return false; }
    virtual bool concat_get_data( sc_digit* dst_p, int low_i ) const
        { return false; }
    virtual uint64 concat_get_uint64() const
        { return 0; }
    virtual void concat_set(int64 src, int low_i) {}
    virtual void concat_set(const sc_signed& src, int low_i) {}
    virtual void concat_set(const sc_unsigned& src, int low_i) {}
    virtual void concat_set(uint64 src, int low_i) {}

    bool and_reduce() const {return false;};
    bool nand_reduce() const {return true;}
    bool or_reduce() const {return false;}
    bool nor_reduce() const {return true;}
    bool xor_reduce() const {return false;}
    bool xnor_reduce() const {return true;}

    operator uint_type() const {return (uint_type)0;}

    uint_type value() const {return (uint_type)0;}
    int to_int() const {return (int)0;}
    unsigned int to_uint() const {return (unsigned int)0;}
    long to_long() const {return (long)0;}
    unsigned long to_ulong() const {return (unsigned long)0;}
    int64 to_int64() const {return (int64)0;}
    uint64 to_uint64() const {return (uint64)0;}
    double to_double() const {return double(0);}
    long long_low() const {return (long)0;}
    long long_high() const {return (long)0;}

    const std::string to_string( sc_numrep numrep = SC_DEC ) const {
        return std::string("0");
    }
    const std::string to_string( sc_numrep numrep, bool w_prefix ) const {
        return std::string("0");
    }
    void print( ::std::ostream& os = ::std::cout ) const { os << "0"; }
    void scan( ::std::istream& is = ::std::cin ) {std::string s; is >> s;}
};


inline ::std::ostream& operator << (::std::ostream& os, const sct_zero_width&) {
    os << "0"; return os;
}
} // namespace sc_dt

namespace sc_core {
inline void 
sc_trace(sc_trace_file*, const sc_dt::sct_zero_width&, const std::string&) {}
} // namespace sc_core

#endif /* SCT_ZERO_WIDTH_H */

