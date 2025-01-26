#ifndef NOTFOUNDEXCEPTION_HPP
#define NOTFOUNDEXCEPTION_HPP

#include <exception>
#include <string>

class NotFoundException: public std::exception {
    private:
        std::string message;
    public:
        NotFoundException();
        NotFoundException(const std::string &reason);
        const char *what() const throw();
        ~NotFoundException() throw();
};







#endif