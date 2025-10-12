#include "aggregation_builder_tests.h"
#include "collection_tests.h"
#include "database_tests.h"
#include "gridfs_tests.h"
#include "query_builder_tests.h"
#include "serialization_tests.h"
#include "update_builder_tests.h"
#include <iostream>

void print_result(const std::string &test_name, bool success)
{
    std::cout << "========================================" << std::endl;
    std::cout << "Test Suite: " << test_name << " - Result: " << (success ? "PASSED" : "FAILED") << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
}

int main()
{
    bool all_passed = true;

    bool db_tests_passed = run_database_tests();
    print_result("Database and Connection", db_tests_passed);
    all_passed &= db_tests_passed;

    bool collection_tests_passed = run_collection_tests();
    print_result("Collection CRUD", collection_tests_passed);
    all_passed &= collection_tests_passed;

    bool query_tests_passed = run_query_builder_tests();
    print_result("Query Builder", query_tests_passed);
    all_passed &= query_tests_passed;

    bool update_tests_passed = run_update_builder_tests();
    print_result("Update Builder", update_tests_passed);
    all_passed &= update_tests_passed;

    bool agg_tests_passed = run_aggregation_builder_tests();
    print_result("Aggregation Builder", agg_tests_passed);
    all_passed &= agg_tests_passed;

    bool gridfs_tests_passed = run_gridfs_tests();
    print_result("GridFS", gridfs_tests_passed);
    all_passed &= gridfs_tests_passed;

    bool serialization_tests_passed = run_serialization_tests();
    print_result("Serialization", serialization_tests_passed);
    all_passed &= serialization_tests_passed;

    if (all_passed)
    {
        std::cout << "All test suites passed!" << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "One or more test suites failed." << std::endl;
        return 1;
    }
}
