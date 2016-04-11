cmake_minimum_required(VERSION 2.8)
project(cppcodec)

option(build_tests "Build tests (run as 'test' target)" ON)

# These flags are for binaries built by this particular CMake project (test_cppcodec, base64enc, etc.).
# In your own project that uses cppcodec, you might want to specify a different standard or error level.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")

set(PUBLIC_HEADERS
    # base32
    cppcodec/base32_crockford.hpp
    cppcodec/base32_default_crockford.hpp
    cppcodec/base32_default_hex.hpp
    cppcodec/base32_default_rfc4648.hpp
    cppcodec/base32_hex.hpp
    cppcodec/base32_rfc4648.hpp
    # base64
    cppcodec/base64_default_rfc4648.hpp
    cppcodec/base64_default_url.hpp
    cppcodec/base64_rfc4648.hpp
    cppcodec/base64_url.hpp
    # hex
    cppcodec/hex_default_lower.hpp
    cppcodec/hex_default_upper.hpp
    cppcodec/hex_lower.hpp
    cppcodec/hex_upper.hpp
    # other stuff
    cppcodec/parse_error.hpp
    cppcodec/data/access.hpp
    cppcodec/data/raw_result_buffer.hpp
    cppcodec/detail/base32.hpp
    cppcodec/detail/base64.hpp
    cppcodec/detail/codec.hpp
    cppcodec/detail/config.hpp
    cppcodec/detail/hex.hpp
    cppcodec/detail/stream_codec.hpp)

add_library(cppcodec OBJECT ${PUBLIC_HEADERS}) # unnecessary for building, but makes headers show up in IDEs
set_target_properties(cppcodec PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(tool)

if (build_tests)
    enable_testing()
    add_subdirectory(test)
endif()

foreach(h ${PUBLIC_HEADERS})
    get_filename_component(HEADER_INCLUDE_DIRECTORY include/${h} PATH) # use DIRECTORY instead of PATH once requiring CMake 3.0
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${h} DESTINATION ${HEADER_INCLUDE_DIRECTORY} COMPONENT "headers")
endforeach()
