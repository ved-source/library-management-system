import unittest
from datetime import datetime, timedelta
from exceptions import (
    BookNotFoundException,
    MemberNotFoundException,
    BookNotAvailableException,
    LendingLimitExceededException,
    BookNotBorrowedException,
)
from models import Book, BookCopy, BookStatus, Member
from services import Catalog, MemberRegistry, LendingService
from search import SearchByTitle, SearchByAuthor, SearchByIsbn


class TestLibraryManagementSystem(unittest.TestCase):

    def setUp(self):
        self.catalog = Catalog()
        self.registry = MemberRegistry()
        self.lending_service = LendingService(self.catalog, self.registry)

        # Setup initial test data
        self.book1 = Book("123-456", "Test Driven Development", "Kent Beck", 2002)
        self.copy1 = BookCopy("TC-001", self.book1)
        self.catalog.add_copy(self.copy1)

        self.member1 = Member("M-001", "Alice", "alice@example.com")
        self.registry.register_member(self.member1)

    def test_add_and_search_books(self):
        # Test Search Strategy by Title
        results = self.catalog.search_books(SearchByTitle(), "Test Driven")
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].barcode, "TC-001")

        # Test Search Strategy by Author
        results = self.catalog.search_books(SearchByAuthor(), "Kent Beck")
        self.assertEqual(len(results), 1)

        # Test Search Strategy by ISBN
        results = self.catalog.search_books(SearchByIsbn(), "123-456")
        self.assertEqual(len(results), 1)

    def test_successful_borrow_and_return(self):
        # Borrow book
        lending = self.lending_service.borrow_book("M-001", "TC-001")
        self.assertIsNotNone(lending)
        self.assertEqual(lending.copy.status, BookStatus.ISSUED)
        self.assertTrue(lending.is_active())

        # Check copy availability
        self.assertFalse(self.copy1.is_available())

        # Return book
        returned_lending = self.lending_service.return_book("M-001", "TC-001")
        self.assertEqual(returned_lending.copy.status, BookStatus.AVAILABLE)
        self.assertFalse(returned_lending.is_active())
        self.assertIsNotNone(returned_lending.returned_date)

    def test_borrow_unavailable_book_raises_error(self):
        # First borrow
        self.lending_service.borrow_book("M-001", "TC-001")

        # Create another member
        member2 = Member("M-002", "Bob", "bob@example.com")
        self.registry.register_member(member2)

        # Second borrow of same copy should raise BookNotAvailableException
        with self.assertRaises(BookNotAvailableException):
            self.lending_service.borrow_book("M-002", "TC-001")

    def test_borrow_nonexistent_book_raises_error(self):
        with self.assertRaises(BookNotFoundException):
            self.lending_service.borrow_book("M-001", "NON-EXISTENT")

    def test_nonexistent_member_raises_error(self):
        with self.assertRaises(MemberNotFoundException):
            self.lending_service.borrow_book("M-999", "TC-001")

    def test_borrow_limit_exceeded(self):
        # Limit is 5. Let's add 6 copies of different books
        copies = []
        for i in range(6):
            book = Book(f"ISBN-{i}", f"Book {i}", "Author", 2020)
            copy = BookCopy(f"BC-{i}", book)
            self.catalog.add_copy(copy)
            copies.append(copy)

        # Borrow 5 books successfully
        for i in range(5):
            self.lending_service.borrow_book("M-001", f"BC-{i}")

        # The 6th borrow should raise LendingLimitExceededException
        with self.assertRaises(LendingLimitExceededException):
            self.lending_service.borrow_book("M-001", "BC-5")

    def test_return_unborrowed_book_raises_error(self):
        # M-001 returns TC-001 which she has not borrowed
        with self.assertRaises(BookNotBorrowedException):
            self.lending_service.return_book("M-001", "TC-001")


if __name__ == "__main__":
    unittest.main()
