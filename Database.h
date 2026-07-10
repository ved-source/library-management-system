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

    void initialize_database();

public:
    std::vector<Book> books;
    std::vector<Member> members;
    std::vector<Loan> loans;

    Database(std::string db_path = "library.db");
    ~Database();

    void load_data();

    // Query helpers executing SQL statements
    void insert_book(const Book& book);
    void insert_member(const Member& member);
    void insert_loan(const Loan& loan);
    void update_book_status(const std::string& barcode, bool is_issued);
    void mark_loan_returned(const std::string& member_id, const std::string& barcode);
};

#endif // DATABASE_H
