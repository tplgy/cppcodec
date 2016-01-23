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
 *  Adapted from https://github.com/ahmed-masud/libbase32,
 *  commit 79761b2b79b0545697945efe0987a8d3004512f9.
 *  Quite different now.
 */

#ifndef CPPCODEC_DETAIL_BASE32
#define CPPCODEC_DETAIL_BASE32

#include <stdint.h>
#include <stdlib.h> // for abort()

#include "../data/access.hpp"
#include "../parse_error.hpp"
#include "stream_codec.hpp"

namespace cppcodec {
namespace detail {

template <typename CodecVariant>
class base32 : public CodecVariant::template codec_impl<base32<CodecVariant>>
{
public:
    static inline constexpr uint8_t binary_block_size() { return 5; }
    static inline constexpr uint8_t encoded_block_size() { return 8; }

    template <typename Result, typename ResultState>
    static void encode_block(Result& encoded, ResultState&, const uint8_t* src);

    template <typename Result, typename ResultState>
    static void encode_tail(Result& encoded, ResultState&, const uint8_t* src, size_t src_len);

    template <typename Result, typename ResultState>
    static void pad(Result&, ResultState&, ...) { } // lower priority overload

    template <typename Result, typename ResultState, typename V = CodecVariant,
            typename std::enable_if<V::generates_padding()>::type* = nullptr>
    static void pad(Result& encoded, ResultState&, size_t remaining_src_len);

    template <typename Result, typename ResultState>
    static void decode_block(Result& decoded, ResultState&, const uint8_t* idx);

