//
// Created by ripopov on 12/18/17.
//

#ifndef ELAB_ALLOC_H
#define ELAB_ALLOC_H

#include <sc_elab/allocated_node.h>

#include <cstdint>
#include <typeinfo>

#ifndef _MSC_VER

template<typename T> std::string MANGLED_TYPENAME() {
    return typeid(T).name();
}

template<typename T> std::string MANGLED_TYPENAME(T &&var) {
    return typeid(var).name();
}

#else

inline std::string adjust_msvc_rawname (const char *rawname)
{
    std::string name = (rawname + 1);
    if (name[0] == '?')
        return name;
    else
        return "?A" + name;
}

template<typename T> std::string MANGLED_TYPENAME() {
    return adjust_msvc_rawname(typeid(T).raw_name());
}

template<typename T> std::string MANGLED_TYPENAME(T &&var) {
    return adjust_msvc_rawname(typeid(var).raw_name());
}


#endif // !_MSC_VER

/**
 *  Dynamic memory allocation tracking for SystemC elaboration
 *
 *  Currently elaborator supports following allocation methods:
 *
 *  1.
 *  template<class T, class... Args>
 *  T* sc_new(Args&&... args)
 *
 *  usage example:  my_module = sc_new<my_module> ("my_module");
 *
 *  2.
 *  template<class T>
 *  T* sc_new_array(size_t n)
 *
 *  usage example:   int * array_ptr = sc_new_array<int> (10);  // create int[10]
 *
 *  3.
 *  All classes derived from sc_objects can use raw new, it is overloaded in
 *  sc_object
 *
 */
namespace sc_elab {

///
/// Store information about dynamically allocated object
/// @param ptr - pointer to newly allocated object
/// @param is_array - true for new[]
/// @param type_name - mangled type name
/// @param sizeof_alloc - size of object
/// @param array_size - N, number of array elements for new[N]
///
void trace_alloc_impl(void *ptr,
                      bool is_array,
                      const char *type_name,
                      size_t sizeof_alloc,
                      size_t array_size = 0);


}

namespace sc_core
{

/// Allocate singular object dynamically and store information for SVC elaboration
template<class T, class... Args>
T* sc_new(Args&&... args)
{
    T* rptr = ::new T(std::forward<Args>(args)...);
    sc_elab::trace_alloc_impl(rptr, false, MANGLED_TYPENAME<T>().c_str() , sizeof(T));
    return rptr;
}

/// Allocate array dynamically and store information for SVC elaboration
template<class T>
T* sc_new_array(size_t n)
{
    T* rptr = ::new T[n];
    if (n > 0)
        sc_elab::trace_alloc_impl(rptr, true, MANGLED_TYPENAME<T>().c_str(), sizeof(T)*n, n);
    return rptr;
}

}

#endif // ELAB_ALLOC_H