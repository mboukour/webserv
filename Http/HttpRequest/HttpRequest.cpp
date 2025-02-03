#include "HttpRequest.hpp"
#include "../../Exceptions/HttpErrorException/HttpErrorException.hpp"


#include <cstddef>
#include <exception>
#include <iterator>
#include <sstream>
#include <iostream> // to remove later
#include <stdexcept>
#include <string>
#include <vector>

HttpRequest::HttpRequest(const std::string &request, const std::vector<Server> &servers, int serverPort): requestBlock(NULL) {

    this->primalRequest = request;
    std::stringstream ss(request);
    std::string line;
    bool hostFound = false;
    bool contentLengthFound = false;
    if (!std::getline(ss, line))
        throw HttpErrorException("HTTP/1.1", BAD_REQUEST, "Bad Request", "empty request", ""); // we assume the version?
    std::stringstream requestLine(line);
    if (!(requestLine >> this->method >> this->path >> this->version))
        throw HttpErrorException("HTTP/1.1" ,BAD_REQUEST, "Bad Request", "invalid request line", "");
    size_t pos = this->path.find("?");
    if (pos != std::string::npos)
    {
        this->queryString = this->path.substr(pos + 1);
        this->path = this->path.substr(0, pos);
    }
    if (this->version != "HTTP/1.1")
        throw HttpErrorException(this->version, HTTP_VERSION_NOT_SUPPORTED, "Http Version Not Supported", "version not supported: " + this->version, "");
    if (this->path.length() > URI_MAX_SIZE)
        throw HttpErrorException(this->version ,URI_TOO_LONG, "Uri Too Long", "uri too long", "");
    // implement better matching? 

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
        if (key == "Host")
        {
            hostFound = true;
            const Server &server = getServer(value, servers, serverPort);
            this->server = &server;
            this->requestBlock = &server;
            std::string toMatch = this->path;
            if (toMatch[toMatch.size() - 1] == '/')
                toMatch = toMatch.substr(0, toMatch.size() - 1);
               for (std::vector<Location>::const_iterator it = server.locationsCbegin();
                it != server.locationsCend(); it++) {
                    // DEBUG && std::cout << "Current location: " << it->getLocationName() << '\n';
                    if (this->path == it->getLocationName() || toMatch == it->getLocationName())
                    {
                        this->requestBlock = &(*it);
                        std::cout << "MATCHED\n";
                        break ;
                    }
                }
        }
        setIsCgi();
    }

    // if (line != "\r") throw std::logic_error("Invalid headers terminator");
    if (!hostFound) throw std::logic_error("Host header not found");
    // if (!contentTypeFound) throw std::logic_error("Content-Type header not found");
    if (!this->requestBlock->isMethodAllowed(this->method))
        throw HttpErrorException(this->version, METHOD_NOT_ALLOWED, "Method Not Allowed", "method not allowed: " + this->method, "");


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

const Server *HttpRequest::getServer(void) const {
    return this->server;
}

std::string HttpRequest::getQueryString(void) const {
    return this->queryString;
}

void HttpRequest::setIsCgi(void) {
    size_t pos = this->path.find_last_of('.');
    if (pos == std::string::npos)
    {
        this->isCgi = false;
        return ;
    }
    std::string extension = this->path.substr(pos + 1);
    this->isCgi =  (extension == "php" || extension == "py" || extension == "pl"); // add more if we need to
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