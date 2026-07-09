import sys
from datetime import datetime
from models import Book, BookCopy, Member, BookStatus
from services import Catalog, MemberRegistry, LendingService
from search import SearchByTitle, SearchByAuthor, SearchByIsbn
from exceptions import LibraryException


class LibraryFacade:
    """Facade class to simplify client interaction with the Library Management System."""

    def __init__(self):
        self.catalog = Catalog()
        self.registry = MemberRegistry()
        self.lending_service = LendingService(self.catalog, self.registry)

    def add_book(self, isbn: str, title: str, author: str, publication_year: int) -> Book:
        book = Book(isbn, title, author, publication_year)
        self.catalog.add_book(book)
        return book

    def add_book_copy(self, barcode: str, isbn: str, title: str, author: str, publication_year: int) -> BookCopy:
        book = Book(isbn, title, author, publication_year)
        copy = BookCopy(barcode, book)
        self.catalog.add_copy(copy)
        return copy

    def add_existing_book_copy(self, barcode: str, isbn: str) -> BookCopy:
        # Check if book exists in catalog
        book = self.catalog._books.get(isbn)
        if not book:
            raise LibraryException(f"Book metadata with ISBN {isbn} must be added first.")
        copy = BookCopy(barcode, book)
        self.catalog.add_copy(copy)
        return copy

    def register_member(self, member_id: str, name: str, email: str) -> Member:
        member = Member(member_id, name, email)
        self.registry.register_member(member)
        return member

    def borrow_book(self, member_id: str, barcode: str):
        return self.lending_service.borrow_book(member_id, barcode)

    def return_book(self, member_id: str, barcode: str):
        return self.lending_service.return_book(member_id, barcode)

    def search_by_title(self, title: str):
        return self.catalog.search_books(SearchByTitle(), title)

    def search_by_author(self, author: str):
        return self.catalog.search_books(SearchByAuthor(), author)

    def search_by_isbn(self, isbn: str):
        return self.catalog.search_books(SearchByIsbn(), isbn)

    def get_member_loans(self, member_id: str):
        return self.lending_service.get_active_loans_for_member(member_id)


# Color codes for pretty terminal printing
class Colors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def log_step(title: str):
    print(f"\n{Colors.BOLD}{Colors.BLUE}=== {title} ==={Colors.ENDC}")


def log_success(msg: str):
    print(f"{Colors.GREEN}[SUCCESS] {msg}{Colors.ENDC}")


def log_info(msg: str):
    print(f"{Colors.CYAN}[INFO] {msg}{Colors.ENDC}")


def log_error(msg: str):
    print(f"{Colors.FAIL}[ERROR] {msg}{Colors.ENDC}")


