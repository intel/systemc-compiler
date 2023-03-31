/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_sel_type.h"
#include "sct_assert.h"
#include "systemc.h"

using namespace sct;

// SC type zero width in member and local variables, arrays and record fields
template <unsigned N>
struct MyModule : public sc_module 
{
    sc_signal<sc_uint<8>>  s;
    sc_signal<sc_uint<8>>  t;
    
    SC_HAS_PROCESS(MyModule);
    
    MyModule(const sc_module_name& name) : sc_module(name) 
    {
        SC_METHOD(copyCtorIssue); sensitive << s;
        SC_METHOD(varProc); sensitive << s;
        SC_METHOD(concatProc); sensitive << s; 
        SC_METHOD(unaryProc); sensitive << s; 
        SC_METHOD(compoundProc); sensitive << s; 
        SC_METHOD(binaryProc); sensitive << s << as; 
        SC_METHOD(castProc); sensitive << s; 
        SC_METHOD(partSelProc); sensitive << s; 
        SC_METHOD(compareProc); sensitive << s; 
        
        SC_METHOD(arrProc); sensitive << s;
        SC_METHOD(recSomeProc); sensitive << s;
        SC_METHOD(recAllProc); sensitive << s;
        SC_METHOD(recArrProc); sensitive << s;
    }   

    sct_uint<N> zf() {
        sct_uint<0> a;
        sct_uint<0> b;
        return (a,b);
    }
    void copyCtorIssue() {
        sct_uint<N> a;
        sct_int<10> b;
    	sct_uint<16> c;
        int i;
        unsigned u;
        
        sct_uint<N> d1 = a;
        
        sct_uint<N> d2 = b;    // OK
        sct_uint<N> d3 = c;    // OK
        sct_uint<N> d4 = i;    // OK
        sct_uint<N> d5 = u;    // OK
        sct_uint<N> d6 = (a,a);
        sct_uint<N> d7 = zf();
        
        t.write( 1+d1+d2+d3+d4+d5+d6+d7 );
    }
    
    sct_uint<N> m;
    void varProc() {
        sct_uint<N> b;
        
        sct_assert_const(b == 0);
        sct_assert_const(b == b);
        sct_assert_const(m == b);
        
        b = 1;
        m = b;
        
        int l = b;
        sct_assert_const(l == 0);
        sct_assert_const(l == b);
        sct_assert_const(b == l);
        
        l = m;
        l = m + 1;
        l = m - b;
        l = 2*(b + 1);
        sct_assert_const(l == 2);
        
        b = l;
        sct_assert_const(b == 0);
        b = l * m;
        m = l + 1;
        
        int j;
        l = j = m;
        sct_assert_const(j == 0);
        sct_assert_const(l == 0);
        l = m = 1;
        sct_assert_const(m == 0);
        sct_assert_const(l == 0);
        l = m = b;
    }
    
    void concatProc() {
        const sct_uint<4> a = 0xF;
        const sct_uint<0> b;
        sct_uint<0> nb;
    	sct_uint<16> c;
        c = (a, true);
        c = (a, !nb);
        
        c = concat(b, b);
        sct_assert_const(c == 0);
        cout << "concat(ZWI, ZWI) " << hex << c << endl;

        c = concat(a, b);
        sct_assert_const(c == 0xF);
        cout << "concat(0xF, ZWI) " << hex << c << endl;
        c = concat(b, a);
        sct_assert_const(c == 0xF);
        cout << "concat(ZWI, 0xF) " << hex << c << endl;
        c = concat(b, concat(a, b));
        sct_assert_const(c == 0xF);
        cout << "concat(ZWI, 0xF, ZWI) " << hex << c << endl;
        c = concat(a, concat(b, concat(a, b)));
        sct_assert_const(c == 0xFF);
        cout << "concat(0xF, ZWI, 0xF, ZWI) " << hex << c << endl;
        c = concat(concat(a, concat(b, a)), b);
        
        c = (a, b);
        sct_assert_const(c == 0xF);
        cout << "(0xF, ZWI) " << hex << c << endl;
        c = (b, a);
        sct_assert_const(c == 0xF);
        cout << "(ZWI, 0xF) " << hex << c << endl;
        c = (b, a, b);
        sct_assert_const(c == 0xF);
        cout << "(ZWI, 0xF, ZWI) " << hex << c << endl;
        c = (a, b, a, b);
        sct_assert_const(c == 0xFF);
        cout << "(0xF, ZWI, 0xF, ZWI) " << hex << c << endl;
        c = (a, (b, a), (b, b)); 
        c = (a, sc_uint<10>(b));
        c = (b, sc_uint<10>(11));
        
        sc_int<4> i;
        sc_bigint<23> bi;
        c = concat(i, b);
        c = concat(b, bi);
        c = concat(i, concat(b, concat(bi, b)));
    }
    
