/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#include <sc_tool/utils/CommandLine.h>

#include <cassert>
#include <malloc.h>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <wordexp.h>
#endif

using namespace std;

// TODO: Replace with C++
static const char **split_commandline(const char *cmdline, int *argc)
{
    int i;
    const char **argv = nullptr;
    assert(argc);

    if (!cmdline) {
        return nullptr;
    }

    // Posix.
#ifndef _WIN32
    {
        wordexp_t p;

        // Note! This expands shell variables.
        if (wordexp(cmdline, &p, 0)) {
            return nullptr;
        }

        *argc = p.we_wordc;

        if (!(argv = (const char **)calloc(*argc, sizeof(char *)))) {
            goto fail;
        }

        for (size_t i = 0; i < p.we_wordc; i++) {
            if (!(argv[i] = strdup(p.we_wordv[i]))) {
                goto fail;
            }
        }

        wordfree(&p);

        return argv;
        fail:
        wordfree(&p);
    }
#else // WIN32
    {
        wchar_t **wargs = nullptr;
        size_t needed = 0;
        wchar_t *cmdlinew = nullptr;
        size_t len = strlen(cmdline) + 1;

        if (!(cmdlinew = (wchar_t *)calloc(len, sizeof(wchar_t))))
            goto fail;

        if (!MultiByteToWideChar(CP_ACP, 0, cmdline, -1, cmdlinew, len))
            goto fail;

        if (!(wargs = CommandLineToArgvW(cmdlinew, argc)))
            goto fail;

        if (!(argv = (const char **)calloc(*argc, sizeof(char *))))
            goto fail;

        // Convert from wchar_t * to ANSI char *
        for (i = 0; i < *argc; i++)
        {
            // Get the size needed for the target buffer.
            // CP_ACP = Ansi Codepage.
            needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1,
                                        nullptr, 0, nullptr, nullptr);

            if (!(argv[i] = (const char *)malloc(needed)))
                goto fail;

            // Do the conversion.
            needed = (size_t)WideCharToMultiByte(CP_ACP, 0, wargs[i], -1,
                                                 (char *)argv[i], needed, nullptr, nullptr);
        }

        if (wargs) LocalFree(wargs);
        if (cmdlinew) free(cmdlinew);
        return argv;

    fail:
        if (wargs) LocalFree(wargs);
        if (cmdlinew) free(cmdlinew);
    }
#endif // WIN32

    if (argv) {
        for (i = 0; i < *argc; i++) {
            if (argv[i]) {
                free(const_cast<char *>(argv[i]));
            }
        }

        free(argv);
    }

    return nullptr;
}


namespace sc_elab {

ArgcArgv parseToArgcArgv(const std::string &commandLine)
{
    ArgcArgv results;
    results.argv = split_commandline(commandLine.c_str(), &results.argc);
    return results;
}

}