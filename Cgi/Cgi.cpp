#include "Cgi.hpp"
#include <cstddef>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include "../Exceptions/HttpErrorException/HttpErrorException.hpp"


Cgi::Cgi(const HttpRequest &request) {
    this->setCgiNames(request);
    this->env["REQUEST_METHOD"] = request.getMethod();
    this->env["CONTENT_LENGTH"] = request.getBodySize();
    this->env["CONTENT_TYPE"] = request.getHeader("Content-Type");
    this->env["QUERY_STRING"] = request.getQueryString();
    this->env["SERVER_PROTOCOL"] = request.getVersion();
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->env["SERVER_NAME"] = request.getServer()->getServerName();
    std::stringstream ss;
    ss << request.getServer()->getPort();
    this->env["SERVER_PORT"] = ss.str();
    int socket[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) == -1)
        throw std::logic_error("Socketpair failed");
    this->_env = this->swaptoDoublePointer();
    pid_t pid = fork();
    if (pid == -1)
        throw std::logic_error("Fork failed");
    if (pid == 0) {
        close(socket[0]); // parent side of the sockets
        dup2(socket[1], STDOUT_FILENO); // this will need to only write the result
        // close(socket[1]); // for leaks ?
        char *argv[3];
        std::string interpreterPath = this->interpreterPath;
        std::string scriptName = this->scriptName;
        argv[0] = const_cast<char*>(interpreterPath.data());
        argv[1] = const_cast<char*>(scriptName.data());
        argv[2] = NULL;
        execve(this->interpreterPath.c_str(), argv, this->_env);
        cleanCgi();
        close(socket[1]);
        std::cerr << "CGI did not work\n";
        std::exit(1);
    }
    
    cleanCgi();
    close(socket[1]);
    clock_t start = clock();

    while (waitpid(pid, NULL, WNOHANG) != -1) {
        if ((clock() - start) / CLOCKS_PER_SEC > CGI_TIMEOUT)
        {
            kill(pid, SIGKILL);
            close(socket[0]);
            throw HttpErrorException(request.getVersion() ,504, "Gateway Timeout", "The CGI script took too long to respond", request.getRequestBlock()->getErrorPageHtml(504));
        }
    } // wait for the child to finish or timeout, increase the timeout in case of chunked encoding??????
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(socket[0], buffer, 4096, 0)) > 0) {
        this->cgiResponse.append(buffer, bytesRead);
    }
    if (bytesRead == -1)
        throw std::logic_error("Error reading from the socket");
    close(socket[0]);
};

std::string Cgi::getCgiResponse() const {
    return this->cgiResponse;
}

void Cgi::setCgiNames(const HttpRequest &request) {

    std::string extension = request.getPath();
    size_t pos = extension.find_last_of('.');
    size_t slashPos = extension.find_last_of('/');
    if (pos == std::string::npos || slashPos == std::string::npos)
        throw std::logic_error("No valid file extension found"); // this should never happen since we check for valid extensions before using the cgi class, but just in case
    this->scriptName = request.getRequestBlock()->getRoot() + extension.substr(slashPos + 1);
    extension = extension.substr(pos + 1);
    try {
    
         this->interpreterPath = request.getRequestBlock()->getCgiPath(extension);
    } catch (const std::out_of_range& exec) {    
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
};

bool Cgi::isValidCgiExtension(const std::string &extension) { // add more if we need to
    return (extension == "php" || extension == "py" || extension == "pl");
};

char **Cgi::swaptoDoublePointer() {
    try {
        _env = new char*[env.size() + 1];
    } catch (const std::bad_alloc &) {
        throw std::logic_error("Memory allocation failed for CGI environment");
    }
    int i = 0;
    try {
        for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it) {
            std::string entry = it->first + "=" + it->second;
            _env[i] = new char[entry.size() + 1];
            std::strcpy(_env[i], entry.c_str());
            i++;
        }
        _env[i] = NULL;
    } catch (const std::bad_alloc &) {
        cleanCgi();
        throw std::logic_error("Memory allocation failed for CGI environment variables");
    }
    return _env;
}


void Cgi::cleanCgi(void) {
    if (_env) {
        for (int i = 0; _env[i] != NULL; i++)
            delete[] _env[i];
        delete[] _env;
        _env = NULL;
    }
}

