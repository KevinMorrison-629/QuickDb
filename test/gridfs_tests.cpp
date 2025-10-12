#include "gridfs_tests.h"
#include "quickdb/quickdb.h"
#include "test_runner.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Helper to remove files after test
void remove_file(const std::string &path) { std::remove(path.c_str()); }

bool test_gridfs_cycle()
{
    QDB::Database db("mongodb://localhost:27017");
    auto bucket = db.get_gridfs_bucket("qdb_test_db");

    // 1. Create a test file
    std::string source_path = "source.tmp";
    std::string dest_path = "dest.tmp";
    std::string original_content = "Hello, GridFS!\nThis is a test file with multiple lines.\r\nAnd special chars.";

    // FIX: Open the source file in binary mode to prevent newline translation
    std::ofstream source_file(source_path, std::ios::binary);
    source_file << original_content;
    source_file.close();

    // 2. Upload the file
    bsoncxx::oid file_id;
    try
    {
        file_id = bucket.upload_from_file("test_file.txt", source_path);
    }
    catch (const QDB::Exception &e)
    {
        remove_file(source_path);
        FAIL_TEST("GridFS upload failed with exception: " + std::string(e.what()));
    }

    // 3. Download the file
    try
    {
        bucket.download_to_file(file_id, dest_path);
    }
    catch (const QDB::Exception &e)
    {
        remove_file(source_path);
        remove_file(dest_path);
        FAIL_TEST("GridFS download failed with exception: " + std::string(e.what()));
    }

    // 4. Compare contents
    // FIX: Open the downloaded file in binary mode for a correct comparison
    std::ifstream downloaded_file(dest_path, std::ios::binary);
    std::string downloaded_content((std::istreambuf_iterator<char>(downloaded_file)), std::istreambuf_iterator<char>());

    ASSERT_TRUE(original_content == downloaded_content, "Downloaded content must match original.");

    // 5. Delete the file from GridFS
    try
    {
        bucket.delete_file(file_id);
    }
    catch (const QDB::Exception &e)
    {
        remove_file(source_path);
        remove_file(dest_path);
        FAIL_TEST("GridFS delete failed with exception: " + std::string(e.what()));
    }

    // Cleanup local files
    remove_file(source_path);
    remove_file(dest_path);

    return true;
}

bool run_gridfs_tests()
{
    bool success = true;
    success &= run_test_case(test_gridfs_cycle, "GridFS: Upload/Download/Delete Cycle");
    return success;
}