    void unaryProc() {
        sct_uint<0> a;
        sct_uint<10> u;
        sct_int<10>  i;
        
        u = a;
        i = a;
        a = a;
        
        a = -a;
        a = +a;
        u = a;
        i = -a;
        i = !a;
        u = ~a;
        int l = +a;
        l = ~a || a;
        
        a++;
        l = a--;
        i = ++a + 1; 
    }
    
    void compoundProc() {
        sct_uint<0> a;
        sct_uint<10> u = 2;
        sct_int<10>  i = 3;
        int l = a; 
        bool b;
        
        a += a;
        a -= l;
        l -= a;
        a *= u;
        i >>= a;
        a ^= b;
        u &= a;
    }
    
    sc_signal<sct_uint<0>> as;
    void binaryProc() {
        int l;
        sct_uint<0> a;
        const sct_uint<10> u = 2;
        const sct_int<10>  i = 3;
        const sc_biguint<10> bu = 4;
        const sc_bigint<10>  bi =5;
        
        l = a + a;
        sct_assert_const(l == 0);
        
        l = u + a;
        sct_assert_const(l == 2);
        cout << "u + a " << l << endl;
        l = a + i;
        sct_assert_const(l == 3);
        l = a + 3;
        sct_assert_const(l == 3);
        l = 2 * a;
        sct_assert_const(l == 0);
        cout << "a + i " << l << endl;
        sc_bigint<10> l1 = sc_bigint<10>(l / bi);
        sct_assert_const(l1 == 0);
        cout << "a / bi " << l1 << endl;
        sc_biguint<10> l2 = sc_biguint<10>((a + 1) * bu);
        sct_assert_const(l2 == 4);
        cout << "(a + 1) * bu " << l2 << endl;

        l = a++;
        sct_assert_const(l == 0);
        cout << "a++ " << l << endl;
        a += 1;
        l = a;
        sct_assert_const(l == 0);
        cout << "a += 1 " << l << endl;

        l = a >> u;
        l = u >> (a, a);
        l = a << i;
        l = a << a;
        
        sct_uint<10> nu;
        //a = as;
        l = s.read();
        l = as.read();
        as.write(1);
        as.write(l);
    }
    
    void castProc() {
        const sct_uint<0> b;
        int l = b.length();
        sct_assert_const(l == 0);
        cout << "b.length() " << l << endl;
        
        l = b.or_reduce();
        sct_assert_const(l == 0);
        cout << "b.or_reduce() " << l << endl;
        
        l = b.nand_reduce();
        sct_assert_const(l);
        cout << "b.nand_reduce() " << l << endl;

        l = b.to_int();
        sct_assert_const(l == 0);
        l = b.to_uint();
        sct_assert_const(l == 0);
        l = b.to_uint64();
        sct_assert_const(l == 0);
        l = b.to_long();
        sct_assert_const(l == 0);
        
        l = (unsigned)b;
        sct_assert_const(l == 0);
        
        sct_uint<0> nb;
        nb = sct_uint<0>(l);
        nb = sct_uint<0>(m);
        l = sct_uint<0>(l) + 1;
        l = sct_uint<0>(nb);
        nb = l;
    }
    
    void partSelProc() {
        int l;
        sct_uint<0> a;
        sct_uint<10> u;
        sct_int<10>  i;
        sc_biguint<10> bu;
        sc_bigint<10>  bi;
        
        l = a.bit(0);
        sct_assert_const(l == 0);
        cout << "a.bit(0) " << l << endl;
        u = a[0];
        cout << "a[0] " << u << endl;
        bi = a.bit(0);
        cout << "a[0] " << bi << endl;
        
        l = a.range(0, 0);
        sct_assert_const(l == 0);
        cout << "a.range(0, 0) " << l << endl;
        i = a.range(0, 0);
        cout << "a.range(0, 0) " << l << endl;
        bu = a(0, 0);
        cout << "a[0, 0] " << l << endl;
        
        a[0] = l;
        a.bit(0) = u;
        a(0, 0) = 1;
        a.range(0, 0) = i;
    }
    
