#pragma once

#include "quickdb/components/field.h"

#include <vector>

namespace QDB
{
    /// @brief A fluent interface for building MongoDB query filters.
    ///
    /// This class allows for the construction of query documents in a type-safe
    /// manner without needing to manually create BSON.
    class Query
    {
    public:
        Query() = default;

        /// @brief Creates a query to find a document by its ObjectId string.
        /// @param id_str The hexadecimal string representation of the ObjectId.
        /// @return A Query object for the specified ID.
        static Query by_id(const std::string &id_str)
        {
            Query q;
            q.add_condition("_id", FieldValue(bsoncxx::oid(id_str)));
            return q;
        }

        /// @brief Creates a query to find a document by its ObjectId.
        /// @param id The ObjectId.
        /// @return A Query object for the specified ID.
        static Query by_id(const bsoncxx::oid &id)
        {
            Query q;
            q.add_condition("_id", FieldValue(id));
            return q;
        }

        /// @brief Creates a logical OR query from a list of queries.
        /// @param queries An initializer list of Query objects.
        /// @return A new Query object representing the $or condition.
        static Query Or(const std::initializer_list<Query> &queries)
        {
            Query q;
            std::vector<FieldValue> query_docs;
            for (const auto &query : queries)
            {
                query_docs.emplace_back(query.get_fields());
            }
            q._query_map["$or"] = FieldValue(query_docs);
            return q;
        }

        /// @brief Creates a logical OR query from a list of queries.
        /// @param queries A vector of Query objects.
        /// @return A new Query object representing the $or condition.
        static Query Or(const std::vector<Query> &queries)
        {
            Query q;
            std::vector<FieldValue> query_docs;
            for (const auto &query : queries)
            {
                query_docs.emplace_back(query.get_fields());
            }
            q._query_map["$or"] = FieldValue(query_docs);
            return q;
        }

        /// @brief Creates a logical AND query from a list of queries.
        /// @param queries An initializer list of Query objects.
        /// @return A new Query object representing the $and condition.
        static Query And(const std::initializer_list<Query> &queries)
        {
            Query q;
            std::vector<FieldValue> query_docs;
            for (const auto &query : queries)
            {
                query_docs.emplace_back(query.get_fields());
            }
            q._query_map["$and"] = FieldValue(query_docs);
            return q;
        }

        /// @brief Creates a logical AND query from a list of queries.
        /// @param queries A vector of Query objects.
        /// @return A new Query object representing the $and condition.
        static Query And(const std::vector<Query> &queries)
        {
            Query q;
            std::vector<FieldValue> query_docs;
            for (const auto &query : queries)
            {
                query_docs.emplace_back(query.get_fields());
            }
            q._query_map["$and"] = FieldValue(query_docs);
            return q;
        }

        /// @brief Adds an equality condition to the query.
        /// @tparam T The type of the value.
        /// @param field The document field to match.
        /// @param value The value to match.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &eq(const std::string &field, const T &value)
        {
            add_condition(field, FieldValue(value));
            return *this;
        }

        /// @brief Adds a "not equal" ($ne) condition.
        /// @tparam T The type of the value.
        /// @param field The document field.
        /// @param value The value to compare against.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &ne(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$ne", FieldValue(value));
            return *this;
        }

