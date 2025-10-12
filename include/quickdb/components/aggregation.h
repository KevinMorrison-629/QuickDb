#pragma once

#include "quickdb/components/document.h"
#include "quickdb/components/field.h"
#include "quickdb/components/query.h"
#include <mongocxx/pipeline.hpp>
#include <vector>

namespace QDB
{
    /**
     * @brief A helper class to build BSON documents for aggregation stages.
     */
    class DocumentBuilder
    {
    public:
        DocumentBuilder() = default;

        /**
         * @brief Constructs a builder with an initial key-value pair.
         * @param key The document field.
         * @param value The value for the field.
         */
        DocumentBuilder(const std::string &key, const FieldValue &value) { _doc_map[key] = value; }

        /**
         * @brief Adds a field to the document.
         * @param key The document field.
         * @param value The value for the field.
         * @return A reference to the current builder for chaining.
         */
        DocumentBuilder &add_field(const std::string &key, const FieldValue &value)
        {
            _doc_map[key] = value;
            return *this;
        }

        /**
         * @brief Adds a nested document as a field.
         * @param key The document field.
         * @param builder The nested DocumentBuilder.
         * @return A reference to the current builder for chaining.
         */
        DocumentBuilder &add_field(const std::string &key, const DocumentBuilder &builder)
        {
            _doc_map[key] = FieldValue(builder._doc_map);
            return *this;
        }

        /**
         * @brief Builds the BSON document view from the added fields.
         * @return A bsoncxx basic document.
         */
        bsoncxx::builder::basic::document build() const
        {
            bsoncxx::builder::basic::document builder;
            for (const auto &[key, fv] : _doc_map)
            {
                AppendToDocument(builder, key, fv);
            }
            return builder;
        }

    private:
        std::unordered_map<std::string, FieldValue> _doc_map;
    };

    /**
     * @brief A fluent interface for building MongoDB aggregation pipelines.
     */
    class Aggregation
    {
    public:
        Aggregation() = default;

        /**
         * @brief Adds a $match stage to the pipeline.
         * @param query The query filter to apply.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &match(const Query &query)
        {
            bsoncxx::builder::basic::document match_doc;
            for (const auto &[key, fv] : query.get_fields())
            {
                AppendToDocument(match_doc, key, fv);
            }
            _pipeline.match(match_doc.view());
            return *this;
        }

        /**
         * @brief Adds a $group stage to the pipeline.
         * @param group_doc The document defining the group stage.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &group(const DocumentBuilder &group_doc)
        {
            _pipeline.group(group_doc.build().view());
            return *this;
        }

        /**
         * @brief Adds a $project stage to the pipeline.
         * @param project_doc The document defining the projection.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &project(const DocumentBuilder &project_doc)
        {
            _pipeline.project(project_doc.build().view());
            return *this;
        }

        /**
         * @brief Adds a $sort stage to the pipeline.
         * @param sort_doc The document defining the sort order.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &sort(const DocumentBuilder &sort_doc)
        {
            _pipeline.sort(sort_doc.build().view());
            return *this;
        }

        /**
         * @brief Adds a $lookup (left outer join) stage to the pipeline.
         * @param from The target collection to join with.
         * @param local_field The field from the input documents.
         * @param foreign_field The field from the documents of the "from" collection.
         * @param as The output array field name.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &lookup(const std::string &from, const std::string &local_field, const std::string &foreign_field,
                            const std::string &as)
        {
            DocumentBuilder lookup_doc;
            lookup_doc.add_field("from", FieldValue(from));
            lookup_doc.add_field("localField", FieldValue(local_field));
            lookup_doc.add_field("foreignField", FieldValue(foreign_field));
            lookup_doc.add_field("as", FieldValue(as));
            _pipeline.lookup(lookup_doc.build().view());
            return *this;
        }

        /**
         * @brief Adds an $unwind stage to deconstruct an array field.
         * @param field The field path to an array field (e.g., "$items").
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &unwind(const std::string &field)
        {
            bsoncxx::builder::basic::document unwind_doc;
            unwind_doc.append(bsoncxx::builder::basic::kvp("path", field));
            _pipeline.unwind(unwind_doc.view());
            return *this;
        }

        /**
         * @brief Adds a $limit stage to the pipeline.
         * @param limit The maximum number of documents to pass to the next stage.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &limit(int64_t limit)
        {
            _pipeline.limit(limit);
            return *this;
        }

        /**
         * @brief Adds a $skip stage to the pipeline.
         * @param skip The number of documents to skip.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &skip(int64_t skip)
        {
            _pipeline.skip(skip);
            return *this;
        }

        /**
         * @brief Adds a $count stage to the pipeline.
         * @param output_field The name of the output field that will contain the count.
         * @return A reference to the current Aggregation object for chaining.
         */
        Aggregation &count(const std::string &output_field)
        {
            _pipeline.count(output_field);
            return *this;
        }

        /**
         * @brief Gets the underlying mongocxx::pipeline object.
         * @return The configured mongocxx::pipeline.
         */
        const mongocxx::pipeline &to_mongocxx() const { return _pipeline; }

    private:
        mongocxx::pipeline _pipeline{};
    };
} // namespace QDB
