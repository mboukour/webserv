#ifndef HTTPERROREXCPETION_HPP
#define HTTPERROREXCPETION_HPP

#include <string>
#include <exception>


enum ERRORCODES {
    BAD_REQUEST=400,
    METHOD_NOT_ALLOWED=405,
    PAYLOAD_TOO_LARGE=413,
    UNAUTHORIZED=401,
    FORBIDDEN=403,
    NOT_FOUND=404,
    CONFLICT=409,
    URI_TOO_LONG=414,
    INTERNAL_SERVER_ERROR=500,
    NOT_IMPLEMENTED=501,
    SERVICE_UNAVAILABLE=503,
    GATEWAY_TIMEOUT=504,
    HTTP_VERSION_NOT_SUPPORTED=505
};

class HttpRequest;
class HttpErrorException : public std::exception {
    private:
        std::string message;

        std::string version;
        int statusCode;
        std::string reasonPhrase;
        std::string allowedHeader;
        bool closeConnection;
        std::string body;
        
        HttpErrorException();

        void setAllowedHeader(const HttpRequest &request);
        std::string getErrorPageHtml(void) const;

        static const std::string defaultErrorHtml;
        
    public:
        static std::string getReasonPhrase(int statusCode);
        HttpErrorException(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &message, const std::string &body);
        HttpErrorException(int statusCode, const HttpRequest &request, const std::string &message);
        HttpErrorException(int statusCode, const std::string &message); // to use for errors with no request
        int getStatusCode(void) const;
        std::string getReasonPhrase(void);
        std::string getResponseString(void) const;
        virtual const char* what() const throw();
        virtual ~HttpErrorException() throw();
};

#endif