#include "ServerFactory.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

ServerFactory::ServerFactory() {}



bool ServerFactory::isValidSucccessCode(const std::string &code) {
    return (code == "100" || // Continue
            code == "101" || // Switching Protocols
            code == "200" || // OK
            code == "201" || // Created
            code == "202" || // Accepted
            code == "203" || // Non-Authoritative Information
            code == "204" || // No Content
            code == "205" || // Reset Content
            code == "206" || // Partial Content
            code == "300" || // Multiple Choices
            code == "301" || // Moved Permanently
            code == "302" || // Found
            code == "303" || // See Other
            code == "304" || // Not Modified
            code == "305" || // Use Proxy
            code == "307");  // Temporary Redirect
}

bool ServerFactory::isValidErrorCode(const std::string &code) {
    return (code == "400" || // Bad Request
            code == "401" || // Unauthorized
            code == "402" || // Payment Required
            code == "403" || // Forbidden
            code == "404" || // Not Found
            code == "405" || // Method Not Allowed
            code == "406" || // Not Acceptable
            code == "407" || // Proxy Authentication Required
            code == "408" || // Request Timeout
            code == "409" || // Conflict
            code == "410" || // Gone
            code == "411" || // Length Required
            code == "412" || // Precondition Failed
            code == "413" || // Request Entity Too Large
            code == "414" || // Request-URI Too Long
            code == "415" || // Unsupported Media Type
            code == "416" || // Requested Range Not Satisfiable
            code == "417" || // Expectation Failed
            code == "500" || // Internal Server Error
            code == "501" || // Not Implemented
            code == "502" || // Bad Gateway
            code == "503" || // Service Unavailable
            code == "504" || // Gateway Timeout
            code == "505");  // HTTP Version Not Supported
}
bool ServerFactory::isValidDirective(const std::string &directive) // TO IMPROVE
{
    return (directive == "listen" || directive == "server_name" || directive == "error_page"
            || directive == "client_max_body_size" || directive == "root" || directive == "methods"
            || directive == "autoindex" || directive == "index" || directive == "return" 
            || directive == "upload_path" || directive == "cgi" || directive == "mime_types");
}

bool ServerFactory::isAcceptedSubBlock(const std::string &directive)
{
    return (directive == "location");
}


void ServerFactory::parseMethods(ABlock &result, const stringVec &directives) {
    if (directives.size() == 1 || directives.size() > 4)
        throw std::logic_error("Invalid methods directive found");
    result.setAllMethodsAsDenied();
    for (std::vector<std::string>::const_iterator l = directives.begin(); l != directives.end(); l++) {
        if (*l == "GET") {
            if (result.getMethod(GET))
                throw std::logic_error("Recurring accepted methods are not accepted.");
            result.setMethod(GET, true);
        }   
        else if (*l == "POST") {
            if (result.getMethod(POST))
                throw std::logic_error("Recurring accepted methods are not accepted.");
            result.setMethod(POST, true);
        }
        else if (*l == "DELETE") {
            if (result.getMethod(DELETE))
                throw std::logic_error("Recurring accepted methods are not accepted.");
            result.setMethod(DELETE, true);
        }
    }
}

void ServerFactory::parseClientMaxBodySize(ABlock &result, const stringVec &directives) {
    result.setIsLimited(true);
    if (directives.size() != 2)
        throw std::logic_error("Invalid client max body size directive");
    std::stringstream ss(directives.begin()[1]);
    ssize_t integerPart;
    ss >> integerPart;
    if (ss.fail())
        throw std::logic_error("Invalid client max body size directive");
    if (!ss.eof()) {
        std::string remaining;
        ss >> remaining;
        if (ss.fail())
            throw std::logic_error("Invalid client max body size directive");
        std::string dummyString;
        ss >> dummyString;
        if (!ss.eof())
            throw std::logic_error("Invalid client max body size directive");
        if (remaining.size() > 1)
            throw std::logic_error("Invalid unit in max body size directive");

        char unit;
        if (remaining.size() == 1) {
            unit = remaining[0];
            if (unit != 'k' && unit != 'K' && unit != 'm' && unit != 'M' && unit != 'g' && unit != 'G')
                throw std::logic_error("Invalid unit found on max body size directive");
        }
        else {
            unit = -1;
        }
        std::cout << "REM: " << remaining << std::endl;
        if (integerPart < 0)
            throw std::logic_error("Can't have negative max body size");
        // OVERFLOW RISK: BE CAREFUL!!
        if (unit == 'k' || unit == 'K')
            integerPart *= KB;
        else if (unit == 'm' || unit == 'M')
            integerPart *= MB;
        else if (unit == 'g' || unit == 'G')
            integerPart *= GB;
        result.setMaxBodySize(integerPart);
    } else 
        result.setMaxBodySize(integerPart);
}

