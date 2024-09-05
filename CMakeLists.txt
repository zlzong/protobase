cmake_minimum_required(VERSION 2.6)
project(protobase)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX ../bin)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third/spdLog)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -pipe -Wall -Werror -Wno-unused-variable -lrt -pthread")
elseif (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "Release Version.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -pipe -Wall -Werror -Wno-unused-variable -lrt -pthread")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -Wno-error=shadow -Wno-sign-compare")
message(STATUS "USING GCC VERSION: ${CMAKE_CXX_COMPILER_VERSION}")

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.2.9)
    message(STATUS "GCC Version > 7.3.0, USING -Wno-error=unused-result -Werror=return-type")
    set(CMAKE_CXX_FLAGS "-Wno-error=unused-result -Werror=return-type ${CMAKE_CXX_FLAGS}")
endif ()

include_directories(${THIRD_PARTY_DIR}/inc)
#link_directories(${THIRD_PARTY_DIR}/lib)

include_directories(src)
add_subdirectory(src)
