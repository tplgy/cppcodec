/**
 *  Copyright (C) 2015 Trustifier Inc.
 *  Copyright (C) 2015 Ahmed Masud
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
 *
 *  Originally from https://github.com/ahmed-masud/libbase32,
 *  commit 79761b2b79b0545697945efe0987a8d3004512f9.
 */

#ifndef CPPCODEC_DETAIL_BASE32
#define CPPCODEC_DETAIL_BASE32

#include <sstream>
#include <string.h>
#include <stdint.h>

#include "../parse_error.hpp"
#include "codec.hpp"

namespace cppcodec {
namespace detail {

static constexpr const char alphabet[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', /* 10 */
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',           /* 18 */
    'J', 'K',                                         /* 20 */
    'M', 'N',                                         /* 22 */
    'P', 'Q', 'R', 'S', 'T',                          /* 27 */
    'V', 'W', 'X', 'Y', 'Z'                           /* 32 */
};

class base32_crockstr
{
public:
    static inline bool is_whitespace(char c)
    {
        return c == '-'; // "Hyphens (-) can be inserted into strings [for readability]."
    }

    static inline uint8_t index_of(char c)
    {
        int idx;

        if (!c) {
            throw parse_error("parse error: encountered null character in encoded string");
        }

        // FIXME: support lower-case characters for decoding
        if (c >= '0' && c <= '9') {
            idx = c - '0';
        } else if (c >= 'A' && c <= 'H') {
            idx = c - 'A' + 10;         /* no I */
        } else if (c >= 'J' && c <= 'K') { /* no L */
            idx = c - 'J' + 18;
        } else if (c >= 'M' && c <= 'N') { /* no O */
            idx = c - 'M' + 20;
        } else if (c >= 'P' && c <= 'T') { /* no U */
            idx = c - 'P' + 22;
        } else if (c >= 'V' && c <= 'Z') {
            idx = c - 'V' + 27;
        } else if (is_whitespace(c)) {
            idx = -1;
        } else {
            std::ostringstream strstr;
            strstr << "parse error: character [" << static_cast<int>(c) << "] out of bounds";
            throw parse_error(strstr.str());
        }

#if defined(DEBUG)
        fprintf(stderr, "I think that index_of(%c) == %d whereas alphabet[%d] = %c\n", c, idx, idx, alphabet[idx]);
#endif
        return idx;
    }

    template <typename Result, typename ResultState> static void encode(
            Result& encoded_result, ResultState&, const unsigned char* binary, size_t binary_size);

    template <typename Result, typename ResultState> static void decode(
            Result& binary_result, ResultState&, const char* encoded, size_t encoded_size);

    size_t static encoded_size(size_t binary_size) noexcept;
    size_t static decoded_max_size(size_t encoded_size) noexcept;

private:
    template <typename Result, typename ResultState>
    static void encode_block(Result& encoded, ResultState&, const unsigned char* src);

    template <typename Result, typename ResultState> static void encode_tail(
            Result& encoded, ResultState&, const unsigned char* src, size_t src_len);

    template <typename Result, typename ResultState> static void decode_block(
            Result& decoded, ResultState& state, const unsigned char* idx);

