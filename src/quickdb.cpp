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

    Database::~Database() = default; // Default destructor is fine with unique_ptr

} // namespace QDB
