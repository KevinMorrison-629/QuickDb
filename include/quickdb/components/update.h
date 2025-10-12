#pragma once

#include "quickdb/components/field.h"

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace QDB
{
    /**
     * @brief A fluent interface for building MongoDB update documents.
     */
    class Update
    {
    public:
        Update() = default;

        /**
         * @brief Adds a "$set" operation to the update.
         * @param field The document field to set.
         * @param value The value to set for the field.
         */
        template <typename T> Update &set(const std::string &field, const T &value)
        {
            add_operator_field("$set", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$push" operation to the update.
         * @param field The array field to modify.
         * @param value The value to append to the array.
         */
        template <typename T> Update &push(const std::string &field, const T &value)
        {
            add_operator_field("$push", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$push" operation with an "$each" modifier.
         * @param field The array field to modify.
         * @param values A vector of values to append to the array.
         */
        template <typename T> Update &push_each(const std::string &field, const std::vector<T> &values)
        {
            std::unordered_map<std::string, FieldValue> each_map;
            each_map["$each"] = FieldValue(values);
            add_operator_field("$push", field, FieldValue(each_map));
            return *this;
        }

        /**
         * @brief Adds a "$pull" operation to the update.
         * @param field The array field to modify.
         * @param value The value to remove from the array.
         */
        template <typename T> Update &pull(const std::string &field, const T &value)
        {
            add_operator_field("$pull", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$pull" operation with an "$each" modifier.
         * @param field The array field to modify.
         * @param values A vector of values to remove from the array.
         */
        template <typename T> Update &pull_each(const std::string &field, const std::vector<T> &values)
        {
            std::unordered_map<std::string, FieldValue> each_map;
            each_map["$each"] = FieldValue(values);
            add_operator_field("$pull", field, FieldValue(each_map));
            return *this;
        }

        /**
         * @brief Adds a "$pullAll" operation to remove all instances of the specified values from an array.
         * @param field The array field to modify.
         * @param values A vector of values to remove from the array.
         */
        template <typename T> Update &pullAll(const std::string &field, const std::vector<T> &values)
        {
            add_operator_field("$pullAll", field, FieldValue(values));
            return *this;
        }

        /**
         * @brief Adds an "$addToSet" operation to the update.
         * @param field The array field to modify.
         * @param value The value to add to the set.
         */
        template <typename T> Update &add_to_set(const std::string &field, const T &value)
        {
            add_operator_field("$addToSet", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$bit" operation to perform bitwise AND, OR, and XOR updates.
         * @param field The field to update.
         * @param operation The bitwise operation to perform ("and", "or", or "xor").
         * @param value The integer value to use in the bitwise operation.
         */
        Update &bit(const std::string &field, const std::string &operation, int32_t value)
        {
            if (operation != "and" && operation != "or" && operation != "xor")
            {
                // For safety, you might want to throw an exception for invalid operations
                return *this;
            }
            std::unordered_map<std::string, FieldValue> bit_op_map = {{operation, FieldValue(value)}};
            add_operator_field("$bit", field, FieldValue(bit_op_map));
            return *this;
        }

        /**
         * @brief Adds an "$inc" operation to the update.
         * @param field The numeric field to increment.
         * @param amount The amount to increment by.
         */
        template <typename T> Update &inc(const std::string &field, const T &amount)
        {
            add_operator_field("$inc", field, FieldValue(amount));
            return *this;
        }

        /**
         * @brief Adds a "$mul" operation to multiply a field's value.
         * @param field The numeric field to multiply.
         * @param amount The number to multiply by.
         */
        template <typename T> Update &mul(const std::string &field, const T &amount)
        {
            add_operator_field("$mul", field, FieldValue(amount));
            return *this;
        }

        /**
         * @brief Adds a "$min" operation to update a field if the new value is less than the current value.
         * @param field The field to update.
         * @param value The value to compare against.
         */
        template <typename T> Update &min(const std::string &field, const T &value)
        {
            add_operator_field("$min", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$max" operation to update a field if the new value is greater than the current value.
         * @param field The field to update.
         * @param value The value to compare against.
         */
        template <typename T> Update &max(const std::string &field, const T &value)
        {
            add_operator_field("$max", field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "$pop" operation to remove the first or last element of an array.
         * @param field The array field.
         * @param direction -1 to remove the first element, 1 to remove the last.
         */
        Update &pop(const std::string &field, int direction)
        {
            add_operator_field("$pop", field, FieldValue(static_cast<int32_t>(direction)));
            return *this;
        }

        /**
         * @brief Adds a "$rename" operation to rename a field.
         * @param old_name The current name of the field.
         * @param new_name The new name for the field.
         */
        Update &rename(const std::string &old_name, const std::string &new_name)
        {
            add_operator_field("$rename", old_name, FieldValue(new_name));
            return *this;
        }

        /**
         * @brief Adds a "$currentDate" operation to set a field to the current date.
         * @param field The field to set.
         * @param as_timestamp If true, sets as a BSON timestamp; otherwise, sets as a BSON date.
         */
        Update &current_date(const std::string &field, bool as_timestamp = false)
        {
            add_operator_field("$currentDate", field, FieldValue(as_timestamp));
            return *this;
        }

        /**
         * @brief Adds an "$unset" operation to the update.
         * @param field The field to remove.
         */
        Update &unset(const std::string &field)
        {
            add_operator_field("$unset", field, FieldValue(""));
            return *this;
        }

        /**
         * @brief Gets the underlying field map representing the update document.
         */
        const std::unordered_map<std::string, FieldValue> &get_fields() const { return _update_map; }

    private:
        /**
         * @brief Helper function to construct the nested update document structure.
         */
        void add_operator_field(const std::string &op, const std::string &field, const FieldValue &fv)
        {
            auto it = _update_map.find(op);
            if (it == _update_map.end())
            {
                _update_map.emplace(op, FieldValue(std::unordered_map<std::string, FieldValue>{{field, fv}}));
            }
            else
            {
                if (std::holds_alternative<std::unordered_map<std::string, FieldValue>>(it->second.value))
                {
                    auto &existing_map = std::get<std::unordered_map<std::string, FieldValue>>(it->second.value);
                    existing_map[field] = fv;
                }
            }
        }

        std::unordered_map<std::string, FieldValue> _update_map;
    };

} // namespace QDB
