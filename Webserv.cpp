#include "Webserv.hpp"
#include "Parser/Parser.hpp"
#include "Parser/ServerFactory/ServerFactory.hpp"
#include "Debug/Debug.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <algorithm>


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


int main(int ac, char **av)
{
    std::string line;
    
    
    if (ac != 2)
    {
        std::cerr << "Usage: ./Webserv {config_file}\n";
        return 1;
    }
    try {
        std::ifstream file(av[1]);
        if (!file.is_open())
            throw std::logic_error("Unable to open file");
        Parser parser(file);
        Block res = parser.parseConfigFile();
        std::vector<Server> servers = ServerFactory::createServers(res.subBlocks);
        printServers(servers);
        // printBlock(res);
        // std::cout << "-------------\n";
        // printBlockTree(res);
    } catch (std::exception &exc) {
        std::cerr << "Parsing error: " << exc.what() << '\n';
        std::exit(EXIT_FAILURE);
    }
}