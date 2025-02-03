#ifndef CGI_HPP
#define CGI_HPP

#include "../Http/HttpRequest/HttpRequest.hpp"
#include <string>
#include <map>

#define CGI_TIMEOUT 5

class Cgi {
    private:
        Cgi();

        static bool isValidCgiExtension(const std::string &extension);
        static std::string getInterpreterPath(const std::string &extension, const HttpRequest &request);
        static std::string getScriptName(const HttpRequest &request);
        static std::map<std::string, std::string> createCgiEnv(const HttpRequest &request);
        static char **convertEnvToDoublePointer(const std::map<std::string, std::string> &env);
        static void cleanupEnv(char **env);

    public:
        static std::string getCgiResponse(const HttpRequest &request);

};

#endif