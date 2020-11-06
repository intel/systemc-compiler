/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>


// declaration of a variable of type sc_trace_file*,
// assignment to a variable of type sc_trace_file*,
// calls to sc_close_isdb_trace_file, sc_close_wif_trace_file, and sc_close_vcd_trace_file,
// declaration or definition of any function named sc_trace, and
// calls to any function named sc_trace.

  struct packet_type {
   long info;
   int seq;
   int retry;
   inline bool operator == (const packet_type& rhs) const
   {
   return (rhs.info == info && rhs.seq == seq &&
   rhs.retry == retry);
   }
  };

SC_MODULE(ports_arrays) {
  private:
    unsigned info;
    bool flag;
  public:
  sc_in<sc_uint<2> > a[4];
  sc_in<sc_uint<2> > b[4];
  sc_out<sc_uint<3> > o[4];


  void body () {
    int i;
    for (i=0; i < 4; i ++) {
      o[i].write(a[i].read() + b[i].read());

    }
  }

  inline friend void sc_trace(sc_trace_file *tf, const ports_arrays & v,
  const std::string& NAME ) {
    sc_trace(tf,v.info, NAME + ".info");
    sc_trace(tf,v.flag, NAME + ".flag");
  }

  SC_CTOR(ports_arrays) {
    int j;
    SC_METHOD(body);
      for (j=0; j<4; j++) {
        sensitive << a[j] << b[j];
      }
  }
};

// Testbench to generate test vectors
int sc_main (int argc, char* argv[]) {
  sc_signal<sc_uint<2> > a[4];
  sc_signal<sc_uint<2> > b[4];
  sc_signal<sc_uint<3> > o[4];

  int z;

  ports_arrays prt_ar("PORT_ARRAY");
    for (z=0; z<4; z++) {
      prt_ar.a[z](a[z]);
      prt_ar.b[z](b[z]);
      prt_ar.o[z](o[z]);
    }
  sc_start();
  // declaration of a variable of type sc_trace_file*,
  sc_trace_file *wf = sc_create_vcd_trace_file("ports_arrays");
  sc_trace_file *tf;

  sc_trace_file *my_trace_file;

  // assignment to a variable of type sc_trace_file*,
  tf=sc_create_vcd_trace_file("ports_arrays");
  my_trace_file = sc_create_wif_trace_file("my_trace");



  // calls to sc_close_isdb_trace_file, sc_close_wif_trace_file, and sc_close_vcd_trace_file,
  //sc_close_isdb_trace_file(my_trace_file);
  sc_close_wif_trace_file(my_trace_file);
  sc_close_vcd_trace_file(my_trace_file);


  // declaration or definition of any function named sc_trace, and calls to any function named sc_trace.
  packet_type p;
  //sc_trace(tf,p.info);
  sc_trace(tf, a,"temp");

  return 0;// Terminate simulation
}

