# QuickDB Library Interface

This document outlines the public interface for the QuickDB C++ library. It is intended for developers who want to use this library to interact with a MongoDB database.

The library is organized around a few key components:

-   **Database**: The main entry point for connecting to the database and managing transactions.
-   **Document**: A base class for your data models.
-   **Collection**: Represents a MongoDB collection and provides methods for CRUD, aggregation, and index management.
-   **Query**: A fluent interface for building database queries.
-   **Update**: A fluent interface for building update operations.
-   **Aggregation**: A fluent interface for building aggregation pipelines.
-   **GridFSBucket**: A handler for storing and retrieving large files.
-   **Options**: A set of classes for specifying options for database operations.
-   **FieldValue**: A type-safe wrapper for BSON data types.
-   **Exception**: A custom exception for handling library-specific errors.

---

## `QDB::Database`

The `Database` class is the main entry point. It manages the connection pool to your MongoDB instance. Create one `Database` object at the start of your application and share it.

### Public Functions

-   **`Database(const std::string &uri)`**
    -   **Description:** Constructs a `Database` object using a standard MongoDB connection string.
    -   **Parameters:** `uri` - A standard MongoDB connection string.
    -   **Example:** `QDB::Database db("mongodb://localhost:27017");`

-   **`Database(const std::string &user, const std::string &pass, ...)`**
    -   **Description**: Constructs a `Database` object for an authenticated connection.
    -   **Parameters**: `user`, `pass`, `host`, `port`, `auth_db`, `max_pool_size`.

-   **`template <typename T> Collection<T> get_collection(...)`**
    -   **Description**: Gets a type-safe handle to a collection. `T` must inherit from `QDB::Document`.
    -   **Parameters**: `db_name`, `collection_name`.
    -   **Returns**: A `QDB::Collection<T>` object.
    -   **Example**: `auto users = db.get_collection<User>("my_app", "users")`;

-   **`GridFSBucket get_gridfs_bucket(const std::string &db_name, ...)`**
    -   **Description**: Gets a handle to a GridFS bucket for large file storage.
    -   **Parameters**: `db_name`, `bucket_name` (optional, defaults to "fs").
    -   **Returns**: A `QDB::GridFSBucket` object.

-   **`void with_transaction(std::function<void(mongocxx::client_session &session)> callback)`**
    -   **Description**: Executes a series of operations within an atomic transaction. It automatically handles starting, committing, and aborting the transaction.
    -   **Use Case**: Ensure that multiple database operations either all succeed or all fail together.

### Thread Safety

The `QDB::Database` object is designed to be thread-safe. It manages a connection pool, and its methods can be called from multiple threads concurrently. It is recommended to create a single `Database` instance and share it throughout your application.

---

## `QDB::Document`

An abstract base class for your data models. It defines the contract for serialization and deserialization.

### Public Functions

-   **`virtual std::unordered_map<std::string, FieldValue> to_fields() const = 0;`**
    -   **Description:** A pure virtual function. Implement to convert your class members into a map of `FieldValue` objects for storage.

-   **`virtual void from_fields(const std::unordered_map<std::string, FieldValue> &fields) = 0;`**
    -   **Description:** A pure virtual function. Implement to populate your class members from a map of `FieldValue` objects retrieved from the database.

-   **`std::string get_id_str() const`**
    -   **Description:** Gets the string representation of the document's `_id`.

-   **`FieldValue get_id() const`**
    -   **Description:** Gets the raw `bsoncxx::oid` object for the document's `_id`.

### Protected Members

-   **`bsoncxx::oid _id`**
    -   **Description:** Stores the document's unique `_id`. It is automatically managed by the library.

---

## `QDB::Collection<T>`

Provides the interface for performing operations on a collection. `T` must be a subclass of `QDB::Document`.

**Use Case:** After getting a `Collection` object from the `Database`, use its methods to insert, find, update, and delete documents.

### CRUD Operations

