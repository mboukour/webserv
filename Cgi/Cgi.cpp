#include "Cgi.hpp"
#include <iostream>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <sstream>
#include "../Exceptions/HttpErrorException/HttpErrorException.hpp"

std::string Cgi::getCgiResponse(const HttpRequest &request) {
    // Validate CGI extension
    std::string extension = request.getPath().substr(request.getPath().find_last_of('.') + 1);
    if (!isValidCgiExtension(extension)) {
        throw std::logic_error("Invalid CGI extension");
    }
    std::string interpreterPath = getInterpreterPath(extension, request);
    std::string scriptName = getScriptName(request);
    if (access(scriptName.c_str(), F_OK) == -1)
        throw HttpErrorException(NOT_FOUND, request, "The requested script was not found");

    std::map<std::string, std::string> env = createCgiEnv(request);
    char **envp = convertEnvToDoublePointer(env);

    int socket[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket) == -1) {
        cleanupEnv(envp);
        throw std::logic_error("Socketpair failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        cleanupEnv(envp);
        close(socket[0]);
        close(socket[1]);
        throw std::logic_error("Fork failed");
    }

    if (pid == 0) {
        close(socket[0]);
        dup2(socket[1], STDOUT_FILENO);
        close(socket[1]);
        char *argv[3];
        argv[0] = const_cast<char*>(interpreterPath.c_str());
        argv[1] = const_cast<char*>(scriptName.c_str());
        argv[2] = NULL;
        execve(interpreterPath.c_str(), argv, envp);
        cleanupEnv(envp);
        std::cerr << "CGI execution failed\n";
        std::exit(1);
    }


    cleanupEnv(envp);
    close(socket[1]);

    clock_t start = clock();
    while (waitpid(pid, NULL, WNOHANG) != -1) {
        if ((clock() - start) / CLOCKS_PER_SEC > CGI_TIMEOUT) {
            kill(pid, SIGKILL);
            close(socket[0]);
            throw HttpErrorException(GATEWAY_TIMEOUT, request, "The CGI script took too long to respond");
        }
    }

    std::string cgiResponse;
    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(socket[0], buffer, sizeof(buffer), 0)) > 0) {
        cgiResponse.append(buffer, bytesRead);
    }

    if (bytesRead == -1) {
        close(socket[0]);
        throw std::logic_error("Error reading from the socket");
    }

    close(socket[0]);
    return cgiResponse;
}

bool Cgi::isValidCgiExtension(const std::string &extension) {
    return (extension == "php" || extension == "py" || extension == "pl");
}

std::string Cgi::getInterpreterPath(const std::string &extension, const HttpRequest &request) {
    try {
        return request.getRequestBlock()->getCgiPath(extension);
    } catch (const std::out_of_range &) {
        if (extension == "php") return "/usr/bin/php-cgi";
        if (extension == "py") return "/usr/bin/python3";
        if (extension == "pl") return "/usr/bin/perl";
        throw std::logic_error("No interpreter found for this extension");
    }
}

std::string Cgi::getScriptName(const HttpRequest &request) {
    std::string path = request.getPath();
    size_t slashPos = path.find_last_of('/');
    if (slashPos == std::string::npos) {
        throw std::logic_error("Invalid script path");
    }
    return request.getRequestBlock()->getRoot() + path.substr(slashPos + 1);
}

std::map<std::string, std::string> Cgi::createCgiEnv(const HttpRequest &request) {
    std::map<std::string, std::string> env;
    env["REQUEST_METHOD"] = request.getMethod();
    std::stringstream ss;
    ss << request.getBodySize();
    env["CONTENT_LENGTH"] = ss.str();
    env["CONTENT_TYPE"] = request.getHeader("Content-Type");
    env["QUERY_STRING"] = request.getQueryString();
    env["SERVER_PROTOCOL"] = request.getVersion();
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_NAME"] = request.getServer()->getServerName();
    ss.clear();
    ss << request.getServer()->getPort();
    env["SERVER_PORT"] = ss.str();
    return env;
}

char **Cgi::convertEnvToDoublePointer(const std::map<std::string, std::string> &env) {
    char **envp = new char*[env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); it++) {
        std::string entry = it->first + "=" + it->second;
        envp[i] = new char[entry.size() + 1];
        std::strcpy(envp[i], entry.c_str());
        i++;
    }
    envp[i] = NULL;
    return envp;
}

void Cgi::cleanupEnv(char **env) {
    if (env) {
        for (int i = 0; env[i] != NULL; i++) {
            delete[] env[i];
        }
        delete[] env;
    }
}