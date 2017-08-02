cmake_minimum_required(VERSION 2.8)

project(Bench)

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

add_subdirectory(benchmark-1.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-exceptions -fno-rtti")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} zerog benchmark)
