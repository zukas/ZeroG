cmake_minimum_required(VERSION 2.8)

project(utils)

file(GLOB SOURCES
    "*.h"
    "*.cpp"
#    "memory/*.h"
#    "memory/*.cpp"
    "containers/*.h"
    "containers/*.cpp"
#    "pointers/*.h"
#    "pointers/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME} common)
