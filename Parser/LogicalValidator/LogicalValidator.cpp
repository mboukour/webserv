#include "LogicalValidator.hpp"

LogicalValidator::LogicalValidator() {}

bool LogicalValidator::isValidErrorCode(const std::string &code) // TO IMPROVE
{
    return (code == "404" || code == "500" || code == "502" || code == "503" || code == "504");
}


bool LogicalValidator::isValidDirective(const std::string &directive) // TO IMPROVE
{
    return (directive == "listen" || directive == "server_name" || directive == "error_page"
            || directive == "client_max_body_size" || directive == "root");
}

void LogicalValidator::checkLogicError(const std::vector<Block> &serverBlocks)
{
    for (std::vector<Block>::const_iterator it = serverBlocks.cbegin();
         it != serverBlocks.cend(); it++)
    {
        bool foundRoot = false;
        if (it->blockName.size() != 1 || it->blockName[0] != "server")
            throw std::logic_error("All server blocks must be under the name: server");
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
    }
}
