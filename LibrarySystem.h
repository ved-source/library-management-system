#ifndef LIBRARY_SYSTEM_H
#define LIBRARY_SYSTEM_H

#include "Database.h"
#include <string>
#include <vector>

class LibrarySystem {
private:
    Database& db;
    int loan_counter;

    std::string generate_loan_id();
    std::string get_current_date();
    std::string get_due_date(int days);

public:
    static const int DEFAULT_LOAN_DAYS = 14;

    LibrarySystem(Database& database);

    // Business Logic Methods
    void add_book(const std::string& barcode, const std::string& isbn, 
                  const std::string& title, const std::string& author);
                  
    // Updated to support polymorphic member type registration
    void register_member(const std::string& type, const std::string& id, 
                         const std::string& name, const std::string& email);

    Loan borrow_book(const std::string& member_id, const std::string& barcode);
    
    void return_book(const std::string& member_id, const std::string& barcode);

    // Search Methods
    std::vector<Book> search_by_title(const std::string& title);
    std::vector<Book> search_by_author(const std::string& author);
    std::vector<Book> search_by_isbn(const std::string& isbn);

    // Helper queries
    int get_active_loans_count(const std::string& member_id);
    std::vector<Loan> get_member_active_loans(const std::string& member_id);
    Book get_book_by_barcode(const std::string& barcode);
    
    // Updated to return polymorphic pointer rather than value copy to prevent object slicing
    Member* get_member_by_id(const std::string& id);
};

#endif // LIBRARY_SYSTEM_H
