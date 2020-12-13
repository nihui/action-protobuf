project(hello)
cmake_minimum_required(VERSION 3.10)

find_package(OpenMP REQUIRED)

message("OpenMP_C_INCLUDE_DIRS = ${OpenMP_C_INCLUDE_DIRS}")
message("OpenMP_CXX_INCLUDE_DIRS = ${OpenMP_CXX_INCLUDE_DIRS}")
message("OpenMP_C_LIBRARIES = ${OpenMP_C_LIBRARIES}")
message("OpenMP_CXX_LIBRARIES = ${OpenMP_CXX_LIBRARIES}")

add_executable(hello hello.cpp)

if(TARGET OpenMP::OpenMP_CXX)
    target_link_libraries(hello PRIVATE OpenMP::OpenMP_CXX)
elseif(OpenMP_CXX_FOUND OR OPENMP_FOUND)
    target_compile_options(hello PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(hello PRIVATE "${OpenMP_CXX_FLAGS}")
endif()
