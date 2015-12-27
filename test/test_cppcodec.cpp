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

#define CATCH_CONFIG_MAIN
#include "catch/single_include/catch.hpp"

#include <cppcodec/base32_crockford.hpp>
#include <cppcodec/base32_rfc4648.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <cppcodec/base64_url.hpp>
#include <cppcodec/hex_lower.hpp>
#include <cppcodec/hex_upper.hpp>
#include <stdint.h>
#include <string.h> // for memcmp()
#include <vector>

TEST_CASE("Douglas Crockford's base32", "[base32][crockford]") {
    using base32 = cppcodec::base32_crockford;

    SECTION("encoded size calculation") {
        REQUIRE(base32::encoded_size(0) == 0);
        REQUIRE(base32::encoded_size(1) == 2);
        REQUIRE(base32::encoded_size(2) == 4);
        REQUIRE(base32::encoded_size(3) == 5);
        REQUIRE(base32::encoded_size(4) == 7);
        REQUIRE(base32::encoded_size(5) == 8);
        REQUIRE(base32::encoded_size(6) == 10);
        REQUIRE(base32::encoded_size(10) == 16);
    }

    SECTION("maximum decoded size calculation") {
        REQUIRE(base32::decoded_max_size(0) == 0);
        REQUIRE(base32::decoded_max_size(1) == 0);
        REQUIRE(base32::decoded_max_size(2) == 1);
        REQUIRE(base32::decoded_max_size(3) == 1);
        REQUIRE(base32::decoded_max_size(4) == 2);
        REQUIRE(base32::decoded_max_size(5) == 3);
        REQUIRE(base32::decoded_max_size(6) == 3);
        REQUIRE(base32::decoded_max_size(7) == 4);
        REQUIRE(base32::decoded_max_size(8) == 5);
        REQUIRE(base32::decoded_max_size(9) == 5);
        REQUIRE(base32::decoded_max_size(10) == 6);
        REQUIRE(base32::decoded_max_size(16) == 10);
    }

    std::string hello = "Hello World";
    std::string hello_encoded = "91JPRV3F41BPYWKCCG";
    std::string hello_encoded_null = "91JPRV3F41BPYWKCCG00";

    const uint8_t* hello_uint_ptr = reinterpret_cast<const uint8_t*>(hello.data());
    const uint8_t* hello_uint_ptr_encoded = reinterpret_cast<const uint8_t*>(hello_encoded.data());

    std::vector<uint8_t> hello_uint_vector(hello_uint_ptr, hello_uint_ptr + hello.size());
    std::vector<char> hello_char_vector_encoded(
            hello_encoded.data(), hello_encoded.data() + hello_encoded.size());
    std::vector<uint8_t> hello_uint_vector_encoded(
            hello_uint_ptr_encoded, hello_uint_ptr_encoded + hello_encoded.size());

    SECTION("encoding data") {
        REQUIRE(base32::encode(std::vector<uint8_t>()) == "");
        REQUIRE(base32::encode(std::vector<uint8_t>({0})) == "00");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0})) == "0000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0})) == "00000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0})) == "0000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0})) == "00000000");
        REQUIRE(base32::encode(std::vector<uint8_t>({0, 0, 0, 0, 0, 0})) == "0000000000");

        // Constructing an std::string reduces the size of the char array by one (null terminator).
        // Therefore, the result for passing the string literal directly ends up encoding
        // one more character, which produces two more symbols in this particular case.
        REQUIRE(base32::encode(std::string("Hello World")) == hello_encoded);
        REQUIRE(base32::encode("Hello World") == hello_encoded_null);

        REQUIRE(base32::encode(std::string("foo")) == "CSQPY");
        REQUIRE(base32::encode(std::string("lowercase UPPERCASE 1434567 !@#$%^&*"))
                == "DHQQESBJCDGQ6S90AN850HAJ8D0N6H9064T36D1N6RVJ08A04CJ2AQH658");
        REQUIRE(base32::encode(std::string("Wow, it really works!")) == "AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44");
    }

    SECTION("decoding data") {
        REQUIRE(base32::decode("") == std::vector<uint8_t>());
        REQUIRE(base32::decode("00") == std::vector<uint8_t>({0}));
        REQUIRE(base32::decode("0000") == std::vector<uint8_t>({0, 0}));
        REQUIRE(base32::decode("00000") == std::vector<uint8_t>({0, 0, 0}));
        REQUIRE(base32::decode("0000000") == std::vector<uint8_t>({0, 0, 0, 0}));
        REQUIRE(base32::decode("00000000") == std::vector<uint8_t>({0, 0, 0, 0, 0}));
        REQUIRE(base32::decode("0000000000") == std::vector<uint8_t>({0, 0, 0, 0, 0, 0}));

        // For decoding data, the result should be the same whether or not there is
        // a null terminator at the end, because the input is a string (not binary array).
        REQUIRE(base32::decode(std::string("91JPRV3F41BPYWKCCG")) == hello_uint_vector);
        REQUIRE(base32::decode("91JPRV3F41BPYWKCCG") == hello_uint_vector);

        REQUIRE(base32::decode<std::string>("CSQPY") == "foo");
        REQUIRE(base32::decode<std::string>("DHQQESBJCDGQ6S90AN850HAJ8D0N6H9064T36D1N6RVJ08A04CJ2AQH658")
                == "lowercase UPPERCASE 1434567 !@#$%^&*");

        // Lowercase should decode just as well as uppercase.
        REQUIRE(base32::decode<std::string>("AXQQEB10D5T20WK5C5P6RY90EXQQ4TVK44") == "Wow, it really works!");
        REQUIRE(base32::decode<std::string>("axqqeb10d5t20wk5c5p6ry90exqq4tvk44") == "Wow, it really works!");
    }

    SECTION("encode() overloads") {
        // Other convenient overloads for taking raw pointer input.
        REQUIRE(base32::encode(hello.data(), hello.size()) == hello_encoded);
        REQUIRE(base32::encode(hello_uint_ptr, hello.size()) == hello_encoded);

        // Reused result pointer. Put the extra null terminator version in the middle to test resizing.
        std::string result;
        REQUIRE((base32::encode(result, hello_uint_ptr, hello.size()), result) == hello_encoded);
        REQUIRE((base32::encode(result, "Hello World"), result) == hello_encoded_null);
        REQUIRE((base32::encode(result, hello.data(), hello.size()), result) == hello_encoded);

        // Templated result. Use std::vector<uint8_t> to exercise non-char array types.
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello) == hello_uint_vector_encoded);
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello.data(), hello.size()) == hello_uint_vector_encoded);
        REQUIRE(base32::encode<std::vector<uint8_t>>(hello_uint_ptr, hello.size()) == hello_uint_vector_encoded);

        // Raw pointer output.
        std::vector<char> hello_char_result;
        hello_char_result.resize(base32::encoded_size(hello.size()));
        REQUIRE(hello_char_result.size() == hello_char_vector_encoded.size());

        size_t result_size;
        result_size = base32::encode(hello_char_result.data(), hello_char_result.size(), hello);
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);

        result_size = base32::encode(
                hello_char_result.data(), hello_char_result.size(), hello.data(), hello.size());
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);

        result_size = base32::encode(
                hello_char_result.data(), hello_char_result.size(), hello_uint_ptr, hello.size());
        REQUIRE(result_size == hello_char_vector_encoded.size());
        REQUIRE(hello_char_result == hello_char_vector_encoded);
    }

    SECTION("decode() overloads") {
        // Other convenient overloads for taking raw pointer input.
        REQUIRE(base32::decode(hello_encoded.data(), hello_encoded.size()) == hello_uint_vector);

        // Reused result pointer. Put a different string in the middle to test resizing.
        std::vector<uint8_t> result;
        REQUIRE((base32::decode(result, hello_encoded.data(), hello_encoded.size()), result)
                == hello_uint_vector);
        REQUIRE((base32::decode(result, "00"), result) == std::vector<uint8_t>({0}));
        REQUIRE((base32::decode(result, hello_encoded), result) == hello_uint_vector);

        // Templated result. Use std::string to exercise non-uint8_t array types.
        REQUIRE(base32::decode<std::string>(hello_encoded) == hello);
        REQUIRE(base32::decode<std::string>(hello_uint_vector_encoded) == hello);
        REQUIRE(base32::decode<std::string>(hello_encoded.data(), hello_encoded.size()) == hello);

        // Raw pointer output.
        std::vector<uint8_t> hello_uint_result;
        std::vector<char> hello_char_result;
        size_t hello_decoded_max_size = base32::decoded_max_size(hello_encoded.size());
        REQUIRE(hello.size() <= hello_decoded_max_size);

        hello_char_result.resize(hello_decoded_max_size);
        size_t result_size = base32::decode(
                hello_char_result.data(), hello_char_result.size(), hello_encoded);
        REQUIRE(result_size == hello.size());
        REQUIRE(std::string(hello_char_result.data(), hello_char_result.data() + result_size) == hello);

        hello_char_result.resize(hello_decoded_max_size);
        result_size = base32::decode(
                hello_char_result.data(), hello_char_result.size(),
                hello_encoded.data(), hello_encoded.size());
        REQUIRE(result_size == hello.size());
        REQUIRE(std::string(hello_char_result.data(), hello_char_result.data() + result_size) == hello);

        hello_uint_result.resize(hello_decoded_max_size);
        result_size = base32::decode(
                hello_uint_result.data(), hello_uint_result.size(), hello_encoded);
        REQUIRE(result_size == hello.size());
        hello_uint_result.resize(result_size);
        REQUIRE(hello_uint_result == hello_uint_vector);

        hello_uint_result.resize(hello_decoded_max_size);
        result_size = base32::decode(
                hello_uint_result.data(), hello_uint_result.size(),
                hello_encoded.data(), hello_encoded.size());
        REQUIRE(result_size == hello.size());
        hello_uint_result.resize(result_size);
        REQUIRE(hello_uint_result == hello_uint_vector);
    }
}
