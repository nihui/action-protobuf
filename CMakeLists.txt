project(hello)

find_package(Protobuf REQUIRED)

add_executable(hello hello.c)
target_link_libraries(hello PRIVATE ${PROTOBUF_LIBRARIES})
