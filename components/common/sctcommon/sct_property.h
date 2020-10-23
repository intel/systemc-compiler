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

#ifndef SCT_PROPERTY_H
#define SCT_PROPERTY_H

#include "sysc/kernel/sc_spawn.h"
#include "systemc.h"
#include <unordered_map>
#include <string>

namespace sct_property_utils {

/// Parse time interval or single time string
/// \param first -- low interval value or single time
/// \param second -- high interval value or single time
void parseTimes(const std::string& s, size_t& first, size_t& second);

/// Form string with one or more iteration values
template <class Type>
std::string getIterStr(Type val) {
    return std::to_string(val);
}

template <class Type, class... Types>
std::string getIterStr(Type val, Types... args) {
    std::string s = std::to_string(val) + "_" + getIterStr(args...);
    return s;
}
}

//=============================================================================
    
namespace sc_core {
    
/** 
 * Time range structure 
 */ 
struct sct_time {

    int lo;
    int hi;
    
    sct_time(int time) : 
        lo(time), hi(time) 
    {}

    sct_time(int lo, int hi) : lo(lo), hi(hi) 
    {}
};
    
/** 
 * Assertion property class 
 */ 
class sct_property {
private:
    /// Number of elements in @size, specified at first @check() call
    size_t pastSize = 0;
    /// Time interval size in cycles, one for single time
    size_t timeInt = 0;
    /// Antecedent (left) expression past values plus current value
    std::vector<bool> leftPast;
    /// Consequent (right) expression past values plus current value
    std::vector<bool> rightPast;
    /// The oldest element indices, will be replaced at this cycle
    size_t lindx = 0;
    size_t rindx = 0;
    /// Property expression string or variable name for error message
    std::string msg;
    
protected:
    /// Time specified and intialization done
    bool initalized = false;

    void init(int time) 
    {
        timeInt = 0;
        pastSize = time;
        
        if (pastSize) {
            leftPast.resize(pastSize, false);
        }
        initalized = true;
    }
    
    void init(int loTime, int hiTime) 
    {
        if (loTime > hiTime) {
            int t = hiTime; hiTime = loTime; loTime = t;
        }
        
        timeInt = hiTime-loTime;
        pastSize = hiTime;
        
        if (pastSize) {
            leftPast.resize(pastSize, false);
            if (timeInt) rightPast.resize(timeInt, false);
        }
        initalized = true;
    }
    
public:
    explicit sct_property(std::string s) : 
        msg(s)
    {}

    explicit sct_property(int time, std::string s) : 
        msg(s)
    {
        init(time);
    }

    explicit sct_property(int loTime, int hiTime, std::string s) : 
        msg(s)
    {
        init(loTime, hiTime);
    }
    
    sct_property(const sct_property&) = default;
    
    sct_property& operator =(const sct_property&) = default;
    
    virtual ~sct_property() = default;
    
    /// Put antecedent(left) and consequent (right) expressions every cycle
    void check(bool lexpr, bool rexpr);
};

//=============================================================================

/**
 * Assertion property with assertion expression lambdas 
 */
template <class LEXPR, class REXPR, class TIMES>
class sct_property_expr : public sct_property 
{
private:
    LEXPR lexpr;
    REXPR rexpr;
    TIMES times;
    
public:
    using ThisType = sct_property_expr<LEXPR, REXPR, TIMES>*;
    
    // SystemC simulation constructor
    // \param propstr -- property string
    explicit sct_property_expr(LEXPR lexpr, REXPR rexpr, TIMES times, 
                               const std::string& propstr) : 
        sct_property(propstr),
        lexpr(lexpr), rexpr(rexpr), times(times)
    {}

    void operator()() 
    {
        if (!initalized) {
            auto t = times();
            this->init(t.lo, t.hi);
        }
        
        this->check(lexpr(), rexpr());
    }
};

//=============================================================================

/**
 * Assertion property class storage
 */
class sct_property_storage {
private:
    /// Static container of created @sct_property instances
    static std::unordered_map<std::size_t, sct_property*> stor;
    
public:
    sct_property_storage() = delete;
    
    static std::size_t calcHash(const std::string& handle) {
        return std::hash<std::string>()(handle);
    }
    
    /// Create or get property for given process handle, used in process scope
    template <class LEXPR, class REXPR, class TIMES>
    static sct_property* getProperty(LEXPR lexpr, REXPR rexpr, 
                                     sc_process_b* proc, 
                                     TIMES times,
                                     const std::string& propstr) {
        
        // Get current thread clock event
        std::vector<const sc_event*> procEvents = proc->get_static_events();
        assert (procEvents.size() == 1 && "Incorrect event number");
        const sc_event* event = procEvents.front();
        
        return getProperty(lexpr, rexpr, event, times, propstr);
    }
    
    /// Create or get property for given process handle, used in loop
    template <class LEXPR, class REXPR, class TIMES, class... IterTypes>
    static sct_property* getProperty(LEXPR lexpr, REXPR rexpr, 
                                     sc_process_b* proc,
                                     TIMES times,
                                     const std::string& propstr,
                                     IterTypes... iters
                                     ) {
        
        std::string propIterStr = propstr + "_ITER#" + 
                                  sct_property_utils::getIterStr(iters...);
        
        return getProperty(lexpr, rexpr, proc, times, propIterStr);
    }    

    template <class LEXPR, class REXPR, class EVENT, class TIMES>
    static sct_property* getProperty(LEXPR lexpr, REXPR rexpr, EVENT* event, 
                                     TIMES times, const std::string& propstr) {
        
        // Join object hierarchical name and file name with line
        std::string procname = sc_get_current_process_handle().name();
        std::string hashStr =  procname + ":" + propstr;
        
        size_t hash = calcHash(hashStr);
        auto i = stor.find(hash);
        
        if (i == stor.end()) {
            auto propInst = new sct_property_expr<LEXPR, REXPR, TIMES>(
                                        lexpr, rexpr, times, propstr);

            // Remove spaces/dots from @hashStr to provide correct process name
            hashStr.erase(remove_if(hashStr.begin(), hashStr.end(), isspace), 
                          hashStr.end());
            hashStr.erase(remove_if(hashStr.begin(), hashStr.end(), 
                          [](char c){return c == '.';}), hashStr.end());

            // Create spawned process                                   
            sc_spawn_options opt;
            opt.spawn_method();
            opt.dont_initialize();
            opt.set_sensitivity(event);
            sc_spawn(*propInst, hashStr.c_str(), &opt);

            stor.emplace(hash, propInst);
            return propInst;
            
        } else {
            return i->second;
        }
    }
};

}

#endif /* SCT_PROPERTY_H */

