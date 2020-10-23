//
// Created by ripopov on 10/3/18.
//

#ifndef SCTOOL_PROCESS_TYPE_INFO_H
#define SCTOOL_PROCESS_TYPE_INFO_H

namespace sc_elab
{

struct process_type_info {
    std::string mangled_host_type; /// Linkage name of host module
    std::string function_name; /// Name of function
};

}

#endif //SCTOOL_PROCESS_TYPE_INFO_H
