cmake_minimum_required(VERSION 3.14)
project(LibraryManagementSystem CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch SQLite3 amalgamation from official source at configure time
FetchContent_Declare(
    sqlite3
    URL "https://www.sqlite.org/2026/sqlite-amalgamation-3530300.zip"
)

FetchContent_MakeAvailable(sqlite3)

# Build SQLite3 as a static library compiled with C compiler
add_library(sqlite3 STATIC 
    ${sqlite3_SOURCE_DIR}/sqlite3.c
)
# Make sqlite3.h available to dependent targets
target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})

# Main interactive simulator target
add_executable(library_simulator 
    main.cpp 
    Database.cpp 
    LibrarySystem.cpp
)
target_link_libraries(library_simulator PRIVATE sqlite3)

# Unit test runner target
add_executable(test_runner 
    tests.cpp 
    Database.cpp 
    LibrarySystem.cpp
)
target_link_libraries(test_runner PRIVATE sqlite3)
