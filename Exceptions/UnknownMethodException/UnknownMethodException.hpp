#ifndef UnknownMethodException_HPP
#define UnknownMethodException_HPP

#include <exception>
#include <string>

class UnknownMethodException {
    private:
        std::string message;
    public:
        UnknownMethodException();
        UnknownMethodException(const std::string &method);
        const char *what() const throw();
};




#endif