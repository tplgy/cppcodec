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

#include <sstream>
#include <string.h>
#include <stdint.h>
#include <type_traits>

#include "../parse_error.hpp"
#include "codec.hpp"

namespace cppcodec {
namespace detail {

static constexpr const char base32_crockford_alphabet[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // at index 10
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',           // 18 - no I
    'J', 'K',                                         // 20 - no L
    'M', 'N',                                         // 22 - no O
    'P', 'Q', 'R', 'S', 'T',                          // 27 - no U
    'V', 'W', 'X', 'Y', 'Z'                           // 32
};

class base32_crockford_base
{
public:
    static inline constexpr bool generates_padding() { return false; }
    static inline constexpr bool requires_padding() { return false; }
    static inline constexpr bool is_padding_symbol(char /*c*/) { return false; }

    static inline constexpr char symbol(uint8_t index)
    {
        return base32_crockford_alphabet[index];
    }

    static inline constexpr uint8_t index_of(char c)
    {
        return (c >= '0' && c <= '9') ? (c - '0')
                // upper-case letters
                : (c >= 'A' && c <= 'H') ? (c - 'A' + 10) // no I
                : (c >= 'J' && c <= 'K') ? (c - 'J' + 18) // no L
                : (c >= 'M' && c <= 'N') ? (c - 'M' + 20) // no O
                : (c >= 'P' && c <= 'T') ? (c - 'P' + 22) // no U
                : (c >= 'V' && c <= 'Z') ? (c - 'V' + 27)
                // lower-case letters
                : (c >= 'a' && c <= 'h') ? (c - 'a' + 10) // no I
                : (c >= 'j' && c <= 'k') ? (c - 'j' + 18) // no L
                : (c >= 'm' && c <= 'n') ? (c - 'm' + 20) // no O
                : (c >= 'p' && c <= 't') ? (c - 'p' + 22) // no U
                : (c >= 'v' && c <= 'z') ? (c - 'v' + 27)
                : (c == '-') ? -1 // "Hyphens (-) can be inserted into strings [for readability]."
                : (c == '\0') ? -2 // stop at end of string
                : throw symbol_error(c);
    }

    static inline constexpr bool should_ignore(uint8_t index) { return index == -1; }
    static inline constexpr bool is_special_character(uint8_t index) { return index > 32; }
    static inline constexpr bool is_eof(uint8_t index) { return index == -2; }
};

// base32_crockstr is a concatenative iterative (i.e. streaming) interpretation of Crockford base32.
// It interprets the statement "zero-extend the number to make its bit-length a multiple of 5"
// to mean zero-extending it on the right.
// (The other possible interpretation is base32_crocknum, a place-based single number encoding system.
// See http://merrigrove.blogspot.ca/2014/04/what-heck-is-base64-encoding-really.html for more info.)
class base32_crockstr : public base32_crockford_base
{
};

// RFC 4648 uses a simple alphabet: A-Z starting at index 0, then 2-7 starting at index 26.
static constexpr const char base32_rfc4648_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

class base32_rfc4648
{
public:
    static inline constexpr bool generates_padding() { return true; }
    static inline constexpr bool requires_padding() { return true; }
    static inline constexpr char padding_symbol() { return '='; }

    static inline constexpr char symbol(uint8_t index)
    {
        return base32_rfc4648_alphabet[index];
    }

    static inline constexpr uint8_t index_of(char c)
    {
        return (c >= 'A' && c <= 'Z') ? (c - 'A')
                : (c >= '2' && c <= '7') ? (c - '2' + 26)
                : (c == padding_symbol()) ? -3
                : (c == '\0') ? -2 // stop at end of string
                : throw symbol_error(c);
    }

    // RFC4648 does not specify any whitespace being allowed in base32 encodings.
    static inline constexpr bool should_ignore(char /*index*/) { return false; }
    static inline constexpr bool is_special_character(uint8_t index) { return index > 32; }
    static inline constexpr bool is_eof(uint8_t index) { return index == -2; }
    static inline constexpr bool is_padding_symbol(uint8_t index) { return index == -3; }
};

template <typename CodecVariant>
class base32
{
public:
    template <typename Result, typename ResultState> static void encode(
            Result& encoded_result, ResultState&, const unsigned char* binary, size_t binary_size);

