#ifndef BADREQUESTEXCEPTION_HPP
#define BADREQUESTEXCEPTION_HPP

#include <exception>
#include <string>


class BadRequestException : public std::exception {
    private:
        std::string message;

    public:
        BadRequestException();
        BadRequestException(const std::string &reason);
        const char *what() const throw();
        ~BadRequestException() throw();
};

#endif