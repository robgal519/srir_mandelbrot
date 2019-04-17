enable_testing()
add_executable(MandelbrotClientTest tests.cpp)
target_include_directories(MandelbrotClientTest PUBLIC ../src)
target_link_libraries(MandelbrotClientTest MandelbrotClientLib -lgtest -lgtest_main)