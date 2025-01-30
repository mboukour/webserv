#include "HttpRequest.hpp"
#include "../../Exceptions/HttpRequestParseException/HttpRequestParseException.hpp"
#include "../../Exceptions/NotImplementedException/NotImplementedException.hpp"
#include "../../Exceptions/PayloadTooLargeException/PayloadTooLargeException.hpp"
#include "../../Exceptions/MethodNotAllowedException/MethodNotAllowedException.hpp"
#include "../../Exceptions/UnknownMethodException/UnknownMethodException.hpp"
#include "../../Exceptions/NotFoundException/NotFoundException.hpp"

#include <sstream>
#include <iostream> // to remove later


HttpRequest::HttpRequest(const std::string &request, const Server& server): requestBlock(&server) {

    this->primalRequest = request;
    std::stringstream ss(request);
    std::string line;
    bool hostFound = false;
    if (!std::getline(ss, line))
        throw HttpRequestParseException("empty request");
    std::stringstream requestLine(line);
    if (!(requestLine >> this->method >> this->path >> this->version))
        throw HttpRequestParseException("invalid request line");


    // implement better matching? 
    std::cout << "Path: " << this->path << '\n';
    std::string toMatch = this->path;
    if (toMatch[toMatch.size()] != '/')
        toMatch = toMatch + '/';
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
        if (!this->requestBlock->isMethodAllowed(this->method))
            throw MethodNotAllowedException(this->method);
        
    }
    catch(const UnknownMethodException& e)
    {
        throw NotImplementedException(this->method);
    }
    
    this->bodySize = 0;
    while(getline(ss, line) && line != "\r") {
        if (line[line.size() - 1] == '\r')
            line[line.size() - 1] = '\0';
        std::stringstream lineSs(line);
        std::string key;
        std::string value;

        lineSs >> key;
        if (lineSs.fail() || lineSs.eof() || key[key.size() - 1] != ':') throw HttpRequestParseException("invalid header");
        key = key.substr(0, key.size() - 1);
        if (key == "Host")
            hostFound = true;
        // else if (key == "Content-Type")
        //     contentTypeFound = true;
        lineSs >> value;
        if (lineSs.fail()) throw HttpRequestParseException("invalid header");
        if (key == "Content-Length")
        {
            std::stringstream l(value);
            l >> this->bodySize;
            if (l.fail()) throw HttpRequestParseException("invalid content length header");
            std::string dummy;
            l >> dummy;
            if (!l.eof()) throw HttpRequestParseException("invalid content length header");
            if (this->requestBlock->getIsLimited() && this->bodySize > this->requestBlock->getMaxBodySize())
                throw PayloadTooLargeException(this->requestBlock->getMaxBodySize());
            continue;
        }
        this->headers[key] = value;
    }

    if (line != "\r") throw std::logic_error("Invalid headers terminator");
    if (!hostFound) throw std::logic_error("Host header not found");
    // if (!contentTypeFound) throw std::logic_error("Content-Type header not found");
    // if (!contentLengthFound) throw std::logic_error("Content-Length header not found");
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

