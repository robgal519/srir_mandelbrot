cmake_minimum_required(VERSION 3.13)
project(MandelbrotClient)

set(CMAKE_CXX_STANDARD 11)

add_executable(MandelbrotClient Main.cpp)
add_subdirectory(tests)
add_subdirectory(tests_pipeside)
add_subdirectory(src)
target_include_directories(MandelbrotClient PUBLIC src)
target_link_libraries(MandelbrotClient MandelbrotClientLib -lSDL2)

