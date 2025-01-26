#ifndef FORBIDDENEXCEPTION_HPP
#define FORBIDDENEXCEPTION_HPP

#include <exception>
#include <string>

class ForbiddenException : public std::exception {
    private:
        std::string message;

    public:
        ForbiddenException();
        ForbiddenException(const std::string &reason);
        const char *what() const throw();
        ~ForbiddenException() throw();
};

#endif