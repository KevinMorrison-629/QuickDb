#pragma once

#include "quickdb/components/document.h"
#include "quickdb/components/field.h"
#include <string>
#include <unordered_map>

namespace QDB
{
    /**
     * @brief A template base class that uses a static schema method to implement serialization automatically.
     * * @tparam Derived The class inheriting from Model (CRTP pattern).
     */
    template <typename Derived> class Model : public Document
    {
    public:
        /**
         * @brief Serializes the object to a map of FieldValues using the defined schema.
         * @return std::unordered_map<std::string, FieldValue>
         */
        std::unordered_map<std::string, FieldValue> to_fields() const override
        {
            std::unordered_map<std::string, FieldValue> fields;

            // Visitor lambda: Takes a name and a value, converts value to FieldValue, and stores it.
            auto serializer = [&](const std::string &name, const auto &value) { fields[name] = FieldValue(value); };

            // Call the static schema method of the Derived class
            // We cast *this to const Derived& because we are serializing (reading) the data.
            Derived::schema(static_cast<const Derived &>(*this), serializer);

            return fields;
        }

        /**
         * @brief Deserializes a map of FieldValues into the object using the defined schema.
         * @param fields The map of fields from the database.
         */
        void from_fields(const std::unordered_map<std::string, FieldValue> &fields) override
        {
            // Visitor lambda: Takes a name and a reference to a class member.
            // Finds the name in the map and populates the member.
            auto deserializer = [&](const std::string &name, auto &member)
            {
                auto it = fields.find(name);
                if (it != fields.end())
                {
                    // Use the helper as<T>() method from FieldValue to convert safely.
                    member = it->second.template as<std::decay_t<decltype(member)>>();
                }
            };

            // Call the static schema method of the Derived class
            // We cast *this to Derived& because we are deserializing (writing) to the data.
            Derived::schema(static_cast<Derived &>(*this), deserializer);
        }
    };
} // namespace QDB