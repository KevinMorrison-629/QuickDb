#pragma once

#include "quickdb/components/aggregation.h"
#include "quickdb/components/document.h"
#include "quickdb/components/exception.h"
#include "quickdb/components/field.h"
#include "quickdb/components/options.h"
#include "quickdb/components/query.h"
#include "quickdb/components/update.h"

// Standard library includes
#include <functional> // For std::reference_wrapper
#include <iostream>
#include <optional>
#include <type_traits>
#include <vector>

// MongoDB C++ driver includes
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/index_view.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/find_one_and_delete.hpp>
#include <mongocxx/options/find_one_and_replace.hpp>
#include <mongocxx/options/find_one_and_update.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/result/delete.hpp>
#include <mongocxx/result/insert_many.hpp>
#include <mongocxx/result/insert_one.hpp>
#include <mongocxx/result/update.hpp>

// Forward-declare client_session
namespace mongocxx
{
    class client_session;
}

namespace QDB
{
    /// @brief A template class providing a type-safe wrapper around a mongocxx::collection.
    /// @tparam T A class that inherits from QDB::Document.
    template <typename T> class Collection
    {
        static_assert(std::is_base_of_v<Document, T>, "Template argument T must be a subclass of QDB::Document");

    public:
        /// @brief Constructs a Collection handler.
        /// @param client_entry A unique_ptr to the connection pool entry.
        /// @param collection_handle The underlying mongocxx collection handle.
        Collection(std::unique_ptr<mongocxx::pool::entry> client_entry, mongocxx::collection collection_handle)
            : _client_entry(std::move(client_entry)), _collection_handle(std::move(collection_handle))
        {
        }

        /// @brief Creates a single document in the collection.
        /// @param doc The document object to insert.
        /// @param session An optional session to use for the operation.
        /// @return The number of documents inserted (1 on success).
        int64_t create_one(T &doc, std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto bson_doc = to_bson_doc(doc.to_fields());
                bsoncxx::v_noabi::stdx::optional<mongocxx::result::insert_one> result;
                if (session)
                {
                    result = _collection_handle.insert_one(session->get(), bson_doc.view());
                }
                else
                {
                    result = _collection_handle.insert_one(bson_doc.view());
                }

                if (result)
                {
                    doc._id = result->inserted_id().get_oid().value;
                    return 1;
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to create document: " + std::string(e.what()));
            }
        }

        /// @brief Creates multiple documents in the collection.
        /// @param docs A vector of document objects to insert.
        /// @param session An optional session to use for the operation.
        /// @return The number of documents inserted.
        int64_t create_many(std::vector<T> &docs,
                            std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
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

                bsoncxx::v_noabi::stdx::optional<mongocxx::result::insert_many> result;
                if (session)
                {
                    result = _collection_handle.insert_many(session->get(), bson_docs);
                }
                else
                {
                    result = _collection_handle.insert_many(bson_docs);
                }

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
                throw QDB::Exception("Failed to create many documents: " + std::string(e.what()));
            }
        }

        /// @brief Finds a single document matching the query.
        /// @param query The query filter.
        /// @param options The find options (e.g., sort, projection).
        /// @param session An optional session to use for the operation.
        /// @return An std::optional containing the found document, or std::nullopt.
        std::optional<T> find_one(const Query &query, const FindOptions &options = FindOptions{},
                                  std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                bsoncxx::v_noabi::stdx::optional<bsoncxx::document::value> result;
                if (session)
                {
                    result = _collection_handle.find_one(session->get(), filter.view(), options.to_mongocxx());
                }
                else
                {
                    result = _collection_handle.find_one(filter.view(), options.to_mongocxx());
                }

                if (result)
                {
                    return from_bson_doc(result->view());
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to find one document: " + std::string(e.what()));
            }
        }

