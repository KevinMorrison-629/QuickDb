#pragma once
#include "quickdb/components/document.h"
#include "quickdb/components/field.h"
#include <string>
#include <vector>

// A mock User document for testing purposes
class User : public QDB::Document
{
public:
    std::string name;
    int32_t age;
    std::string email;
    std::vector<std::string> tags;

    // Default constructor
    User() = default;

    // Parameterized constructor for easy initialization in tests
    User(std::string n, int32_t a, std::string e, std::vector<std::string> t)
        : name(std::move(n)), age(a), email(std::move(e)), tags(std::move(t))
    {
    }

    std::unordered_map<std::string, QDB::FieldValue> to_fields() const override
    {
        std::unordered_map<std::string, QDB::FieldValue> fields;
        fields["name"] = name;
        fields["age"] = age;
        fields["email"] = email;
        fields["tags"] = QDB::FieldValue(tags);
        return fields;
    }

    void from_fields(const std::unordered_map<std::string, QDB::FieldValue> &fields) override
    {
        QDB::get_field(fields, "name", name);
        QDB::get_field(fields, "age", age);
        QDB::get_field(fields, "email", email);
        // Use the return value of get_field to safely extract the vector
        if (fields.count("tags"))
        {
            tags = fields.at("tags").as<std::vector<std::string>>();
        }
        else
        {
            tags.clear();
        }
    }

    // Equality operator for easy comparison in tests
    bool operator==(const User &other) const
    {
        return get_id_str() == other.get_id_str() && name == other.name && age == other.age && email == other.email &&
               tags == other.tags;
    }
};