    template <typename Result, typename ResultState> static void decode_tail(
            Result& decoded, ResultState& state, const unsigned char* idx, size_t idx_len);
};

//
//     11111111 10101010 10110011  10111100 10010100
// => 11111 11110 10101 01011 00111 01111 00100 10100
//

template <typename Result, typename ResultState>
inline void base32_crockstr::encode_block(
    Result& encoded, ResultState& state, const unsigned char* src)
{
    put(encoded, state, alphabet[(src[0] >> 3) & 0x1F]); // first 5 bits
    put(encoded, state, alphabet[(src[0] << 2) & 0x1C | ((src[1] >> 6) & 0x3)]); // last 3 + next 2 bits
    put(encoded, state, alphabet[(src[1] >> 1) & 0x1F]); // next 5 bits (tail has 1 bit)
    put(encoded, state, alphabet[(src[1] << 4) & 0x10 | ((src[2] >> 4) & 0xF)]); // last 1 + next 4 bits
    put(encoded, state, alphabet[(src[2] << 1) & 0x1E | ((src[3] >> 7) & 0x1)]);
    put(encoded, state, alphabet[(src[3] >> 2) & 0x1F]);
    put(encoded, state, alphabet[(src[3] << 3) & 0x18 | ((src[4] >> 5) & 0x7)]);
    put(encoded, state, alphabet[(src[4] & 0x1F)]);
}

template <typename Result, typename ResultState>
inline void base32_crockstr::encode_tail(
        Result& encoded, ResultState& state, const unsigned char* src, size_t src_len)
{
    if (!src_len || src_len > 5) {
        abort();
        return;
    }

    put(encoded, state, alphabet[(src[0] >> 3) & 0x1F]); // encoded size 1
    if (src_len == 1) {
        put(encoded, state, alphabet[(src[0] << 2) & 0x1C]); // size 2
        return;
    }
    put(encoded, state, alphabet[(src[0] << 2) & 0x1C | (src[1] >> 6) & 0x3]); // size 2
    put(encoded, state, alphabet[(src[1] >> 1) & 0x1F]); // size 3
    if (src_len == 2) {
        put(encoded, state, alphabet[(src[1] << 4) & 0x10]); // size 4
        return;
    }
    put(encoded, state, alphabet[(src[1] << 4) & 0x10 | (src[2] >> 4) & 0xF]); // size 4
    if (src_len == 3) {
        put(encoded, state, alphabet[(src[2] << 1) & 0x1E]); // size 5
        return;
    }
    put(encoded, state, alphabet[(src[2] << 1) & 0x1E | (src[3] >> 7) & 0x1]); // size 5
    put(encoded, state, alphabet[(src[3] >> 2) & 0x1F]); // size 6
    if (src_len == 4) {
        put(encoded, state, alphabet[(src[3] << 3) & 0x18]); // size 7
        return;
    }
    // src_len == 5
    put(encoded, state, alphabet[(src[3] << 3) & 0x18 | (src[4] >> 5) & 0x7]); // size 7
    put(encoded, state, alphabet[(src[4]) & 0x1F]); // size 8
}

template <typename Result, typename ResultState>
inline void base32_crockstr::encode(
        Result& encoded_result, ResultState& state,
        const unsigned char* src, size_t src_size)
{
    const unsigned char* src_end = src + src_size - 5;

    for (; src <= src_end; src += 5) {
        encode_block(encoded_result, state, src);
    }
    src_end += 5;

    if (src_end > src) {
        encode_tail(encoded_result, state, src, src_end - src);
    }
}

template <typename Result, typename ResultState>
inline void base32_crockstr::decode_block(
        Result& decoded, ResultState& state, const unsigned char* idx)
{
    put(decoded, state, (uint8_t)(((idx[0] << 3) & 0xF8) | ((idx[1] >> 2) & 0x7)));
    put(decoded, state, (uint8_t)(((idx[1] << 6) & 0xC0) | ((idx[2] << 1) & 0x3E) | ((idx[3] >> 4) & 0x1)));
    put(decoded, state, (uint8_t)(((idx[3] << 4) & 0xF0) | ((idx[4] >> 1) & 0xF)));
    put(decoded, state, (uint8_t)(((idx[4] << 7) & 0x80) | ((idx[5] << 2) & 0x7C) | ((idx[6] >> 3) & 0x3)));
    put(decoded, state, (uint8_t)(((idx[6] << 5) & 0xE0) | (idx[7] & 0x1F)));
}

template <typename Result, typename ResultState>
inline void base32_crockstr::decode_tail(
        Result& decoded, ResultState& state, const unsigned char* idx, size_t idx_len)
{
    if (!idx_len || idx_len > 7) {
        abort();
        return;
    }

    if (idx_len == 1) {
        put(decoded, state, (uint8_t)((idx[0] << 3) & 0xF8)); // decoded size 1
        return;
    }
    put(decoded, state, (uint8_t)(((idx[0] << 3) & 0xF8) | ((idx[1] >> 2) & 0x7))); // size 1

    if (idx_len == 2) {
        put(decoded, state, (uint8_t)((idx[1] << 6) & 0xC0)); // size 2
        return;
    }
    if (idx_len == 3) {
        put(decoded, state, (uint8_t)(((idx[1] << 6) & 0xC0) | ((idx[2] << 1) & 0x3E))); // size 2
        return;
    }
    put(decoded, state, (uint8_t)(((idx[1] << 6) & 0xC0) | ((idx[2] << 1) & 0x3E) | ((idx[3] >> 4) & 0x1))); // size 2

    if (idx_len == 4) {
        put(decoded, state, (uint8_t)((idx[3] << 4) & 0xF0)); // size 3
        return;
    }
    put(decoded, state, (uint8_t)(((idx[3] << 4) & 0xF0) | ((idx[4] >> 1) & 0xF))); // size 3

    if (idx_len == 5) {
        put(decoded, state, (uint8_t)((idx[4] << 7) & 0x80)); // size 4
        return;
    }
    if (idx_len == 6) {
        put(decoded, state, (uint8_t)(((idx[4] << 7) & 0x80) | ((idx[5] << 2) & 0x7C))); // size 4
        return;
    }
    // idx_len == 7
    put(decoded, state, (uint8_t)(((idx[4] << 7) & 0x80) | ((idx[5] << 2) & 0x7C) | ((idx[6] >> 3) & 0x3))); // size 4
    put(decoded, state, (uint8_t)((idx[6] << 5) & 0xE0)); // size 5
}

template <typename Result, typename ResultState>
inline void base32_crockstr::decode(
        Result& binary_result, ResultState& state,
        const char* src_encoded, size_t src_size)
{
    const char* src = src_encoded;
    const char* src_end = src + src_size;

    uint8_t idx[8] = {};
    uint8_t idx_len = 0;

    while (src < src_end) {
        if ((idx[idx_len] = index_of(*(src++))) != -1) { // ignore whitespace
            ++idx_len;
            if (idx_len == 8) {
                decode_block(binary_result, state, idx);
                idx_len = 0;
            }
        }
    }

    if (idx_len)  {
        decode_tail(binary_result, state, idx, idx_len);
    }
}

inline size_t base32_crockstr::encoded_size(size_t binary_size) noexcept
{
    // TODO: adapt size to take tail into consideration
    size_t size = binary_size * 8;
    return (size / 5) + ((size % 5) ? 1 : 0);
}

inline size_t base32_crockstr::decoded_max_size(size_t encoded_size) noexcept
{
    // TODO: adapt size to take tail into consideration
    size_t size = encoded_size * 5;
    return (size / 8) + ((size % 8) ? 1 : 0);
}

} // namespace detail
} // namespace cppcodec

#endif // CPPCODEC_DETAIL_BASE32
