# QuickDB Library Interface

This document outlines the public interface for the QuickDB C++ library. It is intended for developers who want to use this library to interact with a MongoDB database.

The library is organized around a few key components:

-   **Database**: The main entry point for connecting to the database.
-   **Document**: A base class for your data models.
-   **Collection**: Represents a MongoDB collection and provides methods for CRUD operations.
-   **Query**: A fluent interface for building database queries.
-   **FieldValue**: A type-safe wrapper for BSON data types.
-   **Exception**: A custom exception for handling library-specific errors.

---

## `QDB::Database`

The `Database` class is the main entry point for interacting with the QuickDB library. It manages the connection to your MongoDB instance.

**Use Case:** Create a `Database` object at the start of your application to establish a connection pool to MongoDB.

### Public Functions

-   **`Database(const std::string &uri)`**
    -   **Description:** Constructs a `Database` object and initializes the connection pool to the MongoDB instance specified by the URI.
    -   **Parameters:**
        -   `uri`: A standard MongoDB connection string.
    -   **Example:** `QDB::Database db("mongodb://localhost:27017");`

-   **`template <typename T> Collection<T> get_collection(const std::string &db_name, const std::string &collection_name)`**
    -   **Description:** Gets a type-safe handle to a collection. The template argument `T` must be a class that inherits from `QDB::Document`.
    -   **Parameters:**
        -   `db_name`: The name of the database.
        -   `collection_name`: The name of the collection.
    -   **Returns:** A `QDB::Collection<T>` object for interacting with the specified collection.
    -   **Example:** `auto user_collection = db.get_collection<User>("my_app", "users");`

### Thread Safety

The `QDB::Database` object is designed to be thread-safe. It manages a connection pool, and its methods can be called from multiple threads concurrently. It is recommended to create a single `Database` instance and share it throughout your application.

---

## `QDB::Document`

The `Document` class is an abstract base class that you must inherit from to create your own data models. It defines the contract for serialization and deserialization of your objects to and from the database representation.

**Use Case:** Define a class for each type of document you want to store in a collection. For example, a `User` class would inherit from `QDB::Document`.

### Public Functions

-   **`virtual std::unordered_map<std::string, FieldValue> to_fields() const = 0;`**
    -   **Description:** A pure virtual function that you must implement. This function should convert the members of your class into a map of `FieldValue` objects.
    -   **Returns:** An `std::unordered_map<std::string, FieldValue>` where keys are the field names in the database.

-   **`virtual void from_fields(const std::unordered_map<std::string, FieldValue> &fields) = 0;`**
    -   **Description:** A pure virtual function that you must implement. This function should populate the members of your class from a map of `FieldValue` objects retrieved from the database.
    -   **Parameters:**
        -   `fields`: A map of field names to `FieldValue` objects.

-   **`std::string get_id_str() const`**
    -   **Description:** A helper function to get the string representation of the document's `_id`.
    -   **Returns:** The ObjectId as a string, or an empty string if the `_id` is not an ObjectId.

-   **`FieldValue get_id() const`**
    -   **Description:** Gets the `FieldValue` representing the document's `_id`.
    -   **Returns:** A `FieldValue` object for the `_id`.

### Protected Members

-   **`FieldValue _id`**
    -   **Description:** This member stores the document's unique identifier (`_id`). It is automatically managed by the library (e.g., populated on insertion).

---

## `QDB::Collection<T>`

This class provides the interface for performing operations on a MongoDB collection. The template parameter `T` must be a class that inherits from `QDB::Document`.

**Use Case:** After getting a `Collection` object from the `Database`, use its methods to insert, find, update, and delete documents.

### Public Functions

-   **`void insert_one(T &doc)`**
    -   **Description:** Inserts a single document into the collection. The `_id` of the `doc` object is populated upon successful insertion.
    -   **Parameters:**
        -   `doc`: The document object to insert.

-   **`void insert_many(std::vector<T> &docs)`**
    -   **Description:** Inserts multiple documents into the collection.
    -   **Parameters:**
        -   `docs`: A vector of document objects to insert.

