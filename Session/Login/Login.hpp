#ifndef LOGIN_HPP
#define LOGIN_HPP

#include <map>
#include <string>

#include "../../Http/HttpRequest/HttpRequest.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"

#define NAME_MAX_LENGTH 16

class Login {
    private:
         // key: session-id  // first: username //
        static std::string getLogin(const HttpRequest &request, std::map<std::string, std::string> &userCreds);
        static std::string postLogin(const HttpRequest& request, HttpResponse &response,  std::map<std::string, std::string> &userCreds);
        static std::string deleteLogin(const HttpRequest &request, std::map<std::string, std::string> &userCreds);
        static std::string generateUniqueSessionID(std::map<std::string, std::string> &userCreds);
        
    public:
        static void respondToLogin(const HttpRequest &request, std::map<std::string, std::string> &userCreds, int clientFd);
};





#endif