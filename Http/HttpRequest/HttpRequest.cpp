#include "HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"


#include <exception>
#include <sstream>
#include <iostream> // to remove later


HttpRequest::HttpRequest(const std::string &request, const Server& server): requestBlock(&server) {

    this->primalRequest = request;
    std::stringstream ss(request);
    std::string line;
    bool hostFound = false;
    bool contentLengthFound = false;
    if (!std::getline(ss, line))
        throw HttpErrorException("HTTP/1.1", BAD_REQUEST, "Bad Request", "empty request", server.getErrorPageHtml(BAD_REQUEST)); // we assume the version?
    std::stringstream requestLine(line);
    if (!(requestLine >> this->method >> this->path >> this->version))
        throw HttpErrorException("HTTP/1.1" ,BAD_REQUEST, "Bad Request", "invalid request line", server.getErrorPageHtml(BAD_REQUEST));

    if (this->version != "HTTP/1.1")
        throw HttpErrorException(this->version, HTTP_VERSION_NOT_SUPPORTED, "Http Version Not Supported", "version not supported: " + this->version, server.getErrorPageHtml(HTTP_VERSION_NOT_SUPPORTED));
    if (this->path.length() > URI_MAX_SIZE)
        throw HttpErrorException(this->version ,URI_TOO_LONG, "Uri Too Long", "uri too long", server.getErrorPageHtml(URI_TOO_LONG));
    // implement better matching? 
    std::cout << "Path: " << this->path << '\n';
    std::string toMatch = this->path;
    if (toMatch[toMatch.size() - 1] == '/')
        toMatch = toMatch.substr(0, toMatch.size() - 1);
    std::cout << "To Match: " << toMatch << '\n';
    for (std::vector<Location>::const_iterator it = server.locationsCbegin();
        it != server.locationsCend(); it++) {
            std::cout << "Current location: " << it->getLocationName() << '\n';
            if (this->path == it->getLocationName() || toMatch == it->getLocationName())
            {
                this->requestBlock = &(*it);
                std::cout << "MATCHED\n";
                break ;
            }
        }

    try
    {
        try {
        
            if (!this->requestBlock->isMethodAllowed(this->method))
                throw HttpErrorException(this->version, METHOD_NOT_ALLOWED, "Method Not Allowed", "method not allowed: " + this->method, requestBlock->getErrorPageHtml(METHOD_NOT_ALLOWED));
        } catch (const std::exception &exec) {
                throw HttpErrorException(this->version, NOT_IMPLEMENTED, "Not Implemented", "method not implemented: " + this->method, requestBlock->getErrorPageHtml(NOT_IMPLEMENTED));
        }

        
    }
    catch(const std::exception& e)
    {
        throw HttpErrorException(this->version, NOT_IMPLEMENTED, "Not Implemented", "method not implemented", requestBlock->getErrorPageHtml(NOT_IMPLEMENTED));
    }
    
    this->bodySize = 0;
    while(getline(ss, line) && line != "\r") {
        if (line[line.size() - 1] == '\r')
            line[line.size() - 1] = '\0';
        std::stringstream lineSs(line);
        std::string key;
        std::string value;

        lineSs >> key;
        // if (lineSs.fail() || lineSs.eof() || key[key.size() - 1] != ':') throw HttpErrorException(this->version, BAD_REQUEST, "Bad Request", "invalid header", requestBlock->getErrorPageHtml(BAD_REQUEST));
        key = key.substr(0, key.size() - 1);
        if (key == "Host")
            hostFound = true;
        // else if (key == "Content-Type")
        //     contentTypeFound = true;
        lineSs >> value;
        // if (lineSs.fail()) throw HttpErrorException(this->version, BAD_REQUEST, "Bad Request", "invalid header", requestBlock->getErrorPageHtml(BAD_REQUEST));
        if (key == "Content-Length")
        {
            std::stringstream l(value);
            l >> this->bodySize;
            if (l.fail()) throw HttpErrorException(this->version, BAD_REQUEST, "Bad Request", "invalid content length header", requestBlock->getErrorPageHtml(BAD_REQUEST));
            std::string dummy;
            l >> dummy;
            if (!l.eof()) throw HttpErrorException(this->version, BAD_REQUEST, "Bad Request", "invalid content length header", requestBlock->getErrorPageHtml(BAD_REQUEST));
            if (this->requestBlock->getIsLimited() && this->bodySize > this->requestBlock->getMaxBodySize())
                throw  HttpErrorException(this->version, PAYLOAD_TOO_LARGE, "Payload Too Large", "payload too large", requestBlock->getErrorPageHtml(PAYLOAD_TOO_LARGE));
            contentLengthFound = true;
            continue;
        }
        this->headers[key] = value;
    }

    // if (line != "\r") throw std::logic_error("Invalid headers terminator");
    if (!hostFound) throw std::logic_error("Host header not found");
    // if (!contentTypeFound) throw std::logic_error("Content-Type header not found");
    if (this->method == "POST")
    {
        if (!contentLengthFound) // or no Transfer-Encoding??
            throw HttpErrorException(this->version, BAD_REQUEST, "Bad Request", "Content-Length header not found", requestBlock->getErrorPageHtml(BAD_REQUEST));
    }
    // if (static_cast<size_t>(ss.rdbuf()->in_avail()) != this->bodySize) throw std::logic_error("Content-Length header and actual length don't match");

    if (this->bodySize > 0)
    {
        this->body.resize(this->bodySize);
        ss.read(&this->body[0], this->bodySize);
    }
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

