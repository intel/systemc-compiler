/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/* 
 * SC object abstract class.
 *
 * Author: Mikhail Moiseev
 */

#include <clang/Basic/SourceLocation.h>

#include <string>
#include <unordered_set>

#ifndef SCOBJECT_H
#define SCOBJECT_H

namespace sc {
    
/// ScObject name generator for port dynamic allocation
struct ScDynamicName 
{
    clang::SourceLocation   loc;
    /// Unique ID generator
    static uint64_t         id_gen;
    
    ScDynamicName(const clang::SourceLocation& loc_) : 
        loc(loc_)
    {}
    
    uint64_t getId();
    
    /// Provide only one name generator per source location
    bool operator == (const ScDynamicName& rhs) const;
};
}

namespace std {
// Hash function for @ScDynamicName
template<> 
struct hash<sc::ScDynamicName>  
{
    std::size_t operator () (const sc::ScDynamicName& obj) const {
        using std::hash;
        return (std::hash<unsigned>()(obj.loc.getRawEncoding()));
    }
};
}

namespace sc {

// Any SC object instance
class ScObject {
    /// Unique ID generator
    static uint64_t id_gen;
    
protected:
    // Object unique ID
    uint64_t  id;     
    std::string     name;

public:
    enum Kind {scInPort, scOutPort, scSignal, scModule, scUnknown};

    explicit ScObject(std::string name_) : id(id_gen++), name(name_) 
    {}
    
    /// Copy constructor
    ScObject(const ScObject& rhs) {
        id   = rhs.id;
        name = rhs.name;
    }
    
    virtual ~ScObject() {}

    /// Get object kind
    virtual Kind getObjectKind() const = 0;
    
    uint64_t getId() {return id;}

    std::string getName() const {
        return name;
    }
    
    virtual bool operator == (const ScObject& rhs);
    
    virtual std::string asString(bool debug = true) const = 0;
};
}


#endif /* SCOBJECT_H */

