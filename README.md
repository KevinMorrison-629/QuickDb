# QuickDb

A C++ library for easily interacting with MongoDB, built on top of the official MongoDB C++ driver. It provides a simple, document-oriented interface for C++ applications.

---

## Features

-   Type-safe, modern C++17 interface.
-   Fluent query builder for intuitive database queries.
-   Automatic serialization/deserialization from C++ objects to BSON.
-   Connection pooling for efficient database access.

---

## Getting Started

### Prerequisites

-   A C++17 compliant compiler (e.g., GCC, Clang, MSVC).
-   CMake (version 3.20 or later).
-   `vcpkg` for dependency management.

### Build Instructions

1.  **Clone the repository:**
    ```bash
    git clone <repository-url>
    cd quickdb
    ```

2.  **Install dependencies using vcpkg:**
    This project uses `vcpkg` to manage dependencies. The required `mongo-cxx-driver` will be installed automatically if you have `vcpkg` integrated with your shell. If not, you can install it manually:
    ```bash
    vcpkg install mongo-cxx-driver
    ```

3.  **Configure and build with CMake:**
    You must provide the path to the `vcpkg.cmake` toolchain file when configuring the project. This is already defined in `CMakePresets.json`.
    ```bash
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
    cmake --build build
    ```

### Integration with your project

To use QuickDb in your own CMake project, you can include it as a submodule.

1.  **Add QuickDb as a submodule:**
    ```bash
    git submodule add <repository-url> extern/quickdb
    ```

2.  **Update your `CMakeLists.txt`:**
    ```cmake
    # Add the submodule directory
    add_subdirectory(extern/quickdb)

    # ... later in your CMakeLists.txt ...

    # Link against the quickdb library
    target_link_libraries(your_executable_or_library PRIVATE quickdb)
    ```

Make sure you also configure your main project with the `vcpkg.cmake` toolchain file so that the dependencies are resolved correctly.

---

## Example Usage

Here is a complete example of how to use QuickDb.

First, define your data model by inheriting from `QDB::Document`:

```cpp
#include "quickdb/quickdb.h"
#include <iostream>

class User : public QDB::Document {
public:
    std::string name;
    int age;

    std::unordered_map<std::string, QDB::FieldValue> to_fields() const override {
        return {
            {"name", QDB::FieldValue(name)},
            {"age", QDB::FieldValue(age)}
        };
    }

    void from_fields(const std::unordered_map<std::string, QDB::FieldValue>& fields) override {
        get_field(fields, "name", name);
        get_field(fields, "age", age);
    }
};
```

Now, you can use the `Database` and `Collection` classes to interact with MongoDB:

```cpp
int main() {
    try {
        // Connect to the database
        QDB::Database db("mongodb://localhost:27017");

        // Get a handle to the collection
        auto user_collection = db.get_collection<User>("my_app", "users");

        // Create and insert a new user
        User new_user;
        new_user.name = "John Doe";
        new_user.age = 30;
        user_collection.insert_one(new_user);
        std::cout << "Inserted user with ID: " << new_user.get_id_str() << std::endl;

        // Find the user
        auto query = QDB::Query().eq("name", "John Doe");
        auto found_user_optional = user_collection.find_one(query);

        if (found_user_optional) {
            User found_user = *found_user_optional;
            std::cout << "Found user: " << found_user.name << ", Age: " << found_user.age << std::endl;
        }

    } catch (const QDB::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
```

