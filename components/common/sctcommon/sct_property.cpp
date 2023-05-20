/******************************************************************************
 * Copyright (c) 2020-2023, Intel Corporation. All rights reserved.
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

std::string sct_property_utils::getFileName(const std::string& s)
{
  return s.substr(s.find_last_of("/\\") + 1);
}


//=============================================================================
namespace sc_core {

// Static container of created @sct_property instances
std::unordered_map<std::size_t, sct_property_base*> sct_property_storage::stor;

}