#ifndef HTTPERROREXCPETION_HPP
#define HTTPERROREXCPETION_HPP



#include <string>
#include <exception>

class HttpErrorException : public std::exception {
    private:
        std::string message;

        std::string version;
        int statusCode;
        std::string reasonPhrase;

        HttpErrorException();

        std::string getErrorPageHtml(void) const;

        static const std::string errorHtml;
    public:
        HttpErrorException(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &message);
        int getStatusCode(void) const;
        std::string getReasonPhrase(void);
        std::string getResponseString(void) const;
        virtual const char* what() const throw();
        virtual ~HttpErrorException() throw();
};

#endif