cmake_minimum_required(VERSION 2.8)

project(dummy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-exceptions -fno-rtti -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fPIC")

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} zerog)
