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
} // namespace QDB
