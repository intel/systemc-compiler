/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>
#include <string>

// std::string and const char* variable tests
template<unsigned N>
struct memory_stub_str : public sc_module 
{
    const std::string __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub_str(const sc_module_name& name, 
                             const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }
};

template<unsigned N>
struct memory_stub_str_noconst : public sc_module 
{
    std::string __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub_str_noconst(const sc_module_name& name, 
                             const char* verilogName = "") :
        __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }
};

template<unsigned N>
struct memory_stub_char : public sc_module 
{
    const char* __SC_TOOL_MODULE_NAME__;
    
    explicit memory_stub_char(const sc_module_name& name, 
                              const char* verilogName = "") 
        : __SC_TOOL_MODULE_NAME__(verilogName)
    {
    }
};

struct module_with_memory : public sc_module 
{
    memory_stub_str<1>      stubInst1{"stubInst1", "short_name"};
    memory_stub_str<2>      stubInst2{"stubInst2", "long_memory_verilog_name"};
    memory_stub_char<1>     stubInst3{"stubInst3", "char_const_name"};
    memory_stub_char<2>     stubInst4{"stubInst4", nullptr};
    memory_stub_str_noconst<3> stubInst5{"stubInst5", "nonconst_verilog_name"};

    SC_CTOR(module_with_memory)
    {}     
};

int sc_main(int argc, char **argv) {

    module_with_memory mod{"mod"};
    sc_start();

    return 0;
}


