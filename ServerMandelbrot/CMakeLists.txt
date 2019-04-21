cmake_minimum_required( VERSION 2.6)
project (ServerMandelbrot)

find_package(MPI REQUIRED)

add_executable(ServerMandelbrot main.cpp)
target_include_directories(ServerMandelbrot PUBLIC ${MPI_INCLUDE_PATH})
target_link_libraries(ServerMandelbrot pthread ${MPI_LIBRARIES})