def main():
    print(f"{Colors.BOLD}{Colors.HEADER}====================================================")
    print("      LIBRARY MANAGEMENT SYSTEM LLD DEMONSTRATION")
    print(f"===================================================={Colors.ENDC}")

    library = LibraryFacade()

    # ----------------------------------------------------
    log_step("1. Populating Catalog & Adding Books/Copies")
    # ----------------------------------------------------
    b1 = library.add_book_copy("BC-001", "978-0132350884", "Clean Code", "Robert C. Martin", 2008)
    # Add a second copy of Clean Code
    b1_copy = library.add_existing_book_copy("BC-002", "978-0132350884")
    
    b2 = library.add_book_copy("BC-003", "978-0134494166", "Design Patterns: Elements of Reusable Object-Oriented Software", "Erich Gamma", 1994)
    b3 = library.add_book_copy("BC-004", "978-0135957059", "The Pragmatic Programmer", "Andy Hunt", 1999)
    
    log_success(f"Added metadata and copies for: '{b1.book.title}' (2 copies: BC-001, BC-002)")
    log_success(f"Added copy for: '{b2.book.title}' (Barcode: BC-003)")
    log_success(f"Added copy for: '{b3.book.title}' (Barcode: BC-004)")

    # ----------------------------------------------------
    log_step("2. Registering Members")
    # ----------------------------------------------------
    m1 = library.register_member("M-001", "Amrita Sen", "amrita@example.com")
    m2 = library.register_member("M-002", "John Doe", "john@example.com")
    log_success(f"Registered Member: {m1}")
    log_success(f"Registered Member: {m2}")

    # ----------------------------------------------------
    log_step("3. Performing Search Queries (Strategy Pattern)")
    # ----------------------------------------------------
    log_info("Searching for books with title containing 'Clean':")
    results = library.search_by_title("Clean")
    for r in results:
        print(f"  - {r}")

    log_info("Searching for books by author containing 'Erich Gamma':")
    results = library.search_by_author("Erich Gamma")
    for r in results:
        print(f"  - {r}")

    log_info("Searching for books by ISBN '978-0135957059':")
    results = library.search_by_isbn("978-0135957059")
    for r in results:
        print(f"  - {r}")

    # ----------------------------------------------------
    log_step("4. Member Borrows Book Copies Successfully")
    # ----------------------------------------------------
    log_info(f"Issuing BC-001 ('Clean Code') to {m1.name}...")
    loan1 = library.borrow_book(m1.member_id, "BC-001")
    log_success(f"Lending Created: {loan1}")
    log_info(f"Current Book Status of BC-001: {library.catalog.get_copy('BC-001').status.value}")

    # ----------------------------------------------------
    log_step("5. Attempt to Borrow Already Issued Book (Failure Scenario)")
    # ----------------------------------------------------
    log_info(f"Attempting to issue BC-001 to {m2.name}...")
    try:
        library.borrow_book(m2.member_id, "BC-001")
    except LibraryException as e:
        log_error(f"Error as expected: {e}")

    # ----------------------------------------------------
    log_step("6. Borrow Second Copy of the Same Book (Success Scenario)")
    # ----------------------------------------------------
    log_info(f"Issuing BC-002 (second copy of 'Clean Code') to {m2.name}...")
    loan2 = library.borrow_book(m2.member_id, "BC-002")
    log_success(f"Lending Created: {loan2}")

    # ----------------------------------------------------
    log_step("7. Borrow Limit Test (Failure Scenario - Max 5 books)")
    # ----------------------------------------------------
    # M-001 already has 1 book (BC-001). Let's try to borrow 5 more to exceed limit (Max is 5).
    # First, let's add enough books so we can try to borrow them.
    for i in range(10, 16):
        barcode = f"BC-{i:03d}"
        library.add_book_copy(barcode, f"ISBN-DUMMY-{i}", f"Dummy Book {i}", "Author Dummy", 2020)
    
    log_info(f"Registering 4 more loans for {m1.name}...")
    for i in range(10, 14):
        barcode = f"BC-{i:03d}"
        library.borrow_book(m1.member_id, barcode)
    
    active_loans = library.get_member_loans(m1.member_id)
    log_info(f"{m1.name} active loan count: {len(active_loans)}")
    for loan in active_loans:
        print(f"  - {loan}")

    log_info(f"Attempting to borrow the 6th book (Barcode: BC-014) for {m1.name}...")
    try:
        library.borrow_book(m1.member_id, "BC-014")
    except LibraryException as e:
        log_error(f"Error as expected: {e}")

    # ----------------------------------------------------
    log_step("8. Returning a Book Copy")
    # ----------------------------------------------------
    log_info(f"Returning BC-001 ('Clean Code') from {m1.name}...")
    returned_loan = library.return_book(m1.member_id, "BC-001")
    log_success(f"Return completed! lending details: {returned_loan}")
    log_info(f"Status of BC-001 after return: {library.catalog.get_copy('BC-001').status.value}")

    # ----------------------------------------------------
    log_step("9. Attempt to Return a Book Not Borrowed (Failure Scenario)")
    # ----------------------------------------------------
    log_info(f"Attempting to return BC-001 again from {m1.name}...")
    try:
        library.return_book(m1.member_id, "BC-001")
    except LibraryException as e:
        log_error(f"Error as expected: {e}")

    print(f"\n{Colors.BOLD}{Colors.HEADER}====================================================")
    print("             DEMONSTRATION COMPLETED SUCCESSFULLY")
    print(f"===================================================={Colors.ENDC}")


if __name__ == "__main__":
    main()
