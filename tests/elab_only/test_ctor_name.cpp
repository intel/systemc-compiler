/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include "systemc.h"

// Module constructor with various types of name parameter
// Used for demonstration SC kernel provides incorrect module for process 
// if this module does not have sc_module_name parameter. 
// Passing module name to sc_module parent not required.
// Process module specified as its parent module, that leads to error in 
// creating process in ProcessView::getLocation(). That cannot be fixed.
// See #101.

struct mod_with_name : public sc_module
{
    sc_signal<bool> s{"s"};
    
    SC_HAS_PROCESS(mod_with_name);
    
    mod_with_name(::sc_core::sc_module_name)    // OK
    //mod_with_name(const sc_module_name& name) // OK
    //mod_with_name() : sc_module()             // Error : sc_module_name parameter required
    //mod_with_name() :                         // Error : No hostClass
    //      sc_module(sc_module_name("m0")) 
    {  
        SC_METHOD(proc);
        sensitive << s;
    }
    
    void proc() {
        //auto h = sc_get_current_process_handle();
        //std::cout << "proc() parent object " 
        //          << h.get_parent_object()->name() << std::endl;
        bool b = s;
    }
};

SC_MODULE(Top) {
    
    const char* name1 = "mod1";
    std::string name2 = "mod2";    
    
    mod_with_name m0;
    //mod_with_name m0;

    SC_CTOR(Top) : m0(name2.c_str()) {} 
};

int sc_main (int argc, char **argv) {
    Top top{"top"};
    sc_start();
    return 0;
}