        /// @brief Adds a "greater than" ($gt) condition.
        /// @tparam T The type of the value.
        /// @param field The document field.
        /// @param value The value to compare against.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &gt(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$gt", FieldValue(value));
            return *this;
        }

        /// @brief Adds a "greater than or equal" ($gte) condition.
        /// @tparam T The type of the value.
        /// @param field The document field.
        /// @param value The value to compare against.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &gte(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$gte", FieldValue(value));
            return *this;
        }

        /// @brief Adds a "less than" ($lt) condition.
        /// @tparam T The type of the value.
        /// @param field The document field.
        /// @param value The value to compare against.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &lt(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$lt", FieldValue(value));
            return *this;
        }

        /// @brief Adds a "less than or equal" ($lte) condition.
        /// @tparam T The type of the value.
        /// @param field The document field.
        /// @param value The value to compare against.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &lte(const std::string &field, const T &value)
        {
            add_operator_condition(field, "$lte", FieldValue(value));
            return *this;
        }

        /// @brief Adds an "in" ($in) condition.
        /// @tparam T The type of the values in the vector.
        /// @param field The document field.
        /// @param values A vector of values to match against.
        /// @return A reference to the current Query object for chaining.
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

        /// @brief Adds an "all" ($all) condition to match arrays containing all specified elements.
        /// @tparam T The type of the values in the vector.
        /// @param field The array field.
        /// @param values A vector of values that must all be present.
        /// @return A reference to the current Query object for chaining.
        template <typename T> Query &all(const std::string &field, const std::vector<T> &values)
        {
            std::vector<FieldValue> fv_vector;
            for (const auto &val : values)
            {
                fv_vector.emplace_back(val);
            }
            add_operator_condition(field, "$all", FieldValue(fv_vector));
            return *this;
        }

        /// @brief Adds an "exists" ($exists) condition.
        /// @param field The document field to check for existence.
        /// @param value True to check for existence, false for non-existence.
        /// @return A reference to the current Query object for chaining.
        Query &exists(const std::string &field, bool value = true)
        {
            add_operator_condition(field, "$exists", FieldValue(value));
            return *this;
        }

        /// @brief Adds a "modulo" ($mod) condition.
        /// @param field The field to test.
        /// @param divisor The divisor for the modulo operation.
        /// @param remainder The remainder to match.
        /// @return A reference to the current Query object for chaining.
        Query &mod(const std::string &field, int64_t divisor, int64_t remainder)
        {
            std::vector<FieldValue> fv_vector;
            fv_vector.emplace_back(divisor);
            fv_vector.emplace_back(remainder);
            add_operator_condition(field, "$mod", FieldValue(fv_vector));
            return *this;
        }

        /// @brief Adds an "element match" ($elemMatch) condition for arrays.
        /// @param field The array field to query.
        /// @param query The query to apply to elements of the array.
        /// @return A reference to the current Query object for chaining.
        Query &elemMatch(const std::string &field, const Query &query)
        {
            add_operator_condition(field, "$elemMatch", FieldValue(query.get_fields()));
            return *this;
        }

        /// @brief Adds a regular expression match ($regex) condition.
        /// @param field The document field to match.
        /// @param pattern The regex pattern.
        /// @param options MongoDB regex options (e.g., "i" for case-insensitivity).
        /// @return A reference to the current Query object for chaining.
        Query &regex(const std::string &field, const std::string &pattern, const std::string &options = "")
        {
            // BSON format for regex is a nested document: { field: { $regex: 'pattern', $options: 'i' } }
            std::unordered_map<std::string, FieldValue> regex_map;
            regex_map["$regex"] = FieldValue(pattern);
            if (!options.empty())
            {
                regex_map["$options"] = FieldValue(options);
            }
            _query_map[field] = FieldValue(regex_map);
            return *this;
        }

        /// @brief Adds a text search condition ($text).
        /// @param search_term The string to search for.
        /// @return A reference to the current Query object for chaining.
        Query &text(const std::string &search_term)
        {
            // BSON format for text search: { $text: { $search: "term" } }
            std::unordered_map<std::string, FieldValue> text_search_map;
            text_search_map["$search"] = FieldValue(search_term);
            _query_map["$text"] = FieldValue(text_search_map);
            return *this;
        }

        /// @brief Gets the underlying field map representing the query.
        /// @return A constant reference to the query's field map.
        const std::unordered_map<std::string, FieldValue> &get_fields() const { return _query_map; }

    private:
        /// @brief Adds a simple key-value condition to the query map.
        /// @param field The field name.
        /// @param fv The field value.
        void add_condition(const std::string &field, const FieldValue &fv) { _query_map[field] = fv; }

        /// @brief Adds a condition with a MongoDB operator (e.g., $gt, $ne).
        /// @param field The field name.
        /// @param op The MongoDB operator.
        /// @param fv The field value.
        void add_operator_condition(const std::string &field, const std::string &op, const FieldValue &fv)
        {
            // [FIX] This logic now correctly merges operator conditions instead of overwriting them.
            auto it = _query_map.find(field);
            if (it != _query_map.end() && it->second.type == FieldType::FT_OBJECT)
            {
                // If the field already has an operator, add the new one to the existing sub-document.
                auto &map = std::get<std::unordered_map<std::string, FieldValue>>(it->second.value);
                map[op] = fv;
            }
            else
            {
                // Otherwise, create a new sub-document for the operator.
                std::unordered_map<std::string, FieldValue> condition_map = {{op, fv}};
                _query_map[field] = FieldValue(condition_map);
            }
        }

        /// @brief The internal map holding the query conditions.
        std::unordered_map<std::string, FieldValue> _query_map;
    };

} // namespace QDB