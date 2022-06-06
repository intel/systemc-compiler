/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */


#include <rtti_sysc/SystemCRTTI.h>

#include <systemc.h>
#include <sysc/kernel/sc_method_process.h>
#include <sysc/kernel/sc_module_registry.h>
#include <sc_elab/elab_alloc.h>
#include <typeinfo>

namespace sc_elab {


struct DummyPolymorphic {
    virtual ~DummyPolymorphic() {}
};

std::string getDynamicMangledTypeName(const void *objPtr)
{
    // TODO:: Is vptr location is fixed to offset == 0?
    const DummyPolymorphic *dp = reinterpret_cast<const DummyPolymorphic*>(objPtr);
    return MANGLED_TYPENAME(*dp);
}

void *getTopModulePtr(const std::string &name)
{
    if (!name.empty()) {
        auto *top = sc_find_object(name.c_str());
        if (!top) {
            cout << "Error: Can't find top module " << name << "\n";
            exit(EXIT_FAILURE);
        }
        return top;
    }

    sc_module *topMod = nullptr;

    size_t topModCount = 0;

    for (auto topObj : sc_get_top_level_objects()) {
        if (sc_module* mod = dynamic_cast<sc_module*>(topObj )) {
            ++topModCount;
            topMod = mod;
        }
    }

    if (topModCount == 0) {
        cout << "Error: SystemC design has no modules";
        exit(EXIT_FAILURE);
    } else if (topModCount > 1) {
        cout << "Error: SystemC design has multiple top-level modules";
        exit(EXIT_FAILURE);
    }

    return dynamic_cast<void*>(topMod);
}

sc_core::sc_port_base ** getFirstParentPortPtr(sc_core::sc_port_base *port)
{
    return port->first_parent_port_ptr();
}


llvm::APSInt getScIntValue(const void *objPtr)
{
    auto* obj = (const sc_int_base*)(objPtr);

    llvm::APSInt val(obj->length(), false);
    val = obj->to_int64();
    return val;
}

llvm::APSInt getScUIntValue(const void *objPtr)
{
    auto* obj = (const sc_uint_base*)(objPtr);
    llvm::APSInt val(obj->length(), true);
    val = obj->to_uint64();
    return val;
}

llvm::APSInt getScBigIntValue(const void *objPtr)
{
    auto* obj = (const sc_signed*)(objPtr);
    llvm::APSInt val(obj->length(), false); 
    // 64bit is OK as no larger integer literls supported in CPP
    val = obj->to_int64();    
    return val;
}

llvm::APSInt getScBigUIntValue(const void *objPtr)
{
    auto* obj = (const sc_unsigned*)(objPtr);
    llvm::APSInt val(obj->length(), true);
    // 64bit is OK as no larger integer literls supported in CPP
    val = obj->to_uint64();
    return val;
}

llvm::APSInt getScBitVectorValue(const void *objPtr)
{
    auto* obj = (const sc_bv_base*)(objPtr);
    llvm::APSInt val(obj->length(), true);
    // 64bit is OK as no larger integer literls supported in CPP
    val = obj->to_uint64();     
    return val;
}

process_type_info getProcessTypeInfo(const sc_core::sc_object *procPtr)
{
    const sc_process_b *pb = dynamic_cast<const sc_process_b*>(procPtr);
    return proc_info_map.at(pb);
}

std::vector<sc_elab::port_sens_proc>
getSensitiveProcs(const sc_core::sc_port_base *portPtr)
{
    const sc_core::sc_port_base *pDyn
    = dynamic_cast<const sc_core::sc_port_base *>(portPtr);
    return pDyn->get_sensitive_procs();
}

sc_core::sc_simcontext *getScSimContext()
{
    return sc_core::sc_get_curr_simcontext();
}

} // namespace sc_elab

