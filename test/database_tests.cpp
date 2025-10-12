#include "database_tests.h"
#include "quickdb/quickdb.h"
#include "test_runner.h"
#include "user_document.h"
#include <iostream>

bool test_successful_connection()
{
    try
    {
        QDB::Database db("mongodb://localhost:27017");
        // Attempt a real operation to confirm connection
        auto collection = db.get_collection<User>("qdb_test_db", "test_connection");
        collection.count_documents();
    }
    catch (const QDB::Exception &e)
    {
        FAIL_TEST("Should connect to a valid local instance, but threw: " + std::string(e.what()));
    }
    return true;
}

bool test_connection_failure()
{
    // Using an invalid port to ensure failure
    try
    {
        QDB::Database db("mongodb://localhost:9999/?serverSelectionTimeoutMS=1000");
        // FIX: An operation is required to trigger the lazy connection attempt.
        auto collection = db.get_collection<User>("qdb_test_db", "test_fail");
        // This line should not be reached. count_documents will throw.
        collection.count_documents();
    }
    catch (const QDB::Exception &)
    {
        // Exception was caught as expected.
        return true;
    }
    // If the try block completes without throwing, the test fails.
    return false;
}

bool test_transaction_commit()
{
    // TODO: Implement a test that creates two documents in a transaction
    // and asserts they both exist after the transaction commits.
    return true;
}

bool test_transaction_abort()
{
    QDB::Database db("mongodb://localhost:27017");
    auto collection = db.get_collection<User>("qdb_test_db", "users");
    collection.delete_many(QDB::Query{});

    try
    {
        db.with_transaction(
            [&](mongocxx::client_session &session)
            {
                User user1("In Transaction", 1, "tx@test.com", {});
                collection.create_one(user1, session);
                // Throw an exception to force an abort
                throw std::runtime_error("Forced abort");
            });
    }
    catch (const QDB::Exception &)
    {
        // Expected to catch the wrapped exception from the transaction failure.
    }

    // After the aborted transaction, no documents should exist.
    ASSERT_TRUE(collection.count_documents() == 0, "No documents should exist after transaction abort.");

    return true;
}

bool run_database_tests()
{
    bool success = true;
    success &= run_test_case(test_successful_connection, "Successful Connection");
    success &= run_test_case(test_connection_failure, "Connection Failure");
    success &= run_test_case(test_transaction_commit, "Transaction Successful Commit (STUB)");
    success &= run_test_case(test_transaction_abort, "Transaction Abort on Exception");
    return success;
}