    template <typename Result, typename ResultState>
    static void decode_tail(Result& decoded, ResultState&, const uint8_t* idx, size_t idx_len);
};

//
//     11111111 10101010 10110011  10111100 10010100
// => 11111 11110 10101 01011 00111 01111 00100 10100
//

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::encode_block(
        Result& encoded, ResultState& state, const uint8_t* src)
{
    using V = CodecVariant;
    data::put(encoded, state, V::symbol((src[0] >> 3) & 0x1F)); // first 5 bits
    data::put(encoded, state, V::symbol(((src[0] << 2) & 0x1C) | ((src[1] >> 6) & 0x3))); // last 3 + next 2
    data::put(encoded, state, V::symbol((src[1] >> 1) & 0x1F)); // next 5 bits (tail has 1 bit)
    data::put(encoded, state, V::symbol(((src[1] << 4) & 0x10) | ((src[2] >> 4) & 0xF))); // last 1 + next 4
    data::put(encoded, state, V::symbol(((src[2] << 1) & 0x1E) | ((src[3] >> 7) & 0x1)));
    data::put(encoded, state, V::symbol((src[3] >> 2) & 0x1F));
    data::put(encoded, state, V::symbol(((src[3] << 3) & 0x18) | ((src[4] >> 5) & 0x7)));
    data::put(encoded, state, V::symbol((src[4] & 0x1F)));
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::encode_tail(
        Result& encoded, ResultState& state, const uint8_t* src, size_t remaining_src_len)
{
    using V = CodecVariant;

    data::put(encoded, state, V::symbol((src[0] >> 3) & 0x1F)); // encoded size 1
    if (remaining_src_len == 1) {
        data::put(encoded, state, V::symbol((src[0] << 2) & 0x1C)); // size 2
        return;
    }
    data::put(encoded, state, V::symbol(((src[0] << 2) & 0x1C) | ((src[1] >> 6) & 0x3))); // size 2
    data::put(encoded, state, V::symbol((src[1] >> 1) & 0x1F)); // size 3
    if (remaining_src_len == 2) {
        data::put(encoded, state, V::symbol((src[1] << 4) & 0x10)); // size 4
        return;
    }
    data::put(encoded, state, V::symbol(((src[1] << 4) & 0x10) | ((src[2] >> 4) & 0xF))); // size 4
    if (remaining_src_len == 3) {
        data::put(encoded, state, V::symbol((src[2] << 1) & 0x1E)); // size 5
        return;
    }
    data::put(encoded, state, V::symbol(((src[2] << 1) & 0x1E) | ((src[3] >> 7) & 0x1))); // size 5
    data::put(encoded, state, V::symbol((src[3] >> 2) & 0x1F)); // size 6
    if (remaining_src_len == 4) {
        data::put(encoded, state, V::symbol((src[3] << 3) & 0x18)); // size 7
        return;
    }
    abort(); // not reached: encode_block() should be called if remaining_src_len > 4, not this function
}

template <typename CodecVariant>
template <typename Result, typename ResultState, typename V,
        typename std::enable_if<V::generates_padding()>::type*>
inline void base32<CodecVariant>::pad(
        Result& encoded, ResultState& state, size_t remaining_src_len)
{
    switch (remaining_src_len) {
    case 1: // 2 symbols, 6 padding characters
        data::put(encoded, state, CodecVariant::padding_symbol());
        data::put(encoded, state, CodecVariant::padding_symbol());
    case 2: // 4 symbols, 4 padding characters
        data::put(encoded, state, CodecVariant::padding_symbol());
    case 3: // 5 symbols, 3 padding characters
        data::put(encoded, state, CodecVariant::padding_symbol());
        data::put(encoded, state, CodecVariant::padding_symbol());
    case 4: // 7 symbols, 1 padding character
        data::put(encoded, state, CodecVariant::padding_symbol());
    }
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::decode_block(
        Result& decoded, ResultState& state, const uint8_t* idx)
{
    put(decoded, state, (uint8_t)(((idx[0] << 3) & 0xF8) | ((idx[1] >> 2) & 0x7)));
    put(decoded, state, (uint8_t)(((idx[1] << 6) & 0xC0) | ((idx[2] << 1) & 0x3E) | ((idx[3] >> 4) & 0x1)));
    put(decoded, state, (uint8_t)(((idx[3] << 4) & 0xF0) | ((idx[4] >> 1) & 0xF)));
    put(decoded, state, (uint8_t)(((idx[4] << 7) & 0x80) | ((idx[5] << 2) & 0x7C) | ((idx[6] >> 3) & 0x3)));
    put(decoded, state, (uint8_t)(((idx[6] << 5) & 0xE0) | (idx[7] & 0x1F)));
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::decode_tail(
        Result& decoded, ResultState& state, const uint8_t* idx, size_t idx_len)
{
    if (idx_len == 1) {
        throw invalid_input_length(
                "invalid number of symbols in last base32 block: found 1, expected 2, 4, 5 or 7");
    }
    if (idx_len == 3) {
        throw invalid_input_length(
                "invalid number of symbols in last base32 block: found 3, expected 2, 4, 5 or 7");
    }
    if (idx_len == 6) {
        throw invalid_input_length(
                "invalid number of symbols in last base32 block: found 6, expected 2, 4, 5 or 7");
    }

    // idx_len == 2: decoded size 1
    put(decoded, state, (uint8_t)(((idx[0] << 3) & 0xF8) | ((idx[1] >> 2) & 0x7)));
    if (idx_len == 2) {
        return;
    }
    // idx_len == 4: decoded size 2
    put(decoded, state, (uint8_t)(((idx[1] << 6) & 0xC0) | ((idx[2] << 1) & 0x3E) | ((idx[3] >> 4) & 0x1)));
    if (idx_len == 4) {
        return;
    }
    // idx_len == 5: decoded size 3
    put(decoded, state, (uint8_t)(((idx[3] << 4) & 0xF0) | ((idx[4] >> 1) & 0xF)));
    if (idx_len == 5) {
        return;
    }
    // idx_len == 7: decoded size 4
    put(decoded, state, (uint8_t)(((idx[4] << 7) & 0x80) | ((idx[5] << 2) & 0x7C) | ((idx[6] >> 3) & 0x3)));
}

} // namespace detail
} // namespace cppcodec

#endif // CPPCODEC_DETAIL_BASE32
