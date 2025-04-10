#include "Cgi.hpp"
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <sstream>
#include <utility>
#include "../Exceptions/HttpErrorException/HttpErrorException.hpp"
#include "CgiState/CgiState.hpp"
#include "../Server/ServerManager/ServerManager.hpp"


CgiState* Cgi::initCgi(const HttpRequest &request, int clientFd, int epollFd) {
    std::pair<std::string, std::string> namePair = getNamePair(request);
    std::string extension = namePair.first.substr(namePair.first.find_last_of('.') + 1);
    std::string interpreterPath = getInterpreterPath(extension, request);
    std::string toExecute = request.getRequestBlock()->getRoot() + namePair.first;
    std::map<std::string, std::string> env = createCgiEnv(request, namePair.first, namePair.second);
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
        dup2(socket[1], STDIN_FILENO);
        close(socket[1]);
        char *argv[3];
        argv[0] = const_cast<char*>(interpreterPath.c_str());
        argv[1] = const_cast<char*>(toExecute.c_str());
        argv[2] = NULL;
        execve(interpreterPath.c_str(), argv, envp);
        cleanupEnv(envp);
        std::cerr << "CGI execution failed\n";
        std::exit(1);
    }


    cleanupEnv(envp);
    close(socket[1]);
    fcntl(socket[0], F_SETFL, O_NONBLOCK);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    EpollEvent *event = new EpollEvent(socket[0], pid , epollFd, ServerManager::getClientState(clientFd));
    ev.data.ptr = event;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, socket[0], &ev);
    ServerManager::registerEpollEvent(socket[0], event);
    return event->getCgiState();
}

bool Cgi::isValidCgiExtension(const std::string &extension, const HttpRequest &request) {
    try {
        request.getRequestBlock()->getCgiPath(extension);
        return true;
    } catch (const std::out_of_range &) {
        if (extension == "py" || extension == "pl" || extension == "js" || extension == "sh" || extension == "rb" ||
            extension == "java" || extension == "go" || extension == "bash" ||
            extension == "lua" || extension == "php" || extension == "php7" || extension == "php8") {
                std::string interpreterPath = getInterpreterPath(extension, request);
                if (access(interpreterPath.c_str(), X_OK) == 0)
                    return true;
                else
                    return false;
            }
        return false;
    }
}



std::string Cgi::getInterpreterPath(std::string extension, const HttpRequest &request) {
    if (extension[extension.size() - 1] == '/')
        extension = extension.substr(0, extension.size() - 1);
    try {
        return request.getRequestBlock()->getCgiPath(extension);
    } catch (const std::out_of_range &) {
        std::string lookFor;
        if (extension == "py") lookFor = "/usr/bin/python3";
        else if (extension == "pl") lookFor = "/usr/bin/perl";
        else if (extension == "js") lookFor = "/usr/bin/node";
        else if (extension == "rb") lookFor = "/usr/bin/ruby";
        else if (extension == "java") lookFor = "/usr/bin/java";
        else if (extension == "go") lookFor = "/usr/bin/go";
        else if (extension == "bash") lookFor = "/bin/bash";
        else if (extension == "sh") lookFor = "/bin/sh";
        else if (extension == "lua") lookFor = "/usr/bin/lua";
        else if (extension == "php" || extension == "php7" || extension == "php8") lookFor = "/usr/bin/php-cgi";
        else throw HttpErrorException(NOT_IMPLEMENTED, request , "The requested CGI extension is not supported: " + extension);
        return lookFor;
    }
}

std::pair<std::string, std::string> Cgi::getNamePair(const HttpRequest &request) {
    std::stringstream ss(request.getPath());
    std::string scriptName;
    std::string pathInfo;
    std::string word;
    const std::string &root = request.getRequestBlock()->getRoot();
    while(getline(ss, word, '/')) {
        word += "/";
        scriptName += word;
        if (word.find('.') == std::string::npos) {
            continue;
        }

        std::string toCheck = (root + scriptName);
        toCheck = toCheck.substr(0, toCheck.size() - 1);
        if (access(toCheck.c_str(), F_OK) != -1) {
            scriptName = scriptName.substr(0, scriptName.size() - 1);
            pathInfo = request.getPath().substr(scriptName.size());
            break;
        }
    }
    std::pair<std::string, std::string> res(scriptName, pathInfo);
    return res;
}

std::map<std::string, std::string> Cgi::createCgiEnv(const HttpRequest &request, const std::string &scriptName, const std::string &pathInfo) {
    std::map<std::string, std::string> env;
    env["SERVER_SOFTWARE"] = "Webserv/1.0";

    std::string serverName(request.getServer()->getServerName());
    if (serverName.empty())
    {
        serverName = request.getHeader("Host");
        size_t pos = serverName.find(':');
        if (pos != std::string::npos) {
            serverName = serverName.substr(0, pos);
        }
    }
    if (!serverName.empty())
        env["SERVER_NAME"] = serverName;

    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = request.getVersion();
    env["REQUEST_METHOD"] = request.getMethod();
    env["PATH_INFO"] = pathInfo;

    std::string rootSlash = request.getRequestBlock()->getRoot();
    if (rootSlash[rootSlash.size() - 1] == '/')
        rootSlash = rootSlash.substr(0, rootSlash.size() - 2);
    env["PATH_TRANSLATED"] =  rootSlash + scriptName;
    env["SCRIPT_NAME"] = scriptName;

    env["QUERY_STRING"] = request.getQueryString();
    const std::string &contentType = request.getHeader("Content-Type");
    if (!contentType.empty())
        env["CONTENT_TYPE"] = request.getHeader("Content-Type");
    std::stringstream ss;
    ss << request.getContentLength();
    env["CONTENT_LENGTH"] = ss.str();

    env["HTTP_HOST"] = request.getHeader("Host");
    env["HTTP_USER_AGENT"] = request.getHeader("User-Agent");
    env["HTTP_ACCEPT"] = request.getHeader("Accept");
    env["HTTP_ACCEPT_LANGUAGE"] = request.getHeader("Accept-Language");
    env["HTTP_ACCEPT_ENCODING"] = request.getHeader("Accept-Encoding");
    env["HTTP_CONNECTION"] = request.getHeader("Connection");
    env["HTTP_REFERER"] = request.getHeader("Referer");
    env["HTTP_COOKIE"] = request.getHeader("Cookie");


    ss.clear();
    ss.str("");
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
