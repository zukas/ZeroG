cmake_minimum_required(VERSION 2.8)

project(renderer)

file(GLOB SOURCES
    "*.h"
    "*.cpp"
)

file(GLOB PRIVATE_SOURCES
    "vk/*.h"
    "vk/*.cpp"
)

include_directories(${VULKAN_INCLUDE_DIR})
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${PRIVATE_SOURCES})

target_link_libraries(${PROJECT_NAME} common memory glfw ${VULKAN_LIBRARY})
