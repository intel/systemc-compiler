/******************************************************************************
* Copyright (c) 2020, Intel Corporation. All rights reserved.
* 
* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
* 
*****************************************************************************/

#include <systemc.h>

// Constants and enums in namespace
namespace ns
{
    // ----- Namespace Variables
    const sc_int<1> scvar_const(1);
    const int ivar_const(2);
    const bool var_const_bool = true;

    enum en_compiler {
        C1_ENUM,
        C2_ENUM,
        C3_ENUM
    };
    enum en_user {
        U1_ENUM=1,
        U2_ENUM=2,
        U3_ENUM=0
    };

}
// ----- Global Variables
const int ivar_const_global(3);
const sc_int<1> scvar_const_global(1);
const bool var_const_global_bool = true;

enum en_compiler_g {
    C1_ENUM_GLOBAL,
    C2_ENUM_GLOBAL,
    C3_ENUM_GLOBAL

};
enum en_user_g {
    U1_ENUM_GLOBAL=1,
    U2_ENUM_GLOBAL=2,
    U3_ENUM_GLOBAL=0
};


SC_MODULE(MyModule) {
    sc_in<bool>     in{"in"};
    sc_signal<int>  sig{"sig"};
    sc_signal<int>  sig_g{"sig_g"};
    sc_out<bool>    out{"out"};
    SC_CTOR(MyModule) {
        SC_METHOD(methodProc);
        sensitive << in << sig << sig_g;
    }    
    void methodProc() {
        using namespace ns;

        // LOCAL WORKS: const sc_int<1> scvar_const_global(1);
        // LOCAL WORKS: const sc_int<1> scvar_const(1);

        en_compiler cvar1, cvar2, cvar3;
        en_user uvar1, uvar2, uvar3;
        en_compiler_g cvar1_g, cvar2_g, cvar3_g;
        en_user_g uvar1_g, uvar2_g, uvar3_g;


        cvar1 = C1_ENUM;
        cvar2 = scvar_const?cvar1:C2_ENUM;
        // THIS WORKS: cvar2 = ivar_const?cvar1:C2_ENUM;
        cvar3 = !var_const_bool ? C3_ENUM : C2_ENUM;
        uvar1 = U3_ENUM;
        uvar2 = U1_ENUM;
        uvar3 = U2_ENUM;


        cvar1_g = C1_ENUM_GLOBAL;
        // SC VARIABLE DOESNT WORK
        cvar2_g = scvar_const_global ? cvar1_g : C2_ENUM_GLOBAL;

        // NON SC_ variables work: cvar2_g = var_const_global_bool?cvar1_g:C2_ENUM_GLOBAL;
        cvar3_g = !var_const_bool ? C3_ENUM_GLOBAL : C2_ENUM_GLOBAL;


        uvar1_g = U3_ENUM_GLOBAL;
        uvar2_g = U1_ENUM_GLOBAL;
        uvar3_g = U2_ENUM_GLOBAL;


        sig = cvar1;
        sig_g = cvar1_g;
    	bool b = in;   // Use in, it need to be in sensitive list
        if ((sig != 0) && (sig_g !=0)) {     // Use sig, it need to be in sensitive list
    	    out = (uvar2) & b;
        } else {
            out = (uvar1);
        }
        
        int i = ivar_const_global;
        i = ivar_const + i;
        
    }
};


SC_MODULE(tb) {

    MyModule top_mod{"top_mod"};

    sc_signal<bool> s1;
    
    SC_CTOR(tb) {
        top_mod.in(s1);
        top_mod.out(s1);
    }

};


int sc_main(int argc, char **argv) {

    cout << "test_promote_ports\n";

    tb tb_inst{"tb_inst"};
    sc_start();

    return 0;
}