-   **`std::optional<T> find_one(const Query &query)`**
    -   **Description:** Finds a single document that matches the query.
    -   **Parameters:**
        -   `query`: A `QDB::Query` object specifying the filter.
    -   **Returns:** An `std::optional<T>` containing the document if found, otherwise `std::nullopt`.

-   **`std::vector<T> find(const Query &query)`**
    -   **Description:** Finds all documents that match the query.
    -   **Parameters:**
        -   `query`: A `QDB::Query` object specifying the filter.
    -   **Returns:** A `std::vector<T>` of the found documents.

-   **`int64_t replace_one(const Query &filter, const T &update)`**
    -   **Description:** Replaces a single document that matches the filter with a new document.
    -   **Parameters:**
        -   `filter`: A `QDB::Query` object specifying which document to replace.
        -   `update`: The new document object.
    -   **Returns:** The number of documents modified.

-   **`int64_t delete_one(const Query &filter)`**
    -   **Description:** Deletes a single document that matches the filter.
    -   **Parameters:**
        -   `filter`: A `QDB::Query` object specifying which document to delete.
    -   **Returns:** The number of documents deleted.

-   **`int64_t delete_many(const Query &filter)`**
    -   **Description:** Deletes all documents that match the filter.
    -   **Parameters:**
        -   `filter`: A `QDB::Query` object specifying which documents to delete.
    -   **Returns:** The number of documents deleted.

-   **`int64_t count_documents(const Query &filter)`**
    -   **Description:** Counts the number of documents that match the filter.
    -   **Parameters:**
        -   `filter`: A `QDB::Query` object specifying the filter.
    -   **Returns:** The number of matching documents.

---

## `QDB::Query`

The `Query` class provides a fluent interface for building filters for database operations.

**Use Case:** Create and chain methods on a `Query` object to define the criteria for `find`, `delete`, and other collection methods.

### Public Functions

-   **`static Query by_id(const std::string &id_str)`**
    -   **Description:** A static factory method to create a query that filters by a document's `_id`.
    -   **Parameters:**
        -   `id_str`: The string representation of the ObjectId.
    -   **Returns:** A `Query` object.

-   **`Query &eq(const std::string &field, const T &value)`**
    -   **Description:** Adds an equality condition (`==`).
-   **`Query &ne(const std::string &field, const T &value)`**
    -   **Description:** Adds a "not equal" condition (`!=`).
-   **`Query &gt(const std::string &field, const T &value)`**
    -   **Description:** Adds a "greater than" condition (`>`).
-   **`Query &gte(const std::string &field, const T &value)`**
    -   **Description:** Adds a "greater than or equal" condition (`>=`).
-   **`Query &lt(const std::string &field, const T &value)`**
    -   **Description:** Adds a "less than" condition (`<`).
-   **`Query &lte(const std::string &field, const T &value)`**
    -   **Description:** Adds a "less than or equal" condition (`<=`).
-   **`Query &in(const std::string &field, const std::vector<T> &values)`**
    -   **Description:** Adds an "in" condition (field value must be in the given vector).

**Example:**
```cpp
// Find a user by email
auto query = QDB::Query().eq("email", "test@example.com");
auto user = user_collection.find_one(query);

// Find users older than 25
auto age_query = QDB::Query().gt("age", 25);
auto older_users = user_collection.find(age_query);
```

### Advanced Query Examples

**Querying Nested Documents:**

To query based on a field within a nested document, use dot notation in the field name.

```cpp
// Assuming a document structure like: { address: { city: "New York" } }
auto new_york_query = QDB::Query().eq("address.city", "New York");
```

**Querying Elements in an Array:**

To find documents where an array contains a specific value:

```cpp
// Assuming a document structure like: { tags: ["c++", "mongodb"] }
auto tag_query = QDB::Query().eq("tags", "mongodb");
```

---

## `QDB::FieldValue` and `QDB::FieldType`

The `FieldValue` struct is a wrapper for the various data types that can be stored in MongoDB. It provides a type-safe way to handle BSON data. The `FieldType` enum represents the underlying BSON data type.

**Use Case:** You will interact with `FieldValue` primarily when implementing the `to_fields` and `from_fields` methods in your `Document` subclasses.

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
