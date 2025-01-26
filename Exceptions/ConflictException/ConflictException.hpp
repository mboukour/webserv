#ifndef CONFLICTEXCEPTION_HPP
#define CONFLICTEXCEPTION_HPP

#include <exception>
#include <string>


class ConflictException : public std::exception {
    private:
        std::string message;

    public:
        ConflictException();
        ConflictException(const std::string &reason);
        const char *what() const throw();
        ~ConflictException() throw();
};

#endif