    template <typename Result, typename ResultState> static void decode(
            Result& binary_result, ResultState&, const char* encoded, size_t encoded_size);

    static constexpr size_t encoded_size(size_t binary_size) noexcept;
    static constexpr size_t decoded_max_size(size_t encoded_size) noexcept;

    static inline constexpr uint8_t binary_block_size() { return 5; }
    static inline constexpr uint8_t encoded_block_size() { return 8; }

private:
    template <typename Result, typename ResultState> static void encode_block(
            Result& encoded, ResultState&, const unsigned char* src);

    template <typename Result, typename ResultState> static void encode_tail(
            Result& encoded, ResultState&, const unsigned char* src, size_t src_len);

    template <typename Result, typename ResultState, typename V = CodecVariant> static void pad(
            Result& encoded, ResultState&,
            typename std::enable_if<V::generates_padding(), size_t>::type remaining_src_len);

    template <typename Result, typename ResultState, typename V = CodecVariant> static void pad(
            Result&, ResultState&, typename std::enable_if<!V::generates_padding(), size_t>::type)
    {
    }

    template <typename Result, typename ResultState> static void decode_block(
            Result& decoded, ResultState&, const unsigned char* idx);

    template <typename Result, typename ResultState> static void decode_tail(
            Result& decoded, ResultState&, const unsigned char* idx, size_t idx_len);
};

//
//     11111111 10101010 10110011  10111100 10010100
// => 11111 11110 10101 01011 00111 01111 00100 10100
//

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::encode_block(
    Result& encoded, ResultState& state, const unsigned char* src)
{
    using V = CodecVariant;
    data::put(encoded, state, V::symbol((src[0] >> 3) & 0x1F)); // first 5 bits
    data::put(encoded, state, V::symbol((src[0] << 2) & 0x1C | ((src[1] >> 6) & 0x3))); // last 3 + next 2
    data::put(encoded, state, V::symbol((src[1] >> 1) & 0x1F)); // next 5 bits (tail has 1 bit)
    data::put(encoded, state, V::symbol((src[1] << 4) & 0x10 | ((src[2] >> 4) & 0xF))); // last 1 + next 4
    data::put(encoded, state, V::symbol((src[2] << 1) & 0x1E | ((src[3] >> 7) & 0x1)));
    data::put(encoded, state, V::symbol((src[3] >> 2) & 0x1F));
    data::put(encoded, state, V::symbol((src[3] << 3) & 0x18 | ((src[4] >> 5) & 0x7)));
    data::put(encoded, state, V::symbol((src[4] & 0x1F)));
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::encode_tail(
        Result& encoded, ResultState& state, const unsigned char* src, size_t remaining_src_len)
{
    using V = CodecVariant;

    data::put(encoded, state, V::symbol((src[0] >> 3) & 0x1F)); // encoded size 1
    if (remaining_src_len == 1) {
        data::put(encoded, state, V::symbol((src[0] << 2) & 0x1C)); // size 2
        return;
    }
    data::put(encoded, state, V::symbol((src[0] << 2) & 0x1C | (src[1] >> 6) & 0x3)); // size 2
    data::put(encoded, state, V::symbol((src[1] >> 1) & 0x1F)); // size 3
    if (remaining_src_len == 2) {
        data::put(encoded, state, V::symbol((src[1] << 4) & 0x10)); // size 4
        return;
    }
    data::put(encoded, state, V::symbol((src[1] << 4) & 0x10 | (src[2] >> 4) & 0xF)); // size 4
    if (remaining_src_len == 3) {
        data::put(encoded, state, V::symbol((src[2] << 1) & 0x1E)); // size 5
        return;
    }
    data::put(encoded, state, V::symbol((src[2] << 1) & 0x1E | (src[3] >> 7) & 0x1)); // size 5
    data::put(encoded, state, V::symbol((src[3] >> 2) & 0x1F)); // size 6
    if (remaining_src_len == 4) {
        data::put(encoded, state, V::symbol((src[3] << 3) & 0x18)); // size 7
        return;
    }
    abort(); // not reached: encode_block() should be called if remaining_src_len > 4, not this function
}

template <typename CodecVariant>
template <typename Result, typename ResultState, typename V>
inline void base32<CodecVariant>::pad(
        Result& encoded, ResultState& state,
        typename std::enable_if<V::generates_padding(), size_t>::type remaining_src_len)
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
inline void base32<CodecVariant>::encode(
        Result& encoded_result, ResultState& state,
        const unsigned char* src, size_t src_size)
{
    const unsigned char* src_end = src + src_size - binary_block_size();

    for (; src <= src_end; src += binary_block_size()) {
        encode_block(encoded_result, state, src);
    }
    src_end += binary_block_size();

    if (src_end > src) {
        auto remaining_src_len = src_end - src;
        if (!remaining_src_len || remaining_src_len >= binary_block_size()) {
            abort();
            return;
        }
        encode_tail(encoded_result, state, src, remaining_src_len);
        pad(encoded_result, state, remaining_src_len);
    }
}

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::decode_block(
        Result& decoded, ResultState& state, const unsigned char* idx)
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
        Result& decoded, ResultState& state, const unsigned char* idx, size_t idx_len)
{
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

template <typename CodecVariant>
template <typename Result, typename ResultState>
inline void base32<CodecVariant>::decode(
        Result& binary_result, ResultState& state,
        const char* src_encoded, size_t src_size)
{
    const char* src = src_encoded;
    const char* src_end = src + src_size;

    using V = CodecVariant;

    uint8_t idx[8] = {};
    uint8_t last_value_idx = 0;

    while (src < src_end) {
        if (CodecVariant::should_ignore(idx[last_value_idx] = CodecVariant::index_of(*(src++)))) {
            continue;
        }
        if (CodecVariant::is_special_character(idx[last_value_idx])) {
            break;
        }

        ++last_value_idx;
        if (last_value_idx == encoded_block_size()) {
            decode_block(binary_result, state, idx);
            last_value_idx = 0;
        }
    }

    uint8_t last_idx = last_value_idx;
    if (CodecVariant::is_padding_symbol(idx[last_value_idx])) {
        // We're in here because we just read a (first) padding character. Try to read more.
        ++last_idx;
        while (src < src_end) {
            if (CodecVariant::is_eof(idx[last_idx] = CodecVariant::index_of(*(src++)))) {
                break;
            }
            if (!CodecVariant::is_padding_symbol(idx[last_idx])) {
                throw padding_error();
            }

            ++last_idx;
            if (last_idx > encoded_block_size()) {
                throw padding_error();
            }
        }
    }

    if (last_value_idx)  {
        if (CodecVariant::requires_padding() && last_idx != encoded_block_size()) {
            // If the input is not a multiple of the block size then the input is incorrect.
            throw padding_error();
        }
        if (last_value_idx >= encoded_block_size()) {
            abort();
            return;
        }
        decode_tail(binary_result, state, idx, last_value_idx);
    }
}

template <typename CodecVariant>
inline constexpr size_t base32<CodecVariant>::encoded_size(size_t binary_size) noexcept
{
    return CodecVariant::generates_padding()
            // With padding, the encoded size is a multiple of 8 bytes.
            // To calculate that, round the binary size up to multiple of 5, then convert.
            ? (binary_size + 4 - ((binary_size + 4) % 5)) * 8 / 5
            // No padding: only pad to the next multiple of 5 bits, i.e. at most a single extra byte.
            : (binary_size * 8 / 5) + (((binary_size * 8) % 5) ? 1 : 0);
}

template <typename CodecVariant>
inline constexpr size_t base32<CodecVariant>::decoded_max_size(size_t encoded_size) noexcept
{
    return CodecVariant::requires_padding()
            ? encoded_size * 5 / 8
            : (encoded_size * 5 / 8) + (((encoded_size * 5) % 8) ? 1 : 0);
}

} // namespace detail
} // namespace cppcodec

#endif // CPPCODEC_DETAIL_BASE32
