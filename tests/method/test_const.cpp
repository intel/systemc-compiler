/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <sct_assert.h>
#include <systemc.h>
#include <stdint.h>
#include <iostream>
#include <string>

using namespace sc_core;


// Constant, template parameter, literal access in method process body analysis

template<int N,unsigned int P=0> struct Log2
    { enum { value = Log2<N/2,P+1>::value }; };
template <unsigned p>
    struct Log2<0, p> { enum { value = p }; };
template <unsigned p>
    struct Log2<1, p> { enum { value = p }; };
template<int N> struct bits_one {
        enum { value = (Log2<(N-1)>::value + 1) };
};

template<unsigned N, class T>
class A : public sc_module {
public:
    static const bool   CONST_A = 1;
    static const bool   CONST_AA = 0;
    const int           CONST_B = 4;
    const sc_uint<3>    CONST_C = 5;
    static const unsigned CONST_CC = 5;
    static const unsigned CONST_D = 12;
    static const unsigned LOG_D = bits_one<CONST_D>::value;
    static const unsigned CONST_Z = 0;
    
    static const unsigned A0 = 3;
    static const unsigned A1 = 2;
    static const unsigned C1 = (A0 > A1) ? A0 : A1;
    static const unsigned C2 = std::max(A0, A1);
    static const unsigned C3 = std::min(A0, A1);
    
    const unsigned ARR[5] = {1, 2, 3, 4, 5};
    
    // High bit is set
    const sc_uint<3>    CONST_U = 5;
    const sc_int<3>     CONST_S = -2;
    
    // High bit not set
    const sc_uint<5>    CONST_U1 = 5;
    const sc_int<5>     CONST_S1 = -2;

    sc_signal<sc_uint<3>> a{"a"};

    int* p1;
    int* p2;
    
    const int* p4;
    const int* p5;
    
    const int c3 = 42;
    const int c4 = 43;
    int c5 = 44;
    
    int                 k;
    sc_uint<16>         x;
    
    sc_clock clk{"clk", 10, SC_NS};
    sc_signal<bool> arstn{"arstn", 1};
    
    SC_HAS_PROCESS(A);

    sc_signal<bool> dummy{"dummy"};
    
    A(const sc_module_name& name) : sc_module(name) {
        p1 = nullptr;
        p2 = sc_new<int>();
        // Update constant value in constructor allowed
        int* p3 = const_cast<int*>(&c3);
        *p3 = 1;
        
        p4 = &c4;
        p5 = &c5;

        SC_METHOD(const_init);
        sensitive << a;
        
        SC_METHOD(std_min_max);
        sensitive << a;

        SC_METHOD(long_literal);
        sensitive << a;

        SC_METHOD(long_literal2);
        sensitive << a;

        SC_METHOD(signed_long_literal);
        sensitive << a;
        
        SC_METHOD(smem_if_loop_const); sensitive << dummy;
        SC_METHOD(cout_test); sensitive << dummy;
        
        SC_METHOD(template1); sensitive << dummy;
        SC_METHOD(const1); sensitive << dummy;
        SC_METHOD(const2); sensitive << dummy;
        SC_METHOD(const_array); sensitive << dummy;
        SC_METHOD(sign_const); sensitive << dummy;
        SC_METHOD(cond_stmt_const); sensitive << dummy;
        SC_METHOD(int_const_expr); sensitive << dummy;
        
        SC_METHOD(switch_const); sensitive << dummy;
        SC_METHOD(cond_const); sensitive << dummy;
        SC_METHOD(loop_const); sensitive << dummy;
        SC_METHOD(loop_const_init);
        sensitive << a;

        SC_METHOD(sc_type_const); sensitive << dummy;
        
        SC_METHOD(local_static_const); sensitive << dummy;
        
        SC_THREAD(local_static_const1);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(local_static_const2);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);
        
        SC_THREAD(local_static_const3);
        sensitive << clk.posedge_event();
        async_reset_signal_is(arstn, false);

        SC_METHOD(neg_literal); sensitive << dummy;
        
        SC_METHOD(binary_const_pointers); sensitive << dummy;
        SC_METHOD(pointer_to_const); sensitive << dummy;
        
