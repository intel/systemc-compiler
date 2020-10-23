/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Mikhail Moiseev
 */

#include "sc_tool/systemc/ScModule.h"
#include "sc_tool/cfg/ScState.h"

#include <iostream>

namespace sc {
using namespace std;

// ============================================================================
// @CxxClassType implementation
    
void CxxClassType::addBase(std::shared_ptr<CxxClassType> parent) {
    bases.push_back(parent);
}

vector<std::shared_ptr<CxxClassType> > CxxClassType::getBases() {
    return bases;
}

clang::QualType CxxClassType::getType() {
    return type;
}

string CxxClassType::getName() {
    return type.getAsString();
}

bool CxxClassType::operator == (const CxxClassType& rhs) const 
{
    if (bases.size() != rhs.bases.size()) {
        return false;
    }
    
    for (unsigned i = 0; i < bases.size(); i++) {
        if ( !(*(bases[i]) == *(rhs.bases[i])) ) {
            return false;
        }
    }

    return true;
}

string CxxClassType::asString() {
    string basestr;
    for (auto i : bases) {
        basestr += i->getName()+"; ";
    }
    return (name + ", base classes " + basestr);
}

// ============================================================================
// CxxClass implementation

string CxxClass::getName() const {
    return varName;
}

string CxxClass::asString(bool debug) const {
    string chistr;
    for (auto&& i : childs) {
        chistr += i->asString()+"; ";
    }
    return (getName() + ", childs = " + chistr);
}

void CxxClass::print() const {
    cout << "Childs " << childs.size() << endl;
    for (auto&& i : childs) {
        cout << "  " << i->getName() << endl;;
    }
}
    
    
// ============================================================================
// ScModule implementation

string ScModule::getName() const {
    return varName;
}

string ScModule::asString(bool debug) const {
    string chistr;
    for (auto&& i : childs) {
        chistr += i->asString()+"; ";
    }
    string portstr;
    for (auto&& i : ports) {
        portstr += i->asString()+"; ";
    }
    return (getName()+"_"+ ((debug) ? to_string(id) : "") + ", type = " + 
            type->getName() + ", childs = " + chistr + ", ports = " + portstr);        
}

void ScModule::print() const {
    using namespace std;
    
    cout << "Childs " << childs.size() << endl;
    for (auto&& i : childs) {
        cout << "  " << i->getName() << endl;;
    }
    
    cout << "Ports " << ports.size() << endl;
    for (auto&& i : ports) {
        cout << "  " << i->getName() << endl;
    }
    
    cout << "Bindings " << binds.size() << endl;
    for (auto&& i : binds) {
        cout << "  "<< i.first.first->getName() << "." << i.first.second->getName() << " -> " 
             << i.second.first->getName() << "." << i.second.second->getName() << endl;
    }
    
    cout << "Processes " << processes.size() << endl;
    for (auto&& entry : processes) {
        auto i = entry.first;
        cout << ((i->kind==pkMethod) ? " METHOD " : " THREAD ") << i->func->getNameAsString() << endl;
        if (i->kind==pkThread) {
            cout << "  clock " << ((i->clock == nullptr) ? "-" : 
                                   ((i->clockEdge == cePos)?"(pos) ":"(neg) ")+i->clock->getName())<< endl;
            cout << "  async reset " << ((i->asyncReset.first == nullptr) ? "-" : i->asyncReset.first->getName()+
                                    " "+((i->asyncReset.second)?" (hi)":" (lo)"))<< endl;
            for (auto&& j : i->syncResets) {
                cout << "  sync reset " << j.first->getName()+" "+((j.second)?" (hi)":" (lo)") << endl;
            }
        }    
    }
    
}
}