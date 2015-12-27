# For cppcodec itself, don't prefer system headers over development ones.
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

add_executable(test_cppcodec test_cppcodec.cpp)
add_test(cppcodec test_cppcodec)
