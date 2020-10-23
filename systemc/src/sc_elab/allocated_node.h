//
// Created by ripopov on 9/25/18.
//

#ifndef SCTOOL_ALLOCATED_NODE_H
#define SCTOOL_ALLOCATED_NODE_H

#include <string>
#include <vector>

namespace sc_core
{
    class sc_module;
}

namespace sc_elab
{

/// allocated_node - information about memory allocation
struct allocated_node {
    void *ptr; /// ptr to allocated block
    sc_core::sc_module* host_mod_p; /// owning sc_module
    std::size_t size_bytes; /// size of allocation in bytes
    bool is_array; /// true for new[] allocations
    std::string mangled_type_name;
    std::size_t array_size; // number of elements in array for new [X]
};

/// get traced dynamic allocations for specified module
const std::vector<allocated_node>&
get_module_allocs(const sc_core::sc_module* mod_ptr);

/// Calls before_end_of_elaboration callbacks
void finalize_elaboration();

/// Generate mangled names for sc_objects, allocated using raw new or new[]
void finalize_module_allocations();

}

#endif //SCTOOL_ALLOCATED_NODE_H