    void compareProc() {
        bool l;
        const sct_uint<0> a;
        const sct_uint<0> a_;
        const sct_uint<10> u = 1;
        const sct_int<10>  i = 2;
        const sc_biguint<10> bu = 3;
        const sc_bigint<10>  bi = 4;
        
        l = a == a_;
        sct_assert_const(l);
        cout << "a == a_ " << l << endl;
        l = a == u;
        sct_assert_const(!l);
        sct_assert_read(u);
        cout << "a == u " << l << endl;
        l = a != i;
        sct_assert_const(l);
        sct_assert_read(i);
        cout << "a != i " << l << endl;

        l = a == bu;
        sct_assert_const(!l);
        sct_assert_read(bu);
        cout << "a == bu " << l << endl;
        l = bu > a;
        sct_assert_const(l);
        cout << "bu > a " << l << endl;
        l = bi != a;
        sct_assert_const(l);
        sct_assert_read(bi);
        cout << "bi != a " << l << endl;
    }
    
// -----------------------------------------------------------------------------

    sct_uint<N> marr[3];
    sct_uint<N> parr[3] = {0,1,2};
    sct_uint<N> darr[2][2];
    void arrProc() {
        unsigned k = s.read();
    	sct_uint<N> barr[3];
        sct_uint<N> carr[3] = {0,1,2};
        
        barr[0] = carr[1];
        marr[0] = parr[1] + barr[k];
        parr[k+1] = barr[k];
        
        int l = barr[0];
        l = marr[1];
        l = marr[k] + 1;
        l = marr[k+1] - barr[0];
        l = 2*(carr[2] + 1);
        
        barr[0] = l;
        barr[k] = l * marr[1];
        marr[k+1] = l + 1;
        
        int j;
        l = j = marr[1];
        l = marr[k] = 1;
        l = marr[0] = barr[k];
        
        l = darr[1][0];
        l = darr[k][k+1];
        darr[1][0] = carr[k];
        darr[k][k+1] = l;
    }
    
// -----------------------------------------------------------------------------    
    
    // Some fields are zero width
    struct Some {
        sct_uint<N> m;
        sct_uint<N> arr[2];
        unsigned mu;
        Some() = default;
        Some(unsigned par) : mu(par) {}
        bool operator == (const Some& oth) {
            return (m == oth.m && mu == oth.mu && 
                    arr[0] == oth.arr[0] && arr[1] == oth.arr[1]);
        }
    };
    
    // All fields are zero width
    struct All {
        sct_uint<N> m;
        sct_uint<N> arr[2];
        All() = default;
        All(unsigned par) {m = par;}
        bool operator == (const All& oth) {
            return (m == oth.m && 
                    arr[0] == oth.arr[0] && arr[1] == oth.arr[1]);
        }
    };
    
    Some mr;
    void recSomeProc() {
        unsigned k = s.read();
        Some lr;
        Some lr2{}; 
        Some lr3(42); 
        Some lr4{42}; 
        
        lr.m = 1;
        mr.m = 2;
        lr.arr[0] = 2;
        mr.arr[k] = 2;
        mr.arr[k] = lr.arr[k] + 1;
        lr = mr;
        
        int l = lr.arr[0];
        l = lr.arr[1];
        l = lr.arr[k];
        l = lr.m + lr.mu;
        l = mr.mu + lr.m;
        l = mr.arr[k] + lr.arr[1];
        
        mr.m = lr.mu;
        mr.mu = lr.mu + mr.m;
        lr.arr[1] = lr.mu;
        lr.arr[k] = l;
        mr.arr[k+1] = l;
        
        int j;
        l = j = mr.arr[1];
        l = mr.arr[k] = 1;
        l = mr.arr[0] = lr.arr[k];
    }
    
    All ma;
    void recAllProc() {
        unsigned k = s.read();
        All la;
        All la2{}; 
        All la3(42); 
        All la4{42}; 
        
        la.m = 1;
        ma.m = 2;
        la.arr[0] = 2;
        ma.arr[k] = 2;
        ma.arr[k] = la.arr[k] + 1;
        la = ma;
        
        int l = la.arr[0];
        l = la.arr[1];
        l = la.arr[k];
        l = la.m + 1;
        l = 2 + la.m;
        l = ma.arr[k] + la.arr[1];
        
        ma.m = 1;
        la.arr[1] = 42;
        la.arr[k] = l;
        ma.arr[k+1] = l;
        
        int j;
        l = j = ma.arr[1];
        l = ma.arr[k] = 1;
        l = ma.arr[0] = la.arr[k];
    }
    
    // Array of record with array
    Some sarr[3];
    All aarr[3];
    void recArrProc() {
        unsigned k = s.read();
        Some lsarr[3];
        All  laarr[3];
        
        sarr[0]   = lsarr[1];
        sarr[k+1] = lsarr[k];
        
        laarr[0]  = aarr[1];
        laarr[k]  = aarr[k+1];
    }
};

int sc_main(int argc, char **argv) 
{
    MyModule<0> top_mod{"top_mod"};
    sc_start();

    return 0;
}


