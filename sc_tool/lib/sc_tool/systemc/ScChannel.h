/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/* 
 * SC specific types and constants.
 *
 * Author: Mikhail Moiseev
 */

#ifndef SCCOMMON_H
#define SCCOMMON_H

#include "sc_tool/systemc/ScObject.h"

#include <clang/AST/Type.h>
#include <string>

namespace sc {

enum ScClockEdge {cePos, ceNeg};

/// SC port instance
class ScChannel : public ScObject {
protected:
    /// Value type information
    const clang::QualType  type;

    explicit ScChannel(std::string name, const clang::QualType& type_) :
        ScObject(name), type(type_)
    {}
    
    ScChannel(const ScChannel& rhs) : 
        ScObject(rhs), type(rhs.type)
    {}
    
public:
    const clang::QualType getType() const {
        return type;
    }
};

// ---------------------------------------------------------------------------

class ScInPort : public ScChannel {
public:
    explicit ScInPort(std::string name, const clang::QualType& type) :
            ScChannel(name, type)
    {}
            
    ScInPort(const ScInPort& rhs) : ScChannel(rhs) 
    {}

    // Get object kind
    virtual Kind getObjectKind() const {
        return Kind::scInPort;
    }
    
    virtual std::string asString(bool debug = true) const;
};

// ---------------------------------------------------------------------------

class ScOutPort : public ScChannel {
public:
    explicit ScOutPort(std::string name, const clang::QualType& type) :
            ScChannel(name, type)
    {}
            
    ScOutPort(const ScOutPort& rhs) : ScChannel(rhs) 
    {}

    // Get object kind
    virtual Kind getObjectKind() const {
        return Kind::scOutPort;
    }
    
    virtual std::string asString(bool debug = true) const;
};

// ---------------------------------------------------------------------------

class ScSignal : public ScChannel {
public:
    explicit ScSignal(std::string name, const clang::QualType& type) :
            ScChannel(name, type)
    {}
            
    ScSignal(const ScSignal& rhs) : ScChannel(rhs) 
    {}

    // Get object kind
    virtual Kind getObjectKind() const {
        return Kind::scSignal;
    }
    
    virtual std::string asString(bool debug = true) const;
};
}

// ===========================================================================

namespace std {
// Hash function for @ScPort
template<> 
struct hash<sc::ScChannel>  
{
    std::size_t operator () (const sc::ScChannel& obj) const {
        using std::hash;
        bool isIn = (obj.getObjectKind() == sc::ScObject::scInPort);
        return (std::hash<std::string>()(obj.getName()) ^ 
                std::hash<bool>()(isIn));
    }    
};

}

#endif /* SCCOMMON_H */
