#pragma once

#include "quickdb/components/collection.h" // Note: May need forward declarations to avoid circular includes
#include "quickdb/components/exception.h"
#include "quickdb/components/gridfs.h"

#include <cstdint>
#include <memory>
#include <string>

// Use the library's official forward-declaration headers.
// This avoids re-declaration errors and keeps this header file lightweight.
#include <mongocxx/instance-fwd.hpp>
#include <mongocxx/pool-fwd.hpp>

namespace QDB
{
    // Forward-declare the GridFSBucket class to avoid including its header here.
    class GridFSBucket;

    /// @brief Main database class for managing connections and collections.
    class Database
    {
    public:
        /// @brief Constructor that takes a raw MongoDB URI string.
        /// @param uri The MongoDB connection URI.
        Database(const std::string &uri);

        /// @brief Constructor for authenticated connections.
        /// This constructor builds the URI string for you.
        /// @param user The username for authentication.
        /// @param pass The password for authentication.
        /// @param host The database host.
        /// @param port The database port.
        /// @param auth_db The authentication database.
        /// @param max_pool_size The maximum size of the connection pool.
        Database(const std::string &user, const std::string &pass, const std::string &host = "localhost",
                 std::uint16_t port = 27017, const std::string &auth_db = "admin", std::uint32_t max_pool_size = 50);

        ~Database();

        // Disable copy and move semantics to ensure single ownership of the connection.
        Database(const Database &) = delete;
        Database &operator=(const Database &) = delete;
        Database(Database &&) = delete;
        Database &operator=(Database &&) = delete;

        /// @brief The factory method for getting a type-safe collection handle.
        /// @tparam T The Document subclass for this collection.
        /// @param db_name The name of the database.
        /// @param collection_name The name of the collection.
        /// @return A type-safe Collection object.
        template <typename T> Collection<T> get_collection(const std::string &db_name, const std::string &collection_name)
        {
            auto client_entry = std::make_unique<mongocxx::pool::entry>(m_pool->acquire());
            auto collection_handle = (*(*client_entry))[db_name][collection_name];
            return Collection<T>(std::move(client_entry), collection_handle);
        }

        /// @brief Executes a series of operations within a transaction.
        ///
        /// This method handles the entire lifecycle of a transaction. It starts a session,
        /// begins a transaction, executes the user-provided callback, and then attempts
        /// to commit. If the callback throws an exception, the transaction is automatically
        /// aborted. This ensures atomicity for all operations within the callback.
        /// @param callback A function object that takes a `mongocxx::client_session&` and
        /// contains the database operations to be executed atomically. All
        /// collection methods called within this callback MUST be passed the
        /// session object to be part of the transaction.
        /// @throws QDB::Exception if the transaction fails to start, commit, or abort.
        void with_transaction(std::function<void(mongocxx::client_session &session)> callback);

        /// @brief The factory method for getting a GridFS bucket handle.
        /// @param db_name The name of the database to use for GridFS.
        /// @param bucket_name The optional name of the bucket. Defaults to "fs".
        /// @return A GridFSBucket object for file operations.
        GridFSBucket get_gridfs_bucket(const std::string &db_name, const std::string &bucket_name = "fs");

        /// @brief Pings the database to verify the connection.
        /// @throws QDB::Exception if the ping command fails.
        void ping();

    private:
        /// @brief Gets the singleton mongocxx::instance.
        /// @return A reference to the mongocxx::instance.
        static mongocxx::instance &get_instance();

        /// @brief The connection pool.
        std::unique_ptr<mongocxx::pool> m_pool;
    };
} // namespace QDB
