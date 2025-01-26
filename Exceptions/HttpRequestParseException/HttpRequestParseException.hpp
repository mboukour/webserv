#ifndef HTTPREQUESTPARSEEXCEPTION_HPP
#define HTTPREQUESTPARSEEXCEPTION_HPP

#include <exception>
#include <string>

class HttpRequestParseException: public std::exception {
    private:
        std::string message;
    public:
        HttpRequestParseException();
        HttpRequestParseException(const std::string &reason);
        const char *what() const throw();
         ~HttpRequestParseException() throw();
};




#endif