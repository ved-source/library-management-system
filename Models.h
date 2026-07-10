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

// Base Class demonstrating Inheritance and Polymorphism
class Member {
protected:
    std::string id;
    std::string name;
    std::string email;

public:
    Member(std::string id, std::string name, std::string email)
        : id(id), name(name), email(email) {}

    virtual ~Member() {} // Virtual destructor for memory safety

    std::string get_id() const { return id; }
    std::string get_name() const { return name; }
    std::string get_email() const { return email; }

    // Polymorphic methods
    virtual std::string get_member_type() const = 0;
    virtual int get_borrow_limit() const = 0;
    virtual double get_fine_rate() const = 0;
};

// Subclass: Student Member
class StudentMember : public Member {
public:
    StudentMember(std::string id, std::string name, std::string email)
        : Member(id, name, email) {}

    std::string get_member_type() const override { return "Student"; }
    int get_borrow_limit() const override { return 5; }
    double get_fine_rate() const override { return 0.50; } // $0.50/day fine
};

// Subclass: Faculty Member
class FacultyMember : public Member {
public:
    FacultyMember(std::string id, std::string name, std::string email)
        : Member(id, name, email) {}

    std::string get_member_type() const override { return "Faculty"; }
    int get_borrow_limit() const override { return 10; } // Faculty can borrow more
    double get_fine_rate() const override { return 0.10; } // $0.10/day fine
};

// Factory Pattern: MemberFactory
class MemberFactory {
public:
    static Member* create_member(const std::string& type, const std::string& id, 
                                 const std::string& name, const std::string& email) {
        if (type == "Faculty") {
            return new FacultyMember(id, name, email);
        } else {
            return new StudentMember(id, name, email); // Default is Student
        }
    }
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
