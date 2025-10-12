#include "aggregation_builder_tests.h"
#include "quickdb/quickdb.h"
#include "test_runner.h"
#include "user_document.h"
#include <iostream>

// A document to hold aggregation results that don't map to User
class AgeResult : public QDB::Document
{
public:
    int32_t age;
    int32_t count;

    std::unordered_map<std::string, QDB::FieldValue> to_fields() const override
    {
        return {}; // Not used for reading results
    }
    void from_fields(const std::unordered_map<std::string, QDB::FieldValue> &fields) override
    {
        // Note: _id from group stage is mapped to a field name
        if (fields.count("_id"))
        {
            age = fields.at("_id").as<int32_t>();
        }
        if (fields.count("count"))
        {
            count = fields.at("count").as<int32_t>();
        }
    }
};

bool test_aggregation_pipeline()
{
    QDB::Database db("mongodb://localhost:27017");
    auto collection = db.get_collection<User>("qdb_test_db", "users");
    collection.delete_many(QDB::Query{});

    std::vector<User> users = {
        User("David", 50, "d@d.com", {}),
        User("Dana", 50, "dana@d.com", {}),
        User("Eve", 60, "e@e.com", {}),
    };
    collection.create_many(users);

    QDB::Aggregation agg;
    agg.group(QDB::DocumentBuilder("_id", "$age").add_field("count", QDB::DocumentBuilder("$sum", 1)))
        .sort(QDB::DocumentBuilder("_id", 1));

    auto results = collection.aggregate<AgeResult>(agg);

    ASSERT_TRUE(results.size() == 2, "Aggregation should return 2 groups.");
    ASSERT_TRUE(results[0].age == 50 && results[0].count == 2, "First group (age 50) should have 2 users.");
    ASSERT_TRUE(results[1].age == 60 && results[1].count == 1, "Second group (age 60) should have 1 user.");

    return true;
}

bool run_aggregation_builder_tests()
{
    bool success = true;
    success &= run_test_case(test_aggregation_pipeline, "Aggregation Builder: Pipeline");
    return success;
}
