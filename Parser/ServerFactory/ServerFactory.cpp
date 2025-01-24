#include "ServerFactory.hpp"
#include <iostream>
#include <sstream>
#include <climits>

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
Server ServerFactory::createServer(const Block &serverBlock)
{
    Server result;
    bool foundRoot = false;

    if (serverBlock.blockName.size() != 1 || serverBlock.blockName[0] != "server")
        throw std::logic_error("All server blocks must be under the name: server");
    // Directive checker
    if (serverBlock.directives.cbegin()->cbegin()[0] != "listen")
        throw std::logic_error("Listen directive must be the first directive in each server block");
    for(std::vector<stringVec>::const_iterator ite = serverBlock.directives.cbegin();
        ite != serverBlock.directives.cend(); ite++)
    {
        std::string currentDirective = ite->begin()[0];
        if (!isValidDirective(currentDirective))
            throw std::logic_error("Unknown directive found");
        if (currentDirective == "listen")
        {
            if (ite->size() != 2)
                throw std::logic_error("Invalid listen directive");
            std::stringstream ss(ite->begin()[1]); 
            int port;
            ss >> port;
            if (ss.fail() || !ss.eof() || port < 1 || port > __UINT16_MAX__)
                throw std::logic_error("Invalid port found");
            result.setPort(port);
        } 
        else if (currentDirective == "server_name") 
        {
            if (ite->size() != 2)
                throw std::logic_error("Invalid server name");
            result.setServerName(ite->begin()[1]);
        }
        else if (currentDirective == "error_page")
        {
            if (ite->size() < 3) 
                throw std::logic_error("Invalid error page directive");
            for (stringVec::const_iterator itr = ite->cbegin() + 1; itr != ite->cend() - 1; itr++)
            {
                if (!isValidErrorCode(*itr)) throw std::logic_error("Invalid error code found");
                // Add func
            }
            // itr here will point the error page path (additional checks might be necessary)
        }
        else if (currentDirective == "root")
        {
            if (ite->size() != 2) throw std::logic_error("Invalid root directive");
            foundRoot = true;
            result.setRoot(ite->begin()[1]);
        }
        else if (currentDirective == "client_max_body_size" )
        {
            if (ite->size() != 2)
                throw std::logic_error("Invalid client max body size directive");
            std::stringstream ss(ite->begin()[1]);
            size_t integerPart;
            ss >> integerPart;
            if (ss.fail())
                throw std::logic_error("Invalid client max body size directive");
            if (!ss.eof())
            {

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
                    integerPart *= 1024;
                else if (unit == 'm' || unit == 'M')
                    integerPart *= 1048576;
                else if (unit == 'g' || unit == 'G')
                    integerPart *= 1073741824;
                else
                    throw std::logic_error("Invalid unit found on max body size directive 3");
                result.setMaxBodySize(integerPart);
            }
        }
    }
    if (!foundRoot)
        throw std::logic_error("Root directive not found");
    
    // subBlock checker ----> TO COMPLETE
    for(std::vector<Block>::const_iterator itb = serverBlock.subBlocks.cbegin(); 
        itb != serverBlock.subBlocks.cend(); itb++)
    {
        std::string subBlockName = itb->blockName[0];
        if (!isAcceptedSubBlock(subBlockName))
            throw std::logic_error("Unknown sub block found");
        if (subBlockName == "location" && itb->blockName.size() != 2)
            throw std::logic_error("Invalid location block found");
    }
    return (result);
}


std::vector<Server> ServerFactory::createServers(const std::vector<Block> &serverBlocks) {
    std::vector<Server> result;
    // result.reserve(serverBlocks.size());
    for (std::vector<Block>::const_iterator it = serverBlocks.cbegin(); it != serverBlocks.cend(); it++)
    {
        result.push_back(createServer(*it));
    }
    return (result);
}