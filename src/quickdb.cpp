#include "quickdb/quickdb.h"
#include "quickdb/components/exception.h"

// All mongocxx headers are included ONLY in the .cpp file.
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>

namespace QDB
{
    // The mongocxx::instance must be created once per process.
    // This static method ensures that rule is followed.
    mongocxx::instance &Database::get_instance()
    {
        static mongocxx::instance instance{};
        return instance;
    }

    Database::Database(const std::string &uri_string)
    {
        try
        {
            // First, ensure the global driver instance is initialized.
            get_instance();

            // Then, create the connection pool for this Database object.
            mongocxx::uri uri(uri_string);
            m_pool = std::make_unique<mongocxx::pool>(uri);
        }
        catch (const std::exception &e)
        {
            // Wrap any driver exception in our custom exception type.
            throw QDB::Exception(e.what());
        }
    }

    Database::Database(const std::string &user, const std::string &pass, const std::string &host, std::uint16_t port,
                       const std::string &auth_db, std::uint32_t max_pool_size)
    {
        try
        {
            // First, ensure the global driver instance is initialized.
            get_instance();

            // Assemble the full URI from the provided components.
            std::string uri_string = "mongodb://" + user + ":" + pass + "@" + host + ":" + std::to_string(port) +
                                     "/?authSource=" + auth_db + "&maxPoolSize=" + std::to_string(max_pool_size);

            mongocxx::uri uri(uri_string);
            m_pool = std::make_unique<mongocxx::pool>(uri);
        }
        catch (const std::exception &e)
        {
            // Wrap any driver exception in our custom exception type.
            throw QDB::Exception(e.what());
        }
    }

    Database::~Database() = default; // Default destructor is fine with unique_ptr

    void Database::with_transaction(std::function<void(mongocxx::client_session &session)> callback)
    {
        // Acquire a client from the pool for the scope of this transaction.
        auto client = m_pool->acquire();
        // A session must be started on a specific client.
        auto session = client->start_session();

        try
        {
            // The user's operations are executed within a transaction context.
            session.with_transaction(
                [&](mongocxx::client_session *callback_session)
                {
                    // The mongocxx driver provides a raw pointer here, we pass it as a reference.
                    callback(*callback_session);
                });
        }
        catch (const std::exception &e)
        {
            // If anything inside the lambda fails, mongocxx driver aborts the transaction
            // and throws an exception. We wrap it in our own exception type.
            throw QDB::Exception("Transaction failed: " + std::string(e.what()));
        }
    }

} // namespace QDB
