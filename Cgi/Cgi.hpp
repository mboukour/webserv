#ifndef CGI_HPP
#define CGI_HPP

#include "../Http/HttpRequest/HttpRequest.hpp"
#include "../Http/HttpResponse/HttpResponse.hpp"
#include <string>

class Cgi {
    private:
        int sockets[2];
        int pid;
        int code;
        std::string interpreterPath;
        std::map<std::string, std::string> env;
        char **_env;
        static bool isValidCgiExtension(const std::string &extension);
        static std::string getExtentionPath(const std::string &extension);
        void setInterperterPath(HttpRequest &request);

    public:
        Cgi(HttpRequest &request);
        ~Cgi();
        // void    setEnv();
        char  **swaptoDoublePointer(void);
        void    cleanCgi(void);
        void    execCgi(void);
};

#endif