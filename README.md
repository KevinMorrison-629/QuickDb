# QuickDb

A modern C++17 library for easily interacting with MongoDB, built on top of the official MongoDB C++ driver. It provides a simple, type-safe, and document-oriented interface for C++ applications.

---

# Features

-   **Type-Safe & Modern**: Utilizes C++17 features for a clean and safe API.
-   **Automatic Serialization**: Seamlessly convert your C++ objects to and from BSON.
-   **Fluent Builders**: Intuitive, chainable builders for creating complex queries, updates, and aggregation pipelines.
-   **Connection Pooling**: Efficiently manages database connections for high-performance applications.
-   **Transaction Support**: Execute a series of operations atomically with simple commit/rollback handling.
-   **GridFS Integration**: Easily store and retrieve large files, such as images or videos, directly in the database.
-   **Index Management**: Programmatically create, delete, and list collection indexes.

---

# Getting Started

### Prerequisites

-   A C++17 compliant compiler (e.g., GCC, Clang, MSVC).
-   CMake (version 3.20 or later).

### Build Instructions

This project includes `vcpkg` as a submodule to automatically manage dependencies. The `mongo-cxx-driver` will be downloaded and built as part of the CMake configuration process.

1.  **Clone the Repository:**
    ```bash
    git clone --recurse-submodules https://github.com/KevinMorrison-629/QuickDb
    cd quickdb
    ```

2.  **Configure and build with CMake:**
    ```bash
    cmake -B build -S .

    # Build the project
    cmake --build build
    ```

### Integration

To use `QuickDB` in your own CMake project, add it as a submodule. Your project will automatically use the `vcpkg` instance provided by QuickDB to resolve dependencies.

1.  **Add QuickDB as a submodule to your project:**
    ```bash
    git submodule add --recurse https://github.com/KevinMorrison-629/QuickDb external/quickdb
    ```

2.  **Update your CMakeLists.txt:**
    ```bash
    # CMakeLists.txt of your project
    cmake_minimum_required(VERSION 3.20)
    project(YourApp)

    # Set the toolchain file before the project() command.
    # This ensures vcpkg is configured for your entire project.
    if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/external/quickdb/external/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    endif()

    add_executable(YourApp main.cpp)

    # Add the QuickDb library
    add_subdirectory(external/quickdb)

    # Link against the quickdb library
    target_link_libraries(YourApp PRIVATE quickdb)
    ```


## Example Usage

1.  **Define Your Data Model**
Create a class that inherits from `QDB::Document` and implements the `to_fields` and `from_fields` methods for serialization.

```cpp
    #include "quickdb/quickdb.h"
    #include <iostream>
    #include <vector>

    class User : public QDB::Document {
    public:
        std::string name;
        std::string email;
        int32_t age;
        std::vector<std::string> tags;

        // Default constructor
        User() = default;

        // Convenience constructor
        User(std::string n, int32_t a, std::string e, std::vector<std::string> t)
            : name(std::move(n)), age(a), email(std::move(e)), tags(std::move(t)) {}

        // Serialization: Convert class members to a map of FieldValues
        std::unordered_map<std::string, QDB::FieldValue> to_fields() const override {
            return {
                {"name", name},
                {"email", email},
                {"age", age},
                {"tags", tags}
            };
        }

        // Deserialization: Populate class members from a map of FieldValues
        void from_fields(const std::unordered_map<std::string, QDB::FieldValue>& fields) override {
            QDB::get_field(fields, "name", name);
            QDB::get_field(fields, "email", email);
            QDB::get_field(fields, "age", age);
            QDB::get_field(fields, "tags", tags);
        }
    };

```

2. **Connect and Perform Operations**
Use the `QDB::Database` and `Collection` classes to interact with MongoDB.
```cpp
    int main() {
        try {
            // Connect to the database
            QDB::Database db("mongodb://localhost:27017");

            // Get a handle to a type-safe collection
            auto user_collection = db.get_collection<User>("my_app", "users");

            // --- Create ---
            User new_user("Alice", 30, "alice@example.com", {"developer", "c++"});
            user_collection.create_one(new_user);
            std::cout << "Created user with ID: " << new_user.get_id_str() << std::endl;

            // --- Read ---
            QDB::Query query;
            query.eq("name", "Alice");
            auto found_user_opt = user_collection.find_one(query);

            if (found_user_opt) {
                std::cout << "Found user: " << found_user_opt->name 
                          << ", Age: " << found_user_opt->age << std::endl;
            }

            // --- Update ---
            QDB::Update update;
            update.inc("age", 1).push("tags", "mongodb");
            int64_t modified_count = user_collection.update_one(query, update);
            std::cout << "Modified " << modified_count << " user(s)." << std::endl;

            // --- Delete ---
            int64_t deleted_count = user_collection.delete_one(query);
            std::cout << "Deleted " << deleted_count << " user(s)." << std::endl;

        } catch (const QDB::Exception& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }

```