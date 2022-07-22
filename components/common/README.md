# SctCommon

Common SystemC library for SMEM, ISCS and others

## Temporal assertions in SystemC 

Installation and run tests
1. Clone this repository into SCT_HOME folder 

2. Install SystemC 2.3.3 with C++14 compiler option into SYSTEMC_HOME folder

   If you use another C++ version, set it in $SCT_HOME/components/common/test/sctassert/CMakeLists.txt 
   
3. Apply sct_assert.patch to /scr/sysc/kernel/sc_process.h -- skip that for patched SystemC from this repository:

   $ patch $SYSTEMC_HOME/src/sysc/kernel/sc_process.h $SCT_HOME/components/common/sct_assert.patch
   
4. Compile and run assertion test:

   $ cd $SCT_HOME/components/common/test/sctassert
   
   $ mkdir build
   
   $ cd build
   
   $ cmake ../
   
   $ make 
   
   $ ./sct_assert
