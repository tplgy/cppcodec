/**
 *  Copyright (C) 2015 Topology LP
 *  Copyright (C) 2013 Adam Rudd (bit calculations)
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
 *  Bit calculations adapted from https://github.com/adamvr/arduino-base64,
 *  commit 999595783185a0afcba156d7276dfeaa9cb5382f.
 */

#ifndef CPPCODEC_DETAIL_BASE64
#define CPPCODEC_DETAIL_BASE64

#include <stdint.h>
#include <type_traits>

#include "../data/access.hpp"
#include "../parse_error.hpp"
#include "stream_codec.hpp"

namespace cppcodec {
namespace detail {

static constexpr const char base64_rfc4648_alphabet[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};
static_assert(sizeof(base64_rfc4648_alphabet) == 64, "base64 alphabet must have 64 values");

class base64_rfc4648
{
public:
    template <typename Codec> using codec_impl = stream_codec<Codec, base64_rfc4648>;

    static inline constexpr bool generates_padding() { return true; }
    static inline constexpr bool requires_padding() { return true; }
    static inline constexpr char padding_symbol() { return '='; }

    static inline constexpr char symbol(uint8_t index)
    {
        return base64_rfc4648_alphabet[index];
    }

    static inline constexpr uint8_t index_of(char c)
    {
        return (c >= 'A' && c <= 'Z') ? (c - 'A')
                : (c >= 'a' && c <= 'z') ? (c - 'a' + 26)
                : (c >= '0' && c <= '9') ? (c - '0' + 52)
                : (c == '+') ? (c - '+' + 62)
                : (c == '/') ? (c - '/' + 63)
                : (c == padding_symbol()) ? 254
                : (c == '\0') ? 255 // stop at end of string
                : throw symbol_error(c);
    }

    // RFC4648 does not specify any whitespace being allowed in base64 encodings.
    static inline constexpr bool should_ignore(uint8_t /*index*/) { return false; }
    static inline constexpr bool is_special_character(uint8_t index) { return index > 64; }
    static inline constexpr bool is_padding_symbol(uint8_t index) { return index == 254; }
    static inline constexpr bool is_eof(uint8_t index) { return index == 255; }
};

// The URL and filename safe alphabet is also specified by RFC4648, named "base64url".
// We keep the underscore ("base64_url") for consistency with the other codec variants.
static constexpr const char base64_url_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};
static_assert(sizeof(base64_url_alphabet) == 64, "base64 alphabet must have 64 values");

class base64_url : public base64_rfc4648
{
public:
    template <typename Codec> using codec_impl = stream_codec<Codec, base64_url>;

    static inline constexpr char symbol(uint8_t index)
    {
        return base64_url_alphabet[index];
    }

    static inline constexpr uint8_t index_of(char c)
    {
        return (c >= 'A' && c <= 'Z') ? (c - 'A')
                : (c >= 'a' && c <= 'z') ? (c - 'a' + 26)
                : (c >= '0' && c <= '9') ? (c - '0' + 52)
                : (c == '-') ? (c - '-' + 62)
                : (c == '_') ? (c - '_' + 63)
                : (c == padding_symbol()) ? 254
                : (c == '\0') ? 255 // stop at end of string
                : throw symbol_error(c);
    }
};

template <typename CodecVariant>
class base64 : public CodecVariant::template codec_impl<base64<CodecVariant>>
{
public:
    static inline constexpr uint8_t binary_block_size() { return 3; }
    static inline constexpr uint8_t encoded_block_size() { return 4; }

    template <typename Result, typename ResultState> static void encode_block(
            Result& encoded, ResultState&, const uint8_t* src);

    template <typename Result, typename ResultState> static void encode_tail(
            Result& encoded, ResultState&, const uint8_t* src, size_t src_len);

    template <typename Result, typename ResultState, typename V = CodecVariant> static void pad(
            Result& encoded, ResultState&,
            typename std::enable_if<V::generates_padding(), size_t>::type remaining_src_len);

    template <typename Result, typename ResultState, typename V = CodecVariant> static void pad(
            Result&, ResultState&, typename std::enable_if<!V::generates_padding(), size_t>::type)
    {
    }

    template <typename Result, typename ResultState> static void decode_block(
            Result& decoded, ResultState&, const uint8_t* idx);

    template <typename Result, typename ResultState> static void decode_tail(
            Result& decoded, ResultState&, const uint8_t* idx, size_t idx_len);
};


template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base64<CodecVariant>::encode_block(
    Result& encoded, ResultState& state, const uint8_t* src)
{
    using V = CodecVariant;
    data::put(encoded, state, V::symbol(src[0] >> 2)); // first 6 bits
    data::put(encoded, state, V::symbol(((src[0] & 0x3) << 4) + (src[1] >> 4))); // last 2 + next 4
    data::put(encoded, state, V::symbol(((src[1] & 0xF) << 2) + (src[2] >> 6))); // last 4 + next 2
    data::put(encoded, state, V::symbol(src[2] & 0x3F)); // last 6 bits
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base64<CodecVariant>::encode_tail(
        Result& encoded, ResultState& state, const uint8_t* src, size_t remaining_src_len)
{
    using V = CodecVariant;

    data::put(encoded, state, V::symbol(src[0] >> 2)); // encoded size 1
    if (remaining_src_len == 1) {
        data::put(encoded, state, V::symbol((src[0] & 0x03) << 4)); // size 2
        return;
    }
    data::put(encoded, state, V::symbol(((src[0] & 0x03) << 4) + (src[1] >> 4))); // size 2
    if (remaining_src_len == 2) {
        data::put(encoded, state, V::symbol((src[1] & 0x0f) << 2)); // size 3
        return;
    }
    abort(); // not reached: encode_block() should be called if remaining_src_len > 2, not this function
}

template <typename CodecVariant>
template <typename Result, typename ResultState, typename V>
inline void base64<CodecVariant>::pad(
        Result& encoded, ResultState& state,
        typename std::enable_if<V::generates_padding(), size_t>::type remaining_src_len)
{
    switch (remaining_src_len) {
    case 1: // 2 symbols, 2 padding characters
        data::put(encoded, state, CodecVariant::padding_symbol());
    case 2: // 3 symbols, 1 padding character
        data::put(encoded, state, CodecVariant::padding_symbol());
    }
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base64<CodecVariant>::decode_block(
        Result& decoded, ResultState& state, const uint8_t* idx)
{
    data::put(decoded, state, (uint8_t)((idx[0] << 2) + ((idx[1] & 0x30) >> 4)));
    data::put(decoded, state, (uint8_t)(((idx[1] & 0xF) << 4) + ((idx[2] & 0x3C) >> 2)));
    data::put(decoded, state, (uint8_t)(((idx[2] & 0x3) << 6) + idx[3]));
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base64<CodecVariant>::decode_tail(
        Result& decoded, ResultState& state, const uint8_t* idx, size_t idx_len)
{
    if (idx_len == 1) {
        throw invalid_input_length(
                "invalid number of symbols in last base64 block: found 1, expected 2 or 3");
    }

    // idx_len == 2: decoded size 1
    data::put(decoded, state, (uint8_t)((idx[0] << 2) + ((idx[1] & 0x30) >> 4)));
    if (idx_len == 2) {
        return;
    }

    // idx_len == 3: decoded size 2
    data::put(decoded, state, (uint8_t)(((idx[1] & 0xF) << 4) + ((idx[2] & 0x3C) >> 2)));
}

} // namespace detail
} // namespace cppcodec

#endif // CPPCODEC_DETAIL_BASE64
