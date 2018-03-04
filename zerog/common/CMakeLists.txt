cmake_minimum_required(VERSION 2.8)

project(common)

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${PRIVATE_SOURCES})

add_subdirectory(tests)
