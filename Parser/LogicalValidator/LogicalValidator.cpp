#include "LogicalValidator.hpp"
#include <iostream>
LogicalValidator::LogicalValidator() {}

bool LogicalValidator::isValidErrorCode(const std::string &code) // TO IMPROVE
{
    return (code == "404" || code == "500" || code == "502" || code == "503" || code == "504");
}


bool LogicalValidator::isValidDirective(const std::string &directive) // TO IMPROVE
{
    return (directive == "listen" || directive == "server_name" || directive == "error_page"
            || directive == "client_max_body_size" || directive == "root" || directive == "methods"
            || directive == "autoindex" || directive == "index" || directive == "return" || directive == "upload_store");
}

bool LogicalValidator::isAcceptedSubBlock(const std::string &directive)
{
    return (directive == "location");
}

void LogicalValidator::checkLogicError(const std::vector<Block> &serverBlocks)
{
    // std::vector<Server> res;
    for (std::vector<Block>::const_iterator it = serverBlocks.cbegin();
         it != serverBlocks.cend(); it++)
    {
        // this should ideally create a server object and append it to the end result vector
        // Server server;
        bool foundRoot = false;
        if (it->blockName.size() != 1 || it->blockName[0] != "server")
            throw std::logic_error("All server blocks must be under the name: server");
        // Directive checker
        if (it->directives.cbegin()->cbegin()[0] != "listen")
            throw std::logic_error("Listen directive must be the first directive in each server block");
        for(std::vector<stringVec>::const_iterator ite = it->directives.cbegin();
            ite != it->directives.cend(); ite++)
        {
            std::string currentDirective = ite->begin()[0];
            if (!isValidDirective(currentDirective))
                throw std::logic_error("Unknown directive found");
            if (currentDirective == "listen" && ite->size() != 2) throw std::logic_error("Invalid listen directive");
            else if (currentDirective == "server_name" && ite->size() != 2) throw std::logic_error("Invalid server name");
            else if (currentDirective == "error_page")
            {
                if (ite->size() < 3) throw std::logic_error("Invalid error page directive");
                for (stringVec::const_iterator itr = ite->cbegin() + 1; itr != ite->cend() - 1; itr++)
                    if (!isValidErrorCode(*itr)) throw std::logic_error("Invalid error code found");
                // itr here will point the error page path (additional checks might be necessary)
            }
            else if (currentDirective == "root")
            {
                if (ite->size() != 2) throw std::logic_error("Invalid root directive");
                foundRoot = true;
            }
            else if (currentDirective == "client_max_body_size" && ite->size() != 2 )
                throw std::logic_error("Invalid client max body size directive");
            
        }
        if (!foundRoot)
            throw std::logic_error("Root directive not found");
    
        // subBlock checker
        for(std::vector<Block>::const_iterator itb = it->subBlocks.cbegin(); 
            itb != it->subBlocks.cend(); itb++)
        {
            std::string subBlockName = itb->blockName[0];
            if (!isAcceptedSubBlock(subBlockName))
                throw std::logic_error("Unknown sub block found");
            if (subBlockName == "location" && itb->blockName.size() != 2)
                throw std::logic_error("Invalid location block found");
        }

        // res.push_back(server);
    }
}
