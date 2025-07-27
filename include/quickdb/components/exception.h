#pragma once

#include <stdexcept>
#include <string>

namespace QDB
{
    /// @brief A custom exception class for the QDB library.
    ///
    /// This class is used to wrap and report errors from the underlying
    /// MongoDB C++ driver, ensuring that the user of the library does not
    /// need to handle driver-specific exceptions directly.
    class Exception : public std::runtime_error
    {
    public:
        /// @brief Constructs an Exception with a descriptive message.
        /// @param message The error message.
        explicit Exception(const std::string &message) : std::runtime_error("QDB Exception: " + message) {}
    };

} // namespace QDB