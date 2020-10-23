//
// Created by ripopov on 12/18/17.
//
//

#include <sc_elab/elab_alloc.h>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_module_registry.h>
#include <sysc/communication/sc_prim_channel.h>
#include <sysc/communication/sc_port.h>
#include <sysc/communication/sc_export.h>
#include <iostream>
#include <unordered_map>
#include "elab_alloc.h"

namespace sc_elab
{

typedef std::unordered_map<const sc_core::sc_module *,
                           std::vector<allocated_node>> node_map_t;

/// alloc_node_map stores information about dynamic allocations for each module
node_map_t *get_alloc_node_map()
{
    static node_map_t *alloc_node_map = nullptr;

    if (!alloc_node_map) {
        alloc_node_map = new node_map_t{};
    }

    return alloc_node_map;
}

/// get current active module (parent of currently created sc_objects)
static sc_core::sc_module *curr_module()
{
    auto *simctx = sc_core::sc_curr_simcontext;
    if (!simctx)
        return nullptr;
    simctx->get_module_registry();
    return simctx->hierarchy_curr();
}

/// get dynamic allocations for given module
const std::vector<allocated_node> &get_module_allocs(const sc_core::sc_module *mod_ptr)
{
    return get_alloc_node_map()->operator[](mod_ptr);
}

void trace_alloc_impl(void *ptr,
                      bool is_array,
                      const char *type_name,
                      size_t sizeof_alloc,
                      size_t array_size)
{

    if (curr_module()) {
        allocated_node new_node;
        new_node.ptr = ptr;
        new_node.host_mod_p = curr_module();
        new_node.size_bytes = sizeof_alloc;
        new_node.is_array = is_array;
        if (type_name)
            new_node.mangled_type_name = type_name;
        new_node.array_size = array_size;

        (*get_alloc_node_map())[curr_module()].emplace_back(std::move(new_node));
    }
}

void finalize_elaboration()
{
    std::cout << "Finalize elaboration\n";
    auto *context_p = sc_core::sc_get_curr_simcontext();
    context_p->sc_tool_elaboration();
}

void finalize_module_allocations()
{
    // Iterate over all dynamic allocations and add mangled name where it is absent
    for (auto &allocIter : *get_alloc_node_map()) {

        const sc_core::sc_module *mod_ptr = allocIter.first;
        std::vector<allocated_node> &alloc_nodes = allocIter.second;

        for (auto &node : alloc_nodes) {

            // If mangled_type_name is empty, then it is sc_object allocated using raw new or new[]
            if (node.mangled_type_name.empty()) {

                if (node.is_array) {
                    // TODO:: optimize by reading Linux/Windows ABI manual
                    // Current implementation should be safe, it
                    // finds first child sc_object that is inside allocated node
                    //
                    // CXX ABI may store a cookie before actual array, so we
                    // cant just dereference node.ptr (because it may point to cookie,
                    // instead of first element of array )

                    size_t array_size = 0;
                    size_t node_addr = (size_t) node.ptr;
                    node.ptr = nullptr;

                    // Calculate array size by counting all sc_objects that are
                    // inside allocated node
                    for (auto child_obj : mod_ptr->get_child_objects()) {

                        size_t child_addr = (size_t) child_obj;

                        if (child_addr >= node_addr
                            && child_addr < node_addr + node.size_bytes) {

                            ++array_size;

                            if (!node.ptr) {
                                node.ptr = dynamic_cast<void *>(child_obj);
								node.mangled_type_name = MANGLED_TYPENAME(*child_obj);
                            }
                        }
                    }

                    node.array_size = array_size;

                }
                else {
                    sc_core::sc_object *obj_ptr = static_cast<sc_core::sc_object *>(node.ptr);
                    node.mangled_type_name = MANGLED_TYPENAME(*obj_ptr);
                }
            }
        }
    }
}

}
