# QuickDB C++ User Guide

This document provides a comprehensive set of examples for using the quickdb C++ library to interact with a MongoDB database.

## Setup

First, include the necessary `quickdb` header(s) for your application.

```cpp
// library header:
#include "quickdb/quickdb.h"

// individual components may also be included:
#include "quickdb/components/document.h"
```

## Defining a Document Model

To store C++ objects in a collection, you must define a class that inherits from `QDB::Document`. This requires implementing two methods:
-   to_fields(): Serializes your class members into a map that the library can convert to BSON.
-   from_fields(): Deserializes a map read from the database back into your class members.

```cpp
// An example User class definition
class User : public QDB::Document {
public:
    std.string name;
    std.string email;
    int32_t age = 0;
    std::vector<std::string> tags;

    // Default constructor
    User() = default;

    // Convenience constructor
    User(std::string n, int32_t a, std::string e, std::vector<std::string> t)
        : name(std::move(n)), age(a), email(std::move(e)), tags(std::move(t)) {}

    // Serialization: Converts class members to a map of FieldValues
    std::unordered_map<std::string, QDB::FieldValue> to_fields() const override {
        return {
            {"name", name},
            {"email", email},
            {"age", age},
            {"tags", tags}
        };
    }

    // Deserialization: Populates class members from a map of FieldValues
    void from_fields(const std::unordered_map<std::string, QDB::FieldValue>& fields) override {
        QDB::get_field(fields, "name", name);
        QDB::get_field(fields, "email", email);
        QDB::get_field(fields, "age", age);
        QDB::get_field(fields, "tags", tags);
    }
};
```

## Connecting to the Database
To begin, create an instance of `QDB::Database` with your MongoDB connection string. This object manages a connection pool. From the Database object, you can get a handle to a specific collection.

```cpp
int main() {
    try {
        // Connect to a local MongoDB instance.
        QDB::Database db("mongodb://localhost:27017");

        // Get a type-safe collection handle for the 'users' collection in the 'myDatabase' database
        auto users_collection = db.get_collection<User>("myDatabase", "users");
        
dbname        // Start with a clean slate for this example run.
        users_collection.delete_many({});

        std::cout << "Successfully connected to the database.\n\n";
    }
}
```

## Core Collection Operations (CRUD)
The Collection object provides methods for creating, reading, updating, and deleting documents.


### Create (Insert) Documents
You can create a single document with create_one or insert a std::vector of documents in a single batch operation with create_many. The library will populate the _id of your object(s) upon successful insertion.
```cpp
    // --- CREATE ---
    std::cout << "--- CREATING DOCUMENTS ---\n";

    // Create a single document
    User alice("Alice", 25, "a@a.com", {"dev", "c++"});
    users_collection.create_one(alice);
    std::cout << "Created single user 'Alice' with ID: " << alice.get_id_str() << "\n";

    // Create multiple documents
    std::vector<User> new_users = {
        User("Bob", 35, "b@b.com", {"dev", "js"}),
        User("Charlie", 40, "c@c.com", {}),
    };
    int64_t created_count = users_collection.create_many(new_users);
    std::cout << "Created " << created_count << " new users in a batch.\n\n";
```

### Read (Find) Documents
Use the fluent `QDB::Query` builder to construct filters. `find_one` returns an `std::optional<T>`, while `find_many` returns a `std::vector<T>`.
```cpp
    // --- READ ---
    std::cout << "--- READING DOCUMENTS ---\n";

    // Find one user where age is 25
    auto res_eq = users_collection.find_one(QDB::Query{}.eq("age", 25));
    if (res_eq) {
        std::cout << "Found user with age=25:\n";
        QDB::print_document(*res_eq);
    }

    // Find multiple users older than 30
    auto res_gt = users_collection.find_many(QDB::Query{}.gt("age", 30));
    std::cout << "Found " << res_gt.size() << " user(s) older than 30.\n\n";

    // Combine queries with a logical OR
    auto res_or = users_collection.find_many(QDB::Query::Or({
        QDB::Query{}.eq("name", "Alice"), 
        QDB::Query{}.eq("name", "Bob")
    }));
    std::cout << "Found " << res_or.size() << " users named Alice OR Bob.\n\n";
```

### Update Documents
The `QDB::Update` class provides a fluent interface for building update operations using standard MongoDB operators.
```cpp
    // --- UPDATE ---
    std::cout << "--- UPDATING DOCUMENTS ---\n";

    // Use $set to change a name and $inc to increment an age in one operation
    QDB::Update update_op;
    update_op.set("name", "Charles").inc("age", 1);
    users_collection.update_one(QDB::Query().eq("name", "Charlie"), update_op);

    // Use $push to add a new element to the 'tags' array
    QDB::Update push_op;
    push_op.push("tags", "senior");
    users_collection.update_one(QDB::Query().eq("name", "Charles"), push_op);
    
    auto updated_charlie = users_collection.find_one(QDB::Query().eq("name", "Charles"));
    if (updated_charlie) {
        std::cout << "Updated Charlie's document:\n";
        QDB::print_document(*updated_charlie);
    }
```

