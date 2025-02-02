#ifndef CGI_HPP
#define CGI_HPP

#include "../../Http/HttpRequest/HttpRequest.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"

class Cgi {
    private:
        int sockets[2];
        int pid;
        int code;
        std::string interpreter;
        std::map<std::string, std::string> env;
        char **_env;
    public:
        Cgi();
        ~Cgi();
        void    setEnv(HttpResponse &response);
        char  **swapto_double_pointer(void);
        void    cleanCgi(void);
        void    execCgi(void);
};

#endif