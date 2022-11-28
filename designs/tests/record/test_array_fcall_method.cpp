/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "sct_assert.h"
#include "systemc.h"
#include <iostream>
using namespace sc_core;

// Record array elements as function parameters
class A : public sc_module {
public:

    sc_in<sc_uint<2>>      a{"a"};
    sc_signal<sc_uint<2>>  as{"as"};
    sc_in<sc_int<2>>       as_i{"as_i"};

    sc_signal<int>        s{"s"};


    SC_CTOR(A) 
    {
        for (int i = 0; i < 3; i++) {
            spp[i] = new sc_signal<int>("");
        }
        
        SC_METHOD(rec_arr_elem_func_param_val);
        sensitive << s;
        
        SC_METHOD(rec_arr_elem_func_param_val2);
        sensitive << s;
        
        SC_METHOD(rec_arr_elem_func_param_val3);
        sensitive << s;
        
        SC_METHOD(rec_arr_elem_func_param_ref);
        sensitive << s;
        
        SC_METHOD(rec_arr_elem_func_param_ref2);
        sensitive << s;
        
        SC_METHOD(rec_arr_elem_func_param_ref3);
        sensitive << s;

        SC_METHOD(arr_elem_in_index);
        sensitive << s << srr[0] << *spp[0];

        // TODO: Fix me, #99
        //SC_METHOD(rec_arr_elem_in_index);
        //sensitive << s;

        /*SC_METHOD(record_var_if);
        sensitive << a << as << as_i << s;

        SC_METHOD(record_var_if_bitwise);
        sensitive << a << as << as_i << s;

        SC_METHOD(record_var_if_arith);
        sensitive << a << as << as_i << s;*/
    }
    
//---------------------------------------------------------------------------    
    
    struct MyRec {
        sc_int<2>  a;
        sc_uint<4> b;
    };

    void f1(MyRec par) {
        int k = par.b;
    }

    int f1_two(MyRec par1, MyRec par2) {
        return (par1.b + par2.b);
    }

    // Record array element as function parameter by value
    void rec_arr_elem_func_param_val()
    {
        MyRec sr;
        f1(sr);
        
        MyRec sra[3];
        f1(sra[1]);

        int i = s.read();
        f1(sra[i]);
    }    


    // Two record array elements as function parameters by value
    void rec_arr_elem_func_param_val2()
    {
        MyRec xr, yr;
        f1_two(xr, yr);
        
        MyRec xra[3]; MyRec yra[2];
        f1_two(xra[2], yra[1]);
        f1_two(xra[2], xra[1]);
        f1_two(xra[2], xra[2]);

        f1_two(xra[2], yr);
        int i = f1_two(yr, xra[1]);
    }    

    // Record array element at unknown index as function parameters by value
    void rec_arr_elem_func_param_val3()
    {
        int i = s.read();
        MyRec qr;
        
        MyRec qra[3]; 
        f1_two(qra[i], qra[i]);
        f1_two(qr, qra[i+1]);
        
        if (i) {
            f1_two(qra[i-1], qr);
        } else {
            if (i == 1) f1(qra[i]);
        }
    }    
    
//-----------------------------------------------------------------------------

    void f2(MyRec& par) {
        int k = par.b;
    }

    void f2_two(MyRec& par1, MyRec& par2) {
        int k = par1.b + par2.b;
    }

    int f2_two_cond(MyRec& par1, bool a, MyRec& par2) {
        return (a ? par1.b : par2.b);
    }

    // Record array element as function parameter by reference
    void rec_arr_elem_func_param_ref()
    {
        MyRec vr[3];
        f2(vr[1]);
        
        int i = s.read();
        f2(vr[i]);
    }

    // Two record array elements as function parameters by reference
    void rec_arr_elem_func_param_ref2()
    {
        MyRec er[3];
        MyRec fr;

        f2_two(fr, er[2]);
        f2_two(er[1], er[2]);
        f2_two(er[1], er[1]);

        bool a;
        int i = s.read();
        a = f2_two_cond(er[i], true, fr);
        f2_two_cond(er[i+1], a, er[i+1]);
    }

    int f3_val(sc_uint<4> par) {
        return (par+1);
    }
    
