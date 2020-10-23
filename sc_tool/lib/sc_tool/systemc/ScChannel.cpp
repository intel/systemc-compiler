/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/systemc/ScChannel.h"
#include "sc_tool/diag/ScToolDiagnostic.h"

namespace sc {
using namespace std;

string ScInPort::asString(bool debug) const {
    if (debug) {
        return ("sc_in<"+type.getAsString()+"> "+name+"_"+to_string(id));
    } else {
        SCT_TOOL_ASSERT (false, "ScOutPort::asString no debug");
    }    
}

// ---------------------------------------------------------------------------

string ScOutPort::asString(bool debug) const {
    if (debug) {
        return ("sc_out<"+type.getAsString()+"> "+name+"_"+to_string(id));
    } else {
        SCT_TOOL_ASSERT (false, "ScOutPort::asString no debug");
    }    
}

// ---------------------------------------------------------------------------

string ScSignal::asString(bool debug) const {
    if (debug) {
        return ("sc_signal<"+type.getAsString()+"> "+name+"_"+to_string(id));
    } else {
        SCT_TOOL_ASSERT (false, "ScSignal::asString no debug");
    }    
}
}