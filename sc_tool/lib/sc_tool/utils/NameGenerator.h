/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef NAMEGENERATOR_H
#define NAMEGENERATOR_H

#include <string>
#include <unordered_set>

/// Generates unique names inside module body
class UniqueNamesGenerator {
public:
    void reset() { 
        takenNames.clear(); 
        changedNames.clear(); 
    }
    std::string getUniqueName (const std::string & suggestedName,
                               bool checkLocalNames = false);
    
    /// Check if member name is already used in this module
    /// \param checkLocalNames -- check name conflict with local variables,
    ///                           required for variables created from processes
    bool isTaken(const std::string & name, bool checkLocalNames = false);
    bool isChanged(const std::string & name);
    
    /// Add process local name
    void addLocalName(const std::string& name) {
        localNames.insert(name);
    }

    /// Add module member name
    void addTakenName(const std::string& name) {
        takenNames.insert(name);
    }

    /// Add module member names
    void addTakenNames(std::unordered_set<std::string>& names) {
        takenNames.insert(names.begin(), names.end());
    }

    UniqueNamesGenerator () = default;
    UniqueNamesGenerator (UniqueNamesGenerator &&) = default;
    UniqueNamesGenerator (const UniqueNamesGenerator &) = delete;
    UniqueNamesGenerator& operator=(UniqueNamesGenerator &&) = default;

    void print() const;
    
private:
    // Member names already taken
    std::unordered_set<std::string> takenNames;
    // Member names have collision, store original name which has been modified
    std::unordered_set<std::string> changedNames;
    // Local names in processes 
    std::unordered_set<std::string> localNames;
};

#endif /* NAMEGENERATOR_H */

