cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "QuickDb")

project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# You can also check here and error out if it’s missing:
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(FATAL_ERROR "Please invoke CMake with -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Add the subdirectory for the static library
add_subdirectory(quickdb)

# Add the subdirectory for the test executable
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "Building QuickDb as a standalone project. Including tests.")
    add_subdirectory(test)
endif()