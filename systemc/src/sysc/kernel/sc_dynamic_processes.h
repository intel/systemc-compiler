/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  sc_dynamic_process.h -- Dynamic Process Package Definitions

  Original Author: Andy Goodrich, Forte Design Systems, 5 May 2003

  CHANGE LOG AT THE END OF THE FILE
 *****************************************************************************/


#ifndef SC_DYNAMIC_PROCESSES_H
#define SC_DYNAMIC_PROCESSES_H

#include "sysc/kernel/sc_cmnhdr.h"
#include "sysc/kernel/sc_except.h"
#include "sysc/kernel/sc_spawn.h"
#include "sysc/kernel/sc_join.h"

#include <functional>

namespace sc_unnamed {
using namespace std::placeholders;
} // namespace sc_unnamed

namespace sc_core {

template<typename F, typename... Args>
auto sc_bind( F&& f, Args&&... args )
 -> decltype( std::bind( std::forward<F>(f), std::forward<Args>(args)... ) )
 { return std::bind( std::forward<F>(f), std::forward<Args>(args)... ); }

template<typename R, typename F, typename... Args>
auto sc_bind( F&& f, Args&&... args )
 -> decltype( std::bind<R>( std::forward<F>(f), std::forward<Args>(args)... ) )
 { return std::bind<R>( std::forward<F>(f), std::forward<Args>(args)... ); }

template<typename T>
auto sc_ref( T&& v ) noexcept
 { return std::ref( std::forward<T>(v) ); }

template<typename T>
void sc_ref( const T&& ) = delete;

template<typename T>
auto sc_cref( T&& v ) noexcept
 { return std::cref( std::forward<T>(v) ); }

template<typename T>
void sc_cref( const T&& ) = delete;

} // namespace sc_core

using sc_core::sc_bind;
using sc_core::sc_ref;
using sc_core::sc_cref;

// $Log: sc_dynamic_processes.h,v $
// Revision 1.5  2011/08/26 20:46:09  acg
//  Andy Goodrich: moved the modification log to the end of the file to
//  eliminate source line number skew when check-ins are done.
//
// Revision 1.4  2011/02/18 20:27:14  acg
//  Andy Goodrich: Updated Copyrights.
//
// Revision 1.3  2011/02/13 21:47:37  acg
//  Andy Goodrich: update copyright notice.
//
// Revision 1.2  2008/05/22 17:06:25  acg
//  Andy Goodrich: updated copyright notice to include 2008.
//
// Revision 1.1.1.1  2006/12/15 20:20:05  acg
// SystemC 2.3
//
// Revision 1.5  2006/04/20 17:08:16  acg
//  Andy Goodrich: 3.0 style process changes.
//
// Revision 1.4  2006/04/11 23:13:20  acg
//   Andy Goodrich: Changes for reduced reset support that only includes
//   sc_cthread, but has preliminary hooks for expanding to method and thread
//   processes also.
//
// Revision 1.3  2006/01/13 18:44:29  acg
// Added $Log to record CVS changes into the source.
//

#endif // SC_DYNAMIC_PROCESSES_H
