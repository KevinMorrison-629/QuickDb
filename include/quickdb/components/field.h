#pragma once

#include <chrono> // Required for std::chrono::system_clock::time_point
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

// Include MongoDB C++ driver headers for BSON building.
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types.hpp>

namespace QDB
{
    // Forward-declare Document for use in FieldValue templates
    class Document;

    /// @brief Enumerates the possible BSON data types that a field can represent.
    enum class FieldType : uint8_t
    {
        FT_ARRAY,         ///< Array type.
        FT_BINARY,        ///< Binary data type.
        FT_BOOLEAN,       ///< Boolean type.
        FT_CODE,          ///< JavaScript code type.
        FT_DATE,          ///< Date type.
        FT_DECIMAL_128,   ///< Decimal128 type (high-precision number).
        FT_DOUBLE,        ///< Double-precision floating-point type.
        FT_INT_32,        ///< 32-bit integer type.
        FT_INT_64,        ///< 64-bit integer type.
        FT_MAXKEY,        ///< MaxKey type (internal MongoDB type).
        FT_MINKEY,        ///< MinKey type (internal MongoDB type).
        FT_NULL,          ///< Null type.
        FT_OBJECT,        ///< Embedded document/object type.
        FT_OBJECT_ID,     ///< ObjectId type (unique identifier).
        FT_BSON_REG_EXPR, ///< Regular expression type.
        FT_STRING,        ///< UTF-8 string type.
        FT_BSON_SYMBOL,   ///< Symbol type (deprecated in BSON).
        FT_TIMESTAMP,     ///< Timestamp type (internal MongoDB type).
        FT_UNDEFINED,     ///< Undefined type.
    };

    // Forward declaration for self-referential variant.
    struct FieldValue;

    /// @brief Metafunction to map C++ types to FieldType enum values.
    /// @tparam T The C++ type to map.
    template <typename T, typename = void> struct type_to_fieldtype; // General template

    /// @brief SFINAE specialization for enum types. All enums map to FT_INT_32.
    template <typename T> struct type_to_fieldtype<T, std::enable_if_t<std::is_enum_v<T>>>
    {
        static constexpr FieldType value = FieldType::FT_INT_32;
    };

