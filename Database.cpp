#include "Database.h"
#include "sqlite3.h"
#include <iostream>
#include <stdexcept>

// Singleton Implementation
Database& Database::get_instance(std::string db_path) {
    static Database instance(db_path);
    return instance;
}

Database::Database(std::string db_path) : db_file(db_path), db_handle(nullptr) {
    int rc = sqlite3_open(db_file.c_str(), &db_handle);
    if (rc != SQLITE_OK) {
        std::string err_msg = sqlite3_errmsg(db_handle);
        sqlite3_close(db_handle);
        throw std::runtime_error("Failed to open SQL database: " + err_msg);
    }
    initialize_database();
    seed_data();
    load_data();
}

Database::~Database() {
    clear_members();
    if (db_handle) {
        sqlite3_close(db_handle);
    }
}

void Database::clear_members() {
    for (auto m : members) {
        delete m;
    }
    members.clear();
}

void Database::initialize_database() {
    char* errmsg = nullptr;

    // Create Books Table
    const char* create_books_sql = 
        "CREATE TABLE IF NOT EXISTS books ("
        "barcode TEXT PRIMARY KEY, "
        "isbn TEXT, "
        "title TEXT, "
        "author TEXT, "
        "is_issued INTEGER DEFAULT 0"
        ");";
    
    int rc = sqlite3_exec(db_handle, create_books_sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string err = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("SQL Error creating books table: " + err);
    }

    // Create Members Table (includes 'type' column for polymorphism)
    const char* create_members_sql = 
        "CREATE TABLE IF NOT EXISTS members ("
        "id TEXT PRIMARY KEY, "
        "name TEXT, "
        "email TEXT, "
        "type TEXT"
        ");";
    
    rc = sqlite3_exec(db_handle, create_members_sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string err = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("SQL Error creating members table: " + err);
    }

    // Create Loans Table
    const char* create_loans_sql = 
        "CREATE TABLE IF NOT EXISTS loans ("
        "loan_id TEXT PRIMARY KEY, "
        "member_id TEXT, "
        "barcode TEXT, "
        "issue_date TEXT, "
        "due_date TEXT, "
        "is_returned INTEGER DEFAULT 0"
        ");";
    
    rc = sqlite3_exec(db_handle, create_loans_sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string err = errmsg;
        sqlite3_free(errmsg);
        throw std::runtime_error("SQL Error creating loans table: " + err);
    }
}

void Database::seed_data() {
    // 1. Seed Books if empty
    sqlite3_stmt* stmt = nullptr;
    const char* check_books_sql = "SELECT COUNT(*) FROM books;";
    int count = 0;
    if (sqlite3_prepare_v2(db_handle, check_books_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        // Insert sample books
        const char* insert_sql = "INSERT INTO books (barcode, isbn, title, author, is_issued) VALUES (?, ?, ?, ?, 0);";
        
        std::vector<std::vector<std::string>> sample_books = {
            {"BC-001", "978-0132350884", "Clean Code", "Robert C. Martin"},
            {"BC-002", "978-0132350884", "Clean Code", "Robert C. Martin"},
            {"BC-003", "978-0134494166", "Design Patterns: Elements of Reusable Object-Oriented Software", "Erich Gamma"},
            {"BC-004", "978-0135957059", "The Pragmatic Programmer", "Andy Hunt"}
        };

        for (const auto& sb : sample_books) {
            sqlite3_prepare_v2(db_handle, insert_sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, sb[0].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, sb[1].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, sb[2].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, sb[3].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // 2. Seed Members if empty
    const char* check_members_sql = "SELECT COUNT(*) FROM members;";
    count = 0;
    if (sqlite3_prepare_v2(db_handle, check_members_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        // Insert sample members with types
        const char* insert_sql = "INSERT INTO members (id, name, email, type) VALUES (?, ?, ?, ?);";
        
        std::vector<std::vector<std::string>> sample_members = {
            {"M-001", "Amrita Sen", "amrita@example.com", "Student"},
            {"M-002", "John Doe", "john@example.com", "Faculty"}
        };

        for (const auto& sm : sample_members) {
            sqlite3_prepare_v2(db_handle, insert_sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, sm[0].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, sm[1].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, sm[2].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, sm[3].c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
}

void Database::load_data() {
    books.clear();
    clear_members(); // Clear previous polymorphic objects safely
    loans.clear();

    // 1. Query Books
    const char* select_books = "SELECT barcode, isbn, title, author, is_issued FROM books;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_handle, select_books, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Book b;
            b.barcode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            b.isbn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            b.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            b.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            b.is_issued = (sqlite3_column_int(stmt, 4) == 1);
            books.push_back(b);
        }
        sqlite3_finalize(stmt);
    }

    // 2. Query Members (instantiated via Factory Pattern based on "type")
    const char* select_members = "SELECT id, name, email, type FROM members;";
    rc = sqlite3_prepare_v2(db_handle, select_members, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            // Factory Pattern
            Member* m = MemberFactory::create_member(type, id, name, email);
            members.push_back(m);
        }
        sqlite3_finalize(stmt);
    }

    // 3. Query Loans
    const char* select_loans = "SELECT loan_id, member_id, barcode, issue_date, due_date, is_returned FROM loans;";
    rc = sqlite3_prepare_v2(db_handle, select_loans, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Loan l;
            l.loan_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            l.member_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            l.barcode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            l.issue_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            l.due_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            l.is_returned = (sqlite3_column_int(stmt, 5) == 1);
            loans.push_back(l);
        }
        sqlite3_finalize(stmt);
    }
}

void Database::insert_book(const Book& book) {
    const char* insert_sql = "INSERT INTO books (barcode, isbn, title, author, is_issued) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_handle, insert_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, book.barcode.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, book.isbn.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, book.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, book.author.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, book.is_issued ? 1 : 0);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    load_data(); // Sync cache
}

void Database::insert_member(const Member* member) {
    const char* insert_sql = "INSERT INTO members (id, name, email, type) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_handle, insert_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, member->get_id().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, member->get_name().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, member->get_email().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, member->get_member_type().c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    load_data(); // Sync cache
}

void Database::insert_loan(const Loan& loan) {
    const char* insert_sql = "INSERT INTO loans (loan_id, member_id, barcode, issue_date, due_date, is_returned) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_handle, insert_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, loan.loan_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, loan.member_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, loan.barcode.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, loan.issue_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, loan.due_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 6, loan.is_returned ? 1 : 0);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    load_data(); // Sync cache
}

void Database::update_book_status(const std::string& barcode, bool is_issued) {
    const char* update_sql = "UPDATE books SET is_issued = ? WHERE barcode = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_handle, update_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, is_issued ? 1 : 0);
        sqlite3_bind_text(stmt, 2, barcode.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    load_data(); // Sync cache
}

void Database::mark_loan_returned(const std::string& member_id, const std::string& barcode) {
    const char* update_sql = "UPDATE loans SET is_returned = 1 WHERE member_id = ? AND barcode = ? AND is_returned = 0;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_handle, update_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, member_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, barcode.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    load_data(); // Sync cache
}
