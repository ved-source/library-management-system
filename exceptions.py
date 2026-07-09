class LibraryException(Exception):
    """Base exception for all library management system errors."""
    pass


class BookNotFoundException(LibraryException):
    """Raised when a requested book or copy is not found."""
    pass


class MemberNotFoundException(LibraryException):
    """Raised when a member is not found in the registry."""
    pass


class BookNotAvailableException(LibraryException):
    """Raised when trying to borrow a book that is already issued or lost."""
    pass


class LendingLimitExceededException(LibraryException):
    """Raised when a member tries to borrow more than the permitted number of books."""
    pass


class BookNotBorrowedException(LibraryException):
    """Raised when trying to return a book that was not borrowed by the member or is not currently issued."""
    pass
