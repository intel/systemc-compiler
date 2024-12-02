/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "StringFormat.h"
#include "CppTypeTraits.h"
#include "llvm/ADT/SmallString.h"
#include "sc_tool/diag/ScToolDiagnostic.h"
#include <iostream>

namespace sc {
    
std::string APSintToString(const llvm::APSInt& val, unsigned radix) 
{
    unsigned bitsNeeded = getBitsNeeded(val);
    bool overflow = (radix == 2) ? bitsNeeded > LITERAL_MAX_BIT_NUM :
                    (radix == 16) ? (bitsNeeded >> 4) > LITERAL_MAX_BIT_NUM : 
                                    (bitsNeeded >> 3) > LITERAL_MAX_BIT_NUM;
    if (overflow) {
        ScDiag::reportScDiag(ScDiag::SYNTH_LITER_OVERFLOW);
    }
    
    llvm::SmallString<LITERAL_MAX_BIT_NUM> charVec;
    val.toString(charVec, radix);
    return std::string(charVec.data(), charVec.size());
}

std::string getFileName(const std::string& s) 
{
   char sep = '/';

   size_t i = s.rfind(sep, s.length());
   if (i != std::string::npos) {
      return(s.substr(i+1, s.length() - i));
   }

   return("");
}

std::string removeFileExt(const std::string& s) 
{
   char sep = '.';
   
   size_t i = s.rfind(sep, s.length());
   if (i != std::string::npos) {
       return (s.substr(0, i));
   }
   
   return s;
}

std::string getTail(const std::string& source, const std::size_t length) 
{
  if (length >= source.size()) { 
      return source; 
  }
  return source.substr(source.size() - length);
}


std::optional<std::vector<std::string>> splitString(const std::string& str) 
{
    size_t pos = 0;
    auto i = str.find("\n", pos);
    if (i == std::string::npos) 
        return std::optional<std::vector<std::string>>();
    
    std::vector<std::string> res;
      
    while (true) {
        res.push_back(str.substr(pos, i-pos));
        pos = i+1;
        i = str.find("\n", pos);

        if (i == std::string::npos) {
            // Add last piece if at least one "\n" has been found
            res.push_back(str.substr(pos, std::max(str.size()-pos, size_t(0))));
            return res;
        }
    }
}

void printSplitString(std::ostream &os, const std::string& str, 
                      const std::string& tabStr, const std::string& comment) 
{
    // Do not add ";" if string finishes with "begin" and "{" 
    bool comm = !comment.empty();
    
    if (auto strVec = sc::splitString(str)) {
        if (comm) os << tabStr << comment << std::endl;
        
        for (const auto& s : *strVec) {
            bool noSem = sc::getTail(s, 5) == "begin" || sc::getTail(s, 1) == "{";
            os << tabStr << s << (noSem ? "" : ";") << std::endl;
        }
    } else {
        bool noSem = sc::getTail(str, 5) == "begin" || sc::getTail(str, 1) == "{";
        os << tabStr << str << (noSem ? "" : ";") 
           << ((!noSem && comm) ? (std::string("    ")+comment) : "") << std::endl;
    }
}

void removeAllSubstr(std::string& str, const std::string& pattern) 
{ 
    size_t psize = pattern.size();
    size_t i = str.find(pattern);
  
    for (; i != std::string::npos; i = str.find(pattern)) {
        str.erase(i, psize);
    }
}

bool replaceBitRangeStr(std::string& str, const std::string& pattern) 
{
    size_t psize = pattern.size();
    size_t i = str.find(pattern);
    
    for (; i != std::string::npos; i = str.find(pattern)) {
        str.erase(i, psize-1);
        
        if (str[i] == '(') {
            str[i] = '['; i++;
        } else {
            return false;
        }
        
        while (isdigit(str[i])) i++;
        if (str[i] == ',') {
            str[i]=':'; i++;
            while (isdigit(str[i])) i++;
        }
        
        if (str[i] == ')') {
            str[i] = ']'; i++;
        } else {
            return false;
        }
    }
    return true;
}


bool checkArrayAccessStr(const std::string& str) 
{
    bool isIndex = false;
    for (size_t i = 0; i != str.size(); ++i) {
        char c = str[i];
        
        if (isIndex) {
            if (isdigit(c) || isspace(c)) {
                // Do nothing
            } else 
            if (c == ']') {
                isIndex = false;
            } else {
                return true;
            }
        } else {
            isIndex = c == '[';
        }
    }
    return false;
}

// \param times -- <loTime, hiTime>
std::string parseSvaTime(int lotime, int hitime, unsigned stable) 
{
    if (lotime > hitime) {
        int tmp = lotime; lotime = hitime; hitime = tmp; 
    }
    
    std::string time;
    if (lotime != hitime) {
        if (stable == 0) {
            time = "|-> ##[" + std::to_string(lotime) + ":" + 
                               std::to_string(hitime) + "]";
        } else 
        if (stable == 1) {
            // time interval considered in repetition suffix 
            assert (lotime < 2 && "Low time 0 and 1 allowed for stable");
            time = lotime == 0 ? "|->" : "|=>";
        } else {
            // Unsupported option
            assert (false && "No time interval allowed for rose/fell");
        }
    } else 
    if (lotime == 0) {
        time = "|->";
    } else 
    if (lotime == 1) {
        time = "|=>";
    } else {
        assert (stable == 0 && "Low time 0 and 1 allowed for stable/rose/fell");
        time = "|-> ##" + std::to_string(lotime);
    }
    
    return time;
}

/*
 std::optional<std::string> parseSvaTime(const std::string& origStr) 
{
    using namespace sc;
    
    // Remove all spaces and '#'
    std::string str = origStr;
    str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
    str.erase(remove_if(str.begin(), str.end(), 
                        [](char c){return c == '#';}), str.end());
    
    // Get low time
    size_t i = 0;
    size_t ssize = str.size();
    while (i != ssize && isdigit(str[i])) i++;
    if (i == 0) {
        return std::nullopt;
    }
    unsigned lotime = atoi( str.substr(0, i).c_str() );

    // Get high time after ":"
    unsigned hitime = 0;
    if (i != ssize && str[i] == ':') {
        // If ":" is last symbol return nothing
        if (i == ssize-1) {
            return std::nullopt;
        }
        
        i++; 
        size_t tstart = i;
        while (i != ssize && isdigit(str[i])) i++;
        // If no digits found return nothing
        if (i == tstart) {
            return std::nullopt;
        }
        hitime = atoi( str.substr(tstart, i-tstart).c_str() );

        if (lotime > hitime) {
            auto tmp = hitime; hitime = lotime; lotime = tmp; 
        }
    }
    
    // Check there is no other symbols in time string
    if (i != ssize) {
        return std::nullopt;
    }
    
    std::string time;
    if (hitime != 0) {
        time = "|-> ##[" + std::to_string(lotime) + ":" + 
                           std::to_string(hitime) + "]";
    } else 
    if (lotime == 0) {
        time = "|->";
    } else 
    if (lotime == 1) {
        time = "|=>";
    } else {
        time = "|-> ##" + std::to_string(lotime);
    }
    
    return time;
}*/

}