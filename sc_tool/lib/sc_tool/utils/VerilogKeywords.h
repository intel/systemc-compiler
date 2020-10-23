/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Verilog keywords to prevent SystemC variable name collision with them.
 * 
 * File:   VerilogKeywords.h
 * Author: Mikhail Moiseev
 */

#ifndef VERILOGKEYWORDS_H
#define VERILOGKEYWORDS_H

#include <string>
#include <unordered_set>

namespace sc {

class VerilogKeywords {
protected:
    static bool init;
    static std::string keywords[221];
    static std::unordered_set<std::string> wordset;
    
public:
    // Check if given string is Verilog key word
    static bool check(const std::string& s) {
        using namespace std;
        
        if (!init) {
            for (auto i = cbegin(keywords); i != cend(keywords); ++i) {
                wordset.insert(*i);
            }
            init = true;
        }
        return (wordset.count(s) != 0);
    }
    

};

}

#endif /* VERILOGKEYWORDS_H */