### Deleting Documents
You can remove a single document with `delete_one` or multiple documents matching a filter with `delete_many`.
```cpp
    // --- DELETE ---
    std::cout << "--- DELETING DOCUMENTS ---\n";

    // Delete the first document where the name is "Bob"
    int64_t deleted_one_count = users_collection.delete_one(QDB::Query{}.eq("name", "Bob"));
    std::cout << "Deleted " << deleted_one_count << " user(s) named 'Bob'.\n";

    // Delete all documents where the age is 50
    int64_t deleted_many_count = users_collection.delete_many(QDB::Query{}.eq("age", 50));
    std::cout << "Deleted " << deleted_many_count << " user(s) with age 50.\n";

    std::cout << "Remaining documents in collection: " << users_collection.count_documents() << "\n\n";
```

## Advanced Features

### Aggregation Pipeline

For complex data processing, build a multi-stage aggregation pipeline. If the results have a different structure than your source document, you can create a custom `QDB::Document` class just for deserializing the results.
```cpp
    // --- AGGREGATION ---
    std::cout << "--- AGGREGATION ---\n";
    
    // A custom class to hold the differently-shaped result of our aggregation
    class AgeResult : public QDB::Document {
    public:
        int32_t age;
        int32_t count;
        std::unordered_map<std::string, QDB::FieldValue> to_fields() const override { return {}; }
        void from_fields(const std::unordered_map<std::string, QDB::FieldValue>& fields) override {
            if (fields.count("_id")) age = fields.at("_id").as<int32_t>();
            if (fields.count("count")) count = fields.at("count").as<int32_t>();
        }
    };

    // Create more data to make the aggregation interesting
    users_collection.create_many({
        User("David", 50, "d@d.com", {}),
        User("Dana", 50, "dana@d.com", {}),
    });
    
    // Build a pipeline to group users by age and get the count for each age
    QDB::Aggregation agg;
    agg.group(QDB::DocumentBuilder("_id", "$age").add_field("count", QDB::DocumentBuilder("$sum", 1)))
        .sort(QDB::DocumentBuilder("_id", 1)); // Sort by age ascending

    auto results = users_collection.aggregate<AgeResult>(agg);
    std::cout << "Aggregation Result (User count per age):\n";
    for (const auto& result : results) {
        std::cout << " - Age: " << result.age << ", Count: " << result.count << "\n";
    }
    std::cout << "\n";
```

### Transactions
To execute a series of operations atomically, use the `with_transaction` method. It accepts a lambda containing all operations that should be part of the transaction. If the lambda finishes successfully, the transaction is committed. If it throws an exception, all operations are automatically rolled back.
```cpp
    // --- TRANSACTIONS ---
    std::cout << "--- TRANSACTIONS ---\n";
    try {
        db.with_transaction([&](mongocxx::client_session &session) {
            // To include an operation in the transaction, pass the session object to it.
            User tx_user("In Transaction", 1, "tx@test.com", {});
            users_collection.create_one(tx_user, session);
            
            std::cout << "Created a user inside a transaction.\n";
            // If an exception were thrown here, the user above would not be created.
        });
        std::cout << "Transaction committed successfully.\n\n";
    } catch (const QDB::Exception &e) {
        std::cerr << "Transaction was rolled back: " << e.what() << '\n';
    }
```

### GridFS for Large Files
Use GridFS to store and retrieve files larger than the 16MB BSON document limit.
```cpp
    // --- GRIDFS ---
    std::cout << "--- GRIDFS ---\n";

    // Create a temporary local file to upload
    std::string source_path = "source.tmp";
    std::string dest_path = "dest.tmp";
    std::ofstream(source_path, std::ios::binary) << "Hello, GridFS!";

    // Get a handle to a GridFS bucket for your database
    auto bucket = db.get_gridfs_bucket("myDatabase");

    // Upload, download, and delete the file
    bsoncxx::oid file_id = bucket.upload_from_file("my_text_file.txt", source_path);
    std::cout << "Uploaded file with ID: " << file_id.to_string() << "\n";
    
    bucket.download_to_file(file_id, dest_path);
    std::cout << "Downloaded file to " << dest_path << "\n";

    bucket.delete_file(file_id);
    std::cout << "Deleted file from GridFS.\n\n";
    
    // Cleanup local files
    std::remove(source_path.c_str());
    std::remove(dest_path.c_str());
```

### Error Handling
All library functions can throw a `QDB::Exception` on error. It's recommended to wrap your database logic in a `try...catch` block to handle connection issues, transaction failures, or other operational errors gracefully.
```cpp
{
    try
    {
        ...
    }
    catch (const QDB::Exception& e) {
        std::cerr << "A database error occurred: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
