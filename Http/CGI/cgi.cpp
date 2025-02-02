#include "Cgi.hpp"
/*
    For the enviroment i need:
        REQUEST_METHOD = (POST, GET, DELETE)
        CONTENT_LENGTH = (size of the body)
        QUERY_STRING = (the query string)
        SCRIPT_NAME = (the path to the script)
        SERVER_PROTOCOL = (HTTP/1.1)
        SERVER_NAME = (the server name)
        SERVER_PORT = (the server port)
        REMOTE_ADDR = (the client ip)
        REMOTE_PORT = (the client port)
        GATEWAY_INTERFACE = (CGI/1.1)
*/


Cgi::Cgi() {

};

Cgi::~Cgi() {

};

void    Cgi::setEnv(HttpResponse &response) {
    this->env["REQUEST_METHOD"] = "POST";
    this->env["CONTENT_LENGTH"] = response.getBody().size();
    this->env["SERVER_PROTOCOL"] = response.getVersion();
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
};

char **Cgi::swapto_double_pointer(void) {
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

