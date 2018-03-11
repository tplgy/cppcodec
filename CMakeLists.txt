cmake_minimum_required(VERSION 2.8.5)
project(cppcodec CXX)
set(PROJECT_VERSION 0.1)

include(GNUInstallDirs)
include(CTest)

# These flags are for binaries built by this particular CMake project (test_cppcodec, base64enc, etc.).
# In your own project that uses cppcodec, you might want to specify a different standard or error level.
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")

  # Request C++11
  if(${CMAKE_VERSION} VERSION_LESS 3.1)
    # CMake versions before 3.1 do not understand CMAKE_CXX_STANDARD
    # remove this block once CMake >=3.1 has fixated in the ecosystem
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
  endif()
endif()

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
    cppcodec/detail/stream_codec.hpp)

add_library(cppcodec OBJECT ${PUBLIC_HEADERS}) # unnecessary for building, but makes headers show up in IDEs
set_target_properties(cppcodec PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(tool)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()

foreach(h ${PUBLIC_HEADERS})
    get_filename_component(FINAL_PATH ${h} PATH) # use DIRECTORY instead of PATH once requiring CMake 3.0
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${h} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${FINAL_PATH} COMPONENT "headers")
endforeach()

if (NOT WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cppcodec.pc.in ${CMAKE_CURRENT_BINARY_DIR}/cppcodec-1.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cppcodec-1.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
endif()
