#ifndef CGI_HPP
#define CGI_HPP

#include "../Http/HttpRequest/HttpRequest.hpp"
// #include "../Http/HttpResponse/HttpResponse.hpp"
#include <string>


#define CGI_TIMEOUT 5
// needs to handle big responses (chunked encoding)
class Cgi {
    private:
        std::string interpreterPath;
        std::string scriptName;
        std::map<std::string, std::string> env;

        std::string cgiResponse;
        char **_env;
        static bool isValidCgiExtension(const std::string &extension);
        static std::string getExtentionPath(const std::string &extension);
        void setCgiNames(const HttpRequest &request);

    public:
        Cgi(const HttpRequest &request);
        ~Cgi();
        char  **swaptoDoublePointer(void);
        void    cleanCgi(void);
        std::string    getCgiResponse(void) const;
};

#endif