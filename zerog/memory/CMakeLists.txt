cmake_minimum_required(VERSION 2.8)

project(memory)

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} common)

add_subdirectory(tests)
add_subdirectory(bench)