    /// @brief Specialization of type_to_fieldtype for bool.
    template <> struct type_to_fieldtype<bool>
    {
        static constexpr FieldType value = FieldType::FT_BOOLEAN; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for int32_t.
    template <> struct type_to_fieldtype<int32_t>
    {
        static constexpr FieldType value = FieldType::FT_INT_32; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for int64_t.
    template <> struct type_to_fieldtype<int64_t>
    {
        static constexpr FieldType value = FieldType::FT_INT_64; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for double.
    template <> struct type_to_fieldtype<double>
    {
        static constexpr FieldType value = FieldType::FT_DOUBLE; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for bsoncxx::oid.
    template <> struct type_to_fieldtype<bsoncxx::oid>
    {
        static constexpr FieldType value = FieldType::FT_OBJECT_ID; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for std::string.
    template <> struct type_to_fieldtype<std::string>
    {
        static constexpr FieldType value = FieldType::FT_STRING; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for const char*.
    template <> struct type_to_fieldtype<const char *>
    {
        static constexpr FieldType value = FieldType::FT_STRING; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for char*.
    template <> struct type_to_fieldtype<char *>
    {
        static constexpr FieldType value = FieldType::FT_STRING; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for bsoncxx::types::b_timestamp.
    template <> struct type_to_fieldtype<bsoncxx::types::b_timestamp>
    {
        static constexpr FieldType value = FieldType::FT_TIMESTAMP; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for bsoncxx::types::b_date.
    template <> struct type_to_fieldtype<bsoncxx::types::b_date>
    {
        static constexpr FieldType value = FieldType::FT_DATE; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for std::chrono::system_clock::time_point.
    template <> struct type_to_fieldtype<std::chrono::system_clock::time_point>
    {
        static constexpr FieldType value = FieldType::FT_DATE; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for std::vector<FieldValue>.
    template <> struct type_to_fieldtype<std::vector<FieldValue>>
    {
        static constexpr FieldType value = FieldType::FT_ARRAY; ///< Corresponding FieldType.
    };
    /// @brief Specialization of type_to_fieldtype for std::unordered_map<std::string, FieldValue>.
    template <> struct type_to_fieldtype<std::unordered_map<std::string, FieldValue>>
    {
        static constexpr FieldType value = FieldType::FT_OBJECT; ///< Corresponding FieldType.
    };

    /// @brief A std::variant type alias representing the possible C++ types a field can hold.
    /// This variant is used by FieldValue to store the actual data.
    /// It includes recursive types like std::vector<FieldValue> for arrays and
    /// std::unordered_map<std::string, FieldValue> for nested objects.
    using FieldVariant = std::variant<std::vector<FieldValue>,                    // For FT_ARRAY
                                      std::vector<uint8_t>,                       // For FT_BINARY
                                      bool,                                       // For FT_BOOLEAN
                                      int32_t,                                    // For FT_INT_32
                                      int64_t,                                    // For FT_INT_64
                                      double,                                     // For FT_DOUBLE
                                      std::nullptr_t,                             // For FT_NULL
                                      bsoncxx::oid,                               // For FT_OBJECT_ID
                                      std::string,                                // For FT_STRING, FT_CODE, etc.
                                      bsoncxx::types::b_date,                     // For FT_DATE
                                      bsoncxx::types::b_timestamp,                // For FT_TIMESTAMP
                                      std::unordered_map<std::string, FieldValue> // For FT_OBJECT
                                      >;

    /// @brief Type trait to check if a type is a std::vector.
    template <typename> struct is_std_vector : std::false_type
    {
    };
    /// @brief Specialization of is_std_vector for std::vector.
    template <typename T, typename A> struct is_std_vector<std::vector<T, A>> : std::true_type
    {
    };

    /// @brief Represents a BSON-like field, containing both its type and its value.
    /// This struct is used to build and parse BSON documents in a more type-safe manner
    /// before converting to/from the bsoncxx library's representations.
    struct FieldValue
    {
        /// @brief Default constructor. Initializes type to an undefined state and value to default.
        FieldValue() = default;
        /// @brief Default destructor.
        ~FieldValue() = default;
        /// @brief Constructs a FieldValue with a specific type and value.
        /// @param _type The FieldType of this field.
        /// @param _val The FieldVariant holding the actual data for this field.
        FieldValue(const FieldType &_type, const FieldVariant &_val) : type(_type), value(_val) {}

        /// @brief Template constructor for creating a FieldValue from a raw C++ type.
        /// @tparam T The type of the value.
        /// @param val The value to store.
        template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldValue>>>
        FieldValue(const T &val)
        {
            using DecayedT = std::decay_t<T>;
            type = type_to_fieldtype<DecayedT>::value;

            if constexpr (std::is_enum_v<DecayedT>)
            {
                value = static_cast<int32_t>(val);
            }
            else
            {
                value = val;
            }
        }

        /// @brief Constructs a FieldValue from a std::chrono::system_clock::time_point, converting it to a b_date.
        /// @param tp The time point to store.
        FieldValue(const std::chrono::system_clock::time_point &tp)
            : type(FieldType::FT_DATE), value(bsoncxx::types::b_date{tp})
        {
        }

        /// @brief Template constructor to automatically handle std::vector<T>.
        /// @tparam T The inner type of the vector.
        /// @param vec The vector of values.
        template <typename T> FieldValue(const std::vector<T> &vec) : type(FieldType::FT_ARRAY)
        {
            std::vector<FieldValue> fv_vector;
            fv_vector.reserve(vec.size());
            for (const auto &item : vec)
            {
                if constexpr (std::is_base_of_v<Document, T>)
                {
                    // For documents, we must serialize them to their field map.
                    fv_vector.emplace_back(FieldType::FT_OBJECT, item.to_fields());
                }
                else
                {
                    // For primitives and other vectors, the regular constructor works.
                    fv_vector.emplace_back(item);
                }
            }
            value = fv_vector;
        }

        /// @brief Template method to get the value as a specific type T.
        /// @tparam T The desired type.
        /// @return The value cast to type T. Returns a default-constructed T on failure.
        template <typename T> T as() const
        {
            if constexpr (std::is_same_v<T, FieldValue>)
            {
                return *this;
            }
            else if constexpr (is_std_vector<T>::value)
            {
                using U = typename T::value_type;
                if (type != FieldType::FT_ARRAY)
                    return T{};
                const auto &fv_vector = std::get<std::vector<FieldValue>>(value);
                T result_vector;
                result_vector.reserve(fv_vector.size());
                for (const auto &fv_item : fv_vector)
                {
                    result_vector.push_back(fv_item.as<U>());
                }
                return result_vector;
            }
            else if constexpr (std::is_base_of_v<Document, T>)
            {
                if (type != FieldType::FT_OBJECT)
                    return T{};
                const auto &map = std::get<std::unordered_map<std::string, FieldValue>>(value);
                T doc;
                doc.from_fields(map);
                return doc;
            }
            // [ADDED] This block handles deserialization for enum types.
            else if constexpr (std::is_enum_v<T>)
            {
                // Enums are stored as integers, so we retrieve the integer
                // and static_cast it back to the enum type.
                if (type != FieldType::FT_INT_32)
                    return T{}; // Return default value if type mismatch
                try
                {
                    if (std::holds_alternative<int32_t>(value))
                    {
                        return static_cast<T>(std::get<int32_t>(value));
                    }
                }
                catch (const std::bad_variant_access &)
                {
                    // This catch handles cases where the type in the variant
                    // does not match what we are trying to get.
                }
                return T{}; // Return default-constructed enum value on failure
            }
            else if constexpr (std::is_same_v<T, std::chrono::system_clock::time_point>)
            {
                if (type != FieldType::FT_DATE)
                    return T{}; // Return default constructed time_point
                try
                {
                    if (std::holds_alternative<bsoncxx::types::b_date>(value))
                    {
                        // bsoncxx::types::b_date has an operator to convert to time_point
                        return std::get<bsoncxx::types::b_date>(value);
                    }
                }
                catch (const std::bad_variant_access &)
                {
                    // Should not happen if type is FT_DATE, but for safety.
                }
                return T{}; // Return default constructed time_point
            }
            else
            {
                try
                {
                    if (std::holds_alternative<T>(value))
                        return std::get<T>(value);
                }
                catch (const std::bad_variant_access &)
                {
                }
                return T{};
            }
        }

        FieldType type = FieldType::FT_UNDEFINED; ///< The BSON type of the field.
        FieldVariant value;                       ///< The actual value of the field, stored in a variant.
    };

    /// @brief Equality operator for FieldValue.
    ///
    /// Compares two FieldValue objects for equality. Primarily intended for simple types.
    /// For complex types like arrays or objects, this performs a direct comparison of the variant's content,
    /// which might not be a deep comparison depending on the underlying types' operator==.
    /// @param lhs The left-hand side FieldValue.
    /// @param rhs The right-hand side FieldValue.
    /// @return True if the types are the same and the values are equal, false otherwise.
    inline bool operator==(const FieldValue &lhs, const FieldValue &rhs)
    {
        if (lhs.type != rhs.type)
            return false;
        // Note: This relies on std::variant::operator== which performs
        // element-wise comparison for std::vector and std::unordered_map
        // if their contained types also support operator==.
        return lhs.value == rhs.value;
    }

    //---------------------------------------------------------------
    // Helper functions for converting FieldValue to BSON.
    //---------------------------------------------------------------
    // Forward declarations for recursive calls
    static void AppendToDocument(bsoncxx::builder::basic::document &doc, const std::string &key, const FieldValue &fv);
    static void AppendToArray(bsoncxx::builder::basic::array &arr, const FieldValue &fv);
    template <typename BsonElement> static FieldValue fromBsonElement(const BsonElement &element);

    /// @brief Appends a FieldValue to a BSON array builder.
    /// @param arr The BSON array builder to append to.
    /// @param fv The FieldValue to append.
    static void AppendToArray(bsoncxx::builder::basic::array &arr, const FieldValue &fv)
    {
        using namespace bsoncxx::builder::basic;
        switch (fv.type)
        {
        case FieldType::FT_BOOLEAN:
        {
            arr.append(std::get<bool>(fv.value));
            break;
        }
        case FieldType::FT_INT_32:
        {
            arr.append(std::get<int32_t>(fv.value));
            break;
        }
        case FieldType::FT_INT_64:
        {
            arr.append(std::get<int64_t>(fv.value));
            break;
        }
        case FieldType::FT_DOUBLE:
        {
            arr.append(std::get<double>(fv.value));
            break;
        }
        case FieldType::FT_NULL:
        {
            arr.append(bsoncxx::types::b_null{});
            break;
        }
        case FieldType::FT_STRING:
        {
            arr.append(std::get<std::string>(fv.value));
            break;
        }
        case FieldType::FT_OBJECT_ID:
        {
            arr.append(std::get<bsoncxx::oid>(fv.value));
            break;
        }
        case FieldType::FT_DATE:
        {
            arr.append(std::get<bsoncxx::types::b_date>(fv.value));
            break;
        }
        case FieldType::FT_TIMESTAMP:
        {
            arr.append(std::get<bsoncxx::types::b_timestamp>(fv.value));
            break;
        }
        case FieldType::FT_OBJECT:
        {
            document sub_doc{};
            auto map = std::get<std::unordered_map<std::string, FieldValue>>(fv.value);
            for (const auto &[sub_key, sub_value] : map)
            {
                AppendToDocument(sub_doc, sub_key, sub_value); // Recursive call
            }
            arr.append(sub_doc.view());
            break;
        }
        case FieldType::FT_ARRAY:
        {
            array sub_arr{};
            auto vec = std::get<std::vector<FieldValue>>(fv.value);
            for (const auto &item : vec)
            {
                AppendToArray(sub_arr, item); // Recursive call
            }
            arr.append(sub_arr.view());
            break;
        }
        default:
            // Other types not handled, append null
            arr.append(bsoncxx::types::b_null{});
            break;
        }
    }

    /// @brief Appends a key-FieldValue pair to a BSON document builder.
    /// @param doc The BSON document builder to append to.
    /// @param key The key for the new element.
    /// @param fv The FieldValue to append.
    static void AppendToDocument(bsoncxx::builder::basic::document &doc, const std::string &key, const FieldValue &fv)
    {
        using namespace bsoncxx::builder::basic;

        switch (fv.type)
        {
        case FieldType::FT_BOOLEAN:
        {
            doc.append(kvp(key, std::get<bool>(fv.value)));
            break;
        }
        case FieldType::FT_INT_32:
        {
            doc.append(kvp(key, std::get<int32_t>(fv.value)));
            break;
        }
        case FieldType::FT_INT_64:
        {
            doc.append(kvp(key, std::get<int64_t>(fv.value)));
            break;
        }
        case FieldType::FT_DOUBLE:
        {
            doc.append(kvp(key, std::get<double>(fv.value)));
            break;
        }
        case FieldType::FT_NULL:
        {
            doc.append(kvp(key, bsoncxx::types::b_null{}));
            break;
        }
        case FieldType::FT_STRING:
        {
            doc.append(kvp(key, std::get<std::string>(fv.value)));
            break;
        }
        case FieldType::FT_OBJECT_ID:
        {
            doc.append(kvp(key, std::get<bsoncxx::oid>(fv.value)));
            break;
        }
        case FieldType::FT_DATE:
        {
            doc.append(kvp(key, std::get<bsoncxx::types::b_date>(fv.value)));
            break;
        }
        case FieldType::FT_TIMESTAMP:
        {
            doc.append(kvp(key, std::get<bsoncxx::types::b_timestamp>(fv.value)));
            break;
        }
        case FieldType::FT_OBJECT:
        {
            document sub_doc{};
            auto map = std::get<std::unordered_map<std::string, FieldValue>>(fv.value);
            for (const auto &[sub_key, sub_value] : map)
            {
                AppendToDocument(sub_doc, sub_key, sub_value); // Recursive call
            }
            doc.append(kvp(key, sub_doc.view()));
            break;
        }
        case FieldType::FT_ARRAY:
        {
            array sub_arr{};
            auto vec = std::get<std::vector<FieldValue>>(fv.value);
            for (const auto &item : vec)
            {
                AppendToArray(sub_arr, item); // Call array helper
            }
            doc.append(kvp(key, sub_arr.view()));
            break;
        }
        default:
        { // Other types not handled, append null
            doc.append(kvp(key, bsoncxx::types::b_null{}));
            break;
        }
        }
    }

    /// @brief Converts any BSON element to a FieldValue.
    /// @tparam BsonElement Can be bsoncxx::document::element or bsoncxx::array::element.
    /// @param element The BSON element to convert.
    /// @return The resulting FieldValue.
    template <typename BsonElement> static FieldValue fromBsonElement(const BsonElement &element)
    {
        FieldValue fv;
        switch (element.type())
        {
        case bsoncxx::type::k_bool:
            fv.type = FieldType::FT_BOOLEAN;
            fv.value = element.get_bool().value;
            break;
        case bsoncxx::type::k_int32:
            fv.type = FieldType::FT_INT_32;
            fv.value = element.get_int32().value;
            break;
        case bsoncxx::type::k_int64:
            fv.type = FieldType::FT_INT_64;
            fv.value = element.get_int64().value;
            break;
        case bsoncxx::type::k_double:
            fv.type = FieldType::FT_DOUBLE;
            fv.value = element.get_double().value;
            break;
        case bsoncxx::type::k_string:
            fv.type = FieldType::FT_STRING;
            fv.value = static_cast<std::string>(element.get_string().value);
            break;
        case bsoncxx::type::k_oid:
            fv.type = FieldType::FT_OBJECT_ID;
            fv.value = element.get_oid().value;
            break;
        case bsoncxx::type::k_date:
            fv.type = FieldType::FT_DATE;
            fv.value = element.get_date();
            break;
        case bsoncxx::type::k_timestamp:
            fv.type = FieldType::FT_TIMESTAMP;
            fv.value = element.get_timestamp();
            break;
        case bsoncxx::type::k_document:
        {
            fv.type = FieldType::FT_OBJECT;
            std::unordered_map<std::string, FieldValue> map;
            for (auto inner_element : element.get_document().value)
            {
                map[static_cast<std::string>(inner_element.key())] = fromBsonElement(inner_element); // Recursive call
            }
            fv.value = map;
            break;
        }
        case bsoncxx::type::k_array:
        {
            fv.type = FieldType::FT_ARRAY;
            std::vector<FieldValue> vec;
            for (auto inner_element : element.get_array().value)
            {
                vec.push_back(fromBsonElement(inner_element)); // Recursive call
            }
            fv.value = vec;
            break;
        }
        default:
            fv.type = FieldType::FT_NULL;
            fv.value = nullptr;
            break;
        }
        return fv;
    }

} // namespace QDB
