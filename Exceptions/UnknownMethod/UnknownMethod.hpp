#ifndef UNKNOWNMETHOD_HPP
#define UNKNOWNMETHOD_HPP

#include <exception>
#include <string>

class UnknownMethod {
    private:
        std::string message;
    public:
        UnknownMethod();
        UnknownMethod(const std::string &method);
        const char *what() const throw();
};




#endif