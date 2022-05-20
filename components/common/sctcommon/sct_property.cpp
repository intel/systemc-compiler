/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * SystemC temporal assertions.
 * sct_property, sct_property_expr and sct_property_storage classes. 
 * 
 * Author: Mikhail Moiseev
 */

#include "sct_property.h"

void sct_property_utils::parseTimes(const std::string& s, 
                                   size_t& first, size_t& second) {
    
    const char SEPARATOR = ':';
    size_t i = s.rfind(SEPARATOR, s.length());
    
    if (i != std::string::npos) {
        size_t a = std::stoi(s.substr(0, i));
        size_t b = std::stoi(s.substr(i+1, s.length()-i-1));
        first = (a > b) ? b : a;
        second = (a > b) ? a : b;
        
    } else {
        first = std::stoi(s);
        second = first;
    }
}

//=============================================================================
namespace sc_core {

// Static container of created @sct_property instances
std::unordered_map<std::size_t, sct_property*> sct_property_storage::stor;

// Put antecedent(left) and consequent (right) expressions every cycle
void sct_property::check(bool lexpr, bool rexpr) {

    assert (initalized && "sct_property time is not specified");

    // Stored left expression or current one for single time zero
    bool lcond = pastSize ? leftPast[lindx] : lexpr;

    if (lcond) {
        // Get current right expression for single time
        bool rcond = rexpr;

        // Join all stored right expressions for time interval
        for (size_t i = 0; i < timeInt; i++) {
            rcond = rcond || rightPast[i];
        }

        if (!rcond) {
            std::cout << std::endl << sc_time_stamp() 
                      << ", Error : sct_property violation " << msg 
                      << std::endl;
            assert (false);
        }
    }

    // Store left and right expression values
    if (pastSize) {
        leftPast[lindx] = lexpr;
        lindx = (lindx+1) % pastSize;
    }
    if (timeInt) {
        rightPast[rindx] = rexpr;
        rindx = (rindx+1) % timeInt;
    }
}

} // namespace sc_core
