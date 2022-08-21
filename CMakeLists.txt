cmake_minimum_required(VERSION 3.13)
project(cppcodec CXX)
set(PROJECT_VERSION 0.3)

include(GNUInstallDirs)

# Tests, examples and tools are ON by default if built stand-alone,
# OFF when built as part of another project. Set CPPCODEC_BUILD_* options to override.
set(BUILD_EXAMPLES_DEFAULT OFF)
set(BUILD_TESTING_DEFAULT OFF)
set(BUILD_TOOLS_DEFAULT OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest) # note: defines the BUILD_TESTING option which defaults to ON
    set(BUILD_TESTING_DEFAULT ${BUILD_TESTING})
    set(BUILD_EXAMPLES_DEFAULT ON)
    set(BUILD_TOOLS_DEFAULT ON)
endif()

option(
    CPPCODEC_BUILD_EXAMPLES
    "Build cppcodec example code"
    ${BUILD_EXAMPLES_DEFAULT}
)
option(CPPCODEC_BUILD_TESTING "Build cppcodec tests" ${BUILD_TESTING_DEFAULT})
option(CPPCODEC_BUILD_TOOLS "Build cppcodec tools" ${BUILD_TOOLS_DEFAULT})

# Request C++11, or let the user specify the standard on via -D command line option.
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Relative paths, used also for install(). Use #include paths like this.
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
    cppcodec/base64_default_url_unpadded.hpp
    cppcodec/base64_rfc4648.hpp
    cppcodec/base64_url.hpp
    cppcodec/base64_url_unpadded.hpp
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
    cppcodec/detail/stream_codec.hpp
)

add_library(cppcodec INTERFACE)
target_sources(cppcodec INTERFACE ${PUBLIC_HEADERS})
set_target_properties(cppcodec PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(
    cppcodec
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# These flags are for binaries built by this particular CMake project (test_cppcodec, base64enc, etc.).
# In your own project that uses cppcodec, you might want to specify a different standard or error level.
if(MSVC)
    # MSVC will respect CMAKE_CXX_STANDARD for CMake >= 3.10 and MSVC >= 19.0.24215
    # (VS 2017 15.3). Older versions will use the compiler default, which should be
    # fine for anything except ancient MSVC versions.
    set(CPPCODEC_PRIVATE_COMPILE_OPTIONS /W4) # /WX for failing on errors
else()
    set(CPPCODEC_PRIVATE_COMPILE_OPTIONS -Wall -Wextra -pedantic) # -Werror for failing on errors
endif()

if(CPPCODEC_BUILD_TESTING)
    add_subdirectory(test)
endif()

if(CPPCODEC_BUILD_TOOLS)
    add_subdirectory(tool)
endif()

if(CPPCODEC_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

foreach(h ${PUBLIC_HEADERS})
    get_filename_component(FINAL_PATH ${h} DIRECTORY) # once requiring CMake 3.20, switch to cmake_path()
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/${h}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${FINAL_PATH}
        COMPONENT "headers"
    )
endforeach()

if(NOT WIN32)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cppcodec.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/cppcodec-1.pc
        @ONLY
    )
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/cppcodec-1.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
    )
endif()
