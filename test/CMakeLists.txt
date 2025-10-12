
set(TEST_EXECUTABLE_NAME "qdb_test")

add_executable(${TEST_EXECUTABLE_NAME}
    main.cpp

    database_tests.cpp
    collection_tests.cpp
    query_builder_tests.cpp
    update_builder_tests.cpp
    aggregation_builder_tests.cpp
    gridfs_tests.cpp
    serialization_tests.cpp
)

target_link_libraries(qdb_test PRIVATE
    quickdb
)