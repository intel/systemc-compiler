/******************************************************************************
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception.
 * 
 *****************************************************************************/

/**
 * Author: Roman Popov
 */

#ifndef SCTOOL_INSERTIONORDERSET_H
#define SCTOOL_INSERTIONORDERSET_H

#include <algorithm>
#include <deque>

namespace sc {

/// Set with elements sorted in insertion order
/// Warning :: Has iterator invalidation rules of std::deque
template <typename T>
class InsertionOrderSet : public std::deque<T>
{
public:
    using base_t = std::deque<T>;
    using typename base_t::iterator;

    bool count(const T& val) const {
        return std::find(this->cbegin(), this->cend(), val) != this->cend();
    }


    using base_t::erase;

    void erase(const T& val) {
        auto it = std::find(this->cbegin(), this->cend(), val);
        if (it != this->cend())
            this->erase(it);
    }
    
    template< class InputIt >
    void erase(InputIt first, InputIt last) {
        while (first != last) {
            erase(*first);
            ++first;
        }
    }

    void insert(const T &val) {
        if (!count(val))
            this->push_back(val);
    }

    template< class InputIt >
    void insert(InputIt first, InputIt last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

};

} // namespace sc

#endif //SCTOOL_INSERTIONORDERSET_H
