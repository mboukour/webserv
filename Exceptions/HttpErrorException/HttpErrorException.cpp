#include "HttpErrorException.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"
#include <sstream>

HttpErrorException::HttpErrorException(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &message) : message(message), version(version),   statusCode(statusCode), reasonPhrase(reasonPhrase) {}


const std::string HttpErrorException::errorHtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>{CODE} {PHRASE}</title><style>body {font-family: Arial, sans-serif; background-color: #f9f9f9; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; color: #555;} .container {text-align: center;} h1 {font-size: 50px; margin-bottom: 20px;} p {font-size: 18px; margin: 0;} .error-code {font-weight: bold; color: #d9534f;} hr {margin: 20px auto; border: none; border-top: 1px solid #ddd; width: 80%;}</style></head><body><div class=\"container\"><h1><span class=\"error-code\">{CODE}</span> {PHRASE}</h1><hr><p>The server encountered a temporary error and could not complete your request.</p><p>Please try again later.</p></div></body></html>";

std::string HttpErrorException::getErrorPageHtml(void) const {
    std::string html = errorHtml;
    
    std::stringstream ss;
    ss << this->statusCode;
    std::string statusCodeStr(ss.str());
    size_t pos = 0;

    while ((pos = html.find("{CODE}", pos)) != std::string::npos) {
        html.replace(pos, 6, statusCodeStr);  // 6 is the length of "{CODE}"
        pos += statusCodeStr.length();  // Move past the replacement
    }

    // Replace all occurrences of {PHRASE}
    pos = 0;
    while ((pos = html.find("{PHRASE}", pos)) != std::string::npos) {
        html.replace(pos, 8, this->reasonPhrase);  // 8 is the length of "{PHRASE}"
        pos += this->reasonPhrase.length();  // Move past the replacement
    }

    return html;
}

const char* HttpErrorException::what() const throw() {
    return this->message.c_str();
}

int HttpErrorException::getStatusCode(void) const {
    return this->statusCode;
}
// HttpResponse(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &body)
std::string HttpErrorException::getResponseString() const {
    HttpResponse resp(this->version, this->statusCode, this->reasonPhrase, getErrorPageHtml());
    return resp.toString();
}

HttpErrorException::~HttpErrorException() throw() {}
