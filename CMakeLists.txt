cmake_minimum_required(VERSION 2.8)

project(Project-ZeroG)

option(ZEROG_SANITIZE_ADDRESS "Enable -fsanitize=address" OFF)
option(ZEROG_SANITIZE_KERNEL_ADDRESS "Enable -fsanitize=kernel-address" OFF)
option(ZEROG_SANITIZE_THREAD "Enable -fsanitize=thread" OFF)
option(ZEROG_SANITIZE_LEAK "Enable -fsanitize=leak" OFF)
option(ZEROG_SANITIZE_UNDEFINED "Enable -fsanitize=undefined" OFF)
option(ZEROG_PROFILE "Retain frame pointer" OFF)

set(GLOBAL_LIBS)

if(ZEROG_SANITIZE_ADDRESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    list(APPEND GLOBAL_LIBS asan)
endif()

if(ZEROG_SANITIZE_KERNEL_ADDRESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=kernel-address")
endif()

if(ZEROG_SANITIZE_THREAD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    list(APPEND GLOBAL_LIBS tsan)
endif()

if(ZEROG_SANITIZE_LEAK)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
    list(APPEND GLOBAL_LIBS lsan)
endif()

if(ZEROG_SANITIZE_UNDEFINED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    list(APPEND GLOBAL_LIBS ubsan)
endif()

if(ZEROG_PROFILE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
endif()

add_subdirectory(ZeroG)
add_subdirectory(Dummy)
add_subdirectory(Bench)
