#include "Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

Database::Database(std::string books_path, std::string members_path, std::string loans_path)
    : books_file(books_path), members_file(members_path), loans_file(loans_path) {
    load_data();
}

std::vector<std::string> Database::parse_csv_line(const std::string& line) {
    std::vector<std::string> result;
    std::string current;
    bool in_quotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

std::string Database::escape_csv(const std::string& field) {
    if (field.find(',') != std::string::npos || field.find('"') != std::string::npos) {
        std::string escaped = "\"";
        for (char c : field) {
            if (c == '"') escaped += "\"\"";
            else escaped += c;
        }
        escaped += "\"";
        return escaped;
    }
    return field;
}

void Database::load_data() {
    books.clear();
    members.clear();
    loans.clear();

    // 1. Load Books
    std::ifstream bf(books_file);
    if (bf.is_open()) {
        std::string line;
        // Skip header
        std::getline(bf, line);
        while (std::getline(bf, line)) {
            if (line.empty()) continue;
            auto fields = parse_csv_line(line);
            if (fields.size() >= 5) {
                Book b;
                b.barcode = fields[0];
                b.isbn = fields[1];
                b.title = fields[2];
                b.author = fields[3];
                b.is_issued = (fields[4] == "1");
                books.push_back(b);
            }
        }
        bf.close();
    }

    // 2. Load Members
    std::ifstream mf(members_file);
    if (mf.is_open()) {
        std::string line;
        // Skip header
        std::getline(mf, line);
        while (std::getline(mf, line)) {
            if (line.empty()) continue;
            auto fields = parse_csv_line(line);
            if (fields.size() >= 3) {
                Member m;
                m.id = fields[0];
                m.name = fields[1];
                m.email = fields[2];
                members.push_back(m);
            }
        }
        mf.close();
    }

    // 3. Load Loans
    std::ifstream lf(loans_file);
    if (lf.is_open()) {
        std::string line;
        // Skip header
        std::getline(lf, line);
        while (std::getline(lf, line)) {
            if (line.empty()) continue;
            auto fields = parse_csv_line(line);
            if (fields.size() >= 6) {
                Loan l;
                l.loan_id = fields[0];
                l.member_id = fields[1];
                l.barcode = fields[2];
                l.issue_date = fields[3];
                l.due_date = fields[4];
                l.is_returned = (fields[5] == "1");
                loans.push_back(l);
            }
        }
        lf.close();
    }
}

void Database::save_data() {
    // 1. Save Books
    std::ofstream bf(books_file);
    if (bf.is_open()) {
        bf << "barcode,isbn,title,author,is_issued\n";
        for (const auto& b : books) {
            bf << escape_csv(b.barcode) << ","
               << escape_csv(b.isbn) << ","
               << escape_csv(b.title) << ","
               << escape_csv(b.author) << ","
               << (b.is_issued ? "1" : "0") << "\n";
        }
        bf.close();
    }

    // 2. Save Members
    std::ofstream mf(members_file);
    if (mf.is_open()) {
        mf << "id,name,email\n";
        for (const auto& m : members) {
            mf << escape_csv(m.id) << ","
               << escape_csv(m.name) << ","
               << escape_csv(m.email) << "\n";
        }
        mf.close();
    }

    // 3. Save Loans
    std::ofstream lf(loans_file);
    if (lf.is_open()) {
        lf << "loan_id,member_id,barcode,issue_date,due_date,is_returned\n";
        for (const auto& l : loans) {
            lf << escape_csv(l.loan_id) << ","
               << escape_csv(l.member_id) << ","
               << escape_csv(l.barcode) << ","
               << escape_csv(l.issue_date) << ","
               << escape_csv(l.due_date) << ","
               << (l.is_returned ? "1" : "0") << "\n";
        }
        lf.close();
    }
}

void Database::insert_book(const Book& book) {
    books.push_back(book);
    save_data();
}

void Database::insert_member(const Member& member) {
    members.push_back(member);
    save_data();
}

void Database::insert_loan(const Loan& loan) {
    loans.push_back(loan);
    save_data();
}

void Database::update_book_status(const std::string& barcode, bool is_issued) {
    for (auto& b : books) {
        if (b.barcode == barcode) {
            b.is_issued = is_issued;
            break;
        }
    }
    save_data();
}

void Database::mark_loan_returned(const std::string& member_id, const std::string& barcode) {
    for (auto& l : loans) {
        if (l.member_id == member_id && l.barcode == barcode && !l.is_returned) {
            l.is_returned = true;
            break;
        }
    }
    save_data();
}
