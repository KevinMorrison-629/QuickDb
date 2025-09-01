#pragma once

#include "quickdb/components/document.h"
#include "quickdb/components/exception.h"
#include "quickdb/components/query.h"

#include <memory>
#include <optional>
#include <vector>

// All required mongocxx headers are included here.
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/options/insert.hpp>
#include <mongocxx/pool.hpp>

namespace QDB
{
    template <typename T> class Collection
    {
        friend class Database;

    public:
        void insert_one(T &doc)
        {
            try
            {
                bsoncxx::builder::basic::document builder;
                auto fields = doc.to_fields();
                for (const auto &[key, value] : fields)
                {
                    AppendToDocument(builder, key, value);
                }

                auto result = _collection_handle.insert_one(builder.view());
                if (result && result->inserted_id().type() == bsoncxx::type::k_oid)
                {
                    doc._id = FieldValue(result->inserted_id().get_oid().value);
                }
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        void insert_many(std::vector<T> &docs)
        {
            if (docs.empty())
                return;

            try
            {
                std::vector<bsoncxx::document::value> bson_docs;
                for (const auto &doc : docs)
                {
                    bsoncxx::builder::basic::document builder;
                    auto fields = doc.to_fields();
                    for (const auto &[key, value] : fields)
                    {
                        AppendToDocument(builder, key, value);
                    }
                    bson_docs.push_back(builder.extract());
                }

                _collection_handle.insert_many(bson_docs);
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        std::optional<T> find_one(const Query &query)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : query.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }

                auto maybe_result = _collection_handle.find_one(filter_builder.view());

                if (maybe_result)
                {
                    T doc_obj;
                    auto view = maybe_result->view();
                    std::unordered_map<std::string, FieldValue> fields;

                    for (auto element : view)
                    {
                        fields[std::string(element.key())] = fromBsonElement(element);
                    }

                    doc_obj.from_fields(fields);
                    if (view.find("_id") != view.end())
                    {
                        doc_obj._id = fromBsonElement(view["_id"]);
                    }
                    return doc_obj;
                }
                return std::nullopt;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        std::vector<T> find(const Query &query)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : query.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }

                mongocxx::cursor cursor = _collection_handle.find(filter_builder.view());
                std::vector<T> results;

                for (auto view : cursor)
                {
                    T doc_obj;
                    std::unordered_map<std::string, FieldValue> fields;
                    for (auto element : view)
                    {
                        fields[std::string(element.key())] = fromBsonElement(element);
                    }
                    doc_obj.from_fields(fields);
                    if (view.find("_id") != view.end())
                    {
                        doc_obj._id = fromBsonElement(view["_id"]);
                    }
                    results.push_back(doc_obj);
                }
                return results;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        std::vector<T> find_random(const Query &query, size_t count)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : query.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }

                mongocxx::pipeline p{};
                p.match(filter_builder.view());
                p.sample(static_cast<int64_t>(count));

                auto cursor = _collection_handle.aggregate(p, mongocxx::options::aggregate{});
                std::vector<T> results;

                for (auto view : cursor)
                {
                    T doc_obj;
                    std::unordered_map<std::string, FieldValue> fields;
                    for (auto element : view)
                    {
                        fields[std::string(element.key())] = fromBsonElement(element);
                    }
                    doc_obj.from_fields(fields);
                    if (view.find("_id") != view.end())
                    {
                        doc_obj._id = fromBsonElement(view["_id"]);
                    }
                    results.push_back(doc_obj);
                }
                return results;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        int64_t replace_one(const Query &filter, const T &update)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : filter.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }

                bsoncxx::builder::basic::document update_builder;
                auto fields = update.to_fields();
                for (const auto &[key, value] : fields)
                {
                    AppendToDocument(update_builder, key, value);
                }

                auto result = _collection_handle.replace_one(filter_builder.view(), update_builder.view());
                if (result)
                {
                    return result->modified_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        int64_t delete_one(const Query &filter)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : filter.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }

                auto result = _collection_handle.delete_one(filter_builder.view());
                if (result)
                {
                    return result->deleted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        int64_t delete_many(const Query &filter)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : filter.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }

                auto result = _collection_handle.delete_many(filter_builder.view());
                if (result)
                {
                    return result->deleted_count();
                }
                return 0;
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

        int64_t count_documents(const Query &filter)
        {
            try
            {
                bsoncxx::builder::basic::document filter_builder;
                for (const auto &[key, value] : filter.get_fields())
                {
                    AppendToDocument(filter_builder, key, value);
                }
                return _collection_handle.count_documents(filter_builder.view());
            }
            catch (const std::exception &e)
            {
                throw QDB::Exception(e.what());
            }
        }

    private:
        Collection(std::unique_ptr<mongocxx::pool::entry> client_entry, mongocxx::collection collection_handle)
            : _client_entry(std::move(client_entry)), _collection_handle(std::move(collection_handle))
        {
            static_assert(std::is_base_of<Document, T>::value, "Template argument T must be a subclass of QDB::Document");
        }

        // This unique_ptr owns the client connection, keeping it alive.
        std::unique_ptr<mongocxx::pool::entry> _client_entry;

        // The collection handle itself. It is dependent on the client from _client_entry.
        mongocxx::collection _collection_handle;
    };
} // namespace QDB