-   ``int64_t create_one(T &doc, ...)``: Inserts a single document. Populates `doc._id`.
-   `int64_t create_many(std::vector<T> &docs, ...)`: Inserts multiple documents. Populates `_id` for each doc.
-   `std::optional<T> find_one(const Query &query, ...)`: Finds a single document matching the query.
-   `std::vector<T> find_many(const Query &query, ...)`: Finds all documents matching the query.
-   `int64_t update_one(const Query &filter, const Update &update, ...)`: Updates the first document matching the filter.
-   `int64_t update_many(const Query &filter, const Update &update, ...)`: Updates all documents matching the filter.
-   `int64_t delete_one(const Query &query, ...)`: Deletes the first document matching the query.
-   `int64_t delete_many(const Query &query, ...)`: Deletes all documents matching the query.
-   `int64_t count_documents(const Query &query, ...)`: Counts documents matching the query.

### Atomic Find-and-Modify Operations
These methods perform an operation and return the affected document in a single atomic call.

-   `std::optional<T> find_one_and_update(const Query &query, const Update &update, ...)`
-   `std::optional<T> find_one_and_replace(const Query &query, const T &replacement, ...)`
-   `std::optional<T> find_one_and_delete(const Query &query, ...)`

### Aggregation

-   `template <typename ResultType = T> std::vector<ResultType> aggregate(...)`
    -   **Description**: Executes an aggregation pipeline. `ResultType` must also be a `QDB::Document` subclass, allowing you to deserialize results into a different shape.
    -   **Parameters**: `aggregation` - A `QDB::Aggregation` object.

### Index Management

-   `std::string create_index(const std::string &field, ...)`: Creates a single-field index.
-   `std::string create_compound_index(const std::vector<std::pair<std::string, bool>> &fields)`: Creates a compound index.
-   `void drop_index(const std::string &index_name)`: Drops an index by its name.
-   `std::vector<std::string> list_indexes()`: Lists the names of all indexes on the collection.

---

## `QDB::Query`

A fluent interface for building query filters.

### Static Factory Methods

-   `static Query by_id(const std::string &id_str)`: Creates a query to find a document by its `_id` string.
-   `static Query Or(const std::initializer_list<Query> &queries)`: Creates a logical `$or` query.
-   `static Query And(const std::initializer_list<Query> &queries)`: Creates a logical `$and` query.

### Chaining Methods

-   **Comparison**: `eq` (==), `ne` (!=), `gt` (>), `gte` (>=), `lt` (<), `lte` (<=)
-   **Array**: `in`, `all` (matches arrays containing all specified elements)
-   **Element**: `exists` (checks for the presence or absence of a field)
-   **Evaluation**: `mod`, `regex`, `elemMatch` (queries for a matching element within an array)

## `QDB::Update`

A fluent interface for building update documents for `update_one`, `update_many`, and `find_one_and_update`.

### Chaining Methods

-   **Field**: `set`, `unset`, `rename`, `current_date`
-   **Number**: `inc` (increment), `mul` (multiply), `min`, `max`
-   **Array**: `push`, `push_each`, `pull`, `pull_each`, `pullAll`, `pop`, `add_to_set`
-   **Bitwise**: `bit` (`and`, `or`, `xor`)

### Example

```cpp
auto query = QDB::Query().eq("username", "jane.doe");
auto update = QDB::Update()
                  .set("status", "active")
                  .inc("login_count", 1)
                  .push("history", "Logged in at " + getCurrentTimestamp());

user_collection.update_one(query, update);
```

## `QDB::Aggregation`

A fluent interface for building aggregation pipelines. Used with `collection.aggregate()`.

### Pipeline Stages

-   `match(const Query &query)`: Filters documents.
-   `project(const DocumentBuilder &project_doc)`: Reshapes documents.
-   `group(const DocumentBuilder &group_doc)`: Groups documents and calculates aggregate values.
-   `sort(const DocumentBuilder &sort_doc)`: Sorts documents.
-   `limit(int64_t limit)`: Limits the number of documents.
-   `skip(int64_t skip)`: Skips a number of documents.
-   `unwind(const std::string &field)`: Deconstructs an array field.
-   `lookup(...)`: Performs a left outer join to another collection.
-   `count(const std::string &output_field)`: Counts documents and outputs to a new field.

