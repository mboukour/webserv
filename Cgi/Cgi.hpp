#ifndef CGI_HPP
#define CGI_HPP

#include "../Http/HttpRequest/HttpRequest.hpp"


#include <string>
#include <map>

#define CGI_TIMEOUT 2
class CgiState;
class Cgi {
    private:
        Cgi();

        static std::string getInterpreterPath(std::string extension, const HttpRequest &request);
        static std::pair<std::string, std::string> getNamePair(const HttpRequest &request);
        static std::map<std::string, std::string> createCgiEnv(const HttpRequest &request, const std::string &scriptName, const std::string &pathInfo);
        static char **convertEnvToDoublePointer(const std::map<std::string, std::string> &env);
        static void cleanupEnv(char **env);
    public:
        static bool isValidCgiExtension(const std::string &extension, const HttpRequest &request);
        static CgiState *initCgi(const HttpRequest &request, int clientFd, int epollFd);
        static CgiState *fileCgi(const std::string &filePath, int clientFd, int epollFd);
};

#endif