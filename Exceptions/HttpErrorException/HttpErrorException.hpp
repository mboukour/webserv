#ifndef HTTPERROREXCPETION_HPP
#define HTTPERROREXCPETION_HPP



#include <string>
#include <exception>

class HttpErrorException : public std::exception {
    private:
        std::string message;
        int statusCode;
        std::string reasonPhrase;
        HttpErrorException();
    public:
        HttpErrorException(int statusCode, const std::string &reasonPhrase, const std::string &message);
        std::string getErrorPageHtml(void) const;
        int getStatusCode(void) const;
        std::string getReasonPhrase(void) const;
        virtual const char* what() const throw();
        virtual ~HttpErrorException() throw();
};

#endif