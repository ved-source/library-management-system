#include <iostream>
#include <cassert>
#include <stdexcept>
#include "sqlite3.h"
#include "Database.h"
#include "LibrarySystem.h"

// Clean temporary SQL db file on startup
void reset_test_databases() {
    std::remove("test_library.db");
}

void test_add_and_search_books() {
    reset_test_databases();
    Database& db = Database::get_instance("test_library.db");
    LibrarySystem system(db);

    system.add_book("TC-001", "111-222", "Object Oriented Design", "Grady Booch");
    system.add_book("TC-002", "111-222", "Object Oriented Design", "Grady Booch");
    system.add_book("TC-003", "333-444", "Effective C++", "Scott Meyers");

    // Search by title
    auto title_results = system.search_by_title("Object Oriented");
    assert(title_results.size() == 2);
    assert(title_results[0].barcode == "TC-001" || title_results[0].barcode == "TC-002");

    // Search by author
    auto author_results = system.search_by_author("Scott Meyers");
    assert(author_results.size() == 1);
    assert(author_results[0].barcode == "TC-003");

    // Search by isbn
    auto isbn_results = system.search_by_isbn("111-222");
    assert(isbn_results.size() == 2);

    std::cout << "[SUCCESS] test_add_and_search_books passed.\n";
}

void test_register_member() {
    Database& db = Database::get_instance("test_library.db");
    sqlite3* handle = nullptr;
    sqlite3_open("test_library.db", &handle);
    sqlite3_exec(handle, "DELETE FROM loans; DELETE FROM members; DELETE FROM books;", nullptr, nullptr, nullptr);
    sqlite3_close(handle);

    db.load_data();
    LibrarySystem system(db);

    system.register_member("Student", "M-01", "Alice", "alice@test.com");
    
    // Duplicate registry check
    try {
        system.register_member("Student", "M-01", "Alice Duplicate", "alice@test.com");
        assert(false && "Should have thrown duplicate member exception");
    } catch (const std::exception& e) {
        // Success
    }

    assert(db.members.size() == 1);
    assert(db.members[0]->get_name() == "Alice");
    assert(db.members[0]->get_member_type() == "Student");
    assert(db.members[0]->get_borrow_limit() == 5);
    
    std::cout << "[SUCCESS] test_register_member passed.\n";
}

void test_borrow_and_return_book() {
    Database& db = Database::get_instance("test_library.db");
    // Clear loans and re-insert structures
    sqlite3* handle = nullptr;
    sqlite3_open("test_library.db", &handle);
    sqlite3_exec(handle, "DELETE FROM loans; DELETE FROM members; DELETE FROM books;", nullptr, nullptr, nullptr);
    sqlite3_close(handle);

    db.load_data();
    LibrarySystem system(db);

    system.add_book("TC-001", "123", "Clean Code", "Robert Martin");
    system.register_member("Student", "M-01", "Alice", "alice@test.com");

    // Initial check
    assert(system.get_active_loans_count("M-01") == 0);
    assert(!system.get_book_by_barcode("TC-001").is_issued);

    // Borrow
    Loan loan = system.borrow_book("M-01", "TC-001");
    assert(loan.barcode == "TC-001");
    assert(loan.member_id == "M-01");
    assert(!loan.is_returned);
    assert(system.get_active_loans_count("M-01") == 1);
    assert(system.get_book_by_barcode("TC-001").is_issued);

    // Borrowing already borrowed book raises error
    try {
        system.borrow_book("M-01", "TC-001");
        assert(false && "Should raise book already issued error");
    } catch (const std::exception& e) {
        // Success
    }

    // Return
    system.return_book("M-01", "TC-001");
    assert(system.get_active_loans_count("M-01") == 0);
    assert(!system.get_book_by_barcode("TC-001").is_issued);

    std::cout << "[SUCCESS] test_borrow_and_return_book passed.\n";
}

void test_borrow_limit_exceeded() {
    Database& db = Database::get_instance("test_library.db");
    sqlite3* handle = nullptr;
    sqlite3_open("test_library.db", &handle);
    sqlite3_exec(handle, "DELETE FROM loans; DELETE FROM members; DELETE FROM books;", nullptr, nullptr, nullptr);
    sqlite3_close(handle);
    
    db.load_data();
    LibrarySystem system(db);

    // 1. Student limit check (Limit = 5)
    system.register_member("Student", "M-Student", "Alice Student", "alice@test.com");
    for (int i = 1; i <= 6; ++i) {
        system.add_book("BS-" + std::to_string(i), "ISBN-S-" + std::to_string(i), "Student Book " + std::to_string(i), "Author");
    }
    for (int i = 1; i <= 5; ++i) {
        system.borrow_book("M-Student", "BS-" + std::to_string(i));
    }
    try {
        system.borrow_book("M-Student", "BS-6");
        assert(false && "Student borrow limit of 5 should have been exceeded");
    } catch (const std::exception& e) {
        // Success
    }
    assert(system.get_active_loans_count("M-Student") == 5);

    // 2. Faculty limit check (Limit = 10)
    system.register_member("Faculty", "M-Faculty", "Prof. Bob", "bob@test.com");
    for (int i = 1; i <= 11; ++i) {
        system.add_book("BF-" + std::to_string(i), "ISBN-F-" + std::to_string(i), "Faculty Book " + std::to_string(i), "Author");
    }
    for (int i = 1; i <= 10; ++i) {
        system.borrow_book("M-Faculty", "BF-" + std::to_string(i));
    }
    try {
        system.borrow_book("M-Faculty", "BF-11");
        assert(false && "Faculty borrow limit of 10 should have been exceeded");
    } catch (const std::exception& e) {
        // Success
    }
    assert(system.get_active_loans_count("M-Faculty") == 10);
    
    std::cout << "[SUCCESS] test_borrow_limit_exceeded (polymorphic limits) passed.\n";
}

int main() {
    std::cout << "=== RUNNING C++ LIBRARY MANAGEMENT SYSTEM SQL UNIT TESTS ===\n";
    test_add_and_search_books();
    test_register_member();
    test_borrow_and_return_book();
    test_borrow_limit_exceeded();
    std::cout << "All SQL unit tests completed successfully!\n";
    reset_test_databases();
    return 0;
}
