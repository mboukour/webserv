#include "Cgi.hpp"
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
/*
    For the enviroment i need:
        REQUEST_METHOD = (POST, GET, DELETE)
        CONTENT_LENGTH = (size of the body)
        QUERY_STRING = (the query string)
        SCRIPT_NAME = (the path to the script)
        SERVER_PROTOCOL = (HTTP/1.1)
        SERVER_NAME = (the server name)
        SERVER_PORT = (the server port)
        GATEWAY_INTERFACE = (CGI/1.1)
*/


Cgi::Cgi(HttpRequest &request) {
    this->setInterperterPath(request);
    this->env["REQUEST_METHOD"] = "POST";
    this->env["CONTENT_LENGTH"] = request.getBodySize();
    this->env["CONTENT_TYPE"] = request.getHeader("Content-Type");
    this->env["QUERY_STRING"] = request.getQueryString();
    this->env["SERVER_PROTOCOL"] = request.getVersion();
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->env["SERVER_NAME"] = request.getServer()->getServerName();
    std::stringstream ss;
    ss << request.getServer()->getPort();
    this->env["SERVER_PORT"] = ss.str();

};

void Cgi::setInterperterPath(HttpRequest &request) {

    std::string extension = request.getPath();
    size_t pos = extension.find_last_of('.');
    if (pos == std::string::npos)
        throw std::logic_error("No file extension found");
    extension = extension.substr(pos + 1);
    this->interpreterPath = request.getRequestBlock()->getCgiPath(extension);
    if (this->interpreterPath.empty()) // no cgi path was set for this extension, whe have to find it on our own
    {
        if (extension == "php") // might implement a better way to find the interpreter using the PATH env variable
            this->interpreterPath = "/usr/bin/php-cgi";
        else if (extension == "py")
            this->interpreterPath = "/usr/bin/python3";
        else if (extension == "pl")
            this->interpreterPath = "/usr/bin/perl";
        else // this should never happen since we check for valid extensions before using the cgi class, but just in case
            throw std::logic_error("No interpreter found for this extension");
        }
    // check if the interpreter exists and is executable using fstat
    struct stat buf;
    if (stat(this->interpreterPath.c_str(), &buf) == -1)
        throw std::logic_error("Interpreter not found");    
    if (!(buf.st_mode & S_IXUSR))
        throw std::logic_error("Interpreter is not executable");
}




Cgi::~Cgi() {
    if (_env) {
        for (int i = 0; _env[i] != NULL; i++)
            delete[] _env[i];
        delete[] _env;
        _env = NULL;
    }
};

bool Cgi::isValidCgiExtension(const std::string &extension) { // add more if we need to
    return (extension == "php" || extension == "py" || extension == "pl");
};


char **Cgi::swaptoDoublePointer(void) {
    char **env = new char*[this->env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = this->env.begin(); it != this->env.end(); it++) {
        env[i] = new char[it->first.size() + it->second.size() + 2];
        strcpy(env[i], (it->first + "=" + it->second).c_str());
        i++;
    }
    env[i] = NULL;
    _env = env;
    return (env);
};

void Cgi::cleanCgi(void) {
    if (_env) {
        for (int i = 0; _env[i] != NULL; i++)
            delete[] _env[i];
        delete[] _env;
        _env = NULL;
    }
}

