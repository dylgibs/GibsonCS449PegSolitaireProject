
# Specifies the minimum version of CMake required to build the project.
cmake_minimum_required(VERSION 3.14)

# Gives the project the name "PegSolitaireTests".
project(PegSolitaireTests)

# Tells CMake to use the C++17 standard when compiling the project.
set(CMAKE_CXX_STANDARD 17)

# Requires the compiler to support the C++17 standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generates a compile_commands.json file containing the compiler commands used to build the project. 
# Here for debugging purposes while I was getting this to work.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Loads CMake's FetchContent module, which allows external libraries to be downloaded and included automatically.
include(FetchContent)

# Specifies GoogleTest as an external dependency.
FetchContent_Declare(
  googletest

  # Downloads version 1.17.0 of GoogleTest from its GitHub repository.
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip

  # Uses the time that the files are extracted rather than the timestamps stored in the downloaded archive.
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Tells GoogleTest to use the same C runtime library settings as the rest of the project.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Downloads GoogleTest if necessary and makes it available to the project.
FetchContent_MakeAvailable(googletest)

# Enables CMake's testing functionality.
enable_testing()

# Creates an executable named PegBoard_test.

# PegBoard_test.cpp contains the GoogleTest unit tests.
add_executable(
  PegBoard_test
  PegBoard_test.cpp
)


# GTest::gtest_main provides the main() function needed to start the GoogleTest test program.
target_link_libraries(
  PegBoard_test
  GTest::gtest_main
)

# Loads CMake's GoogleTest integration functionality.
include(GoogleTest)

# Automatically discovers the individual GoogleTest tests in PegBoard_test and makes them available to CTest.
gtest_discover_tests(PegBoard_test)

