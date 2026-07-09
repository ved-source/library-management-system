from datetime import datetime, timedelta
from typing import Dict, List, Optional
from models import Book, BookCopy, BookStatus, BookLending, Member
from exceptions import (
    BookNotFoundException,
    MemberNotFoundException,
    BookNotAvailableException,
    LendingLimitExceededException,
    BookNotBorrowedException,
)
from search import SearchStrategy


class Catalog:
    """Manages books and physical copies in the library."""

    def __init__(self):
        self._books: Dict[str, Book] = {}  # ISBN -> Book
        self._copies: Dict[str, BookCopy] = {}  # Barcode -> BookCopy

    def add_book(self, book: Book) -> None:
        """Register a book metadata in the catalog."""
        if book.isbn not in self._books:
            self._books[book.isbn] = book

    def add_copy(self, copy: BookCopy) -> None:
        """Add a physical copy of a book to the catalog."""
        self.add_book(copy.book)
        self._copies[copy.barcode] = copy

    def get_copy(self, barcode: str) -> Optional[BookCopy]:
        """Retrieve a specific copy by its barcode."""
        return self._copies.get(barcode)

    def get_all_copies(self) -> List[BookCopy]:
        """Return all book copies in the catalog."""
        return list(self._copies.values())

    def search_books(self, strategy: SearchStrategy, query: str) -> List[BookCopy]:
        """Search books using a given search strategy."""
        return strategy.search(query, self.get_all_copies())


class MemberRegistry:
    """Manages library members registration."""

    def __init__(self):
        self._members: Dict[str, Member] = {}  # MemberID -> Member

    def register_member(self, member: Member) -> None:
        """Register a new member in the system."""
        self._members[member.member_id] = member

    def get_member(self, member_id: str) -> Optional[Member]:
        """Retrieve a member by their ID."""
        return self._members.get(member_id)

    def get_all_members(self) -> List[Member]:
        """Return all registered members."""
        return list(self._members.values())


class LendingService:
    """Handles lending operations: borrowing, returning, and checking limits."""

    DEFAULT_LOAN_DAYS = 14
    MAX_LENDING_LIMIT = 5

    def __init__(self, catalog: Catalog, registry: MemberRegistry):
        self.catalog = catalog
        self.registry = registry
        self._lendings: Dict[str, BookLending] = {}  # LendingID -> BookLending
        self._lending_counter = 0

    def _generate_lending_id(self) -> str:
        self._lending_counter += 1
        return f"LEND-{self._lending_counter:05d}"

    def borrow_book(self, member_id: str, barcode: str) -> BookLending:
        """Issue a book copy to a member."""
        # 1. Resolve member
        member = self.registry.get_member(member_id)
        if not member:
            raise MemberNotFoundException(f"Member with ID '{member_id}' not registered.")

        # 2. Resolve book copy
        copy = self.catalog.get_copy(barcode)
        if not copy:
            raise BookNotFoundException(f"Book copy with barcode '{barcode}' not found.")

        # 3. Check if copy is available
        if not copy.is_available():
            raise BookNotAvailableException(
                f"Book copy '{copy.book.title}' (Barcode: {barcode}) is currently {copy.status.value}."
            )

        # 4. Check lending limit of the member
        active_loans = self.get_active_loans_for_member(member_id)
        if len(active_loans) >= self.MAX_LENDING_LIMIT:
            raise LendingLimitExceededException(
                f"Member '{member.name}' has reached the maximum borrowing limit of {self.MAX_LENDING_LIMIT} books."
            )

        # 5. Issue the book
        copy.issue()
        issue_date = datetime.now()
        due_date = issue_date + timedelta(days=self.DEFAULT_LOAN_DAYS)
        lending_id = self._generate_lending_id()

        lending = BookLending(lending_id, copy, member, issue_date, due_date)
        self._lendings[lending_id] = lending
        return lending

    def return_book(self, member_id: str, barcode: str) -> BookLending:
        """Process book return from a member."""
        # Validate member exists
        member = self.registry.get_member(member_id)
        if not member:
            raise MemberNotFoundException(f"Member with ID '{member_id}' not registered.")

        # Validate copy exists
        copy = self.catalog.get_copy(barcode)
        if not copy:
            raise BookNotFoundException(f"Book copy with barcode '{barcode}' not found.")

        # Find active lending for this barcode and member
        lending = self.get_active_loan_for_copy_and_member(barcode, member_id)
        if not lending:
            raise BookNotBorrowedException(
                f"No active loan found for barcode '{barcode}' under member '{member.name}'."
            )

        # Return the book copy and update lending record
        copy.return_copy()
        lending.complete_lending()
        return lending

    def get_active_loans_for_member(self, member_id: str) -> List[BookLending]:
        """Get all active loans for a specific member."""
        return [
            loan
            for loan in self._lendings.values()
            if loan.member.member_id == member_id and loan.is_active()
        ]

    def get_active_loan_for_copy_and_member(self, barcode: str, member_id: str) -> Optional[BookLending]:
        """Find the active loan for a specific copy barcode and member."""
        for loan in self._lendings.values():
            if (
                loan.copy.barcode == barcode
                and loan.member.member_id == member_id
                and loan.is_active()
            ):
                return loan
        return None

    def get_all_lendings(self) -> List[BookLending]:
        """Get all lending transactions (both active and returned)."""
        return list(self._lendings.values())
