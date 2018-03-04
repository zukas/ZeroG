cmake_minimum_required(VERSION 2.8)

project(zerog)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-rtti -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables -fPIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(common)
add_subdirectory(memory)
add_subdirectory(utils)
add_subdirectory(renderer)


file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

file(GLOB PRIVATE_SOURCES
    "internal/*.h"
    "internal/*.cpp"
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${PRIVATE_SOURCES})

target_link_libraries(${PROJECT_NAME} common renderer ${VULKAN_LIBRARY})
