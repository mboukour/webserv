#include "HttpErrorException.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"
#include <sstream>

HttpErrorException::HttpErrorException(const std::string &version, int statusCode, const std::string &reasonPhrase, const std::string &message, const std::string &body) : message(message), version(version),   statusCode(statusCode), reasonPhrase(reasonPhrase), closeConnection(false) , body(body) {
    if (statusCode == PAYLOAD_TOO_LARGE)
        closeConnection = true;
}

HttpErrorException::HttpErrorException(int statusCode, const HttpRequest &request, const std::string &message) : message(message), version(request.getVersion()), statusCode(statusCode), reasonPhrase(getReasonPhrase(statusCode)), closeConnection(false) , body("") {
    if (request.getRequestBlock())
        body = request.getRequestBlock()->getErrorPageHtml(this->statusCode);
    if (statusCode == METHOD_NOT_ALLOWED)
        setAllowedHeader(request);
    else if (statusCode == PAYLOAD_TOO_LARGE) {
        closeConnection = true;
    }
}

HttpErrorException::HttpErrorException(int statusCode, const std::string &message): message(message), version("HTTP/1.1"), statusCode(statusCode), reasonPhrase(getReasonPhrase(statusCode)), body("") {
    if (statusCode == PAYLOAD_TOO_LARGE)
        closeConnection = true;
}

const std::string HttpErrorException::defaultErrorHtml = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>{CODE} {PHRASE}</title><style>body {font-family: Arial, sans-serif; background-color: #f9f9f9; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; color: #555;} .container {text-align: center;} h1 {font-size: 50px; margin-bottom: 20px;} p {font-size: 18px; margin: 0;} .error-code {font-weight: bold; color: #d9534f;} hr {margin: 20px auto; border: none; border-top: 1px solid #ddd; width: 80%;}</style></head><body><div class=\"container\"><h1><span class=\"error-code\">{CODE}</span> {PHRASE}</h1><hr><p>The server encountered a temporary error and could not complete your request.</p><p>Please try again later.</p></div></body></html>";

void HttpErrorException::setAllowedHeader(const HttpRequest &request) {
    if (!request.getRequestBlock())
        return;
    if (request.getRequestBlock()->isMethodAllowed("GET")) {
        this->allowedHeader = "GET";
    }
    if (request.getRequestBlock()->isMethodAllowed("POST")) {
        if (this->allowedHeader.empty())
            this->allowedHeader = "POST";
        else
            this->allowedHeader += ", POST";
    }
    if (request.getRequestBlock()->isMethodAllowed("DELETE")) {
        if (this->allowedHeader.empty())
            this->allowedHeader = "DELETE";
        else
            this->allowedHeader += ", DELETE";
    }
}

std::string HttpErrorException::getErrorPageHtml(void) const {
    std::string html = defaultErrorHtml;
    
    std::stringstream ss;
    ss << this->statusCode;
    std::string statusCodeStr(ss.str());
    size_t pos = 0;

    while ((pos = html.find("{CODE}", pos)) != std::string::npos) {
        html.replace(pos, 6, statusCodeStr);  
        pos += statusCodeStr.length(); 
    }

    pos = 0;
    while ((pos = html.find("{PHRASE}", pos)) != std::string::npos) {
        html.replace(pos, 8, this->reasonPhrase);
        pos += this->reasonPhrase.length(); 
    }

    return html;
}

const char* HttpErrorException::what() const throw() {
    return this->message.c_str();
}

int HttpErrorException::getStatusCode(void) const {
    return this->statusCode;
}

std::string HttpErrorException::getResponseString() const {
    std::string errorPage;
    if (this->body == "")
        errorPage = getErrorPageHtml();
    else
        errorPage = body;

    HttpResponse resp(this->version, this->statusCode, this->reasonPhrase, errorPage);
    std::stringstream ss;
    ss << errorPage.size();
    resp.setHeader("Content-Length", ss.str());
    resp.setHeader("Content-Type", "text/html");
    if (this->statusCode == METHOD_NOT_ALLOWED && !this->allowedHeader.empty())
        resp.setHeader("Allowed", this->allowedHeader);
    if (this->closeConnection)
        resp.setHeader("Connection", "close");
    // std::cout << resp.toString() << std::endl;
    return resp.toString();
}

std::string HttpErrorException::getReasonPhrase(int statusCode) {
    switch (statusCode) {
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocols";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 300:
            return "Multiple Choices";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 305:
            return "Use Proxy";
        case 307:
            return "Temporary Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 402:
            return "Payment Required";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Request Entity Too Large";
        case 414:
            return "Request-URI Too Long";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        default:
            return "Unknown";
    }
}

HttpErrorException::~HttpErrorException() throw() {}