### Example

```cpp
// Calculate the total order value per customer
auto pipeline = QDB::Aggregation()
    .match(QDB::Query().eq("status", "completed"))
    .group(QDB::DocumentBuilder("_id", "$customer_id")
           .add_field("totalValue", QDB::DocumentBuilder("$sum", "$order_total")))
    .sort(QDB::DocumentBuilder("totalValue", -1));

auto results = order_collection.aggregate<CustomerTotal>(pipeline);
```

## `QDB::GridFSBucket`

Handles storage and retrieval of large files in MongoDB.

### Public Functions

-   `bsoncxx::oid upload_from_file(const std::string &filename, const std::string &source_path)`
    -   **Description**: Uploads a local file to GridFS.
    -   **Returns**: The `ObjectId` of the created file.
-   `void download_to_file(bsoncxx::oid file_id, const std::string &destination_path)`
    -   **Description**: Downloads a file from GridFS to the local filesystem.
-   `void delete_file(bsoncxx::oid file_id)`
    -   **Description**: Deletes a file and its chunks from GridFS.

## `QDB::Options`
These classes are passed to `Collection` methods to modify their behavior.

### QDB::FindOptions

-   For `find_one` and `find_many`.
    -   `sort(key, direction)`: Specifies the sort order.
    -   `limit(count)`: Sets the maximum number of documents to return.
    -   `skip(count)`: Skips a number of documents.
    -   `projection(doc)`: Specifies which fields to include or exclude.

### QDB::UpdateOptions

-   For `update_one` and `update_many`.
    -   `upsert(bool)`: If true, creates a new document if no match is found.

### QDB::FindAndModifyOptions

-   For `find_one_and_update`, `find_one_and_replace`, and `find_one_and_delete`.
    -   Inherits `sort()` and `projection()` from `FindOptions`.
    -   `upsert(bool)`: Same as `UpdateOptions`.
    -   `return_document(ReturnDocument)`: Specifies whether to return the document from before (`kBefore`) or after (`kAfter`) the modification.

---

## `QDB::FieldValue` and `QDB::FieldType`

The `FieldValue` struct is a wrapper for the various data types that can be stored in MongoDB. It provides a type-safe way to handle BSON data. The `FieldType` enum represents the underlying BSON data type.

### `FieldValue` Public Members

-   **`template <typename T> T as() const`**
    -   **Description:** Extracts the stored value, converting it to type `T`.
    -   **Returns:** The value as type `T`. If the conversion is not possible, it may throw an exception or return a default-constructed object.

-   **`FieldValue(const T &val)`**
    -   **Description:** A template constructor that allows creating a `FieldValue` from a C++ type.
    -   **Example:** `QDB::FieldValue name("John Doe");`

### `FieldType` Enum

This enum class lists the supported BSON data types. You generally do not need to interact with this directly, as the `FieldValue` constructors and methods handle type management.

Common types include:
- `FT_STRING`
- `FT_INT_32`
- `FT_INT_64`
- `FT_DOUBLE`
- `FT_BOOLEAN`
- `FT_OBJECT_ID`
- `FT_ARRAY`
- `FT_OBJECT`

---

## `QDB::Exception`

This class is a custom exception type for the library, derived from `std::runtime_error`.

**Use Case:** Catch this exception type to handle errors specific to the QuickDB library, such as connection failures or problems during database operations.

### Public Functions

-   **`explicit Exception(const std::string &message)`**
    -   **Description:** Constructs an exception with a descriptive message.

### Error Handling Example

```cpp
#include <iostream>
#include "quickdb/quickdb.h"

int main() {
    try {
        // Attempt to connect to a non-existent server
        QDB::Database db("mongodb://invalid-host:27017");
        auto collection = db.get_collection<QDB::Document>("test", "test");
        collection.count_documents({});
    } catch (const QDB::Exception& e) {
        std::cerr << "Caught a QuickDB Exception: " << e.what() << std::endl;
    }
    return 0;
}
```
