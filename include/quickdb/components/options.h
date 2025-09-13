#pragma once

#include "quickdb/components/field.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/options/find.hpp>
#include <optional>

namespace QDB
{
    /**
     * @brief A class for specifying options for find operations, such as sorting and limiting.
     *
     * This class acts as a wrapper around mongocxx::options::find to integrate
     * with the QDB query builder.
     */
    class FindOptions
    {
    public:
        FindOptions() = default;

        /**
         * @brief Adds a sort criterion to the query options.
         * @param key The field to sort by.
         * @param direction The sort direction (1 for ascending, -1 for descending).
         * @return A reference to the current FindOptions object for chaining.
         */
        FindOptions &sort(const std::string &key, int direction)
        {
            _sort_builder.append(bsoncxx::builder::basic::kvp(key, direction));
            return *this;
        }

        /**
         * @brief Sets the maximum number of documents to return.
         * @param limit The maximum number of documents.
         * @return A reference to the current FindOptions object for chaining.
         */
        FindOptions &limit(int64_t limit)
        {
            _limit = limit;
            return *this;
        }

        /**
         * @brief Gets the underlying mongocxx::options::find object.
         * @return The configured mongocxx::options::find object.
         */
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
            return opts;
        }

    private:
        bsoncxx::builder::basic::document _sort_builder{};
        std::optional<int64_t> _limit;
    };
} // namespace QDB
