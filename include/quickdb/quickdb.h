#pragma once

#include "quickdb/components/collection.h" // Note: May need forward declarations to avoid circular includes
#include "quickdb/components/exception.h"

#include <cstdint>
#include <memory>
#include <string>

// Use the library's official forward-declaration headers.
// This avoids re-declaration errors and keeps this header file lightweight.
#include <mongocxx/instance-fwd.hpp>
#include <mongocxx/pool-fwd.hpp>

namespace QDB
{
    class Database
    {
    public:
        // Constructor that takes a raw MongoDB URI string.
        Database(const std::string &uri);

        // Constructor for authenticated connections.
        // This constructor builds the URI string for you.
        Database(const std::string &user, const std::string &pass, const std::string &host = "localhost",
                 std::uint16_t port = 27017, const std::string &auth_db = "admin", std::uint32_t max_pool_size = 50);

        ~Database();

        // Disable copy and move semantics to ensure single ownership of the connection.
        Database(const Database &) = delete;
        Database &operator=(const Database &) = delete;
        Database(Database &&) = delete;
        Database &operator=(Database &&) = delete;

        // The factory method for getting a type-safe collection handle.
        template <typename T> Collection<T> get_collection(const std::string &db_name, const std::string &collection_name)
        {
            auto client_entry = std::make_unique<mongocxx::pool::entry>(m_pool->acquire());
            auto collection_handle = (*(*client_entry))[db_name][collection_name];
            return Collection<T>(std::move(client_entry), collection_handle);
        }

    private:
        static mongocxx::instance &get_instance();

        std::unique_ptr<mongocxx::pool> m_pool;
    };
} // namespace QDB