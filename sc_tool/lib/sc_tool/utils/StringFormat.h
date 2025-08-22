/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * String formating functions
 * Author: Mikhail Moiseev
 */

#ifndef STRINGFORMAT_H
#define STRINGFORMAT_H

#include "llvm/ADT/APSInt.h"
#include <vector>
#include <string>

namespace sc {
    
/// Maximal symbols in literal in its radix
const unsigned LITERAL_MAX_BIT_NUM = 16384;

/// Covert a number in binary string to decimal string (AI generated)
/// Not used for now
std::string binaryToDecimal(const std::string& binaryString);

/// Get string from APSInt, required since LLVM13
std::string APSintToString(const llvm::APSInt& val, unsigned radix);
    
/// Get file name from file path
std::string getFileName(const std::string& s);

/// Remove extention
std::string removeFileExt(const std::string& s);

/// Get string tail
std::string getTail(const std::string& source, const std::size_t length);

/// Split given string by "\n" and fill string string, return string vector 
/// if it is not empty or none else
std::optional<std::vector<std::string>> splitString(const std::string& str);

/// Split given string by "\n" and print it to os
void printSplitString(std::ostream &os, const std::string& str,
                      const std::string& tabStr, const std::string& comment = "");

/// Remove all substrings @pattern in given string @str
void removeAllSubstr(std::string& str, const std::string& pattern);

/// Replace bit/range with square brackets in given string @str
bool replaceBitRangeStr(std::string& str, const std::string& pattern);

/// Return true for array access at non-literal index
bool checkArrayAccessStr(const std::string& str);

/// Get temporal assertion time string
std::string parseSvaTime(int lotime, int hitime, unsigned stable = 0);

// Escape control symbols for format string
std::string escapeControlSymbols(const std::string& str);

/// Check temporal assertion time string and convert it into SVA form 
//std::optional<std::string> parseSvaTime(const std::string& origStr);

}


#endif /* STRINGFORMAT_H */

