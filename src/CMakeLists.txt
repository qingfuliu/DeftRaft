file(GLOB_RECURSE CORE_SOURCE LIST_DIRECTORIES false ./*.cpp ./*.cc)

add_library(DeftRaft STATIC ${CORE_SOURCE})

target_include_directories(DeftRaft PRIVATE ${CMAKE_SOURCE_DIR}/src/include PUBLIC ${CMAKE_SOURCE_DIR}/include/log)

target_link_libraries(DeftRaft protobuf)

