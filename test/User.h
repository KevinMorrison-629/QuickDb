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

    // Serialization: Convert member variables to FieldValue map
    std::unordered_map<std::string, QDB::FieldValue> to_fields() const override
    {
        return {{"name", {QDB::FieldType::FT_STRING, name}},
                {"email", {QDB::FieldType::FT_STRING, email}},
                {"age", {QDB::FieldType::FT_INT_32, age}}};
    }

    // Deserialization: Populate member variables from FieldValue map
    void from_fields(const std::unordered_map<std::string, QDB::FieldValue> &fields) override
    {
        try
        {
            // Use .at() to throw an exception if a field is missing
            name = std::get<std::string>(fields.at("name").value);
            email = std::get<std::string>(fields.at("email").value);
            age = std::get<int32_t>(fields.at("age").value);
        }
        catch (const std::out_of_range &e)
        {
            // Handle cases where a field might be missing in the BSON document
            std::cerr << "Error deserializing User: missing field. " << e.what() << std::endl;
        }
        catch (const std::bad_variant_access &e)
        {
            // Handle cases where a field has the wrong type in the BSON document
            std::cerr << "Error deserializing User: type mismatch. " << e.what() << std::endl;
        }
    }

    void print() const
    {
        std::cout << "User ID: " << get_id_str() << "\n"
                  << "  Name:  " << name << "\n"
                  << "  Email: " << email << "\n"
                  << "  Age:   " << age << std::endl;
    }
};
