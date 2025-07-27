#include "quickdb/quickdb.h"

#include "User.h"

#include <iostream>
#include <vector>

void cleanup(QDB::Collection<User> &users)
{
    std::cout << "\n--- Cleaning up previous test data ---" << std::endl;
    auto deleted_count = users.delete_many(QDB::Query{});
    std::cout << "Deleted " << deleted_count << " documents." << std::endl;
}

int main()
{
    const std::string MONGO_URI = "mongodb://localhost:27017";
    const std::string DB_NAME = "QDB_TestDB";
    const std::string COLLECTION_NAME = "users";

    try
    {
        // 1. Connect to the database
        QDB::Database db(MONGO_URI);
        std::cout << "Successfully connected to MongoDB." << std::endl;

        // 2. Get a typed collection handle
        auto users = db.get_collection<User>(DB_NAME, COLLECTION_NAME);
        std::cout << "Got collection handle for '" << DB_NAME << "." << COLLECTION_NAME << "'" << std::endl;

        // Clean up any data from previous runs
        cleanup(users);

        // 3. Create and insert a new object
        std::cout << "\n--- Testing insert_one ---" << std::endl;
        User alice;
        alice.name = "Alice";
        alice.email = "alice@example.com";
        alice.age = 30;

        users.insert_one(alice);
        std::cout << "Inserted user. ID is now: " << alice.get_id_str() << std::endl;
        alice.print();

        // 4. Find the user back by their ID
        std::cout << "\n--- Testing find_one by ID ---" << std::endl;
        auto query_by_id = QDB::Query::by_id(alice.get_id_str());
        auto found_alice_opt = users.find_one(query_by_id);

        if (found_alice_opt)
        {
            std::cout << "Found user by ID:" << std::endl;
            found_alice_opt->print();
        }
        else
        {
            std::cerr << "Error: Could not find user by ID." << std::endl;
        }

        // 5. Insert more users
        std::cout << "\n--- Testing insert_many ---" << std::endl;
        User bob;
        bob.name = "Bob";
        bob.email = "bob@example.com";
        bob.age = 45;

        User charlie;
        charlie.name = "Charlie";
        charlie.email = "charlie@example.com";
        charlie.age = 38;

        std::vector<User> new_users = {bob, charlie};
        users.insert_many(new_users);
        std::cout << "Inserted 2 more users." << std::endl;

        // 6. Find all users older than 35
        std::cout << "\n--- Testing find with query (age > 35) ---" << std::endl;
        QDB::Query age_query;
        age_query.gt("age", 35);
        auto older_users = users.find(age_query);

        std::cout << "Found " << older_users.size() << " users older than 35:" << std::endl;
        for (const auto &user : older_users)
        {
            user.print();
        }

        // 7. Test counting
        std::cout << "\n--- Testing count_documents ---" << std::endl;
        std::cout << "Total users in collection: " << users.count_documents(QDB::Query{}) << std::endl;

        // 8. Test deleting
        std::cout << "\n--- Testing delete_one ---" << std::endl;
        QDB::Query email_query;
        email_query.eq("email", "alice@example.com");
        auto deleted_count = users.delete_one(email_query);
        std::cout << "Deleted " << deleted_count << " user(s) with email 'alice@example.com'" << std::endl;
        std::cout << "Total users in collection now: " << users.count_documents(QDB::Query{}) << std::endl;
    }
    catch (const QDB::Exception &e)
    {
        std::cerr << "A database error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
