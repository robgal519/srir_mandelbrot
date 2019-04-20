cmake_minimum_required( VERSION 2.6)
project (mandelbrot)


find_package(MPI REQUIRED)
set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
include_directories(MPI_INCLUDE_PATH)

add_executable(mandelbrot main.cpp)
target_include_directories(mandelbrot PUBLIC /usr/lib/x86_64-linux-gnu/openmpi/include)
target_link_libraries(mandelbrot pthread 
${MPI_LIBRARIES}
)