void ServerFactory::setBlockDirectives(ABlock &result, const stringVec &directives) {
    std::string currentDirective = directives.begin()[0];

    if (currentDirective == "index") {
        if (directives.size() < 2)
            throw std::logic_error("Invalid index directive found");
        for (stringVec::const_iterator itr = directives.begin() + 1; itr != directives.end(); itr++)
            result.addIndex(*itr);
    }
    else if (currentDirective == "methods") 
        parseMethods(result, directives);
    else if (currentDirective == "autoindex") {
        if (directives.size() != 2)
            throw std::logic_error("Invalid autoindex directive found");
        if (directives.begin()[1] == "on")
            result.setAutoIndex(true);
        else if (directives.begin()[1] != "off")
            throw std::logic_error("Invalid autoindex directive found");
    }
    else if (currentDirective == "root") {
        if (directives.size() != 2) throw std::logic_error("Invalid root directive");
        std::string root = directives.begin()[1];
        if (root[root.size() - 1] != '/')
            root += "/";
        result.setRoot(root);
    }
    else if (currentDirective == "client_max_body_size" )
        parseClientMaxBodySize(result, directives);
    else if (currentDirective == "error_page") {
            if (directives.size() < 3) 
                throw std::logic_error("Invalid error page directive");
            const std::string errorPath = *(directives.end() - 1);
            for (stringVec::const_iterator itr = directives.begin() + 1; itr != directives.end() - 1; itr++) {
                if (!isValidErrorCode(*itr)) throw std::logic_error("Invalid error code found");
                const std::string errorCode = *itr;
                result.setErrorPagePath(errorCode, errorPath);
            }
    }
    else if (currentDirective == "cgi") {
        if (directives.size() != 3)
            throw std::logic_error("Invalid cgi directive");
        result.setCgiInfo(directives.begin()[1], directives.begin()[2]);
    } else if (currentDirective == "upload_path") {
        if (directives.size() != 2)
            throw std::logic_error("Invalid upload path directive");
        result.setUploadPath(directives.begin()[1]);
    }
}

std::ostream& operator<<(std::ostream& outputStream, const Location& location);
std::ostream& operator<<(std::ostream& outputStream, const Server& other);

void ServerFactory::validateServerBlock(const Block& serverBlock) {
    if (serverBlock.blockName.size() != 1 || serverBlock.blockName[0] != "server")
        throw std::logic_error("All server blocks must be under the name: server");

    if (serverBlock.directives.begin()->begin()[0] != "listen")
        throw std::logic_error("Listen directive must be the first directive in each server block");
}

void ServerFactory::parseListen(Server& server, const stringVec& directive) {
    if (directive.size() != 2)
        throw std::logic_error("Invalid listen directive");

    std::string value = directive[1];
    if (value[0] == ':')
        throw std::logic_error("Invalid listen directive");

    std::string host;
    int port;
    size_t twoPointPos = value.find(':');
    
    if (twoPointPos != std::string::npos) {
        host = value.substr(0, twoPointPos);
        std::string portStr = value.substr(twoPointPos + 1);
        std::stringstream ss(portStr);
        ss >> port;
        if (ss.fail() || !ss.eof() || port < 1 || port > __UINT16_MAX__)
            throw std::logic_error("Invalid port found");
    } else {
        host = "";
        std::stringstream ss(value);
        ss >> port;
        if (ss.fail() || !ss.eof() || port < 1 || port > __UINT16_MAX__)
            throw std::logic_error("Invalid port found");
    }

    if (!host.empty())
        server.setHost(host);
    server.setPort(port);
}

void ServerFactory::parseDirectives(Server& server, const std::vector<stringVec>& directives) {
    bool serverNameFound = false;
    for (std::vector<stringVec>::const_iterator ite = directives.begin(); ite != directives.end(); ite++) {
        std::string currentDirective = ite->begin()[0];

        if (!isValidDirective(currentDirective))
            throw std::logic_error("Unknown directive found");

        if (currentDirective == "listen")
            parseListen(server, *ite);
        else if (currentDirective == "server_name") {
            if (ite->size() != 2)
                throw std::logic_error("Invalid server name");
            serverNameFound = true;
            server.setServerName(ite->begin()[1]);
        }
        else if (currentDirective == "return")
            throw std::logic_error("The return directive can not be used in the server block.");
        else if (currentDirective == "mime_types") {
            if (ite->size() != 2)
                throw std::logic_error("Invalid mime_types directive");
            server.parseMimeTypeFile(ite->begin()[1]);
        }
        else
            setBlockDirectives(server, *ite);
    }
    if (!serverNameFound)
        throw std::logic_error("Server name directive not found in one or more servers");
}

