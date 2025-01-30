#ifndef URITOOLONGEXCEPTION_HPP
#define URITOOLONGEXCEPTION_HPP

#include <string>
#include <exception>

class UriTooLongException: public std::exception {
    private:
        std::string message;
    public:
        UriTooLongException();
        const char *what() const throw();
        ~UriTooLongException() throw();
};

#endif
