#pragma once

#include "quickdb/components/field.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace QDB
{
    class Document
    {
    public:
        virtual ~Document() = default;

        // User must implement this to convert their class members into a map of FieldValues.
        virtual std::unordered_map<std::string, FieldValue> to_fields() const = 0;

        // User must implement this to populate their class members from a map of FieldValues.
        virtual void from_fields(const std::unordered_map<std::string, FieldValue> &fields) = 0;

        // Helper to get the document's ObjectId as a string.
        std::string get_id_str() const
        {
            if (_id.type == FieldType::FT_OBJECT_ID)
            {
                return std::get<bsoncxx::oid>(_id.value).to_string();
            }
            return "";
        }

        FieldValue get_id() const { return _id; }

        // The Collection<T> class will be a friend to access the protected _id member.
        template <typename T> friend class Collection;

    protected:
        friend class Collection<Document>;

        template <typename T> friend class Collection;

        // Automatically managed by the library.
        FieldValue _id;
    };

    /// @brief Helper to safely get a field and deserialize it into an output variable.
    /// @tparam T The desired C++ type for the output.
    /// @param fields The map of fields from the database.
    /// @param key The name of the field to extract.
    /// @param out_val The variable to populate with the field's value.
    template <typename T>
    void get_field(const std::unordered_map<std::string, FieldValue> &fields, const std::string &key, T &out_val)
    {
        auto it = fields.find(key);
        if (it != fields.end())
        {
            out_val = it->second.as<T>();
        }
    }
} // namespace QDB
