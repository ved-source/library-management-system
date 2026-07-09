from abc import ABC, abstractmethod
from typing import List
from models import BookCopy


class SearchStrategy(ABC):
    """Abstract base class for searching books in the library."""

    @abstractmethod
    def search(self, query: str, copies: List[BookCopy]) -> List[BookCopy]:
        """Filter copies based on the query."""
        pass


class SearchByTitle(SearchStrategy):
    """Search for books whose titles contain the query string (case-insensitive)."""

    def search(self, query: str, copies: List[BookCopy]) -> List[BookCopy]:
        query_lower = query.lower()
        return [copy for copy in copies if query_lower in copy.book.title.lower()]


class SearchByAuthor(SearchStrategy):
    """Search for books whose author names contain the query string (case-insensitive)."""

    def search(self, query: str, copies: List[BookCopy]) -> List[BookCopy]:
        query_lower = query.lower()
        return [copy for copy in copies if query_lower in copy.book.author.lower()]


class SearchByIsbn(SearchStrategy):
    """Search for books matching the exact ISBN."""

    def search(self, query: str, copies: List[BookCopy]) -> List[BookCopy]:
        return [copy for copy in copies if query == copy.book.isbn]
