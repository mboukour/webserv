#ifndef SERVERFACTORY_HPP
#define SERVERFACTORY_HPP

#include <vector>
#include <string>
#include "../Parser.hpp"
#include "../../Server/Server.hpp"



class ServerFactory {
    private:        
        static bool isValidErrorCode(const std::string &code);
        static bool isValidDirective(const std::string &directive);
        static bool isAcceptedSubBlock(const std::string &directive);
        static Server createServer(const Block& serverBlock);
        ServerFactory();
    public:
        static std::vector<Server> createServers(const std::vector<Block> &serverBlocks);
};



#endif