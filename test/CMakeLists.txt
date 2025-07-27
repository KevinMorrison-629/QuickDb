
set(TEST_EXECUTABLE_NAME "qdb_test")

add_executable(${TEST_EXECUTABLE_NAME} main.cpp)

target_link_libraries(qdb_test PRIVATE
    quickdb
)