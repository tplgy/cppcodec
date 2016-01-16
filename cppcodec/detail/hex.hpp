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

#ifndef CPPCODEC_DETAIL_HEX
#define CPPCODEC_DETAIL_HEX

#include <stdint.h>

#include "../data/access.hpp"
#include "../parse_error.hpp"
#include "stream_codec.hpp"

namespace cppcodec {
namespace detail {

class hex_base
{
public:
    static inline constexpr uint8_t index_of(char c)
    {
        // Hex decoding is always case-insensitive (even in RFC 4648),
        // it's only a question of what we want to encode ourselves.
        return (c >= '0' && c <= '9') ? (c - '0')
                : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
                : (c == '\0') ? 255 // stop at end of string
                : throw symbol_error(c);
    }

    // RFC4648 does not specify any whitespace being allowed in base64 encodings.
    static inline constexpr bool should_ignore(uint8_t /*index*/) { return false; }
    static inline constexpr bool is_special_character(uint8_t index) { return index > 64; }
    static inline constexpr bool is_eof(uint8_t index) { return index == 255; }

    static inline constexpr bool generates_padding() { return false; }
    // FIXME: doesn't require padding, but requires a multiple of the encoded block size (2)
    static inline constexpr bool requires_padding() { return false; }
    static inline constexpr bool is_padding_symbol(uint8_t /*index*/) { return false; }
};

template <typename CodecVariant>
class hex : public CodecVariant::template codec_impl<hex<CodecVariant>>
{
public:
    static inline constexpr uint8_t binary_block_size() { return 1; }
    static inline constexpr uint8_t encoded_block_size() { return 2; }

    template <typename Result, typename ResultState> static void encode_block(
            Result& encoded, ResultState&, const uint8_t* src);

    template <typename Result, typename ResultState> static void encode_tail(
            Result& encoded, ResultState&, const uint8_t* src, size_t src_len);

    template <typename Result, typename ResultState> static void pad(Result&, ResultState&, size_t) { }

    template <typename Result, typename ResultState> static void decode_block(
            Result& decoded, ResultState&, const uint8_t* idx);

    template <typename Result, typename ResultState> static void decode_tail(
            Result& decoded, ResultState&, const uint8_t* idx, size_t idx_len);
};


template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void hex<CodecVariant>::encode_block(Result& encoded, ResultState& state, const uint8_t* src)
{
    using V = CodecVariant;
    data::put(encoded, state, V::symbol(src[0] >> 4)); // first 4 bits
    data::put(encoded, state, V::symbol(src[0] & 0xF)); // last 4 bits
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void hex<CodecVariant>::encode_tail(Result&, ResultState&, const uint8_t*, size_t)
{
    // Octet-streaming hex always expands to two symbols per input byte.
    // In order to decode odd-length hex numbers such as 0xF, 0x1a5, etc.,
    // we'll want a place-based single number codec rather than a stream codec.
    // That's a different beast and requires an encode_head(), rather than encode_tail().
    abort();
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void hex<CodecVariant>::decode_block(Result& decoded, ResultState& state, const uint8_t* idx)
{
    data::put(decoded, state, (uint8_t)((idx[0] << 4) | idx[1]));
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void hex<CodecVariant>::decode_tail(Result&, ResultState&, const uint8_t*, size_t)
{
    throw invalid_input_length(
            "odd-length hex input is not supported by the streaming octet decoder, "
            "use a place-based number decoder instead");
}

} // namespace detail
} // namespace cppcodec

#endif // CPPCODEC_DETAIL_HEX
