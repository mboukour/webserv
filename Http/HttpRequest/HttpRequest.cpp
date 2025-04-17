#include "HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include <cstddef>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../Cgi/Cgi.hpp"
#include "../../Utils/AllUtils/AllUtils.hpp"


HttpRequest::HttpRequest(): AHttp(), method(), path(), queryString(), primalRequest(), isCgi(false),
        server(NULL), requestBlock(NULL), cookies(), contentLength(0), reqEntry(NULL),
        isChunked(false), isMultiForm(false), boundary("") {}

HttpRequest::HttpRequest(const std::string &request, const std::vector<Server> &servers, int serverPort): requestBlock(NULL),
	contentLength(0), reqEntry(NULL) ,isChunked(false), isMultiForm(false), boundary("") {

    this->primalRequest = request;
    this->isCgi = false;
    std::stringstream ss(request);
    std::string line;

    if (!std::getline(ss, line))
        throw HttpErrorException("HTTP/1.1", BAD_REQUEST, "Bad Request", "empty request", "");
    std::stringstream requestLine(line);
    if (!(requestLine >> this->method >> this->path >> this->version))
        throw HttpErrorException("HTTP/1.1" ,BAD_REQUEST, "Bad Request", line, "");
    validateRequestLine();
    size_t pos = this->path.find("?");
    if (pos != std::string::npos)
    {
        this->queryString = this->path.substr(pos + 1);
        this->path = this->path.substr(0, pos);
    }
    parseHeaders(ss, servers, serverPort);

    this->bodySize = 0;
    pos = request.find("\r\n\r\n");
    this->body = request.substr(pos + 4);
    this->bodySize = this->body.size();
    if (this->requestBlock->getIsLimited()
        && ((!this->isChunked && this->contentLength > this->requestBlock->getMaxBodySize())
        || this->bodySize > this->requestBlock->getMaxBodySize()))
            throw HttpErrorException(PAYLOAD_TOO_LARGE, *this, "Payload too large");
    this->path = sanitizePath(this->path);
    setIsCgi();
    parseCookies();
}


void HttpRequest::validateRequestLine(void) const {
    if (this->version != "HTTP/1.1")
        throw HttpErrorException(this->version, HTTP_VERSION_NOT_SUPPORTED, "Http Version Not Supported", "version not supported: " + this->version, "");
    for (std::string::const_iterator it = this->path.begin(); it != this->path.end(); it++) {
        if (this->uriAllowedChars.find(*it) == std::string::npos)
            throw HttpErrorException(BAD_REQUEST, "Invalid uri");
    }
    if (this->path.size() > URI_MAX_SIZE)
        throw HttpErrorException(URI_TOO_LONG, *this , "Uri too long");
}

void HttpRequest::setReqEntry(const std::string &reqEntry) {
    this->reqEntry = &reqEntry;
    this->bodySize += reqEntry.size();
}

std::string HttpRequest::getReqEntry(void) const {
    if (!this->reqEntry)
        throw std::logic_error("Trying to get req entry before setting it");
    return *this->reqEntry;
}

