#include "update_builder_tests.h"
#include "quickdb/quickdb.h"
#include "test_runner.h"
#include "user_document.h"
#include <iostream>

bool test_update_operators()
{
    QDB::Database db("mongodb://localhost:27017");
    auto collection = db.get_collection<User>("qdb_test_db", "users");
    collection.delete_many(QDB::Query{});

    User user("Charlie", 40, "c@c.com", {});
    collection.create_one(user);

    // Test $set and $inc
    QDB::Update update;
    update.set("name", "Charles").inc("age", 1);
    collection.update_one(QDB::Query::by_id(user.get_id()), update);

    auto updated_user = collection.find_one(QDB::Query::by_id(user.get_id()));
    ASSERT_TRUE(updated_user.has_value(), "Updated user must exist.");
    ASSERT_TRUE(updated_user->name == "Charles", "Update: $set operator");
    ASSERT_TRUE(updated_user->age == 41, "Update: $inc operator");

    // Test $push
    QDB::Update update2;
    update2.push("tags", "senior");
    collection.update_one(QDB::Query::by_id(user.get_id()), update2);
    auto pushed_user = collection.find_one(QDB::Query::by_id(user.get_id()));
    ASSERT_TRUE(pushed_user->tags.size() == 1 && pushed_user->tags[0] == "senior", "Update: $push operator");

    return true;
}

bool run_update_builder_tests()
{
    bool success = true;
    success &= run_test_case(test_update_operators, "Update Builder: Operators");
    return success;
}
