#ifndef SERVERFACTORY_HPP
#define SERVERFACTORY_HPP

#include <vector>
#include <string>
#include "../Parser.hpp"
#include "../../Server/Server.hpp"

#define KB 1024
#define MB 1048576
#define GB 1073741824


class ServerFactory {
    private:
        static bool isValidSucccessCode(const std::string &code);
        static bool isValidErrorCode(const std::string &code);
        static bool isValidDirective(const std::string &directive);
        static bool isAcceptedSubBlock(const std::string &directive);
        static void setBlockDirectives(ABlock &result, const stringVec &directives);
        static Server createServer(const Block& serverBlock);
        ServerFactory();
    public:
        static void linkLocationsToServer(std::vector<Server> &servers);
        static std::vector<Server> createServers(const std::vector<Block> &serverBlocks);
};



#endif