Location ServerFactory::createLocation(const Block& locationBlock, std::vector<std::string>& foundLocations) {
    if (locationBlock.blockName.size() != 2)
        throw std::logic_error("Invalid location block found");
    if (locationBlock.blockName[1][0] != '/')
        throw std::logic_error("Locations must start with a /");
    if (std::find(foundLocations.begin(), foundLocations.end(), locationBlock.blockName[1]) != foundLocations.end())
        throw std::logic_error("Duplicate location paths are not accepted.");

    Location newLocation;
    newLocation.setLocationName(locationBlock.blockName[1]);
    foundLocations.push_back(locationBlock.blockName[1]);

    for (std::vector<stringVec>::const_iterator ite = locationBlock.directives.begin();
        ite != locationBlock.directives.end(); ite++) {
        std::string currentDirective = ite->begin()[0];
        if (currentDirective == "return") {
            parseReturnDirective(newLocation, *ite);
        }
        else
            setBlockDirectives(newLocation, *ite);
    }
    return newLocation;
}

void ServerFactory::parseReturnDirective(Location& location, const stringVec& directive) {
    const std::string code = directive[1];
    if (directive.size() == 1 || directive.size() > 3)
        throw std::logic_error("Invalid return directive");
    if (!isValidSucccessCode(code) && !isValidErrorCode(code))
        throw std::logic_error("Invalid status code in return directive");
    
    location.setReturnDirective(code, directive.size() == 2 ? "" : directive[2]);
}

void ServerFactory::inheritErrorPages(Server& server) {
    for (std::vector<Location>::iterator it = server.locationsBegin(); it != server.locationsEnd(); it++) {
        for (std::map<std::string, std::string>::const_iterator ite = server.errorPagesCbegin(); 
             ite != server.errorPagesCend(); ite++) {
            if (!hasErrorPage(*it, ite->first))
                it->setErrorPagePath(ite->first, ite->second);
        }
    }
}

bool ServerFactory::hasErrorPage(const Location& location, const std::string& errorCode) {
    for (std::map<std::string, std::string>::const_iterator it = location.errorPagesCbegin(); 
         it != location.errorPagesCend(); it++) {
        if (it->first == errorCode)
            return true;
    }
    return false;
}

Server ServerFactory::createServer(const Block &serverBlock) {
    Server result;

    validateServerBlock(serverBlock);
    parseDirectives(result,serverBlock.directives);

    if (result.getRoot().empty())
        throw std::logic_error("Root directive not found");


    std::vector <std::string> foundLocations;
    for (std::vector<Block>::const_iterator itb = serverBlock.subBlocks.begin(); 
         itb != serverBlock.subBlocks.end(); itb++) {
        std::string subBlockName = itb->blockName[0];
        if (!isAcceptedSubBlock(subBlockName))
            throw std::logic_error("Unknown sub block found");
        if (subBlockName == "location")
        {
            if (itb->blockName.size() != 2)
                throw std::logic_error("Invalid location block found");
            if (itb->blockName[1][0] != '/')
                throw std::logic_error("Locations must start with a /");
        }
        Location newLocation;
        if (std::find(foundLocations.begin(), foundLocations.end(), itb->blockName[1]) != foundLocations.end()) 
            throw std::logic_error("Duplicate location paths are not accepted.");
        newLocation.setLocationName(itb->blockName[1]);
        foundLocations.push_back(itb->blockName[1]);
        for (std::vector<stringVec>::const_iterator ite = itb->directives.begin();
            ite != itb->directives.end(); ite++) {
            std::string currentDirective = ite->begin()[0];
            if (currentDirective == "return") {
                const std::string code = ite->begin()[1];
                if (ite->size() == 1 || ite->size() > 3)
                    throw std::logic_error("Invalid return directive");
                if (!isValidSucccessCode(code) && !isValidErrorCode(code))
                    throw std::logic_error("Invalid status code in return directive");
                if (ite->size() == 2)
                    newLocation.setReturnDirective(code, "");
                else
                    newLocation.setReturnDirective(code, ite->begin()[2]);
            }
            else
                setBlockDirectives(newLocation, *ite);
        }
        result.addLocation(newLocation);
    }

    // Inheriting error codes
    for (std::vector<Location>::iterator it = result.locationsBegin(); it != result.locationsEnd(); it++) {
        for (std::map<std::string, std::string>::const_iterator ite = result.errorPagesCbegin(); ite != result.errorPagesCend(); ite++) {
            const std::string &errorCode = ite->first;
            const std::string &errorPath = ite->second;
            bool foundKey = false;
            for (std::map<std::string, std::string>::const_iterator lE = it->errorPagesCbegin(); lE != it->errorPagesCend(); lE++) {
                if (lE->first == errorCode)
                {
                    foundKey = true;
                    break;
                }
            }   
            if (!foundKey)
                it->setErrorPagePath(errorCode, errorPath);
        }
    }

    return result;
}



std::vector<Server> ServerFactory::createServers(const std::vector<Block> &serverBlocks) {
    std::vector<Server> result;
    for (std::vector<Block>::const_iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
    {
        Server newServer = createServer(*it);
        result.push_back(newServer);
    }
    return (result);
}
