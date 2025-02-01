#include "ServerFactory.hpp"
#include <iostream>
#include <sstream>
#include <climits>
#include <algorithm>

ServerFactory::ServerFactory() {}



bool ServerFactory::isValidErrorCode(const std::string &code) // TO IMPROVE
{

    return (code == "400" || code == "403" || code == "404" || code == "405" || code == "409" || code == "413" || code == "414" || code == "500"  || code == "501" || code == "502" || code == "503" || code == "504" || code == "505");
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
        std::string root = directives.begin()[1];
        if (root[root.size() - 1] != '/')
            root += "/";
        result.setRoot(root);
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
                throw std::logic_error("Inva0lid unit found on max body size directive 3");
            result.setMaxBodySize(integerPart);
        }
    } else if (currentDirective == "error_page") {
            if (directives.size() < 3) 
                throw std::logic_error("Invalid error page directive");
            const std::string errorPath = *(directives.end() - 1);
            for (stringVec::const_iterator itr = directives.begin() + 1; itr != directives.end() - 1; itr++) {
                if (!isValidErrorCode(*itr)) throw std::logic_error("Invalid error code found");
                const std::string errorCode = *itr;
                result.setErrorPagePath(errorCode, errorPath);
            }
        }
    // else
    // {
    //     std::cout << currentDirective << '\n';
    //     throw std::logic_error("Unknown directive found");
    // }
    // else // This normally should have invalid directives?
}
std::ostream& operator<<(std::ostream& outputStream, const Location& location);
std::ostream& operator<<(std::ostream& outputStream, const Server& other);
Server ServerFactory::createServer(const Block &serverBlock) {
    Server result;

    if (serverBlock.blockName.size() != 1 || serverBlock.blockName[0] != "server")
        throw std::logic_error("All server blocks must be under the name: server");

    if (serverBlock.directives.begin()->begin()[0] != "listen")
        throw std::logic_error("Listen directive must be the first directive in each server block");

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
                if (itb->blockName.size() != 3)
                    throw std::logic_error("Invalid return directive");
                
            }
            else
                setBlockDirectives(newLocation, *ite);
        }
        std::cout << newLocation << '\n';
        result.addLocation(newLocation);
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
