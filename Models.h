#ifndef MODELS_H
#define MODELS_H

#include <string>

struct Book {
    std::string barcode;
    std::string isbn;
    std::string title;
    std::string author;
    bool is_issued = false;
};

struct Member {
    std::string id;
    std::string name;
    std::string email;
};

struct Loan {
    std::string loan_id;
    std::string member_id;
    std::string barcode;
    std::string issue_date;
    std::string due_date;
    bool is_returned = false;
};

#endif // MODELS_H
