#pragma once

#include "quickdb/components/field.h"

#include <bsoncxx/oid.hpp>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace QDB
{
    // forward declare the Collection class
    template <typename T> class Collection;

    class Document
    {
    public:
        virtual ~Document() = default;

        // User must implement this to convert their class members into a map of FieldValues.
        virtual std::unordered_map<std::string, FieldValue> to_fields() const = 0;

        // User must implement this to populate their class members from a map of FieldValues.
        virtual void from_fields(const std::unordered_map<std::string, FieldValue> &fields) = 0;

        /// @brief Gets the document's ObjectId as a hex string.
        /// @return The 24-character hex string representation of the _id.
        std::string get_id_str() const { return _id.to_string(); }

        /// @brief Gets the document's raw bsoncxx::oid object.
        /// @return The bsoncxx::oid object for this document.
        bsoncxx::oid get_id() const { return _id; }

        // The Collection<T> class will be a friend to access the protected _id member.
        template <typename T> friend class Collection;

    protected:
        friend class Collection<Document>;

        template <typename T> friend class Collection;

        // Automatically managed by the library
        bsoncxx::oid _id;
    };

    /// @brief Helper to safely get a field and deserialize it into an output variable.
    /// @tparam T The desired C++ type for the output.
    /// @param fields The map of fields from the database.
    /// @param key The name of the field to extract.
    /// @param out_val The variable to populate with the field's value.
    /// @return true if field exists in map, else false
    template <typename T>
    bool get_field(const std::unordered_map<std::string, FieldValue> &fields, const std::string &key, T &out_val)
    {
        auto it = fields.find(key);
        if (it != fields.end())
        {
            out_val = it->second.as<T>();
            return true;
        }

        return false;
    }

    // Forward declaration for the recursive helper
    void print_field_value(const FieldValue &fv, int indent_level);

    /// @brief Prints a key-value pair with proper indentation.
    /// @param key The key to print.
    /// @param fv The FieldValue to print.
    /// @param indent_level The current level of indentation.
    inline void print_kv_pair(const std::string &key, const FieldValue &fv, int indent_level)
    {
        std::cout << std::string(indent_level * 2, ' ') << std::left << std::setw(20) << ("\"" + key + "\":");
        print_field_value(fv, indent_level);
        std::cout << "\n";
    }

    /// @brief Recursively prints the content of a FieldValue.
    /// @param fv The FieldValue to print.
    /// @param indent_level The current indentation level for nested structures.
    inline void print_field_value(const FieldValue &fv, int indent_level)
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
