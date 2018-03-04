CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(memroy-bench
        LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fexceptions -frtti")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions")



add_executable(${PROJECT_NAME} main.cpp)
add_test(${PROJECT_NAME} COMMAND ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE memory benchmark)

