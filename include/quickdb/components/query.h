#pragma once

#include "quickdb/components/field.h"

#include <vector>

namespace QDB
{
    /**
     * @brief A fluent interface for building MongoDB query filters.
     *
     * This class allows for the construction of query documents in a type-safe
     * manner without needing to manually create BSON.
     */
    class Query
    {
    public:
        Query() = default;

        /**
         * @brief Creates a query to find a document by its ObjectId string.
         * @param id_str The hexadecimal string representation of the ObjectId.
         * @return A Query object for the specified ID.
         */
        static Query by_id(const std::string &id_str)
        {
            Query q;
            q.add_condition("_id", FieldValue(bsoncxx::oid(id_str)));
            return q;
        }

        /**
         * @brief Creates a query to find a document by its ObjectId.
         * @param id_str The ObjectId.
         * @return A Query object for the specified ID.
         */
        static Query by_id(const bsoncxx::oid &id)
        {
            Query q;
            q.add_condition("_id", FieldValue(id));
            return q;
        }

        /**
         * @brief Adds an equality condition to the query.
         * @tparam T The type of the value.
         * @param field The document field to match.
         * @param value The value to match.
         * @return A reference to the current Query object for chaining.
         */
        template <typename T> Query &eq(const std::string &field, const T &value)
        {
            add_condition(field, FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "not equal" ($ne) condition.
         */
        template <typename T> Query &ne(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$ne", FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "greater than" ($gt) condition.
         */
        template <typename T> Query &gt(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$gt", FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "greater than or equal" ($gte) condition.
         */
        template <typename T> Query &gte(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$gte", FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "less than" ($lt) condition.
         */
        template <typename T> Query &lt(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$lt", FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds a "less than or equal" ($lte) condition.
         */
        template <typename T> Query &lte(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$lte", FieldValue(value));
            return *this;
        }

        /**
         * @brief Adds an "in" ($in) condition.
         */
        template <typename T> Query &in(const std::string &field, const std::vector<T> &values)
        {
            std::vector<FieldValue> fv_vector;
            for (const auto &val : values)
            {
                fv_vector.emplace_back(val);
            }
            add_operator_condition(field, "$in", FieldValue(fv_vector));
            return *this;
        }

        /**
         * @brief Gets the underlying field map representing the query.
         * @return A constant reference to the query's field map.
         */
        const std::unordered_map<std::string, FieldValue> &get_fields() const { return _query_map; }

    private:
        void add_condition(const std::string &field, const FieldValue &fv) { _query_map[field] = fv; }

        void add_operator_condition(const std::string &field, const std::string &op, const FieldValue &fv)
        {
            std::unordered_map<std::string, FieldValue> condition_map = {{op, fv}};
            _query_map[field] = FieldValue(condition_map);
        }

        std::unordered_map<std::string, FieldValue> _query_map;
    };

} // namespace QDB
