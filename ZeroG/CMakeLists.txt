cmake_minimum_required(VERSION 2.8)

project(zerog)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-exceptions -fno-rtti")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions")

add_subdirectory(Common)
add_subdirectory(Utils)
add_subdirectory(Renderer)


file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

file(GLOB PRIVATE_SOURCES
    "internal/*.h"
    "internal/*.cpp"
)


message("${PROJECT_NAME}:  ${SOURCES}")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${PRIVATE_SOURCES})

target_link_libraries(${PROJECT_NAME} common utils renderer)
