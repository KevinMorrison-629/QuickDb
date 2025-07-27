
add_executable(qdb_test main.cpp)

target_link_libraries(qdb_test PRIVATE
    quickdb
)