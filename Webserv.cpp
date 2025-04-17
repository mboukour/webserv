
#include "Parser/Parser.hpp"
#include "Parser/ServerFactory/ServerFactory.hpp"
#include "Server/ServerManager/ServerManager.hpp"
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "Utils/Logger/Logger.hpp"

#include <vector>
typedef std::vector<std::string> stringVec;

int main(int ac, char **av)
{
    std::string line;
    signal(SIGPIPE, SIG_IGN);

    if (ac != 2 && ac != 3)
    {
        std::cerr << "Usage: /path/to/Webserv {config_file}\n"
                << "Notice: You can set a custom name for the logfile "
                << "by using the command /path/to/Webserv {config_file} {logging_file}"
                << std::endl;
        return 1;
    }

    try {
        if (ac == 3 && !std::string(av[2]).empty())
            Logger::init(av[2]);
        else {
            std::cout << "Defaulting to webserv.log for logging..." << std::endl;
            Logger::init("webserv.log");
        }
    } catch (std::exception &exc) {
        std::cerr << "Fatal error: " << exc.what() << '\n';
        return (1);
    }

    std::vector<Server> servers;
    try {
        std::ifstream file(av[1]);
        if (!file.is_open())
            throw std::logic_error("Unable to open file");
        Parser::initializeTokens(file);
        Block res = Parser::parseConfigFile();
        std::vector<Block> parseBlocks = res.subBlocks;
        servers = ServerFactory::createServers(parseBlocks);

    } catch (const std::exception &exc) {
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
        Logger::getLogStream() << "Fatal execution error: " << e.what() << '\n';
        return (1);
    }
    Logger::close();
    ServerManager::cleanUp();
    return (1);
}