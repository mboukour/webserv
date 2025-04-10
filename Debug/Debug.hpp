#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "../Server/Server.hpp"
#include "../Http/HttpRequest/HttpRequest.hpp"
#define DEBUG 1

const std::string RESET = "\033[0m";
const std::string BLACK = "\033[30m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

std::ostream& operator<<(std::ostream& outputStream, const Server& other);
std::ostream& operator<<(std::ostream& outputStream, const HttpRequest& request);

void printServers(std::vector<Server> servers);

#endif