    int f3_ref(sc_uint<4>& par) {
        par++;
        return par;
    }

    // Record array elements as function parameters in IF, passed by reference
    void rec_arr_elem_func_param_ref3()
    {
        MyRec err[3];
        MyRec frr;
        int i = s.read();
        
        frr.a = f3_val(err[i].b);
        frr.a = f3_ref(err[i+1].b);
        
        f2_two(err[i], err[frr.a]);
    }

    // Use record array element as index for the same array
    sc_signal<int>   srr[3];
    sc_signal<int>*  spp[3];
    void arr_elem_in_index()
    {
        int i = s.read();
        int irr[3];
        irr[irr[1]] = 1;
        irr[irr[i]] = irr[i+1];

        srr[srr[1]] = 1;
        srr[srr[i]] = srr[srr[i+1]+1];

        *spp[spp[1]->read()] = 1;
        *spp[*spp[1]] = 1;
        *spp[*spp[i]] = *spp[i+*spp[i+1]];
    }

    void rec_arr_elem_in_index()
    {
        MyRec err[3];
        err[err[1].b].a = 1;
    }
    
    
    
//-----------------------------------------------------------------------------
    // Function with records
    struct Simple {
        int a;
        sc_uint<2> b;
        Simple(): a(1){

        }
        int geta(){
            return a;
        }
        void seta(int bval){
            a = bval;
        }
    };
    struct Simple_arith {
        sc_uint<32> a;
        sc_uint<16> b;
        Simple_arith(): a(30), b(15){
        }
        sc_uint<32> geta(){
            return a;
        }

        sc_uint<32> getaplusb(){
            return a + b;
        }
        sc_uint<32> getaminusb(){
            return a - b;
        }
        sc_uint<32> getamultb(){
            return a * b;
        }
        sc_uint<32> getadivb(){
            return a / b;
        }
        sc_uint<32> getamodb(){
            return a % b;
        }

        void seta(sc_uint<32> aval){
            a = aval;
        }
        void setb(sc_uint<16> bval){
            b = bval;
        }



    };
    struct Simple_bitwise {
        sc_uint<32> a;
        sc_uint<16> b;
        Simple_bitwise(): a(30), b(15){

        }
        sc_uint<32> geta(){
            return a;
        }
        void seta(sc_uint<32>  aval){
            a = aval;
        }
        void setb(sc_uint<16>  bval){
            b = bval;
        }
        sc_uint<32> getaorb(){
            return a | b;
        }
        sc_uint<32> getaandb(){
            return a & b;
        }
        sc_uint<32> getnota(){
            return !a;
        }
        sc_uint<32> getaxorb(){
            return a ^ b;
        }
        sc_uint<32> getalorb(){
            return a || b;
        }
        sc_uint<32> getalandb(){
            return a && b;
        }
    };
    
    Simple rec[16];
    Simple_arith reca[16];
    Simple_bitwise recb[16];


