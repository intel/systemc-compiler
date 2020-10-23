/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_COMMANDLINE_H
#define SCTOOL_COMMANDLINE_H

#include <string>

namespace sc_elab {

struct ArgcArgv {
    int argc;
    const char **argv;
};

ArgcArgv parseToArgcArgv(const std::string &commandLine);

} // namespace sc_elab

#endif //SCTOOL_COMMANDLINE_H
