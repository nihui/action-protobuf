project(hello)

#find_package(Protobuf REQUIRED)
find_package(protobuf CONFIG REQUIRED)

add_executable(hello hello.c)
target_link_libraries(hello PRIVATE ${PROTOBUF_LIBRARIES})
