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

        static void parseMethods(ABlock &result, const stringVec &directives);
        static void parseClientMaxBodySize(ABlock &result, const stringVec &directives);
        static void setBlockDirectives(ABlock &result, const stringVec &directives);

        static void validateServerBlock(const Block& serverBlock);
        static void parseListen(Server &server, const stringVec &directive);
        static void parseDirectives(Server& server, const std::vector<stringVec>& directives);
        static void parseReturnDirective(Location &location, const stringVec &directive);
        static void inheritErrorPages(Server &server);
        static bool hasErrorPage(const Location &location, const std::string &errorPage);
        static Location createLocation(const Block& locationBlock, std::vector<std::string>& foundLocations);
        static Server createServer(const Block& serverBlock);
        ServerFactory();
    public:
        static void linkLocationsToServer(std::vector<Server> &servers);
        static std::vector<Server> createServers(const std::vector<Block> &serverBlocks);
};



#endif