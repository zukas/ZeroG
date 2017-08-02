cmake_minimum_required(VERSION 2.8)

project(renderer)

add_definitions(-DGLM_FORCE_RADIANS -DGLM_FORCE_SSE2 -DGLEW_STATIC)

find_package(OpenGL REQUIRED)

add_subdirectory(freetype-2.8)
add_subdirectory(freeglut)
add_subdirectory(glfw-3.2.1)
add_subdirectory(glm-0.9.8.4)
add_subdirectory(glew-2.0.0/build/cmake)

include_directories(freetype-2.8/include)
include_directories(freeglut/include)
include_directories(glfw-3.2.1/include)
include_directories(glm-0.9.8.4)
include_directories(glew-2.0.0/include)

#include_directories(${CMAKE_SOURCE_DIR}/ZeroG)

message("include_directories ${INCLUDE_DIRECTORIES}")

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)



message("${PROJECT_NAME}:  ${SOURCES}")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} common utils freetype freeglut glfw glm glew GL)