        SC_METHOD(const_func_call); sensitive << dummy;
        SC_METHOD(sc_int_func); sensitive << dummy;
    }
    
    #define CHECK(ARG) sct_assert(ARG); sct_assert_const(ARG);

    // Constant initialization with function with side-effects
    int m1 = 0;
    
    int getInit() {
        return m1++;
    }
    
    int getInit2() {
        m1++;
        return 1;
    }
    
    void const_init() 
    {
        const int i = getInit();
        const int j = getInit();

        const int k = getInit2();
        int z = i + j + k;
        CHECK(k == 1);
    }
    
// ----------------------------------------------------------------------------
    
    // Check std::max and std::min functions
    void std_min_max() {
        sc_uint<3> d;
        d = a.read().range(C1, C1-1);
        d = a.read().range(C2, C2-1);
        d = a.read().range(C3+1, C3);
    }
    
    // Check long integer literals 
    sc_uint<64> case_long_return(sc_uint<3> asize) {
        switch (asize) {
            case 0: //8b
                return 0x0000000000000001ULL;
            case 1: //16b
                return 0x0000000000000003ULL;
            default:
                return 0xFFFFFFFFFFFFFFFFULL;
        }
    }
    
    sc_uint<64> long_return () {
        return 0xFFFFFFFFFFFFFFFFULL;
    }

    sc_int<64> long_neg_return () {
        return -0x7FFFFFFFFFFFFFFFLL;
    }

    void long_literal() 
    {
        sc_int<64> v1 = -2147483647;
        sc_int<64> v2 = -2147483648;
        sc_int<64> v3 = -2147483649;
        sc_int<64> v4 = 2147483647;
        sc_int<64> v5 = 2147483648;
        sc_int<64> v6 = 2147483649;
        sc_uint<64> v7 = 0xFFFFFFFFFFFFFFFFULL;
        sc_int<64> v8 = -0x7FFFFFFFFFFFFFFFLL;
        
        sc_uint<64> r1 = long_return();
        sc_int<64> r2 = long_neg_return();
        sc_uint<64> r3 = case_long_return(a.read());
        sc_uint<64> r4 = case_long_return(a.read()) << a.read();
    }

    // Literal passed to template function
    template<typename T1>
    void temp_func (T1 par1) {}
    
    void long_literal2() 
    {
        temp_func(2147483648);
        temp_func(-2147483648);
        temp_func(sc_int<32>(2147483648));
        temp_func(sc_int<32>(-2147483648));
    }
    
    void signed_long_literal() 
    {
        sc_int<64> v1 = -(-42);
        sc_int<64> v2 = -(42);
        sc_int<64> v3 = (-42);
        sc_int<64> v4 = (42);

        sc_int<64> w1 = 1-(-42);
        sc_int<64> w2 = -(-42) - 42;
        
        sc_int<64> x1 = (sc_int<33>)(2147483648);
        sc_int<64> x2 = (sc_int<33>)(-2147483648);
        sc_int<64> x3 = -(sc_int<33>)(2147483648);
        sc_int<64> x4 = -(sc_int<33>)(-2147483648);

        sc_int<64> y1 = (sc_int<33>)((2147483648));
        sc_int<64> y2 = (sc_int<33>)((-2147483648));
        sc_int<64> y3 = -(sc_int<33>)((2147483648));
        sc_int<64> y4 = -(sc_int<33>)((-2147483648));

        sc_int<64> y5 = -sc_int<33>(-2147483648);
        sc_int<64> y6 = -sc_int<33>(2147483648);
        sc_int<64> y7 = sc_int<33>(-2147483648);
    }
    
    // BUG from real design in fullAccessPort.h:605 -- FIXED
    void smem_if_loop_const() {
        int k = 0;
        // Empty loop inside of dead branch
        if (CONST_Z > 0) {
            for (int i = 0; i < 1; i++) {
                k = 1;
            }
            k = 2;
        }
    }    

    bool g(bool val) {
        return !val;
    }
    
    // Check no terms for dead code like cout << arguments
    void cout_test() {
        cout << "a";
        cout << g(true);     // Call of g()
    }    
    
    // template parameters
    void template1() {
	int i = N;
        //i = T::A;           // user defined types not supported yet
        i = T::B;
        i = T::C;
        i = 2*T::C + N*T::D;
    }

    // local constants
    void const1() {
        const int AA = 1;
        const unsigned BB = 12;
        const sc_uint<5> CC = 5;
        
        sct_assert_const (AA == 1);
        sct_assert_const (BB == 12);

	int i = 1;
        i = 2 + 3;
        sct_assert_const (i == 5);
        i = AA + BB;
        sct_assert_const (i == 13);
        i = CC;
        sct_assert_const (i == 5);
    }

    // module constants
    void const2() {
        bool b = CONST_A;
        b = CONST_AA;
        int i = CONST_B;
        i = CONST_C;  
        i = CONST_D;
        i = (LOG_D >> 2) + 1;
    }

    // SC data type constants
    void sc_type_const() {
        const sc_uint<2> x = 1;
        sc_int<8> x2 = 2;

        const sc_uint<3> y; 
	sc_int<16> y2;
        
        sct_assert_const (x == 1);
        sct_assert_const (x2 == 2);
        sct_assert_const (y == 0);
        sct_assert_const (y2 == 0);
    }
    
    // module and local constant arrays
    void const_array() {
        int m[3];
        const int A[3] = {1, 2, 3};
        
        int i = T::ARR[1];
        i = A[2];
        i = ARR[4];
        m[1] = 1;
        i = m[1];
        
        m[2] = T::ARR[0] + A[0] + ARR[0];
    }    

    // Check correct signed/unsigned constant
    void sign_const() {
        int i = CONST_U;
        i = CONST_S;
        
        unsigned j = CONST_U;
        j = CONST_S;
        
        sc_uint<3> y = CONST_U;
        y = (sc_uint<3>)CONST_S;
        
        sc_int<3> x = (sc_int<3>)CONST_U;
        x = CONST_S;
        
        i = CONST_U * j + CONST_S;
        
        const bool b1 = CONST_A;
        bool b2 = b1;
        
        const bool b3 = CONST_AA;
        bool b4 = b3;
        
        const bool b5 = CONST_Z;
        bool b6 = b5;
        
        const int z1 = 0;
        const unsigned z2 = 0;
        const sc_uint<4> z3 = 0;
    }
    
    // Ternary statement with constant condition
    void cond_stmt_const() {
        const bool b3 = (true) ? CONST_A : CONST_AA;
        const bool b4 = (false) ? CONST_A : CONST_AA;
        bool b5 = (b3 == b4) ? CONST_A : CONST_AA;
        bool b6 = (b5 == b4) ? b3 : b4;
        
        bool b7 = (CONST_A) ? 1 : 2;
        bool b8 = (CONST_Z) ? 1 : 2;
    }
    
    void int_const_expr() {
        const unsigned u1 = CONST_B + CONST_D;
        unsigned u2 = u1 + LOG_D;
        
        unsigned u3 = CONST_C;
        const sc_uint<3> x1 = CONST_C;
        const unsigned u4 = CONST_C + x1;   // 10
        const sc_uint<4> x2 = x1 + u4;      // 15

        // Check value reduce in assignment
        const sc_uint<4> u5 = CONST_C + x1; // 10
        const sc_uint<3> u6 = u5;           // 2
        const sc_uint<4> u7 = u6;           // 2    
        
        // Check value reduce in type cast
        const unsigned u8 = (sc_uint<3>)u5 + 1; // 3  
        const sc_uint<5> u9 = (sc_uint<1>)u8 + 1; // 2
        const sc_uint<5> u10 = (sc_uint<4>)u9 + (sc_uint<2>)x2; // 5
        
        const sc_int<5> x3 = (sc_int<5>)x1;  // 5
        sc_int<5> x4 = x3 + (sc_int<5>)x2;   // 20
    }
       
    // Constant in switch condition
    void switch_const() {
        int k = 0;
        
        switch (CONST_B) {
            case 1: k = 1; break;
            case 4: k = 2; break;
            default: k = 3;
        }
    }
    
    int g(int val) {
        return val+1;
    }
    
    int f() {
        return k;
    }
    
    // Ternary statement with constant condition
    void cond_const() {
        int a = 1;
        bool b1 = (CONST_A) ? 1 : g(a+1);
        bool b2 = (CONST_Z) ? f() : 2;
        unsigned b3 = (CONST_Z) ? f() : 2;
    }
    
    // Constant in loop conditions
    void loop_const() {
        int k = 0;

        // Loop with declared variable
        for (int i = 0; i < 0; i++) {
            k = 1;
        }
        for (int i = 0; i < 2; i++) {
            // Check that @i has no value after FOR loop initialization
            // This IF should be in Verilog code
            if (i) {
                k = 2;
            }
        }
        
        // Loop with out-scope variable
        int j = 0;
        for (j = 1; j < 1; j++) {
            k = 2;
        }
        // Check that @j has no value after FOR loop initialization
        // This IF should be in Verilog code
        if (j) {
            k = 3;
        }

        for (int i = CONST_B; i < CONST_B; i++) {
            k = 4;
        }
        
        while (CONST_B > CONST_C) {
            k = 5;
        }
    }
    
     // Constant in loop conditions
    void loop_const_init() {
        int k = 0;
        int j = 0;
        for (j = 1; j < 1; j++) {
            k++;
        }
    }
    
    // Static constant declared in (process) function
    void local_static_const()
    {
        static const unsigned CONST_A = 5;
        int i = CONST_A + 1;
    }
    
    // Local constant initialized with non statically determinable value
    sc_signal<unsigned> sig1;
    void local_static_const1()
    {
        const unsigned CONST_E = sig1;
        wait();
        while (true) {
            int i = CONST_E;
            const unsigned CONST_F = sig1;
            i = CONST_E + CONST_F;
            wait(); 
        }
    }

    void local_static_const2()
    {
        static const unsigned CONST_AA = 5;
        const unsigned CONST_AAA = 6;
        sc_uint<CONST_AA> j = 0;
        sc_uint<CONST_AAA> jj = 0;
        wait();
        
        while(true) {
            wait();
        }
    }

    unsigned f1() {
        const unsigned CONST_G1 = sig1;
        const unsigned CONST_G2 = 1;
        return (CONST_G1 + CONST_G2);
    }
    unsigned f2() {
        const unsigned CONST_H1 = sig1;
        const unsigned CONST_H2 = 2;
        return (CONST_H1 + CONST_H2);
    }
    void local_static_const3()
    {
        int i = f1();
        wait();
        while (true) {
            i = f2();
            wait(); 
        }
    }
    
    void neg_literal() {
        __int128_t i = -10;
    }
    
    // Constant in binary && and ||
    void binary_const_pointers() {
        // @p1 is null, @p2 is not null
        bool b1, b2, b3, b4;
        b1 = (p1 != nullptr) && *p1;
        b2 = (p1 == nullptr) || *p1;
        //b3 = (p1 == nullptr) && *p1; -- error reported
        //b4 = (p1 != nullptr) || *p1; -- error reported
        
        b1 = (p2 != nullptr) && *p2;
        b2 = (p2 == nullptr) || *p2;
        b3 = (p2 == nullptr) && *p2;
        b4 = (p2 != nullptr) || *p2;
    }
    
    // Check only constant pointer to constant variable is replaced with integer value
    void pointer_to_const() {
        int j = c3 + *p4 + *p5;
        j = c3;
        j = *p4;
        
        c5 = 2;
        j = *p5;
    }

    // Constant in function call
    // Not supported yet
    unsigned f1(const unsigned val) {
        return val+1;
    }
    
    void const_func_call() {
        const unsigned u1 = f1(1);
        const unsigned u2 = u1;
    }
    
    // SC type functions
    // Not supported yet
    void sc_int_func() {
        const sc_uint<4> u1 = 15;
        const bool b = and_reduce(u1);
        int i = b ? (int)u1 : (int)(u1 + 1);
    }
};

struct TRAITS {
    static const sc_uint<8>  A;
    static const bool        B = true;
    static const int         C = 10;
    static const unsigned    D = 12;
    
    static const unsigned    ARR[4];
};
const sc_uint<8> TRAITS::A = 5;
const unsigned TRAITS::ARR[4] = {1, 2, 3, 4};

class B_top : public sc_module {
public:

    A<3, TRAITS> a_mod{"a_mod"};

    SC_CTOR(B_top) {
    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

