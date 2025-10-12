#pragma once

#include "quickdb/components/exception.h"
#include <functional>
#include <iostream>
#include <string>

// A simple macro to assert a condition is true
#define ASSERT_TRUE(condition, message)                                                                                     \
    if (!(condition))                                                                                                       \
    {                                                                                                                       \
        std::cerr << "FAILED: " << message << " (in " << __FILE__ << ":" << __LINE__ << ")" << std::endl;                   \
        return false;                                                                                                       \
    }

// A macro to immediately fail a test with a specific message.
#define FAIL_TEST(message)                                                                                                  \
    do                                                                                                                      \
    {                                                                                                                       \
        std::cerr << "FAILED: " << message << " (in " << __FILE__ << ":" << __LINE__ << ")" << std::endl;                   \
        return false;                                                                                                       \
    } while (0)

// A simple macro to assert a condition is false
#define ASSERT_FALSE(condition, message)                                                                                    \
    if (condition)                                                                                                          \
    {                                                                                                                       \
        std::cerr << "FAILED: " << message << " (in " << __FILE__ << ":" << __LINE__ << ")" << std::endl;                   \
        return false;                                                                                                       \
    }

// A simple macro to assert that a specific exception is thrown
#define ASSERT_THROWS(expression, exception_type, message)                                                                  \
    try                                                                                                                     \
    {                                                                                                                       \
        expression;                                                                                                         \
        std::cerr << "FAILED: Expected exception " #exception_type " was not thrown. " << message << " (in " << __FILE__    \
                  << ":" << __LINE__ << ")" << std::endl;                                                                   \
        return false;                                                                                                       \
    }                                                                                                                       \
    catch (const exception_type &e)                                                                                         \
    {                                                                                                                       \
        /* Caught expected exception, test passed */                                                                        \
    }                                                                                                                       \
    catch (...)                                                                                                             \
    {                                                                                                                       \
        std::cerr << "FAILED: An unexpected exception was thrown. " << message << " (in " << __FILE__ << ":" << __LINE__    \
                  << ")" << std::endl;                                                                                      \
        return false;                                                                                                       \
    }

// A simple test case runner
inline bool run_test_case(std::function<bool()> test_func, const std::string &test_name)
{
    std::cout << "Running test: " << test_name << "..." << std::endl;
    bool result = test_func();
    if (result)
    {
        std::cout << "PASSED: " << test_name << std::endl;
    }
    // The macros will print the failure message
    std::cout << "----------------------------------------" << std::endl;
    return result;
}
