#include "query_builder_tests.h"
#include "quickdb/quickdb.h"
#include "test_runner.h"
#include "user_document.h"
#include <iostream>

bool test_query_operators()
{
    // This test primarily checks for compilation and basic structure.
    // Correctness is validated by the collection read tests.
    QDB::Database db("mongodb://localhost:27017");
    auto collection = db.get_collection<User>("qdb_test_db", "users");
    collection.delete_many(QDB::Query{});

    User user1({"Alice", 25, "a@a.com", {"dev", "c++"}});
    User user2({"Bob", 35, "b@b.com", {"dev", "js"}});
    collection.create_one(user1);
    collection.create_one(user2);

    // Test eq
    auto res1 = collection.find_one(QDB::Query{}.eq("age", 25));
    ASSERT_TRUE(res1.has_value() && res1->name == "Alice", "Query: eq operator");

    // Test gt
    auto res2 = collection.find_many(QDB::Query{}.gt("age", 30));
    ASSERT_TRUE(res2.size() == 1 && res2[0].name == "Bob", "Query: gt operator");

    // Test logical OR
    auto res3 = collection.find_many(QDB::Query::Or({QDB::Query{}.eq("name", "Alice"), QDB::Query{}.eq("name", "Bob")}));
    ASSERT_TRUE(res3.size() == 2, "Query: OR operator");

    // Test regex
    auto res4 = collection.find_one(QDB::Query{}.regex("email", "b@b.com"));
    ASSERT_TRUE(res4.has_value() && res4->name == "Bob", "Query: regex operator");

    return true;
}

bool run_query_builder_tests()
{
    bool success = true;
    success &= run_test_case(test_query_operators, "Query Builder: Operators");
    // Add more granular tests as needed
    return success;
}
