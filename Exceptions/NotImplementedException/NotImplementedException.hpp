#ifndef NOTIMPLEMENTEDEXCEPTION_HPP
#define NOTIMPLEMENTEDEXCEPTION_HPP

#include <exception>
#include <string>

class NotImplementedException: public std::exception {
    private:
        std::string message;
    public:
        NotImplementedException();
        NotImplementedException(const std::string &reason);
        const char *what() const throw();
        ~NotImplementedException() throw();
};




#endif