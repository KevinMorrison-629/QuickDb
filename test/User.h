#pragma once

#include "quickdb/components/document.h"

#include <iostream>
#include <string>

// The User struct inherits from QDB::Document and implements the required
// serialization and deserialization methods.
struct User : public QDB::Document
{
    std::string name;
    std::string email;
    int32_t age = 0;

    /// @brief Serializes the User object into a map of FieldValues for database storage.
    /// @return An unordered_map representing the document's fields.
    virtual std::unordered_map<std::string, QDB::FieldValue> to_fields() const override
    {
        std::unordered_map<std::string, QDB::FieldValue> fields;
        fields["name"] = name;
        fields["email"] = email;
        fields["age"] = age;
        return fields;
    }

    /// @brief  Deserializes a map of FieldValues from the database into this User object.
    /// @param fields The map of fields retrieved from the database.
    virtual void from_fields(const std::unordered_map<std::string, QDB::FieldValue> &fields) override
    {
        // The get_id() method in the Collection class handles setting the protected _id member.
        // We just need to populate our own class members from the map.
        QDB::get_field(fields, "name", name);
        QDB::get_field(fields, "email", email);
        QDB::get_field(fields, "age", age);
    }
};
