project(hello)
cmake_minimum_required(VERSION 3.10)

find_package(OpenMP REQUIRED)

add_executable(hello hello.cpp)
target_link_libraries(hello PUBLIC OpenMP::OpenMP_CXX)
