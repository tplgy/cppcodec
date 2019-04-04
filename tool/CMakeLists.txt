add_compile_options(${CPPCODEC_PRIVATE_COMPILE_OPTIONS})

add_executable(base32enc base32enc.cpp)
target_link_libraries(base32enc cppcodec)

add_executable(base32dec base32dec.cpp)
target_link_libraries(base32dec cppcodec)

add_executable(base64enc base64enc.cpp)
target_link_libraries(base64enc cppcodec)

add_executable(base64dec base64dec.cpp)
target_link_libraries(base64dec cppcodec)

add_executable(hexenc hexenc.cpp)
target_link_libraries(hexenc cppcodec)

add_executable(hexdec hexdec.cpp)
target_link_libraries(hexdec cppcodec)
