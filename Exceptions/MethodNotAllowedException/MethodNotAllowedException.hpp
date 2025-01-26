#ifndef METHODNOTALLOWEDEXCEPTION_HPP
#define METHODNOTALLOWEDEXCEPTION_HPP

#include <exception>
#include <string>

class MethodNotAllowedException : public std::exception {
    private:
        std::string message;

    public:
        MethodNotAllowedException();
        MethodNotAllowedException(const std::string &reason);
        const char *what() const throw();
        ~MethodNotAllowedException() throw();
};

#endif