        /// @brief Finds all documents matching the query.
        /// @param query The query filter.
        /// @param options The find options (e.g., sort, limit, skip).
        /// @param session An optional session to use for the operation.
        /// @return A std::vector of documents.
        std::vector<T> find_many(const Query &query, const FindOptions &options = FindOptions{},
                                 std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            std::vector<T> results;
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                mongocxx::cursor cursor = session
                                              ? _collection_handle.find(session->get(), filter.view(), options.to_mongocxx())
                                              : _collection_handle.find(filter.view(), options.to_mongocxx());

                for (const auto &view : cursor)
                {
                    results.push_back(from_bson_doc(view));
                }
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to find many documents: " + std::string(e.what()));
            }
            return results;
        }

        /// @brief Updates a single document that matches the filter.
        /// @param filter_query A Query object defining which document to update.
        /// @param update_doc An Update object defining the update operations.
        /// @param options Options for the operation (e.g., upsert).
        /// @param session An optional session to use for the operation.
        /// @return The number of documents modified.
        int64_t update_one(const Query &filter_query, const Update &update_doc,
                           const UpdateOptions &options = UpdateOptions{},
                           std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(filter_query.get_fields());
                auto update = to_bson_doc(update_doc.get_fields());
                auto mongocxx_opts = options.to_mongocxx();

                bsoncxx::v_noabi::stdx::optional<mongocxx::result::update> result;
                if (session)
                {
                    result = _collection_handle.update_one(session->get(), filter.view(), update.view(), mongocxx_opts);
                }
                else
                {
                    result = _collection_handle.update_one(filter.view(), update.view(), mongocxx_opts);
                }

                if (result)
                {
                    return result->modified_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to update one document: " + std::string(e.what()));
            }
        }

        /// @brief Updates all documents that match the filter.
        /// @param filter_query A Query object defining which documents to update.
        /// @param update_doc An Update object defining the update operations.
        /// @param options Options for the operation (e.g., upsert).
        /// @param session An optional session to use for the operation.
        /// @return The number of documents modified.
        int64_t update_many(const Query &filter_query, const Update &update_doc,
                            const UpdateOptions &options = UpdateOptions{},
                            std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(filter_query.get_fields());
                auto update = to_bson_doc(update_doc.get_fields());
                auto mongocxx_opts = options.to_mongocxx();

                bsoncxx::v_noabi::stdx::optional<mongocxx::result::update> result;
                if (session)
                {
                    result = _collection_handle.update_many(session->get(), filter.view(), update.view(), mongocxx_opts);
                }
                else
                {
                    result = _collection_handle.update_many(filter.view(), update.view(), mongocxx_opts);
                }

                if (result)
                {
                    return result->modified_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to update many documents: " + std::string(e.what()));
            }
        }

        /// @brief Deletes a single document that matches the filter.
        /// @param query The query filter.
        /// @param session An optional session to use for the operation.
        /// @return The number of documents deleted.
        int64_t delete_one(const Query &query,
                           std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                bsoncxx::v_noabi::stdx::optional<mongocxx::result::delete_result> result;
                if (session)
                {
                    result = _collection_handle.delete_one(session->get(), filter.view());
                }
                else
                {
                    result = _collection_handle.delete_one(filter.view());
                }

                if (result)
                {
                    return result->deleted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to delete one document: " + std::string(e.what()));
            }
        }

        /// @brief Deletes all documents that match the filter.
        /// @param query The query filter.
        /// @param session An optional session to use for the operation.
        /// @return The number of documents deleted.
        int64_t delete_many(const Query &query,
                            std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                bsoncxx::v_noabi::stdx::optional<mongocxx::result::delete_result> result;
                if (session)
                {
                    result = _collection_handle.delete_many(session->get(), filter.view());
                }
                else
                {
                    result = _collection_handle.delete_many(filter.view());
                }
                if (result)
                {
                    return result->deleted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to delete many documents: " + std::string(e.what()));
            }
        }

        /// @brief Counts the number of documents matching the filter.
        /// @param query The query filter.
        /// @param session An optional session to use for the operation.
        /// @return The number of matching documents.
        int64_t count_documents(const Query &query = Query{},
                                std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                if (session)
                {
                    return _collection_handle.count_documents(session->get(), filter.view());
                }
                else
                {
                    return _collection_handle.count_documents(filter.view());
                }
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to count documents: " + std::string(e.what()));
            }
        }

        /// @brief Executes an aggregation pipeline.
        /// @tparam ResultType The Document subclass to deserialize results into. Defaults to T.
        /// @param aggregation The Aggregation object defining the pipeline.
        /// @param session An optional session to use for the operation.
        /// @return A std::vector of ResultType documents.
        template <typename ResultType = T>
        std::vector<ResultType>
        aggregate(const Aggregation &aggregation,
                  std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            static_assert(std::is_base_of_v<Document, ResultType>, "ResultType must be a subclass of QDB::Document");

            std::vector<ResultType> results;
            try
            {
                mongocxx::cursor cursor = session ? _collection_handle.aggregate(session->get(), aggregation.to_mongocxx())
                                                  : _collection_handle.aggregate(aggregation.to_mongocxx());
                for (const auto &view : cursor)
                {
                    ResultType doc;
                    std::unordered_map<std::string, FieldValue> fields;
                    for (const auto &element : view)
                    {
                        std::string key = static_cast<std::string>(element.key());
                        if (key == "_id" && element.type() == bsoncxx::type::k_oid)
                        {
                            doc._id = element.get_oid().value;
                        }
                        else
                        {
                            fields[key] = fromBsonElement(element);
                        }
                    }
                    doc.from_fields(fields);
                    results.push_back(doc);
                }
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to execute aggregation: " + std::string(e.what()));
            }
            return results;
        }

        /// @brief Finds a single document and updates it in one atomic operation.
        /// @param query The selection criteria for the update.
        /// @param update The modifications to apply.
        /// @param options Options for the operation (e.g., sort, projection, upsert, return_document).
        /// @param session An optional session for transactional context.
        /// @return An std::optional containing the document (either before or after modification), or std::nullopt if no
        /// document was found.
        std::optional<T>
        find_one_and_update(const Query &query, const Update &update,
                            const FindAndModifyOptions &options = FindAndModifyOptions{},
                            std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                auto update_doc = to_bson_doc(update.get_fields());

                mongocxx::options::find_one_and_update mongocxx_opts{};
                if (!options._sort_builder.view().empty())
                {
                    mongocxx_opts.sort(options._sort_builder.view());
                }
                if (!options._projection_builder.view().empty())
                {
                    mongocxx_opts.projection(options._projection_builder.view());
                }
                if (options._upsert.has_value())
                {
                    mongocxx_opts.upsert(options._upsert.value());
                }
                if (options._return_document.has_value())
                {
                    auto rd = (options._return_document.value() == ReturnDocument::kAfter)
                                  ? mongocxx::options::return_document::k_after
                                  : mongocxx::options::return_document::k_before;
                    mongocxx_opts.return_document(rd);
                }

                bsoncxx::v_noabi::stdx::optional<bsoncxx::document::value> result;
                if (session)
                {
                    result = _collection_handle.find_one_and_update(session->get(), filter.view(), update_doc.view(),
                                                                    mongocxx_opts);
                }
                else
                {
                    result = _collection_handle.find_one_and_update(filter.view(), update_doc.view(), mongocxx_opts);
                }

                if (result)
                {
                    return from_bson_doc(result->view());
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("find_one_and_update failed: " + std::string(e.what()));
            }
        }

        /// @brief Finds a single document and replaces it in one atomic operation.
        /// @param query The selection criteria for the replacement.
        /// @param replacement The new document to replace the found one.
        /// @param options Options for the operation (e.g., sort, projection, upsert, return_document).
        /// @param session An optional session for transactional context.
        /// @return An std::optional containing the document (either before or after replacement), or std::nullopt if no
        /// document was found.
        std::optional<T>
        find_one_and_replace(const Query &query, const T &replacement,
                             const FindAndModifyOptions &options = FindAndModifyOptions{},
                             std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());
                auto replacement_doc = to_bson_doc(replacement.to_fields());

                mongocxx::options::find_one_and_replace mongocxx_opts{};
                if (!options._sort_builder.view().empty())
                {
                    mongocxx_opts.sort(options._sort_builder.view());
                }
                if (!options._projection_builder.view().empty())
                {
                    mongocxx_opts.projection(options._projection_builder.view());
                }
                if (options._upsert.has_value())
                {
                    mongocxx_opts.upsert(options._upsert.value());
                }
                if (options._return_document.has_value())
                {
                    auto rd = (options._return_document.value() == ReturnDocument::kAfter)
                                  ? mongocxx::options::return_document::k_after
                                  : mongocxx::options::return_document::k_before;
                    mongocxx_opts.return_document(rd);
                }

                bsoncxx::v_noabi::stdx::optional<bsoncxx::document::value> result;
                if (session)
                {
                    result = _collection_handle.find_one_and_replace(session->get(), filter.view(), replacement_doc.view(),
                                                                     mongocxx_opts);
                }
                else
                {
                    result = _collection_handle.find_one_and_replace(filter.view(), replacement_doc.view(), mongocxx_opts);
                }

                if (result)
                {
                    return from_bson_doc(result->view());
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("find_one_and_replace failed: " + std::string(e.what()));
            }
        }

        /// @brief Finds a single document and deletes it in one atomic operation.
        /// @param query The selection criteria for the deletion.
        /// @param options Options for the operation (e.g., sort, projection).
        /// @param session An optional session for transactional context.
        /// @return An std::optional containing the deleted document, or std::nullopt if no document was found.
        std::optional<T>
        find_one_and_delete(const Query &query, const FindAndModifyOptions &options = FindAndModifyOptions{},
                            std::optional<std::reference_wrapper<mongocxx::client_session>> session = std::nullopt)
        {
            try
            {
                auto filter = to_bson_doc(query.get_fields());

                mongocxx::options::find_one_and_delete mongocxx_opts{};
                if (!options._sort_builder.view().empty())
                {
                    mongocxx_opts.sort(options._sort_builder.view());
                }
                if (!options._projection_builder.view().empty())
                {
                    mongocxx_opts.projection(options._projection_builder.view());
                }

                bsoncxx::v_noabi::stdx::optional<bsoncxx::document::value> result;
                if (session)
                {
                    result = _collection_handle.find_one_and_delete(session->get(), filter.view(), mongocxx_opts);
                }
                else
                {
                    result = _collection_handle.find_one_and_delete(filter.view(), mongocxx_opts);
                }

                if (result)
                {
                    return from_bson_doc(result->view());
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("find_one_and_delete failed: " + std::string(e.what()));
            }
        }

        // --- Index Management ---

        /// @brief Creates a single-field index.
        /// @param field The name of the field to index.
        /// @param ascending True for an ascending index (1), false for descending (-1).
        /// @return The name of the created index.
        std::string create_index(const std::string &field, bool ascending = true)
        {
            try
            {
                bsoncxx::builder::basic::document keys;
                keys.append(bsoncxx::builder::basic::kvp(field, ascending ? 1 : -1));
                auto result = _collection_handle.indexes().create_one(keys.view());
                if (result)
                {
                    return *result;
                }
                return field + (ascending ? "_1" : "_-1");
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to create index: " + std::string(e.what()));
            }
        }

        /// @brief Creates a compound index on multiple fields.
        /// @param fields A vector of pairs, where each pair contains the field name and a boolean for ascending order.
        /// @return The name of the created index.
        std::string create_compound_index(const std::vector<std::pair<std::string, bool>> &fields)
        {
            if (fields.empty())
            {
                throw QDB::Exception("Cannot create a compound index with no fields.");
            }
            try
            {
                bsoncxx::builder::basic::document keys;
                for (const auto &field_pair : fields)
                {
                    keys.append(bsoncxx::builder::basic::kvp(field_pair.first, field_pair.second ? 1 : -1));
                }
                auto result = _collection_handle.indexes().create_one(keys.view());
                if (result)
                {
                    return *result;
                }
                std::string generated_name;
                for (const auto &field_pair : fields)
                {
                    generated_name += field_pair.first + (field_pair.second ? "_1_" : "_-1_");
                }
                if (!generated_name.empty())
                {
                    generated_name.pop_back();
                }
                return generated_name;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to create compound index: " + std::string(e.what()));
            }
        }

        /// @brief Creates a text index on specified fields.
        /// @param fields A vector of field names to include in the text index.
        /// @return The name of the created index.
        std::string create_text_index(const std::vector<std::string> &fields)
        {
            if (fields.empty())
            {
                throw QDB::Exception("Cannot create a text index with no fields.");
            }
            try
            {
                bsoncxx::builder::basic::document keys;
                for (const auto &field : fields)
                {
                    keys.append(bsoncxx::builder::basic::kvp(field, "text"));
                }
                auto result = _collection_handle.indexes().create_one(keys.view());
                if (result)
                {
                    return *result;
                }
                return "text_index";
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to create text index: " + std::string(e.what()));
            }
        }

        /// @brief Drops a specific index by name.
        /// @param index_name The name of the index to drop.
        void drop_index(const std::string &index_name)
        {
            try
            {
                _collection_handle.indexes().drop_one(index_name);
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to drop index '" + index_name + "': " + std::string(e.what()));
            }
        }

        /// @brief Lists the names of all indexes on the collection.
        /// @return A vector of strings, where each string is an index name.
        std::vector<std::string> list_indexes()
        {
            std::vector<std::string> index_names;
            try
            {
                auto cursor = _collection_handle.indexes().list();
                for (const auto &doc : cursor)
                {
                    if (auto ele = doc["name"]; ele && ele.type() == bsoncxx::type::k_string)
                    {
                        index_names.emplace_back(ele.get_string().value);
                    }
                }
                return index_names;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception("Failed to list indexes: " + std::string(e.what()));
            }
        }

    private:
        /// @brief Converts a map of FieldValues to a BSON document.
        /// @param fields The map of fields to convert.
        /// @return The BSON document value.
        bsoncxx::document::value to_bson_doc(const std::unordered_map<std::string, FieldValue> &fields) const
        {
            bsoncxx::builder::basic::document builder;
            for (const auto &[key, value] : fields)
            {
                AppendToDocument(builder, key, value);
            }
            return builder.extract();
        }

        /// @brief Converts a BSON document view to a document of type T.
        /// @param view The BSON document view to convert.
        /// @return The deserialized document object.
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
        /// @brief This unique_ptr owns the client connection, keeping it alive.
        std::unique_ptr<mongocxx::pool::entry> _client_entry;

        /// @brief The collection handle itself. It is dependent on the client from _client_entry.
        mongocxx::collection _collection_handle;
    };
} // namespace QDB