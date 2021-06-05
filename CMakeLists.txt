cmake_minimum_required(VERSION 3.19)
project(hikr-pdf-generator C)

set(CMAKE_C_STANDARD 11)

set(CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
add_executable(hikr hikr.c)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(hikr ${CURL_LIBRARIES})
