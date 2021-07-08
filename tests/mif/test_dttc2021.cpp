/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// DTTC2021
SC_MODULE(Top) 
{
    sc_in_clk       clk{"clk"};
    
    sc_signal<int>   s;
    sc_signal<bool>  rst;

    SC_CTOR(Top) 
    {
        SC_METHOD(method);
        sensitive << in << enable;

        SC_CTHREAD(thread1, clk.pos());
        async_reset_signal_is(rst, false);

        SC_CTHREAD(thread2, clk.pos());
        async_reset_signal_is(rst, false);
    }

sc_signal<bool> enable;
void method() {
   bool b = in; 
   if (enable != 0) { 
      out = b;
   } else {
      out = 0;
   }
}
    
sc_signal<unsigned> a;
void thread1() {
   unsigned i = 0;
   while (true) {
      wait();
      unsigned b = i + 1;
      i = i + a.read() + b;
   }
}
    
sc_signal<int> in;
sc_signal<int> out;
void thread2() {
   sc_uint<8> x = 0;
   out.write(0);              
   wait();                // STATE 0
   while (true) {
      sc_uint<2> y = in.read(); 
      x = y + 1;
      wait();             // STATE 1
      out.write(x);   
}}

    
};

int sc_main(int argc, char **argv) 
{
    sc_clock  clk("clk", sc_time(1, SC_NS));
    Top top{"top"};
    top.clk(clk);
    
    sc_start();

    return 0;
}
