#pragma once

#include <systemc.h>

#ifdef STRATUS

typedef sc_module hls_module;

#define LOG_DEBUG( msg ) (void)0

#else

//#include <tlm_base.h>
//struct hls_module : TLM_BASE::vpCommon {};
typedef sc_core::sc_module hls_module;
#define LOG_DEBUG( msg ) \
    std::cout << msg << "\n"

#define LOG_ERROR( msg ) \
    std::cerr << msg << "\n"

#define LOG_WARNING( msg ) \
    std::cerr << msg << "\n"

#endif

#ifdef __CTOS__
    #define shared_signal(type_name) sc_signal<type_name>
#else
    #define shared_signal(type_name) sc_signal<type_name, SC_MANY_WRITERS>
#endif

#define HLS_IO_SIGNAL_LEVEL 1
#define HLS_IO_TLM_LEVEL 0

#ifndef HLS_IO_DEFAULT_LEVEL
    #define HLS_IO_DEFAULT_LEVEL HLS_IO_SIGNAL_LEVEL
#endif

#if HLS_IO_DEFAULT_LEVEL == HLS_IO_SIGNAL_LEVEL

#else

#endif

