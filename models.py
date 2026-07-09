from datetime import datetime
from enum import Enum
from typing import Optional


class BookStatus(Enum):
    AVAILABLE = "AVAILABLE"
    ISSUED = "ISSUED"
    LOST = "LOST"


class Book:
    """Represents the metadata of a book."""

    def __init__(self, isbn: str, title: str, author: str, publication_year: int):
        self.isbn = isbn
        self.title = title
        self.author = author
        self.publication_year = publication_year

    def __repr__(self) -> str:
        return f"Book(ISBN: {self.isbn}, Title: '{self.title}', Author: '{self.author}')"


class BookCopy:
    """Represents an individual physical copy of a book in the library."""

    def __init__(self, barcode: str, book: Book):
        self.barcode = barcode
        self.book = book
        self.status = BookStatus.AVAILABLE

    def is_available(self) -> bool:
        return self.status == BookStatus.AVAILABLE

    def issue(self) -> None:
        self.status = BookStatus.ISSUED

    def return_copy(self) -> None:
        self.status = BookStatus.AVAILABLE

    def mark_lost(self) -> None:
        self.status = BookStatus.LOST

    def __repr__(self) -> str:
        return f"BookCopy(Barcode: {self.barcode}, Status: {self.status.value}, Book: {self.book.title})"


class Member:
    """Represents a library member registered in the system."""

    def __init__(self, member_id: str, name: str, email: str):
        self.member_id = member_id
        self.name = name
        self.email = email
        self.joined_date = datetime.now()

    def __repr__(self) -> str:
        return f"Member(ID: {self.member_id}, Name: {self.name})"


class BookLending:
    """Represents a borrowing transaction record."""

    def __init__(self, lending_id: str, copy: BookCopy, member: Member, issued_date: datetime, due_date: datetime):
        self.lending_id = lending_id
        self.copy = copy
        self.member = member
        self.issued_date = issued_date
        self.due_date = due_date
        self.returned_date: Optional[datetime] = None

    def is_active(self) -> bool:
        return self.returned_date is None

    def complete_lending(self) -> None:
        self.returned_date = datetime.now()

    def __repr__(self) -> str:
        status = f"Returned on {self.returned_date.strftime('%Y-%m-%d')}" if self.returned_date else f"Due on {self.due_date.strftime('%Y-%m-%d')}"
        return f"Lending(ID: {self.lending_id}, Member: {self.member.name}, Copy: {self.copy.barcode}, Status: {status})"
