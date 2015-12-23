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

#ifndef CPPCODEC_PARSE_ERROR
#define CPPCODEC_PARSE_ERROR

#include <stdexcept>
#include <sstream>

namespace cppcodec {

class parse_error : public std::domain_error
{
public:
    using std::domain_error::domain_error;
};

// Avoids memory allocation, so it can be used in constexpr functions.
class symbol_error : public parse_error
{
public:
    symbol_error(char c)
        : parse_error(static_cast<std::ostringstream&>(std::ostringstream() << "parse error: "
                "character [" << static_cast<int>(c) << " '" << c << "'] out of bounds").str())
        , m_symbol(c)
    {
    }

    symbol_error(const symbol_error&) = default;

    char symbol() const noexcept { return m_symbol; }

private:
    char m_symbol;
};

class padding_error : public parse_error
{
public:
    padding_error()
        : parse_error("parse error: codec expects padded input string but padding was invalid")
    {
    }

    padding_error(const padding_error&) = default;
};

} // namespace cppcodec

#endif // CPPCODEC_PARSE_ERROR