const std::string *HttpRequest::getReqEntryPtr(void) const {
    return (this->reqEntry);
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

void HttpRequest::checkErrors(bool hostFound, bool contentLengthFound, bool isOk) const {
    if (!hostFound)
    throw HttpErrorException(BAD_REQUEST, "Host header not found");

if (this->method != "POST" && this->method != "GET" && this->method != "DELETE")
    throw HttpErrorException(NOT_IMPLEMENTED, *this, "Method not implemented");

if (!this->requestBlock->isMethodAllowed(this->method))
     throw HttpErrorException(METHOD_NOT_ALLOWED, *this, "Method not allowed");

if (this->isMultiForm && this->method == "POST" && isOk == false)
    throw HttpErrorException(BAD_REQUEST, *this, "Content-Type: multipart/form-data: malformed header");

if (this->method == "POST" && (!contentLengthFound && this->isChunked == false))
    throw HttpErrorException(BAD_REQUEST, *this, "Content-Length header not found and request is not chunked");
}

void HttpRequest::parseHeaders(std::stringstream &ss, const std::vector<Server> &servers, int serverPort) {
    std::string line;
    bool hostFound = false;
    bool isOk = true;
    bool contentLengthFound = false;
    while(getline(ss, line) && line != "\r") {
        if (line[line.size() - 1] == '\r')
            line = line.substr(0, line.size() - 1);
        std::string key;
        std::string value;

        size_t pos =  line.find(":");
        if (pos == std::string::npos)
            throw HttpErrorException(BAD_REQUEST, *this, ": not found in header");

        key = line.substr(0, pos);
        value = line.substr(pos + 1);
        AllUtils::removeLeadingSpaces(value);
        if (key == "Content-Length")
        {
            std::stringstream l(value);
            ssize_t check;
            l >> check;
            if (l.fail()) throw HttpErrorException(BAD_REQUEST, *this,  "invalid content length header 1");
            if (check < 0)
                throw HttpErrorException(BAD_REQUEST, *this, "Negative CL");
            this->contentLength = check;
            std::string dummy;
            l >> dummy;
            if (!l.eof())  throw HttpErrorException(BAD_REQUEST,*this, "invalid content length header");
            contentLengthFound = true;
            this->headers[key] = value;
            continue;
        }
        if (key == "Transfer-Encoding") {
            if (value == "chunked")
                this->isChunked = true;
            else if (this->method == "POST")
                throw HttpErrorException(NOT_IMPLEMENTED, *this, "Transfer-Encoding must be \"chunked\"");
            this->headers[key] = value;
            continue;
        }
		if (this->method == "POST" && key == "Content-Type"){
			if (value.length() > 20 && value.compare(0, 10, "multipart/form-data;")){
				size_t bIndx = value.find("boundary");
				if (bIndx == std::string::npos)
					isOk = false;
				else{
					std::string boundary = value.substr(bIndx);
					bIndx = boundary.find("=");
					if (bIndx == std::string::npos || bIndx + 1 >=  boundary.length())
						isOk = false;
					else{
						this->boundary = boundary.substr(bIndx + 1);
						this->isMultiForm = true;
					}
				}
			}
			else isOk = false;
		}
        this->headers[key] = value;
        if (key == "Host" || key == "host")
        {
            hostFound = true;
            const Server &server = getServer(value, servers, serverPort);
            this->server = &server;
            this->requestBlock = &server;
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
    checkErrors(hostFound, contentLengthFound, isOk);
}

void HttpRequest::parseCookies(void) {
    std::string cookieString = getHeader("Cookie");
    if (cookieString.empty())
        return ;
    std::stringstream ss(cookieString);
    std::string singleCookie;
    while(getline(ss, singleCookie, ';')) {
        AllUtils::removeLeadingSpaces(singleCookie);
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
            std::cout << extension << std::endl;
            this->isCgi = Cgi::isValidCgiExtension(extension, *this);
            return;
        }
    }
}

bool HttpRequest::isCgiRequest(void) const {
    return this->isCgi;
}

bool HttpRequest::isChunkedRequest(void) const {
    return this->isChunked;
}

const Server &HttpRequest::getServer(const std::string &host, const std::vector<Server>& servers, int serverPort) {

    std::string actualHost = host;
    size_t pos = actualHost.find(":");
    if (pos != std::string::npos)
        actualHost = actualHost.substr(0, pos);
    const Server *firstServerPort = NULL;
    for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); it++) {
        if (actualHost == it->getServerName() && serverPort == it->getPort())
            return *it;
        if (!firstServerPort && serverPort == it->getPort())
            firstServerPort = &(*it);
    }
    return *firstServerPort;
}

void HttpRequest::printHeaders(std::ostream &printHere) const {
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); it++) {
        printHere << "       " << it->first << ": " << it->second << '\n';
    }
}

bool HttpRequest::isMultiRequest(void) const{
	return this->isMultiForm;
}

const std::string & HttpRequest::getBoundary(void) const{
    return this->boundary;
}
