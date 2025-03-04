#include "HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"


#include <cstddef>
#include <cstdio>
#include <exception>
#include <iterator>
#include <sstream>
#include <iostream> // to remove later
#include <stdexcept>
#include <string>
#include <vector>
#include "../../Cgi/Cgi.hpp"

HttpRequest::HttpRequest(): AHttp() {}

HttpRequest::HttpRequest(const std::string &request, const std::vector<Server> &servers, int serverPort): requestBlock(NULL) {

    this->primalRequest = request;
    this->isCgi = false;
    std::stringstream ss(request);
    std::string line;

    if (!std::getline(ss, line))
        throw HttpErrorException("HTTP/1.1", BAD_REQUEST, "Bad Request", "empty request", ""); // we assume the version?
    std::stringstream requestLine(line);
    if (!(requestLine >> this->method >> this->path >> this->version))
        throw HttpErrorException("HTTP/1.1" ,BAD_REQUEST, "Bad Request", "invalid request line", "");
    // size_t pos = this->path.find("?");
    // if (pos != std::string::npos)
    // {
    //     this->queryString = this->path.substr(pos + 1);
    //     this->path = this->path.substr(0, pos);
    // }
    if (this->version != "HTTP/1.1")
        throw HttpErrorException(this->version, HTTP_VERSION_NOT_SUPPORTED, "Http Version Not Supported", "version not supported: " + this->version, "");
    if (this->path.length() > URI_MAX_SIZE)
        throw HttpErrorException(this->version ,URI_TOO_LONG, "Uri Too Long", "uri too long", "");
    // implement better matching? 
    parseHeaders(ss, servers, serverPort);
    this->bodySize = 0;
    size_t pos = request.find("\r\n\r\n");
    this->body = request.substr(pos + 4);
    this->bodySize = this->body.size();
    setIsCgi();
    parseCookies();
}

void HttpRequest::appendToBody(const std::string &body) {
    this->body += body;
    this->bodySize = this->body.size();
}

std::string HttpRequest::getMethod() const {
    return this->method;
}

std::string HttpRequest::getPath() const {
    return this->path;
}

std::string HttpRequest::toString() const {
    return this->primalRequest;
}

const ABlock *HttpRequest::getRequestBlock(void) const {
    return this->requestBlock;
}

const Server *HttpRequest::getServer(void) const {
    return this->server;
}

std::string HttpRequest::getQueryString(void) const {
    return this->queryString;
}

std::string HttpRequest::getCookie(const std::string &cookie) const {
    return this->cookies.at(cookie);
}
void HttpRequest::removeLeadingSpaces(std::string &str) {
    size_t firstNonSpace = str.find_first_not_of(" \t");
    if (firstNonSpace != std::string::npos) {
        str.erase(0, firstNonSpace);
    }
}
void HttpRequest::parseHeaders(std::stringstream &ss, const std::vector<Server> &servers, int serverPort) {
    std::string line;
    bool hostFound = false;
    bool contentLengthFound = false;
    while(getline(ss, line) && line != "\r") { // looping through headers
        if (line[line.size() - 1] == '\r')
            line[line.size() - 1] = '\0';
        // std::stringstream lineSs(line);
        std::string key;
        std::string value;

        size_t pos =  line.find(":");
        if (pos == std::string::npos)
        {
            if (hostFound)
                throw HttpErrorException(BAD_REQUEST, *this, ": not found in header");
            else
                throw HttpErrorException(BAD_REQUEST, ": not found in header");
        }

        key = line.substr(0, pos);
        value = line.substr(pos + 1);
        removeLeadingSpaces(value);

        if (key == "Content-Length")
        {

            std::stringstream l(value);
            l >> this->contentLength;
            if (l.fail()) throw HttpErrorException(BAD_REQUEST, *this, "invalid content length header");
            std::string dummy;
            l >> dummy;
            if (!l.eof())  throw HttpErrorException(BAD_REQUEST, *this, "invalid content length header");
            if (this->requestBlock->getIsLimited() && this->bodySize > this->requestBlock->getMaxBodySize())
                throw HttpErrorException(PAYLOAD_TOO_LARGE, *this, "payload too large");
            contentLengthFound = true;
             this->headers[key] = value;

            continue;
        }
        this->headers[key] = value;
        if (key == "Host" || key == "host")
        {
            hostFound = true;
            const Server &server = getServer(value, servers, serverPort);
            this->server = &server;
            this->requestBlock = &server; // absolute fallback
            std::string toMatch = this->path;
            bool exactMatchFound = false;
            bool prefixMatchFound = false;
            std::string longestMatch;

            for (std::vector<Location>::const_iterator it = server.locationsCbegin();
                it != server.locationsCend(); it++) {
                std::string locationName = it->getLocationName();

                if (locationName[locationName.size() - 1] == '/') {
                    locationName = locationName.substr(0, locationName.size() - 1);
                }

                if (toMatch == locationName) {
                    this->requestBlock = &(*it);
                    exactMatchFound = true;
                    break;
                }

                if (!exactMatchFound && toMatch.find(locationName) == 0) {
                    if (locationName.length() > longestMatch.length()) {
                        longestMatch = locationName;
                        this->requestBlock = &(*it);
                        prefixMatchFound = true;
                    }
                }
            }
    
            if (!exactMatchFound && !prefixMatchFound) {
                for (std::vector<Location>::const_iterator it = server.locationsCbegin();
                    it != server.locationsCend(); it++) {
                    if (it->getLocationName() == "/") {
                        this->requestBlock = &(*it);
                        break;
                    }
                }
            }
        }
    }

    if (!hostFound)
        throw HttpErrorException(BAD_REQUEST,  "Host header not found");

    if (!this->requestBlock->isMethodAllowed(this->method))
         throw HttpErrorException(METHOD_NOT_ALLOWED, *this, "Method not allowed");

    if (this->method == "POST")
    {
        if (!contentLengthFound) // or no Transfer-Encoding??
            throw HttpErrorException(BAD_REQUEST, *this, "Content-Length header not found");
    }
}

