/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Static logarithm implementation.
 * 
 * Author: Mikhail Moiseev
 */

#ifndef SCT_STATIC_LOG_H
#define	SCT_STATIC_LOG_H

namespace StaticLog
{
    template<int N,unsigned int P=0> struct Log2
        { enum { value = Log2<N/2,P+1>::value }; };
    template <unsigned p>
        struct Log2<0, p> { enum { value = p }; };
    template <unsigned p>
        struct Log2<1, p> { enum { value = p }; };

    // Number of bits to address the given number of items
    // N -numbers of element, ::value - bit width to address N elements
    template<int N> struct bits {
            enum { value = (Log2<(N-1)>::value + 1) };
    };

    template<> struct bits<1> {
            enum { value = 0 };
    };

    // The same as @bits but ::value is not less than 1
    template<int N> struct bits_one {
            enum { value = (Log2<(N-1)>::value + 1) };
    };
    
    // Checking if a number is power of 2, numbers 0 and 1 considered as power of 2
    // Return 1 if given number is power of 2, 0 otherwise
    template<int N> struct is_pow2 {
        enum { value = (N & 1)? 0 : is_pow2<(N >> 1)>::value };
    }; 
    
    template<> struct is_pow2<0> {
        enum { value = 1 };
    }; 

    template<> struct is_pow2<1> {
        enum { value = 1 };
    }; 
}

#endif /* SCT_STATIC_LOG_H */
