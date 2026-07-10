#include <iostream>
#include <cassert>
#include <stdexcept>
#include "Database.h"
#include "LibrarySystem.h"

// Define a test macro or function to clean temporary SQL db file on startup
void reset_test_databases() {
    std::remove("test_library.db");
}

void test_add_and_search_books() {
    reset_test_databases();
    Database db("test_library.db");
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
    reset_test_databases();
    Database db("test_library.db");
    LibrarySystem system(db);

    system.register_member("M-01", "Alice", "alice@test.com");
    
    // Duplicate registry check
    try {
        system.register_member("M-01", "Alice Duplicate", "alice@test.com");
        assert(false && "Should have thrown duplicate member exception");
    } catch (const std::exception& e) {
        // Success
    }

    assert(db.members.size() == 1);
    assert(db.members[0].name == "Alice");
    
    std::cout << "[SUCCESS] test_register_member passed.\n";
}

void test_borrow_and_return_book() {
    reset_test_databases();
    Database db("test_library.db");
    LibrarySystem system(db);

    system.add_book("TC-001", "123", "Clean Code", "Robert Martin");
    system.register_member("M-01", "Alice", "alice@test.com");

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
    reset_test_databases();
    Database db("test_library.db");
    LibrarySystem system(db);

    system.register_member("M-01", "Alice", "alice@test.com");
    
    // Add 6 books
    for (int i = 1; i <= 6; ++i) {
        system.add_book("B-0" + std::to_string(i), "ISBN-" + std::to_string(i), "Book " + std::to_string(i), "Author");
    }

    // Borrow 5 books
    for (int i = 1; i <= 5; ++i) {
        system.borrow_book("M-01", "B-0" + std::to_string(i));
    }

    // 6th borrow should fail
    try {
        system.borrow_book("M-01", "B-06");
        assert(false && "Should raise borrow limit exceeded error");
    } catch (const std::exception& e) {
        // Success
    }

    assert(system.get_active_loans_count("M-01") == 5);
    
    std::cout << "[SUCCESS] test_borrow_limit_exceeded passed.\n";
}

int main() {
    std::cout << "=== RUNNING C++ LIBRARY MANAGEMENT SYSTEM SQL UNIT TESTS ===\n";
    test_add_and_search_books();
    test_register_member();
    test_borrow_and_return_book();
    test_borrow_limit_exceeded();
    std::cout << "All SQL unit tests completed successfully!\n";
    reset_test_databases(); // Cleanup test files
    return 0;
}
