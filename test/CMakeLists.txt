find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(CATCH2 catch2)
endif()

if(CATCH2_FOUND)
    message(STATUS "Found system Catch2, not using bundled version")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CATCH2_CFLAGS}")
else()
    message(STATUS "Did NOT find system Catch2, instead using bundled version")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/catch/single_include)
endif()

add_compile_options(${CPPCODEC_PRIVATE_COMPILE_OPTIONS})

add_executable(test_cppcodec test_cppcodec.cpp)
target_link_libraries(test_cppcodec cppcodec)
add_test(NAME cppcodec COMMAND test_cppcodec)

add_executable(benchmark_cppcodec benchmark_cppcodec.cpp)
target_link_libraries(benchmark_cppcodec cppcodec)

add_executable(minimal_decode minimal_decode.cpp)
target_link_libraries(minimal_decode cppcodec)
