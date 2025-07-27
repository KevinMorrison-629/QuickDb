#pragma once

#include "quickdb/components/collection.h" // Note: May need forward declarations to avoid circular includes

#include "quickdb/components/exception.h"

#include <memory>
#include <string>

#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>

// Forward declarations for mongocxx types
namespace mongocxx
{
    class instance;
    class pool;
} // namespace mongocxx

namespace QDB
{
    class Database
    {
    public:
        // Constructor initializes the instance and connection pool.
        Database(const std::string &uri);

        ~Database();

        // Disable copy and move semantics to ensure single ownership of the connection.
        Database(const Database &) = delete;
        Database &operator=(const Database &) = delete;
        Database(Database &&) = delete;
        Database &operator=(Database &&) = delete;

        // The factory method for getting a type-safe collection handle.
        template <typename T> Collection<T> get_collection(const std::string &db_name, const std::string &collection_name)
        {
            static_assert(std::is_base_of<Document, T>::value, "Template argument T must be a subclass of QDB::Document");

            auto client_entry = std::make_unique<mongocxx::pool::entry>(m_pool->acquire());
            auto collection_handle = (*(*client_entry))[db_name][collection_name];
            return Collection<T>(std::move(client_entry), collection_handle);
        }

    private:
        static mongocxx::instance &get_instance();

        std::unique_ptr<mongocxx::pool> m_pool;
    };
} // namespace QDB