
#ifndef PAYLOADTOOLARGEEXCEPTION_HPP
#define PAYLOADTOOLARGEEXCEPTION_HPP


#include <string>
#include <exception>

class PayloadTooLargeException: public std::exception {
    private:
        std::string message;
    public:
        PayloadTooLargeException();
        PayloadTooLargeException(size_t maxBodySize);
        const char *what() const throw();
        ~PayloadTooLargeException() throw();
};




#endif