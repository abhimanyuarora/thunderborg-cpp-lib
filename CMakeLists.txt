cmake_minimum_required(VERSION 3.0.2)

project(thunderborg-lib)

set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_STANDARD 17)

include_directories(src/)

add_library(${PROJECT_NAME}
    src/thunderborg.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)
