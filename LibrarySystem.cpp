#include "LibrarySystem.h"
#include <stdexcept>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

LibrarySystem::LibrarySystem(Database& database) : db(database) {
    loan_counter = db.loans.size();
}

std::string LibrarySystem::generate_loan_id() {
    loan_counter++;
    std::stringstream ss;
    ss << "LEND-" << std::setw(5) << std::setfill('0') << loan_counter;
    return ss.str();
}

std::string LibrarySystem::get_current_date() {
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", ltm);
    return std::string(buf);
}

std::string LibrarySystem::get_due_date(int days) {
    std::time_t now = std::time(nullptr);
    std::time_t due = now + (days * 24 * 60 * 60);
    std::tm* ltm = std::localtime(&due);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", ltm);
    return std::string(buf);
}

void LibrarySystem::add_book(const std::string& barcode, const std::string& isbn, 
                             const std::string& title, const std::string& author) {
    for (const auto& b : db.books) {
        if (b.barcode == barcode) {
            throw std::runtime_error("Book with barcode '" + barcode + "' already exists.");
        }
    }

    Book b;
    b.barcode = barcode;
    b.isbn = isbn;
    b.title = title;
    b.author = author;
    b.is_issued = false;

    db.insert_book(b);
}

void LibrarySystem::register_member(const std::string& type, const std::string& id, 
                                     const std::string& name, const std::string& email) {
    // Check if member already exists
    for (const auto* m : db.members) {
        if (m->get_id() == id) {
            throw std::runtime_error("Member with ID '" + id + "' already registered.");
        }
    }

    // Factory Pattern: Create temporary member object
    Member* temp_member = MemberFactory::create_member(type, id, name, email);
    db.insert_member(temp_member);
    delete temp_member; // Clean up temporary object
}

Loan LibrarySystem::borrow_book(const std::string& member_id, const std::string& barcode) {
    // 1. Validate member exists
    Member* borrower = nullptr;
    for (auto* m : db.members) {
        if (m->get_id() == member_id) {
            borrower = m;
            break;
        }
    }
    if (!borrower) {
        throw std::runtime_error("Member not found with ID '" + member_id + "'.");
    }

    // 2. Validate book copy exists and is available
    bool book_exists = false;
    Book copy;
    for (const auto& b : db.books) {
        if (b.barcode == barcode) {
            book_exists = true;
            copy = b;
            break;
        }
    }
    if (!book_exists) {
        throw std::runtime_error("Book copy not found with barcode '" + barcode + "'.");
    }
    if (copy.is_issued) {
        throw std::runtime_error("Book '" + copy.title + "' is currently issued.");
    }

    // 3. Validate borrow limits using Polymorphism
    int active_loans = get_active_loans_count(member_id);
    int limit = borrower->get_borrow_limit();
    if (active_loans >= limit) {
        throw std::runtime_error("Borrow limit of " + std::to_string(limit) + " reached for " +
                                 borrower->get_member_type() + " member '" + borrower->get_name() + "'.");
    }

    // 4. Create Loan
    Loan l;
    l.loan_id = generate_loan_id();
    l.member_id = member_id;
    l.barcode = barcode;
    l.issue_date = get_current_date();
    l.due_date = get_due_date(LibrarySystem::DEFAULT_LOAN_DAYS);
    l.is_returned = false;

    db.insert_loan(l);
    db.update_book_status(barcode, true);

    return l;
}

void LibrarySystem::return_book(const std::string& member_id, const std::string& barcode) {
    // 1. Validate active loan exists
    bool active_loan_found = false;
    for (const auto& l : db.loans) {
        if (l.member_id == member_id && l.barcode == barcode && !l.is_returned) {
            active_loan_found = true;
            break;
        }
    }
    if (!active_loan_found) {
        throw std::runtime_error("No active borrowing record found for member '" + member_id + "' and barcode '" + barcode + "'.");
    }

    // 2. Return book
    db.mark_loan_returned(member_id, barcode);
    db.update_book_status(barcode, false);
}

std::vector<Book> LibrarySystem::search_by_title(const std::string& title) {
    std::vector<Book> results;
    std::string lower_title = title;
    std::transform(lower_title.begin(), lower_title.end(), lower_title.begin(), ::tolower);
    
    for (const auto& b : db.books) {
        std::string current_title = b.title;
        std::transform(current_title.begin(), current_title.end(), current_title.begin(), ::tolower);
        if (current_title.find(lower_title) != std::string::npos) {
            results.push_back(b);
        }
    }
    return results;
}

std::vector<Book> LibrarySystem::search_by_author(const std::string& author) {
    std::vector<Book> results;
    std::string lower_author = author;
    std::transform(lower_author.begin(), lower_author.end(), lower_author.begin(), ::tolower);

    for (const auto& b : db.books) {
        std::string current_author = b.author;
        std::transform(current_author.begin(), current_author.end(), current_author.begin(), ::tolower);
        if (current_author.find(lower_author) != std::string::npos) {
            results.push_back(b);
        }
    }
    return results;
}

std::vector<Book> LibrarySystem::search_by_isbn(const std::string& isbn) {
    std::vector<Book> results;
    for (const auto& b : db.books) {
        if (b.isbn == isbn) {
            results.push_back(b);
        }
    }
    return results;
}

int LibrarySystem::get_active_loans_count(const std::string& member_id) {
    int count = 0;
    for (const auto& l : db.loans) {
        if (l.member_id == member_id && !l.is_returned) {
            count++;
        }
    }
    return count;
}

std::vector<Loan> LibrarySystem::get_member_active_loans(const std::string& member_id) {
    std::vector<Loan> results;
    for (const auto& l : db.loans) {
        if (l.member_id == member_id && !l.is_returned) {
            results.push_back(l);
        }
    }
    return results;
}

Book LibrarySystem::get_book_by_barcode(const std::string& barcode) {
    for (const auto& b : db.books) {
        if (b.barcode == barcode) return b;
    }
    throw std::runtime_error("Book not found.");
}

Member* LibrarySystem::get_member_by_id(const std::string& id) {
    for (auto* m : db.members) {
        if (m->get_id() == id) return m;
    }
    throw std::runtime_error("Member not found.");
}
