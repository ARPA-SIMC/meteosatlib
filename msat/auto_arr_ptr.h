/*
 * Copyright (C) 2013  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#ifndef MSAT_AUTOARRPTR_H
#define MSAT_AUTOARRPTR_H

#include <cstddef>

namespace msat {

/**
 * auto_ptr style smart pointer, which automatically allocates and delete[]s
 * arrays.
 *
 * auto_ptr-stype copy semantics not implemented yet, as they have not been
 * needed so far.
 */
template<typename T>
class auto_arr_ptr
{
protected:
    T* ptr;

public:
    auto_arr_ptr() : ptr(0) {}
    auto_arr_ptr(size_t size) : ptr(0) {
        ptr = new T[size];
    }
    ~auto_arr_ptr()
    {
        if (ptr)
            delete[] ptr;
    }

    T* get() { return ptr; }
    const T* get() const { return ptr; }

    T* release()
    {
        T* res = ptr;
        ptr = 0;
        return res;
    }

    const T& operator[](size_t pos) const { return ptr[pos]; }
    T& operator[](size_t pos) { return ptr[pos]; }

private:
    // Disable copy
    auto_arr_ptr(const auto_arr_ptr<T>&);
    auto_arr_ptr<T>& operator=(const auto_arr_ptr<T>&);
};

}

#endif
