#include "HttpRequest.hpp"
#include "../../Exceptions/UnknownMethod/UnknownMethod.hpp"
#include <sstream>
#include <iostream> // to remove later

HttpRequest::HttpRequest(const std::string &request) {

    this->primalRequest = request;
    std::stringstream ss(request);
    std::string line;
    bool hostFound = false;
    // bool contentLengthFound = false;
    // bool contentTypeFound = false;

    if (!std::getline(ss, line)) {
        throw std::runtime_error("Invalid HTTP request: missing request line");
    }
    std::stringstream requestLine(line);
    if (!(requestLine >> this->method >> this->path >> this->version)) {
        throw std::runtime_error("Invalid HTTP request: malformed request line");
    }
    if (this->method != "POST" && this->method != "GET" && this->method != "DELETE") {
        throw UnknownMethod(this->method);
    }
    this->bodySize = 0;
    while(getline(ss, line) && line != "\r") {
        if (line[line.size() - 1] == '\r')
            line[line.size() - 1] = '\0';
        std::stringstream lineSs(line);
        std::string key;
        std::string value;

        lineSs >> key;
        if (lineSs.fail() || lineSs.eof() || key[key.size() - 1] != ':') throw std::logic_error("Invalid header");
        key = key.substr(0, key.size() - 1);
        if (key == "Host")
            hostFound = true;
        // else if (key == "Content-Type")
        //     contentTypeFound = true;
        lineSs >> value;
        if (lineSs.fail()) throw std::logic_error("Invalid header");
        if (key == "Content-Length")
        {
            std::stringstream l(value);
            l >> this->bodySize;
            if (l.fail()) throw std::logic_error("Invalid content length header.");
            std::string dummy;
            l >> dummy;
            if (!l.eof()) throw std::logic_error("Invalid content length header.");
            // contentLengthFound = true;
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