void HttpRequest::parseCookies(void) {
    std::string cookieString = getHeader("Cookie");
    if (cookieString.empty())
        return ;
    std::stringstream ss(cookieString);
    std::string singleCookie;
    while(getline(ss, singleCookie, ';')) {
        removeLeadingSpaces(singleCookie);
        // std::cout << singleCookie << '\n';
        std::stringstream sc(singleCookie);
        std::string key;
        std::string value;
        if (getline(sc, key, '=')) {
            getline(sc, value);
            if (value[value.size() - 1] == '\0')
                value = value.substr(0, value.size() - 1);
            this->cookies[key] = value;
        }
    }
}

size_t HttpRequest::getContentLength(void) const {
    return this->contentLength;
}

void HttpRequest::setIsCgi(void) {

    std::string scriptName;
    std::string word;
    std::stringstream ss(this->path);

    while(getline(ss, word, '/')) {

        size_t pos = word.find_last_of('.');
        if (pos != std::string::npos)
        {
            std::string extension = word.substr(pos + 1);
            this->isCgi = Cgi::isValidCgiExtension(extension, *this);
            return;
        }
    }
}

bool HttpRequest::isCgiRequest(void) const {
    return this->isCgi;
}

const Server &HttpRequest::getServer(const std::string &host, const std::vector<Server>& servers, int serverPort) {

    std::string actualHost = host;
    size_t pos = actualHost.find(":");
    if (pos != std::string::npos)
        actualHost = actualHost.substr(0, pos);
    // std::cout << "server port: " << serverPort << '\n';
    const Server *firstServerPort = NULL;
    for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++) {
        std::cout << "S port: " << it->getPort() << '\n';
        if (actualHost == it->getServerName() && serverPort == it->getPort()) {
            // std::cout << "Found exact match for host: " << actualHost << " and port: " << serverPort << '\n';
            return *it;
        } else {
            // std::cout << "No exact match for host: [" << actualHost << "] and port: " << serverPort << '\n';
            // std::cout << "Current server being checked - Host: [" << it->getServerName() << "], Port: " << it->getPort() << '\n';
        }
        if (!firstServerPort && serverPort == it->getPort())
            firstServerPort = &(*it);
    }
    std::cout << "Accepted default server for port: " << firstServerPort->getPort() << '\n';
    return *firstServerPort;
}

void HttpRequest::printHeaders(void) const {
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); it++) {
        std::cout << it->first << ": " << it->second << '\n';
    }
}