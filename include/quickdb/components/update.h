#pragma once

#include "quickdb/components/field.h" // Assuming field.h is in this path
#include <string>
#include <unordered_map>
#include <variant> // Include for std::get and std::holds_alternative
#include <vector>

namespace QDB
{
    /**
     * @brief A fluent interface for building MongoDB update documents.
     *
     * This class allows for the construction of update operations like $set, $push,
     * and $inc in a type-safe manner without needing to manually create BSON.
     */
    class Update
    {
    public:
        Update() = default;

        /**
         * @brief Adds a "$set" operation to the update.
         * Sets the value of a field in a document.
         * @tparam T The type of the value.
         * @param field The document field to set.
         * @param value The value to set for the field.
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &set(const std::string &field, const T &value)
        {
            add_operator_field("$set", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$push" operation to the update.
         * Appends a specified value to an array field.
         * @tparam T The type of the value to push.
         * @param field The array field to modify.
         * @param value The value to append to the array.
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &push(const std::string &field, const T &value)
        {
            add_operator_field("$push", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$push" operation with an "$each" modifier.
         * Appends multiple values to an array field.
         * @tparam T The type of the elements in the vector.
         * @param field The array field to modify.
         * @param values A vector of values to append to the array.
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &push_each(const std::string &field, const std::vector<T> &values)
        {
            // Create the subdocument for the $each operator, e.g., { "$each": [val1, val2] }
            std::unordered_map<std::string, FieldValue> each_map;
            each_map["$each"] = FieldValue(values);

            // Add the $push operation with the $each subdocument
            add_operator_field("$push", field, FieldValue(each_map));
            return *this;
        }

        /**
         * @brief Adds a "$pull" operation with an "$each" modifier.
         * Pulls multiple values from an array field.
         * @tparam T The type of the elements in the vector.
         * @param field The array field to modify.
         * @param values A vector of values to remove from the array.
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &pull_each(const std::string &field, const std::vector<T> &values)
        {
            // Create the subdocument for the $each operator, e.g., { "$each": [val1, val2] }
            std::unordered_map<std::string, FieldValue> each_map;
            each_map["$each"] = FieldValue(values);

            // Add the $pull operation with the $each subdocument
            add_operator_field("$pull", field, FieldValue(each_map));
            return *this;
        }

        /**
         * @brief Adds a "$pull" operation to the update.
         * Removes all instances of a value from an array.
         * @tparam T The type of the value to remove.
         * @param field The array field to modify.
         * @param value The value to remove from the array.
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &pull(const std::string &field, const T &value)
        {
            add_operator_field("$pull", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds an "$addToSet" operation to the update.
         * Adds a value to an array only if the value does not already exist.
         * @tparam T The type of the value to add.
         * @param field The array field to modify.
         * @param value The value to add to the set.
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &add_to_set(const std::string &field, const T &value)
        {
            add_operator_field("$addToSet", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds an "$inc" operation to the update.
         * Increments a field by a specified amount.
         * @tparam T A numeric type (e.g., int, double).
         * @param field The numeric field to increment.
         * @param amount The amount to increment the field by (can be negative).
         * @return A reference to the current Update object for chaining.
         */
        template <typename T> Update &inc(const std::string &field, const T &amount)
        {
            add_operator_field("$inc", field, FieldValue(amount));
            return *this;
        }

        /**
         * @brief Adds an "$unset" operation to the update.
         * Removes the specified field from a document.
         * @param field The field to remove.
         * @return A reference to the current Update object for chaining.
         */
        Update &unset(const std::string &field)
        {
            // The value for $unset doesn't matter, but an empty string is conventional.
            add_operator_field("$unset", field, FieldValue(""));
            return *this;
        }

        /**
         * @brief Gets the underlying field map representing the update document.
         * @return A constant reference to the update's field map.
         */
        const std::unordered_map<std::string, FieldValue> &get_fields() const { return _update_map; }

    private:
        /**
         * @brief A helper function to construct the nested update document structure.
         * It creates a structure like: { "$operator": { "field": value } }
         */
        void add_operator_field(const std::string &op, const std::string &field, const FieldValue &fv)
        {
            // Find the FieldValue associated with the operator (e.g., "$set").
            auto it = _update_map.find(op);

            if (it == _update_map.end())
            {
                // If the operator doesn't exist yet, create it with a new map containing the field.
                std::unordered_map<std::string, FieldValue> field_map = {{field, fv}};
                _update_map.emplace(op, FieldValue(field_map));
            }
            else
            {
                // If the operator already exists, get a mutable reference to its inner map and add the new field.
                // Your field.h shows that FieldVariant can hold a map of FieldValues.
                // The FieldValue struct exposes its FieldVariant member as a public variable named 'value'.
                if (std::holds_alternative<std::unordered_map<std::string, FieldValue>>(it->second.value))
                {
                    auto &existing_map = std::get<std::unordered_map<std::string, FieldValue>>(it->second.value);
                    existing_map[field] = fv;
                }
                // Optional: else branch to handle the case where the operator's value is not a map,
                // which would indicate an internal logic error.
            }
        }

        std::unordered_map<std::string, FieldValue> _update_map;
    };

} // namespace QDB
