#pragma once

#include "quickdb/components/aggregation.h"
#include "quickdb/components/field.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/find_one_and_delete.hpp>
#include <mongocxx/options/find_one_and_replace.hpp>
#include <mongocxx/options/find_one_and_update.hpp>
#include <mongocxx/options/update.hpp>
#include <optional>

namespace QDB
{
    /// @brief A class for specifying options for find operations.
    ///
    /// This class acts as a wrapper around mongocxx::options::find to integrate
    /// with the QDB query builder.
    class FindOptions
    {
    public:
        FindOptions() = default;

        /// @brief Adds a sort criterion to the query options.
        /// @param key The field to sort by.
        /// @param direction The sort direction (1 for ascending, -1 for descending).
        /// @return A reference to the current FindOptions object for chaining.
        FindOptions &sort(const std::string &key, int direction)
        {
            _sort_builder.append(bsoncxx::builder::basic::kvp(key, direction));
            return *this;
        }

        /// @brief Sets the maximum number of documents to return.
        /// @param limit The maximum number of documents.
        /// @return A reference to the current FindOptions object for chaining.
        FindOptions &limit(int64_t limit)
        {
            _limit = limit;
            return *this;
        }

        /// @brief Sets the number of documents to skip before returning results.
        /// @param skip The number of documents to skip.
        /// @return A reference to the current FindOptions object for chaining.
        FindOptions &skip(int64_t skip)
        {
            _skip = skip;
            return *this;
        }

        /// @brief Sets a projection to limit the fields returned in the matching documents.
        /// @param projection_doc A DocumentBuilder object defining the projection.
        /// @return A reference to the current object for chaining.
        FindOptions &projection(const DocumentBuilder &projection_doc)
        {
            _projection_builder = projection_doc.build().extract();
            return *this;
        }

        /// @brief Gets the underlying mongocxx::options::find object.
        /// @return The configured mongocxx::options::find object.
        mongocxx::options::find to_mongocxx() const
        {
            mongocxx::options::find opts{};
            if (!_sort_builder.view().empty())
            {
                opts.sort(_sort_builder.view());
            }
            if (_limit.has_value())
            {
                opts.limit(_limit.value());
            }
            if (_skip.has_value())
            {
                opts.skip(_skip.value());
            }
            if (_projection_builder)
            {
                opts.projection(_projection_builder->view());
            }
            return opts;
        }

    private:
        /// @brief BSON builder for sort criteria.
        bsoncxx::builder::basic::document _sort_builder{};
        /// @brief Optional BSON document for projection.
        std::optional<bsoncxx::document::value> _projection_builder;
        /// @brief Optional limit on the number of documents to return.
        std::optional<int64_t> _limit;
        /// @brief Optional number of documents to skip.
        std::optional<int64_t> _skip;
    };

    /// @brief A class for specifying options for update operations.
    class UpdateOptions
    {
    public:
        UpdateOptions() = default;

        /// @brief If set to true, a new document is inserted if no document matches the filter.
        /// @param is_upsert True to enable upsert, false to disable.
        /// @return A reference to the current object for chaining.
        UpdateOptions &upsert(bool is_upsert)
        {
            _upsert = is_upsert;
            return *this;
        }

        /// @brief Gets the underlying mongocxx::options::update object.
        /// @return The configured mongocxx::options::update object.
        mongocxx::options::update to_mongocxx() const
        {
            mongocxx::options::update opts{};
            if (_upsert.has_value())
            {
                opts.upsert(_upsert.value());
            }
            return opts;
        }

    private:
        /// @brief Optional flag to enable or disable upsert.
        std::optional<bool> _upsert;
    };

    /// @brief Specifies whether a find-and-modify operation should return the document
    /// from before the modification or after.
    enum class ReturnDocument
    {
        kBefore, ///< Return the document before the modification.
        kAfter   ///< Return the document after the modification.
    };

    /// @brief A class for specifying options for find-and-modify operations like
    /// find_one_and_update, find_one_and_replace, and find_one_and_delete.
    class FindAndModifyOptions
    {
    public:
        FindAndModifyOptions() = default;

        /// @brief Adds a sort criterion to the operation. The first document found in this order will be modified.
        /// @param key The field to sort by.
        /// @param direction The sort direction (1 for ascending, -1 for descending).
        /// @return A reference to the current object for chaining.
        FindAndModifyOptions &sort(const std::string &key, int direction)
        {
            _sort_builder.append(bsoncxx::builder::basic::kvp(key, direction));
            return *this;
        }

        /// @brief Adds a field to the projection, limiting the fields returned.
        /// @param field The field to include or exclude.
        /// @param include 1 to include the field, 0 to exclude it.
        /// @return A reference to the current object for chaining.
        FindAndModifyOptions &projection(const std::string &field, int include)
        {
            _projection_builder.append(bsoncxx::builder::basic::kvp(field, include));
            return *this;
        }

        /// @brief If set to true, a new document is inserted if no document matches the filter.
        /// (Applies to find_one_and_update and find_one_and_replace).
        /// @param is_upsert True to enable upsert, false to disable.
        /// @return A reference to the current object for chaining.
        FindAndModifyOptions &upsert(bool is_upsert)
        {
            _upsert = is_upsert;
            return *this;
        }

        /// @brief Configures whether to return the document before or after the modification.
        /// (Applies to find_one_and_update and find_one_and_replace).
        /// @param rd kBefore or kAfter. Defaults to kBefore.
        /// @return A reference to the current object for chaining.
        FindAndModifyOptions &return_document(ReturnDocument rd)
        {
            _return_document = rd;
            return *this;
        }

    private:
        template <typename T> friend class Collection;

        /// @brief BSON builder for sort criteria.
        bsoncxx::builder::basic::document _sort_builder{};
        /// @brief BSON builder for projection criteria.
        bsoncxx::builder::basic::document _projection_builder{};
        /// @brief Optional flag to enable or disable upsert.
        std::optional<bool> _upsert;
        /// @brief Optional setting for which document version to return.
        std::optional<ReturnDocument> _return_document;
    };

} // namespace QDB
