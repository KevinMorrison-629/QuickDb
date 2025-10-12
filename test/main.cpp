#include "quickdb/quickdb.h"

#include "User.h"

#include <cstdio> // For std::remove
#include <fstream>
#include <iostream>
#include <vector>

void cleanup(QDB::Collection<User> &users)
{
    std::cout << "\n--- Cleaning up previous test data ---" << std::endl;
    auto deleted_count = users.delete_many(QDB::Query{});
    std::cout << "Deleted " << deleted_count << " documents." << std::endl;
}

// Helper function to compare two files
bool compare_files(const std::string &p1, const std::string &p2)
{
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail())
    {
        return false; // file problem
    }

    if (f1.tellg() != f2.tellg())
    {
        return false; // size mismatch
    }

    // seek back to beginning and compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()), std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}

int main()
{
    const std::string MONGO_URI = "mongodb://localhost:27017";
    const std::string DB_NAME = "QDB_TestDB";
    const std::string COLLECTION_NAME = "users";
    const std::string GRIDFS_BUCKET_NAME = "test_files";

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

        users.create_one(alice);
        std::cout << "Inserted user. ID is now: " << alice.get_id_str() << std::endl;
        QDB::print_document(alice);

        // 4. Find the user back by their ID
        std::cout << "\n--- Testing find_one by ID ---" << std::endl;
        auto query_by_id = QDB::Query::by_id(alice.get_id_str());
        auto found_alice_opt = users.find_one(query_by_id);

        if (found_alice_opt)
        {
            std::cout << "Found user by ID:" << std::endl;
            QDB::print_document(*found_alice_opt);
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
        users.create_many(new_users);
        std::cout << "Inserted 2 more users." << std::endl;

        // 6. Find all users older than 35
        std::cout << "\n--- Testing find with query (age > 35) ---" << std::endl;
        QDB::Query age_query;
        age_query.gt("age", 35);
        auto older_users = users.find_many(age_query);

        std::cout << "Found " << older_users.size() << " users older than 35:" << std::endl;
        for (const auto &user : older_users)
        {
            QDB::print_document(user);
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

        // ------------------------------------------------------------------
        // NEW: GridFS Testing Section
        // ------------------------------------------------------------------
        std::cout << "\n--- Testing GridFS ---" << std::endl;
        const std::string source_filename = "quickdb_test_source.txt";
        const std::string dest_filename = "quickdb_test_download.txt";
        bsoncxx::oid file_id;

        // a. Create a temporary local file to upload
        std::cout << "1. Creating temporary source file..." << std::endl;
        {
            std::ofstream source_file(source_filename);
            source_file << "Hello, GridFS!\nThis is a test file for the QuickDB library.\n";
            source_file << "It should be uploaded, downloaded, and then deleted.\n";
        }

        // b. Get a GridFS bucket handle
        auto bucket = db.get_gridfs_bucket(DB_NAME, GRIDFS_BUCKET_NAME);

        // c. Upload the file
        std::cout << "2. Uploading file to GridFS..." << std::endl;
        file_id = bucket.upload_from_file(source_filename, source_filename);
        std::cout << "   - Upload successful. File ID: " << file_id.to_string() << std::endl;

        // d. Download the file
        std::cout << "3. Downloading file from GridFS..." << std::endl;
        bucket.download_to_file(file_id, dest_filename);
        std::cout << "   - Download successful." << std::endl;

        // e. Verify the contents
        std::cout << "4. Verifying file contents..." << std::endl;
        if (compare_files(source_filename, dest_filename))
        {
            std::cout << "   - SUCCESS: Original and downloaded files are identical." << std::endl;
        }
        else
        {
            std::cerr << "   - FAILURE: File contents do not match!" << std::endl;
        }

        // f. Delete the file from GridFS
        std::cout << "5. Deleting file from GridFS..." << std::endl;
        bucket.delete_file(file_id);
        std::cout << "   - Deletion successful." << std::endl;

        // g. Clean up local files
        std::cout << "6. Cleaning up local files..." << std::endl;
        std::remove(source_filename.c_str());
        std::remove(dest_filename.c_str());
        std::cout << "   - Cleanup complete." << std::endl;
        // --- End of GridFS Test ---
    }
    catch (const QDB::Exception &e)
    {
        std::cerr << "A database error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
