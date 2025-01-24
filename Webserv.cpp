#include "Webserv.hpp"
#include "Parser/Parser.hpp"
#include "Parser/ServerFactory/ServerFactory.hpp"
#include "Server/ServerManager/ServerManager.hpp"
#include "Debug/Debug.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <algorithm>


int main(int ac, char **av)
{
    std::string line;
    
    
    if (ac != 2)
    {
        std::cerr << "Usage: ./Webserv {config_file}\n";
        return 1;
    }
    std::vector<Block> parseBlocks;
    try {
        std::ifstream file(av[1]);
        if (!file.is_open())
            throw std::logic_error("Unable to open file");
        Parser parser(file);
        Block res = parser.parseConfigFile();
        parseBlocks = res.subBlocks;

    } catch (std::exception &exc) {
        std::cerr << "Fatal parsing error: " << exc.what() << '\n';
        return (1);
    }


    try
    {
        std::vector<Server> servers = ServerFactory::createServers(parseBlocks);
        ServerManager manager(servers);
        manager.startServerManager();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Fatal Runtime error: " << e.what() << '\n';
        return (1);
    }
}