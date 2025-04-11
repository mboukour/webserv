#include "Webserv.hpp"
#include "Parser/Parser.hpp"
#include "Parser/ServerFactory/ServerFactory.hpp"
#include "Server/ServerManager/ServerManager.hpp"
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "Utils/Logger/Logger.hpp"

int main(int ac, char **av)
{
    std::string line;
    signal(SIGPIPE, SIG_IGN);

    if (ac != 2 && ac != 3)
    {
        std::cerr << "Usage: ./Webserv {config_file}\n";
        std::cerr << "Notice: You can set a custom name for the logfile: ";
        return 1;
    }
    if (ac == 3)
        Logger::init(av[2]);
    else
        Logger::init("webserv.log");
    std::vector<Server> servers;
    try {
        std::ifstream file(av[1]);
        if (!file.is_open())
            throw std::logic_error("Unable to open file");
        Parser::initializeTokens(file);
        Block res = Parser::parseConfigFile();
        std::vector<Block> parseBlocks = res.subBlocks;
        servers = ServerFactory::createServers(parseBlocks);

    } catch (std::exception &exc) {
        std::cerr << "Fatal parsing error: " << exc.what() << '\n';
        Logger::getLogStream() << "Fatal parsing error: " << exc.what() << '\n';
        return (1);
    }


    try
    {
        ServerManager::initialize(servers);
        ServerManager::start();
    }
    catch(const std::exception& e)
    {
        ServerManager::cleanUp();
        Logger::close();
        std::cerr << "Fatal Runtime error: " << e.what() << '\n';
        Logger::getLogStream() << "Fatal parsing error: " << e.what() << '\n';
        return (1);
    }
    Logger::close();
    ServerManager::cleanUp();
    return (1);
}