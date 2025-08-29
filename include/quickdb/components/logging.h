#pragma once

#include "quickdb/components/document.h"

#include <iomanip>
#include <iostream>
#include <string>

namespace QDB
{
    // Forward declaration for the recursive helper
    void print_field_value(const FieldValue &fv, int indent_level);

    /// @brief Prints a key-value pair with proper indentation.
    /// @param key The key to print.
    /// @param fv The FieldValue to print.
    /// @param indent_level The current level of indentation.
    void print_kv_pair(const std::string &key, const FieldValue &fv, int indent_level)
    {
        std::cout << std::string(indent_level * 2, ' ') << std::left << std::setw(20) << ("\"" + key + "\":");
        print_field_value(fv, indent_level);
        std::cout << "\n";
    }

    /// @brief Recursively prints the content of a FieldValue.
    /// @param fv The FieldValue to print.
    /// @param indent_level The current indentation level for nested structures.
    void print_field_value(const FieldValue &fv, int indent_level)
    {
        switch (fv.type)
        {
        case FieldType::FT_OBJECT:
        {
            const auto &map = std::get<std::unordered_map<std::string, FieldValue>>(fv.value);
            std::cout << "{\n";
            for (const auto &[key, value] : map)
            {
                print_kv_pair(key, value, indent_level + 1);
            }
            std::cout << std::string(indent_level * 2, ' ') << "}";
            break;
        }
        case FieldType::FT_ARRAY:
        {
            const auto &vec = std::get<std::vector<FieldValue>>(fv.value);
            std::cout << "[\n";
            for (const auto &item : vec)
            {
                std::cout << std::string((indent_level + 1) * 2, ' ');
                print_field_value(item, indent_level + 1);
                std::cout << ",\n";
            }
            std::cout << std::string(indent_level * 2, ' ') << "]";
            break;
        }
        case FieldType::FT_STRING:
            std::cout << "\"" << fv.as<std::string>() << "\"";
            break;
        case FieldType::FT_OBJECT_ID:
            std::cout << "ObjectId(\"" << fv.as<bsoncxx::oid>().to_string() << "\")";
            break;
        case FieldType::FT_INT_32:
            std::cout << fv.as<int32_t>();
            break;
        case FieldType::FT_INT_64:
            std::cout << fv.as<int64_t>();
            break;
        case FieldType::FT_DOUBLE:
            std::cout << fv.as<double>();
            break;
        case FieldType::FT_BOOLEAN:
            std::cout << (fv.as<bool>() ? "true" : "false");
            break;
        case FieldType::FT_NULL:
            std::cout << "null";
            break;
        default:
            std::cout << "[Unsupported Type]";
            break;
        }
    }

    /// @brief A generic template function to print any QDB::Document subclass.
    /// @tparam T A type that inherits from QDB::Document.
    /// @param doc The document object to print.
    template <typename T> void print_document(const T &doc)
    {
        static_assert(std::is_base_of_v<Document, T>, "T must be a subclass of QDB::Document");
        std::cout << "{\n";
        print_kv_pair("_id", doc.get_id(), 1);

        auto fields = doc.to_fields();
        for (const auto &[key, value] : fields)
        {
            if (key == "_id")
                continue; // Already printed
            print_kv_pair(key, value, 1);
        }
        std::cout << "}\n" << std::endl;
    }

} // namespace QDB