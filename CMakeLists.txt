cmake_minimum_required(VERSION 3.5)

project(CHIP-8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(sfml
GIT_REPOSITORY "https://github.com/SFML/SFML.git"
GIT_TAG "2.5.1")

FetchContent_MakeAvailable(sfml)

add_executable(CHIP-8 main.cpp)
target_link_libraries(CHIP-8 PRIVATE sfml-audio sfml-graphics sfml-system sfml-window )
