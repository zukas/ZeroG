cmake_minimum_required(VERSION 2.8)

project(utils)

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

include_directories(${CMAKE_SOURCE_DIR}/Common)

message("${PROJECT_NAME}:  ${SOURCES}")

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} common)
