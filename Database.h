#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include "Models.h"

class Database {
private:
    std::string books_file;
    std::string members_file;
    std::string loans_file;

    // Helper functions for CSV parsing
    std::vector<std::string> parse_csv_line(const std::string& line);
    std::string escape_csv(const std::string& field);

public:
    std::vector<Book> books;
    std::vector<Member> members;
    std::vector<Loan> loans;

    Database(std::string books_path = "books.csv", 
             std::string members_path = "members.csv", 
             std::string loans_path = "loans.csv");

    void load_data();
    void save_data();

    // Query helpers simulating database transactions
    void insert_book(const Book& book);
    void insert_member(const Member& member);
    void insert_loan(const Loan& loan);
    void update_book_status(const std::string& barcode, bool is_issued);
    void mark_loan_returned(const std::string& member_id, const std::string& barcode);
};

#endif // DATABASE_H
