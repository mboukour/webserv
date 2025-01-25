#include "ServerFactory.hpp"
#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

ServerFactory::ServerFactory() {}

bool ServerFactory::isValidErrorCode(const std::string &code) // TO IMPROVE
{
    return (code == "404" || code == "500" || code == "502" || code == "503" || code == "504");
}


bool ServerFactory::isValidDirective(const std::string &directive) // TO IMPROVE
{
    return (directive == "listen" || directive == "server_name" || directive == "error_page"
            || directive == "client_max_body_size" || directive == "root" || directive == "methods"
            || directive == "autoindex" || directive == "index" || directive == "return" || directive == "upload_store");
}

bool ServerFactory::isAcceptedSubBlock(const std::string &directive)
{
    return (directive == "location");
}

// Todo: change error messages to be more precise (throwException)

void ServerFactory::setBlockDirectives(ABlock &result, const stringVec &directives) {
    std::string currentDirective = directives.begin()[0];


    if (currentDirective == "methods") {
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
        result.setRoot(directives.begin()[1]);
    }
    else if (currentDirective == "client_max_body_size" ) {
        result.setIsLimited(true);
        if (directives.size() != 2)
            throw std::logic_error("Invalid client max body size directive");
        std::stringstream ss(directives.begin()[1]);
        size_t integerPart;
        ss >> integerPart;
        if (ss.fail())
            throw std::logic_error("Invalid client max body size directive");
        if (!ss.eof()) {
            char unit;
            ss >> unit;
            if (ss.fail())
                throw std::logic_error("Invalid client max body size directive");
            std::string dummyString;
            ss >> dummyString;
            if (!ss.eof())
                throw std::logic_error("Invalid client max body size directive");
            // OVERFLOW RISK: BE CAREFUL!!
            if (unit == 'k' || unit == 'K')
                integerPart *= KB;
            else if (unit == 'm' || unit == 'M')
                integerPart *= MB;
            else if (unit == 'g' || unit == 'G')
                integerPart *= GB;
            else
                throw std::logic_error("Invalid unit found on max body size directive 3");
            result.setMaxBodySize(integerPart);
        }
    }
    // else // This normally should have invalid directives?
}
std::ostream& operator<<(std::ostream& outputStream, const Location& location);
Server ServerFactory::createServer(const Block &serverBlock) {
    Server result;

    if (serverBlock.blockName.size() != 1 || serverBlock.blockName[0] != "server")
        throw std::logic_error("All server blocks must be under the name: server");

    // Directive checker
    if (serverBlock.directives.begin()->begin()[0] != "listen")
        throw std::logic_error("Listen directive must be the first directive in each server block");

    // Iterate through directives and check/set them
    for (std::vector<stringVec>::const_iterator ite = serverBlock.directives.begin();
         ite != serverBlock.directives.end(); ite++) {

        std::string currentDirective = ite->begin()[0];

        if (!isValidDirective(currentDirective))
            throw std::logic_error("Unknown directive found");

        if (currentDirective == "listen") {
            if (ite->size() != 2)
                throw std::logic_error("Invalid listen directive");
            std::stringstream ss(ite->begin()[1]); 
            int port;
            ss >> port;
            if (ss.fail() || !ss.eof() || port < 1 || port > __UINT16_MAX__)
                throw std::logic_error("Invalid port found");
            result.setPort(port);
        } 
        else if (currentDirective == "server_name") {
            if (ite->size() != 2)
                throw std::logic_error("Invalid server name");
            result.setServerName(ite->begin()[1]);
        }
        else if (currentDirective == "error_page") {
            if (ite->size() < 3) 
                throw std::logic_error("Invalid error page directive");
            for (stringVec::const_iterator itr = ite->begin() + 1; itr != ite->end() - 1; itr++) {
                if (!isValidErrorCode(*itr)) throw std::logic_error("Invalid error code found");
                // Add func
            }
        }
        else if (currentDirective == "return")
            throw std::logic_error("The return directive can not be used in the server block.");
        else
            setBlockDirectives(result, *ite);
    }

    if (result.getRoot().empty())
        throw std::logic_error("Root directive not found");


    std::vector <std::string> foundLocations;
    for (std::vector<Block>::const_iterator itb = serverBlock.subBlocks.begin(); 
         itb != serverBlock.subBlocks.end(); itb++) {

        std::string subBlockName = itb->blockName[0];
        if (!isAcceptedSubBlock(subBlockName))
            throw std::logic_error("Unknown sub block found");
        if (subBlockName == "location")
            if (itb->blockName.size() != 2)
                throw std::logic_error("Invalid location block found");
        Location newLocation;
        if (std::find(foundLocations.begin(), foundLocations.end(), itb->blockName[1]) != foundLocations.end()) 
            throw std::logic_error("Duplicate location paths are not accepted.");
        newLocation.setLocationName(itb->blockName[1]);
        foundLocations.push_back(itb->blockName[1]);
        for (std::vector<stringVec>::const_iterator ite = itb->directives.begin();
            ite != itb->directives.end(); ite++) {
            std::string currentDirective = ite->begin()[0];
            if (currentDirective == "return") {
                // i will impelement return code later cuz im tired as FUCK
            }
            else
                setBlockDirectives(newLocation, *ite);
        }
        result.locations.push_back(newLocation);
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