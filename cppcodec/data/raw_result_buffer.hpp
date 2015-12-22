/**
 *  Copyright (C) 2015 Topology LP
 *  All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#ifndef CPPCODEC_DETAIL_RAW_RESULT_BUFFER
#define CPPCODEC_DETAIL_RAW_RESULT_BUFFER

#include <stdexcept>
#include <stdint.h> // for size_t
#include <stdlib.h> // for abort()

#include "access.hpp"

namespace cppcodec {
namespace data {

class raw_result_buffer
{
public:
    char last() const { return *(m_ptr - 1); }
    void push_back(char c) { *m_ptr = c; ++m_ptr; }
    size_t size() const { return m_size; }

private:
    char* m_ptr;
    size_t m_size;
};


template <> inline void init<raw_result_buffer>(
        raw_result_buffer& result, empty_result_state&, size_t capacity)
{
    if (capacity > result.size()) {
        abort();
    }
}
template <> inline void finish<raw_result_buffer>(raw_result_buffer& result, empty_result_state&) { }

} // namespace data
} // namespace cppcodec

#endif
