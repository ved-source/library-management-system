#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include "Database.h"
#include "LibrarySystem.h"

void clear_input() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print_separator() {
    std::cout << "\n--------------------------------------------------\n";
}

void handle_add_book(LibrarySystem& system) {
    print_separator();
    std::cout << "[ADD BOOK TO CATALOG]\n";
    std::string barcode, isbn, title, author;
    
    std::cout << "Enter unique barcode: ";
    std::cin >> barcode;
    clear_input();
    
    std::cout << "Enter ISBN: ";
    std::getline(std::cin, isbn);
    
    std::cout << "Enter Book Title: ";
    std::getline(std::cin, title);
    
    std::cout << "Enter Author Name: ";
    std::getline(std::cin, author);

    try {
        system.add_book(barcode, isbn, title, author);
        std::cout << "\n>>> Success: Book added to catalog.\n";
    } catch (const std::exception& e) {
        std::cout << "\n>>> Error: " << e.what() << "\n";
    }
}

void handle_register_member(LibrarySystem& system) {
    print_separator();
    std::cout << "[REGISTER NEW MEMBER]\n";
    std::string id, name, email;
    
    std::cout << "Enter Member ID: ";
    std::cin >> id;
    clear_input();
    
    std::cout << "Enter Full Name: ";
    std::getline(std::cin, name);
    
    std::cout << "Enter Email Address: ";
    std::getline(std::cin, email);

    try {
        system.register_member(id, name, email);
        std::cout << "\n>>> Success: Member registered successfully.\n";
    } catch (const std::exception& e) {
        std::cout << "\n>>> Error: " << e.what() << "\n";
    }
}

void handle_borrow_book(LibrarySystem& system) {
    print_separator();
    std::cout << "[ISSUE BOOK COPY]\n";
    std::string member_id, barcode;
    
    std::cout << "Enter Member ID: ";
    std::cin >> member_id;
    
    std::cout << "Enter Book Barcode to borrow: ";
    std::cin >> barcode;
    clear_input();

    try {
        Loan loan = system.borrow_book(member_id, barcode);
        std::cout << "\n>>> Success: Book issued!\n";
        std::cout << "    Lending Record ID : " << loan.loan_id << "\n";
        std::cout << "    Due Date          : " << loan.due_date << "\n";
    } catch (const std::exception& e) {
        std::cout << "\n>>> Error: " << e.what() << "\n";
    }
}

void handle_return_book(LibrarySystem& system) {
    print_separator();
    std::cout << "[RETURN BOOK COPY]\n";
    std::string member_id, barcode;
    
    std::cout << "Enter Member ID: ";
    std::cin >> member_id;
    
    std::cout << "Enter Book Barcode to return: ";
    std::cin >> barcode;
    clear_input();

    try {
        system.return_book(member_id, barcode);
        std::cout << "\n>>> Success: Book returned. Barcode is now available.\n";
    } catch (const std::exception& e) {
        std::cout << "\n>>> Error: " << e.what() << "\n";
    }
}

void handle_search_books(LibrarySystem& system) {
    print_separator();
    std::cout << "[SEARCH BOOKS]\n";
    std::cout << "1. Search by Title\n";
    std::cout << "2. Search by Author\n";
    std::cout << "3. Search by ISBN\n";
    std::cout << "Enter your choice: ";
    
    int choice;
    std::cin >> choice;
    clear_input();

    std::string query;
    std::vector<Book> results;

    if (choice == 1) {
        std::cout << "Enter Title query: ";
        std::getline(std::cin, query);
        results = system.search_by_title(query);
    } else if (choice == 2) {
        std::cout << "Enter Author query: ";
        std::getline(std::cin, query);
        results = system.search_by_author(query);
    } else if (choice == 3) {
        std::cout << "Enter ISBN: ";
        std::getline(std::cin, query);
        results = system.search_by_isbn(query);
    } else {
        std::cout << "Invalid choice.\n";
        return;
    }

    print_separator();
    std::cout << "Found " << results.size() << " results:\n";
    for (const auto& b : results) {
        std::cout << "  - [" << b.barcode << "] Title: \"" << b.title << "\" | Author: " << b.author 
                  << " | Status: " << (b.is_issued ? "ISSUED" : "AVAILABLE") << "\n";
    }
}

void handle_view_member(LibrarySystem& system) {
    print_separator();
    std::cout << "[MEMBER STATUS & BORROWED BOOKS]\n";
    std::string member_id;
    std::cout << "Enter Member ID: ";
    std::cin >> member_id;
    clear_input();

    try {
        Member m = system.get_member_by_id(member_id);
        std::cout << "\nMember Details:\n";
        std::cout << "  Name  : " << m.name << "\n";
        std::cout << "  Email : " << m.email << "\n";

        auto active_loans = system.get_member_active_loans(member_id);
        std::cout << "\nActive Borrowings (" << active_loans.size() << "/" << LibrarySystem::MAX_BORROW_LIMIT << "):\n";
        for (const auto& l : active_loans) {
            Book b = system.get_book_by_barcode(l.barcode);
            std::cout << "  - [" << l.barcode << "] \"" << b.title << "\" (Issued: " << l.issue_date << ", Due: " << l.due_date << ")\n";
        }
    } catch (const std::exception& e) {
        std::cout << "\n>>> Error: " << e.what() << "\n";
    }
}

void handle_display_catalog(Database& db) {
    print_separator();
    std::cout << "[ENTIRE BOOK CATALOG]\n";
    if (db.books.empty()) {
        std::cout << "No books in the library catalog.\n";
        return;
    }
    for (const auto& b : db.books) {
        std::cout << "  - [" << b.barcode << "] Title: \"" << b.title << "\" | Author: " << b.author 
                  << " | ISBN: " << b.isbn << " | Status: " << (b.is_issued ? "ISSUED" : "AVAILABLE") << "\n";
    }
}

int main() {
    Database db("library.db");
    LibrarySystem system(db);

    std::cout << "==================================================\n";
    std::cout << "      C++ LIBRARY MANAGEMENT SYSTEM SIMULATOR     \n";
    std::cout << "==================================================\n";

    while (true) {
        std::cout << "\n--- MAIN MENU ---\n";
        std::cout << "1. Add a Book\n";
        std::cout << "2. Register a Member\n";
        std::cout << "3. Issue a Book\n";
        std::cout << "4. Return a Book\n";
        std::cout << "5. Search Books\n";
        std::cout << "6. View Member Status\n";
        std::cout << "7. Display All Books\n";
        std::cout << "8. Exit\n";
        std::cout << "Enter your choice (1-8): ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter a number.\n";
            clear_input();
            continue;
        }
        clear_input();

        if (choice == 1) {
            handle_add_book(system);
        } else if (choice == 2) {
            handle_register_member(system);
        } else if (choice == 3) {
            handle_borrow_book(system);
        } else if (choice == 4) {
            handle_return_book(system);
        } else if (choice == 5) {
            handle_search_books(system);
        } else if (choice == 6) {
            handle_view_member(system);
        } else if (choice == 7) {
            handle_display_catalog(db);
        } else if (choice == 8) {
            std::cout << "\nThank you for using the Library Management System. Exiting...\n";
            break;
        } else {
            std::cout << "Invalid choice. Please select between 1 and 8.\n";
        }
    }

    return 0;
}
