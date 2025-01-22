#include "Webserv.hpp"
#include "Parser/Parser.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>


void printStringVec(const stringVec& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i != vec.size() - 1) {
            std::cout << " "; // Separate words with a space
        }
    }
}

// Recursive function to print a Block and its sub-blocks
void printBlock(const Block& block, int level = 0) {
    // Print block name (indentation based on level)
    for (int i = 0; i < level; ++i) {
        std::cout << "    "; // Indentation for better visual structure
    }
    std::cout << "Block Name: ";
    printStringVec(block.blockName);
    std::cout << std::endl;

    // Print directives within the block at the current level
    if (!block.directives.empty()) {
        for (int i = 0; i < level; ++i) {
            std::cout << "    "; // Indentation for directives
        }
        std::cout << "Directives: " << std::endl; // Categorizing directives

        for (size_t i = 0; i < block.directives.size(); ++i) {
            for (int j = 0; j < level + 1; ++j) {
                std::cout << "    "; // Increased indentation for individual directives
            }
            std::cout << "- ";
            printStringVec(block.directives[i]);
            std::cout << std::endl;
        }
    }

    // Print sub-blocks recursively with an increased level of indentation
    if (!block.subBlocks.empty()) {
        for (int i = 0; i < level; ++i) {
            std::cout << "    "; // Indentation for sub-blocks
        }
        std::cout << "Sub-blocks: " << std::endl; // Categorizing sub-blocks

        for (size_t i = 0; i < block.subBlocks.size(); ++i) {
            printBlock(block.subBlocks[i], level + 1);  // Increase indentation for sub-blocks
        }
    }
}
// Recursive function to print a Block and its sub-blocks as a tree structure
void printBlockTree(const Block& block, int level = 0) {
    // Print block name (indentation based on level)
    for (int i = 0; i < level; ++i) {
        std::cout << "    "; // Indentation for better visual structure
    }
    std::cout << "- " << block.blockName.front() << std::endl;  // Print the block name
    
    // Print sub-blocks recursively with an increased level of indentation
    for (const auto& subBlock : block.subBlocks) {
        printBlockTree(subBlock, level + 1);  // Increase indentation for sub-blocks
    }
}

bool isValidErrorCode(const std::string &code) // TO IMPROVE
{
    return (code == "404" || code == "500" || code == "502" || code == "503" || code == "504");
}


bool isValidDirective(const std::string &directive) // TO IMPROVE
{
    return (directive == "listen" || directive == "server_name" || directive == "error_page"
            || directive == "client_max_body_size" || directive == "root");
}

void checkLogicError(const std::vector<Block> &serverBlocks)
{
    for (std::vector<Block>::const_iterator it = serverBlocks.cbegin();
         it != serverBlocks.cend(); it++)
    {
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
            if (currentDirective == "listen" && ite->size() != 2)
                throw std::logic_error("Invalid listen directive");
            else if (currentDirective == "server_name" && ite->size() != 2)
                throw std::logic_error("Invalid server name");
            else if (currentDirective == "error_page")
            {
                if (ite->size() < 3)
                    throw std::logic_error("Invalid error page directive");
                for (stringVec::const_iterator itr = ite->cbegin() + 1; itr != ite->cend() - 1; itr++)
                {
                    if (!isValidErrorCode(*itr))
                        throw std::logic_error("Invalid error code found");
                }
                // itr here will point the error page path (additional checks might be necessary)
            }
            else if (currentDirective == "root" && ite->size() != 2)
                throw std::logic_error("Invalid root directive");
            else if (currentDirective == "client_max_body_size" && ite->size() != 2 )
                throw std::logic_error("Invalid client max body size directive");
        }
    }
}

int main(int ac, char **av)
{
    std::string line;
    
    
    if (ac != 2)
    {
        std::cerr << "Usage: ./Webser {config_file}\n";
        return 1;
    }
    try {
        std::ifstream file(av[1]);
        if (!file.is_open())
            throw std::logic_error("Unable to open file");
        Parser parser(file);
        Block res = parser.parseConfigFile();
        checkLogicError(res.subBlocks);
        printBlock(res);
        std::cout << "-------------\n";
        printBlockTree(res);
    } catch (std::exception &exc) {
        std::cerr << "Parsing error: " << exc.what() << '\n';
        std::exit(EXIT_FAILURE);
    }
}