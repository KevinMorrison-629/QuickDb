#include "collection_tests.h"
#include "quickdb/quickdb.h"
#include "test_runner.h"
#include "user_document.h"
#include <iostream>
#include <vector>

QDB::Database db("mongodb://localhost:27017");
auto collection = db.get_collection<User>("qdb_test_db", "users");

// Helper to clean collection before each test
void cleanup() { collection.delete_many(QDB::Query{}); }

bool test_create_one()
{
    cleanup();
    User user;
    user.name = "John Doe";
    user.age = 30;
    user.email = "john.doe@example.com";

    int64_t count = collection.create_one(user);
    ASSERT_TRUE(count == 1, "create_one should return 1 on success.");
    ASSERT_FALSE(user.get_id_str().empty(), "Document _id should be populated after creation.");

    auto found_user = collection.find_one(QDB::Query::by_id(user.get_id()));
    ASSERT_TRUE(found_user.has_value(), "Created user should be findable by ID.");
    ASSERT_TRUE(user.name == found_user->name, "Retrieved user name should match original.");
    return true;
}

bool test_create_many()
{
    cleanup();
    std::vector<User> users = {
        User("Jane Doe", 28, "jane@example.com", {}),
        User("Peter Pan", 100, "peter@example.com", {}),
    };

    int64_t count = collection.create_many(users);
    ASSERT_TRUE(count == 2, "create_many should return the number of inserted docs.");
    ASSERT_FALSE(users[0].get_id_str().empty(), "First user _id should be populated.");
    ASSERT_FALSE(users[1].get_id_str().empty(), "Second user _id should be populated.");
    ASSERT_TRUE(collection.count_documents(QDB::Query{}) == 2, "Collection count should be 2 after insert.");
    return true;
}

// Stubs for other tests in this category
bool test_read_operations()
{
    // TODO: Implement find_one, find_many, options, etc.
    return true;
}
bool test_update_operations()
{
    // TODO: Implement update_one, update_many, upsert, etc.
    return true;
}
bool test_delete_operations()
{
    // TODO: Implement delete_one, delete_many
    return true;
}
bool test_find_and_modify_ops()
{
    // TODO: Implement find_one_and_update/replace/delete
    return true;
}
bool test_index_management()
{
    // TODO: Implement index creation, listing, and dropping.
    return true;
}

bool run_collection_tests()
{
    bool success = true;
    success &= run_test_case(test_create_one, "Collection: create_one");
    success &= run_test_case(test_create_many, "Collection: create_many");
    success &= run_test_case(test_read_operations, "Collection: Read Operations (STUB)");
    success &= run_test_case(test_update_operations, "Collection: Update Operations (STUB)");
    success &= run_test_case(test_delete_operations, "Collection: Delete Operations (STUB)");
    success &= run_test_case(test_find_and_modify_ops, "Collection: Find-and-Modify (STUB)");
    success &= run_test_case(test_index_management, "Collection: Index Management (STUB)");
    return success;
}
