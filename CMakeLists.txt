cmake_minimum_required(VERSION 2.8)

project(ZeroG)

option(ZEROG_SANITIZE_ADDRESS "Enable -fsanitize=address" OFF)
option(ZEROG_SANITIZE_KERNEL_ADDRESS "Enable -fsanitize=kernel-address" OFF)
option(ZEROG_SANITIZE_THREAD "Enable -fsanitize=thread" OFF)
option(ZEROG_SANITIZE_LEAK "Enable -fsanitize=leak" OFF)
option(ZEROG_SANITIZE_UNDEFINED "Enable -fsanitize=undefined" OFF)
option(ZEROG_PROFILE "Retain frame pointer" OFF)

option(ZEROG_LTO "Link time optimization" OFF)

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

if(ZEROG_LTO)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
endif()

add_definitions(-DSTB_IMAGE_IMPLEMENTATION)
add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)

set(VULKAN_SDK_PATH ${CMAKE_SOURCE_DIR}/3rd/vulkan_sdk)
set(GLM_PATH ${CMAKE_SOURCE_DIR}/3rd/glm-0.9.8.4)
set(GLFW_PATH ${CMAKE_SOURCE_DIR}/3rd/glfw-3.2.1)
set(STB_PATH ${CMAKE_SOURCE_DIR}/3rd/stb)
set(FREETYPE_PATH ${CMAKE_SOURCE_DIR}/3rd/freetype-2.8)
set(BENCHMARK_PATH ${CMAKE_SOURCE_DIR}/3rd/benchmark)

set(GLM_INCLUDE ${GLM_PATH})
set(GLFW_INCLUDE ${GLFW_PATH}/include)
set(STB_INCLUDE ${STB_PATH})
set(FREETYPE_INCLUDE ${FREETYPE_PATH}/include)
set(BENCHMARK_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/benchmark/include)

set(CMAKE_PREFIX_PATH ${VULKAN_SDK_PATH})

add_subdirectory(${GLM_PATH})
add_subdirectory(${GLFW_PATH})
add_subdirectory(${FREETYPE_PATH})
add_subdirectory(${BENCHMARK_PATH})

include_directories(${GLM_INCLUDE})
include_directories(${STB_INCLUDE})
include_directories(${GLFW_INCLUDE})
include_directories(${FREETYPE_INCLUDE})
include_directories(${VULKAN_INCLUDE})
include_directories(${BENCHMARK_INCLUDE})

link_directories(${VULKAN_LIB})

enable_testing()

add_subdirectory(zerog)
add_subdirectory(dummy)



