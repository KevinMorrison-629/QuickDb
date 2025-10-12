#pragma once

#include "quickdb/components/document.h"
#include "quickdb/components/exception.h"
#include "quickdb/components/field.h"
#include "quickdb/components/options.h"
#include "quickdb/components/query.h"
#include "quickdb/components/update.h"

// Standard library includes
#include <iostream>
#include <optional>
#include <type_traits>
#include <vector>

// MongoDB C++ driver includes
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/result/delete.hpp>
#include <mongocxx/result/insert_many.hpp>
#include <mongocxx/result/insert_one.hpp>
#include <mongocxx/result/update.hpp>

namespace QDB
{
    /**
     * @brief A template class providing a type-safe wrapper around a mongocxx::collection.
     * @tparam T A class that inherits from QDB::Document.
     */
    template <typename T> class Collection
    {
        static_assert(std::is_base_of_v<Document, T>, "Template argument T must be a subclass of QDB::Document");

    public:
        Collection(std::unique_ptr<mongocxx::pool::entry> client_entry, mongocxx::collection collection_handle)
            : _client_entry(std::move(client_entry)), _collection_handle(std::move(collection_handle))
        {
        }

        /**
         * @brief Creates a single document in the collection.
         * @param doc The document object to insert.
         * @return The number of documents inserted (1 on success, 0 on failure).
         */
        int64_t create_one(T &doc)
        {
            try
            {
                auto bson_doc = to_bson_doc(doc.to_fields());
                auto result = _collection_handle.insert_one(bson_doc.view());

                if (result)
                {
                    doc._id = result->inserted_id().get_oid().value;
                    return 1;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to create document: " << e.what() << std::endl;
                return 0;
            }
        }

        /**
         * @brief Creates multiple documents in the collection.
         * @param docs A vector of document objects to insert.
         * @return The number of documents inserted.
         */
        int64_t create_many(std::vector<T> &docs)
        {
            if (docs.empty())
                return 0;

            try
            {
                std::vector<bsoncxx::document::value> bson_docs;
                bson_docs.reserve(docs.size());
                for (const auto &doc : docs)
                {
                    bson_docs.push_back(to_bson_doc(doc.to_fields()));
                }

                auto result = _collection_handle.insert_many(bson_docs);
                if (result)
                {
                    const auto &inserted_ids = result->inserted_ids();
                    for (size_t i = 0; i < docs.size(); ++i)
                    {
                        if (auto it = inserted_ids.find(static_cast<int32_t>(i)); it != inserted_ids.end())
                        {
                            docs[i]._id = it->second.get_oid().value;
                        }
                    }
                    return result->result().inserted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to create many documents: " << e.what() << std::endl;
                return 0;
            }
        }

        /**
         * @brief Finds a single document matching the query.
         * @param query The query filter.
         * @param options The find options (e.g., sort, projection).
         * @return An std::optional containing the found document, or std::nullopt.
         */
        std::optional<T> find_one(const Query &query, const FindOptions &options = FindOptions{})
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                auto result = _collection_handle.find_one(filter.view(), options.to_mongocxx());
                if (result)
                {
                    return from_bson_doc(result->view());
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to find one document: " << e.what() << std::endl;
                return std::nullopt;
            }
        }

        /**
         * @brief Finds all documents matching the query.
         * @param query The query filter.
         * @param options The find options (e.g., sort, limit, skip).
         * @return A std::vector of documents.
         */
        std::vector<T> find_many(const Query &query, const FindOptions &options = FindOptions{})
        {
            std::vector<T> results;
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                mongocxx::cursor cursor = _collection_handle.find(filter.view(), options.to_mongocxx());
                for (const auto &view : cursor)
                {
                    results.push_back(from_bson_doc(view));
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to find many documents: " << e.what() << std::endl;
            }
            return results;
        }

        /**
         * @brief Updates a single document that matches the filter.
         * @param filter_query A Query object defining which document to update.
         * @param update_doc An Update object defining the update operations.
         * @return The number of documents modified.
         */
        int64_t update_one(const Query &filter_query, const Update &update_doc)
        {
            try
            {
                auto filter = to_bson_doc(filter_query.get_fields());
                auto update = to_bson_doc(update_doc.get_fields());
                auto result = _collection_handle.update_one(filter.view(), update.view());
                if (result)
                {
                    return result->modified_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to update one document: " << e.what() << std::endl;
                return 0;
            }
        }

        /**
         * @brief Updates all documents that match the filter.
         * @param filter_query A Query object defining which documents to update.
         * @param update_doc An Update object defining the update operations.
         * @return The number of documents modified.
         */
        int64_t update_many(const Query &filter_query, const Update &update_doc)
        {
            try
            {
                auto filter = to_bson_doc(filter_query.get_fields());
                auto update = to_bson_doc(update_doc.get_fields());
                auto result = _collection_handle.update_many(filter.view(), update.view());
                if (result)
                {
                    return result->modified_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to update many documents: " << e.what() << std::endl;
                return 0;
            }
        }

        /**
         * @brief Deletes a single document that matches the filter.
         * @param query The query filter.
         * @return The number of documents deleted.
         */
        int64_t delete_one(const Query &query)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                auto result = _collection_handle.delete_one(filter.view());
                if (result)
                {
                    return result->deleted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to delete one document: " << e.what() << std::endl;
                return 0;
            }
        }

        /**
         * @brief Deletes all documents that match the filter.
         * @param query The query filter.
         * @return The number of documents deleted.
         */
        int64_t delete_many(const Query &query)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                auto result = _collection_handle.delete_many(filter.view());
                if (result)
                {
                    return result->deleted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to delete many documents: " << e.what() << std::endl;
                return 0;
            }
        }

        /**
         * @brief Counts the number of documents matching the filter.
         * @param query The query filter.
         * @return The number of matching documents.
         */
        int64_t count_documents(const Query &query = Query{})
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                return _collection_handle.count_documents(filter.view());
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to count documents: " << e.what() << std::endl;
                return 0;
            }
        }

    private:
        bsoncxx::document::value to_bson_doc(const std::unordered_map<std::string, FieldValue> &fields) const
        {
            bsoncxx::builder::basic::document builder;
            for (const auto &[key, value] : fields)
            {
                AppendToDocument(builder, key, value);
            }
            return builder.extract();
        }

        T from_bson_doc(const bsoncxx::document::view &view) const
        {
            T doc;
            std::unordered_map<std::string, FieldValue> fields;
            for (const auto &element : view)
            {
                if (element.key() == "_id")
                {
                    doc._id = element.get_oid().value;
                }
                else
                {
                    fields[static_cast<std::string>(element.key())] = fromBsonElement(element);
                }
            }
            doc.from_fields(fields);
            return doc;
        }

    private:
        // This unique_ptr owns the client connection, keeping it alive.
        std::unique_ptr<mongocxx::pool::entry> _client_entry;

        // The collection handle itself. It is dependent on the client from _client_entry.
        mongocxx::collection _collection_handle;
    };
} // namespace QDB