    void record_var_if()
    {
        int num1 = 0;
        int num2 = 1;
        int val1 = 0;


        if (num1==0) {
            rec[num1].seta(val1);
        } else {
            rec[num1].seta(1);
        }
        if (num2!=1) {
            rec[num2].seta(0);
        } else {
            rec[num2].seta(1);
        }

        //cout << "rec[0] = "<< rec[0].a << endl;
        //cout << "rec[1] = "<< rec[1].a << endl;
        sct_assert_const(rec[0].geta()==0);
        sct_assert_const(rec[1].geta()==1);

        if (as_i.read()==0) {
            rec[as_i.read()].seta(35);
            rec[s.read()].seta(45);

        } else {
            rec[as_i.read()].seta(25);
            rec[s.read()].seta(50);

        }
    }
    void record_var_if_arith()
    {
        reca[0].seta(16);
        reca[0].setb(5);
        sct_assert_const(reca[0].getaplusb()== 21);
        sct_assert_const(reca[0].getaminusb()== 11);
        sct_assert_const(reca[0].getamultb()== 80);
        sct_assert_const(reca[0].getadivb()== 3);
        sct_assert_const(reca[0].getamodb()== 1);

        for(int i=0; i<17; i++) {
            reca[i].seta(i*3);
            reca[i].setb(i*2+2);
            //cout << "sc_assert (reca[" << i << "].getaplusb() == " << reca[i].getaplusb() << ");" << endl;
            //cout << "sc_assert (reca[" << i << "].getaminusb() == " << reca[i].getaminusb() << ");" << endl;
            //cout << "sc_assert (reca[" << i << "].getamultb() == " << reca[i].getamultb() << ");" << endl;
            //cout << "sc_assert (reca[" << i << "].getadivb() == " << reca[i].getadivb() << ");" << endl;
            //cout << "sc_assert (reca[" << i << "].getamodb() == " << reca[i].getamodb() << ");" << endl;
        }

        sc_assert (reca[0].getaplusb() == 2);
        sc_assert (reca[0].getaminusb() == 4294967294);
        sc_assert (reca[0].getamultb() == 0);
        sc_assert (reca[0].getadivb() == 0);
        sc_assert (reca[0].getamodb() == 0);
        sc_assert (reca[1].getaplusb() == 7);
        sc_assert (reca[1].getaminusb() == 4294967295);
        sc_assert (reca[1].getamultb() == 12);
        sc_assert (reca[1].getadivb() == 0);
        sc_assert (reca[1].getamodb() == 3);
        sc_assert (reca[2].getaplusb() == 12);
        sc_assert (reca[2].getaminusb() == 0);
        sc_assert (reca[2].getamultb() == 36);
        sc_assert (reca[2].getadivb() == 1);
        sc_assert (reca[2].getamodb() == 0);
        sc_assert (reca[3].getaplusb() == 17);
        sc_assert (reca[3].getaminusb() == 1);
        sc_assert (reca[3].getamultb() == 72);
        sc_assert (reca[3].getadivb() == 1);
        sc_assert (reca[3].getamodb() == 1);
        sc_assert (reca[4].getaplusb() == 22);
        sc_assert (reca[4].getaminusb() == 2);
        sc_assert (reca[4].getamultb() == 120);
        sc_assert (reca[4].getadivb() == 1);
        sc_assert (reca[4].getamodb() == 2);
        sc_assert (reca[5].getaplusb() == 27);
        sc_assert (reca[5].getaminusb() == 3);
        sc_assert (reca[5].getamultb() == 180);
        sc_assert (reca[5].getadivb() == 1);
        sc_assert (reca[5].getamodb() == 3);
        sc_assert (reca[6].getaplusb() == 32);
        sc_assert (reca[6].getaminusb() == 4);
        sc_assert (reca[6].getamultb() == 252);
        sc_assert (reca[6].getadivb() == 1);
        sc_assert (reca[6].getamodb() == 4);
        sc_assert (reca[7].getaplusb() == 37);
        sc_assert (reca[7].getaminusb() == 5);
        sc_assert (reca[7].getamultb() == 336);
        sc_assert (reca[7].getadivb() == 1);
        sc_assert (reca[7].getamodb() == 5);
        sc_assert (reca[8].getaplusb() == 42);
        sc_assert (reca[8].getaminusb() == 6);
        sc_assert (reca[8].getamultb() == 432);
        sc_assert (reca[8].getadivb() == 1);
        sc_assert (reca[8].getamodb() == 6);
        sc_assert (reca[9].getaplusb() == 47);
        sc_assert (reca[9].getaminusb() == 7);
        sc_assert (reca[9].getamultb() == 540);
        sc_assert (reca[9].getadivb() == 1);
        sc_assert (reca[9].getamodb() == 7);
        sc_assert (reca[10].getaplusb() == 52);
        sc_assert (reca[10].getaminusb() == 8);
        sc_assert (reca[10].getamultb() == 660);
        sc_assert (reca[10].getadivb() == 1);
        sc_assert (reca[10].getamodb() == 8);
        sc_assert (reca[11].getaplusb() == 57);
        sc_assert (reca[11].getaminusb() == 9);
        sc_assert (reca[11].getamultb() == 792);
        sc_assert (reca[11].getadivb() == 1);
        sc_assert (reca[11].getamodb() == 9);
        sc_assert (reca[12].getaplusb() == 62);
        sc_assert (reca[12].getaminusb() == 10);
        sc_assert (reca[12].getamultb() == 936);
        sc_assert (reca[12].getadivb() == 1);
        sc_assert (reca[12].getamodb() == 10);
        sc_assert (reca[13].getaplusb() == 67);
        sc_assert (reca[13].getaminusb() == 11);
        sc_assert (reca[13].getamultb() == 1092);
        sc_assert (reca[13].getadivb() == 1);
        sc_assert (reca[13].getamodb() == 11);
        sc_assert (reca[14].getaplusb() == 72);
        sc_assert (reca[14].getaminusb() == 12);
        sc_assert (reca[14].getamultb() == 1260);
        sc_assert (reca[14].getadivb() == 1);
        sc_assert (reca[14].getamodb() == 12);
        sc_assert (reca[15].getaplusb() == 77);
        sc_assert (reca[15].getaminusb() == 13);
        sc_assert (reca[15].getamultb() == 1440);
        sc_assert (reca[15].getadivb() == 1);
        sc_assert (reca[15].getamodb() == 13);
        sc_assert (reca[16].getaplusb() == 82);
        sc_assert (reca[16].getaminusb() == 14);
        sc_assert (reca[16].getamultb() == 1632);
        sc_assert (reca[16].getadivb() == 1);
        sc_assert (reca[16].getamodb() == 14);

    }
    void record_var_if_bitwise()
    {
        for(int i=0; i<17; i++) {
            recb[i].seta(i*3);
            recb[i].setb(i*2+2);
            //cout << "sc_assert (recb[" << i << "].getaorb() == " << recb[i].getaorb() << ");" << endl;
            //cout << "sc_assert (recb[" << i << "].getaandb() == " << recb[i].getaandb() << ");" << endl;
            //cout << "sc_assert (recb[" << i << "].getnota() == " << recb[i].getnota() << ");" << endl;
            //cout << "sc_assert (recb[" << i << "].getaxorb() == " << recb[i].getaxorb() << ");" << endl;
            //cout << "sc_assert (recb[" << i << "].getalorb() == " << recb[i].getalorb() << ");" << endl;
            //cout << "sc_assert (recb[" << i << "].getalandb() == " << recb[i].getalandb() << ");" << endl;
        }
        sc_assert (recb[0].getaorb() == 2);
        sc_assert (recb[0].getaandb() == 0);
        sc_assert (recb[0].getnota() == 1);
        sc_assert (recb[0].getaxorb() == 2);
        sc_assert (recb[0].getalorb() == 1);
        sc_assert (recb[0].getalandb() == 0);
        sc_assert (recb[1].getaorb() == 7);
        sc_assert (recb[1].getaandb() == 0);
        sc_assert (recb[1].getnota() == 0);
        sc_assert (recb[1].getaxorb() == 7);
        sc_assert (recb[1].getalorb() == 1);
        sc_assert (recb[1].getalandb() == 1);
        sc_assert (recb[2].getaorb() == 6);
        sc_assert (recb[2].getaandb() == 6);
        sc_assert (recb[2].getnota() == 0);
        sc_assert (recb[2].getaxorb() == 0);
        sc_assert (recb[2].getalorb() == 1);
        sc_assert (recb[2].getalandb() == 1);
        sc_assert (recb[3].getaorb() == 9);
        sc_assert (recb[3].getaandb() == 8);
        sc_assert (recb[3].getnota() == 0);
        sc_assert (recb[3].getaxorb() == 1);
        sc_assert (recb[3].getalorb() == 1);
        sc_assert (recb[3].getalandb() == 1);
        sc_assert (recb[4].getaorb() == 14);
        sc_assert (recb[4].getaandb() == 8);
        sc_assert (recb[4].getnota() == 0);
        sc_assert (recb[4].getaxorb() == 6);
        sc_assert (recb[4].getalorb() == 1);
        sc_assert (recb[4].getalandb() == 1);
        sc_assert (recb[5].getaorb() == 15);
        sc_assert (recb[5].getaandb() == 12);
        sc_assert (recb[5].getnota() == 0);
        sc_assert (recb[5].getaxorb() == 3);
        sc_assert (recb[5].getalorb() == 1);
        sc_assert (recb[5].getalandb() == 1);
        sc_assert (recb[6].getaorb() == 30);
        sc_assert (recb[6].getaandb() == 2);
        sc_assert (recb[6].getnota() == 0);
        sc_assert (recb[6].getaxorb() == 28);
        sc_assert (recb[6].getalorb() == 1);
        sc_assert (recb[6].getalandb() == 1);
        sc_assert (recb[7].getaorb() == 21);
        sc_assert (recb[7].getaandb() == 16);
        sc_assert (recb[7].getnota() == 0);
        sc_assert (recb[7].getaxorb() == 5);
        sc_assert (recb[7].getalorb() == 1);
        sc_assert (recb[7].getalandb() == 1);
        sc_assert (recb[8].getaorb() == 26);
        sc_assert (recb[8].getaandb() == 16);
        sc_assert (recb[8].getnota() == 0);
        sc_assert (recb[8].getaxorb() == 10);
        sc_assert (recb[8].getalorb() == 1);
        sc_assert (recb[8].getalandb() == 1);
        sc_assert (recb[9].getaorb() == 31);
        sc_assert (recb[9].getaandb() == 16);
        sc_assert (recb[9].getnota() == 0);
        sc_assert (recb[9].getaxorb() == 15);
        sc_assert (recb[9].getalorb() == 1);
        sc_assert (recb[9].getalandb() == 1);
        sc_assert (recb[10].getaorb() == 30);
        sc_assert (recb[10].getaandb() == 22);
        sc_assert (recb[10].getnota() == 0);
        sc_assert (recb[10].getaxorb() == 8);
        sc_assert (recb[10].getalorb() == 1);
        sc_assert (recb[10].getalandb() == 1);
        sc_assert (recb[11].getaorb() == 57);
        sc_assert (recb[11].getaandb() == 0);
        sc_assert (recb[11].getnota() == 0);
        sc_assert (recb[11].getaxorb() == 57);
        sc_assert (recb[11].getalorb() == 1);
        sc_assert (recb[11].getalandb() == 1);
        sc_assert (recb[12].getaorb() == 62);
        sc_assert (recb[12].getaandb() == 0);
        sc_assert (recb[12].getnota() == 0);
        sc_assert (recb[12].getaxorb() == 62);
        sc_assert (recb[12].getalorb() == 1);
        sc_assert (recb[12].getalandb() == 1);
        sc_assert (recb[13].getaorb() == 63);
        sc_assert (recb[13].getaandb() == 4);
        sc_assert (recb[13].getnota() == 0);
        sc_assert (recb[13].getaxorb() == 59);
        sc_assert (recb[13].getalorb() == 1);
        sc_assert (recb[13].getalandb() == 1);
        sc_assert (recb[14].getaorb() == 62);
        sc_assert (recb[14].getaandb() == 10);
        sc_assert (recb[14].getnota() == 0);
        sc_assert (recb[14].getaxorb() == 52);
        sc_assert (recb[14].getalorb() == 1);
        sc_assert (recb[14].getalandb() == 1);
        sc_assert (recb[15].getaorb() == 45);
        sc_assert (recb[15].getaandb() == 32);
        sc_assert (recb[15].getnota() == 0);
        sc_assert (recb[15].getaxorb() == 13);
        sc_assert (recb[15].getalorb() == 1);
        sc_assert (recb[15].getalandb() == 1);
        sc_assert (recb[16].getaorb() == 50);
        sc_assert (recb[16].getaandb() == 32);
        sc_assert (recb[16].getnota() == 0);
        sc_assert (recb[16].getaxorb() == 18);
        sc_assert (recb[16].getalorb() == 1);
        sc_assert (recb[16].getalandb() == 1);

    }
    
};

class B_top : public sc_module {
public:
    sc_signal<sc_uint<2>>  a{"a"};
    sc_signal<sc_uint<2>>  as{"as"};
    sc_signal<sc_int<2>>  as_i{"as_i"};


    A a_mod{"a_mod"};

    SC_CTOR(B_top) {
        a_mod.a(a);
        a_mod.as_i(as_i);

    }
};

int sc_main(int argc, char *argv[]) {
    B_top b_mod{"b_mod"};
    sc_start();
    return 0;
}

