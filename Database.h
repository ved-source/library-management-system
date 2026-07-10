#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include "Models.h"

// Forward declaration to avoid exposing sqlite3 headers to clients
struct sqlite3;

class Database {
private:
    sqlite3* db_handle;
    std::string db_file;

    // Singleton Pattern: Private constructor and destructor
    Database(std::string db_path);
    ~Database();

    // Prevent copying
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void initialize_database();
    void seed_data(); // Automatically seed database if empty

public:
    // Singleton Accessor
    static Database& get_instance(std::string db_path = "library.db");

    std::vector<Book> books;
    std::vector<Member*> members; // Polymorphic pointers
    std::vector<Loan> loans;

    void load_data();
    void clear_members(); // Cleanup polymorphic pointers

    // Query helpers executing SQL statements
    void insert_book(const Book& book);
    void insert_member(const Member* member);
    void insert_loan(const Loan& loan);
    void update_book_status(const std::string& barcode, bool is_issued);
    void mark_loan_returned(const std::string& member_id, const std::string& barcode);
};

#endif // DATABASE_H
