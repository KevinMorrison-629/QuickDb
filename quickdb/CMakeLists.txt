
set(LIB_NAME "quickdb")

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${LIB_NAME} STATIC
    ${SOURCES}
)

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)

target_link_libraries(${LIB_NAME} PUBLIC
    $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
    $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
)