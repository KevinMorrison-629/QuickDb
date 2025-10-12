#include "serialization_tests.h"
#include "test_runner.h"
#include "user_document.h"
#include <iostream>

bool test_serialization_cycle()
{
    User original_user;
    original_user.name = "Serialize Me";
    original_user.age = 99;
    original_user.email = "ser@example.com";
    original_user.tags = {"a", "b", "c"};

    // to_fields
    auto fields = original_user.to_fields();
    ASSERT_TRUE(fields["name"].as<std::string>() == "Serialize Me", "to_fields: name correct");
    ASSERT_TRUE(fields["age"].as<int32_t>() == 99, "to_fields: age correct");
    ASSERT_TRUE(fields["tags"].as<std::vector<std::string>>().size() == 3, "to_fields: tags size correct");

    // from_fields
    User new_user;
    new_user.from_fields(fields);

    // This test focuses on the user-defined fields, not the database-managed _id.
    // We assert individual members for correctness.
    ASSERT_TRUE(original_user.name == new_user.name, "Serialization cycle: name should match.");
    ASSERT_TRUE(original_user.age == new_user.age, "Serialization cycle: age should match.");
    ASSERT_TRUE(original_user.email == new_user.email, "Serialization cycle: email should match.");
    ASSERT_TRUE(original_user.tags == new_user.tags, "Serialization cycle: tags should match.");

    return true;
}

bool run_serialization_tests()
{
    bool success = true;
    success &= run_test_case(test_serialization_cycle, "Serialization: to_fields/from_fields Cycle");
    return success;
}
