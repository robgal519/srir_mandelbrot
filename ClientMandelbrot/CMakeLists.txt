cmake_minimum_required(VERSION 3.13)
project(ClientMandelbrot)

set(CMAKE_CXX_STANDARD 11)

add_executable(ClientMandelbrot Main.cpp)
add_subdirectory(ClientMandelbrotLib)

#find_package(SDL2 REQUIRED) # Find*.cmake doesn't work on Arch for SDL2, no time for investigation.

target_include_directories(ClientMandelbrot PUBLIC ClientMandelbrotLib)
target_link_libraries(ClientMandelbrot ClientMandelbrotLib -lSDL2)
