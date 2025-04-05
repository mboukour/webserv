#ifndef LOGIN_HPP
#define LOGIN_HPP

#include <map>
#include <string>

#include "../../Http/HttpRequest/HttpRequest.hpp"
#include "../../Http/HttpResponse/HttpResponse.hpp"

#define NAME_MAX_LENGTH 16

class Login {
    private:
        static std::map<std::string, std::string> userCreds; // maybe implemenet timeout mechanism // also maybe true login mechanism
         // key: session-id  // first: username //
        static std::string getLogin(const HttpRequest &request);
        static std::string postLogin(const HttpRequest& request, HttpResponse &response);
        static std::string deleteLogin(const HttpRequest &request);
        static std::string generateUniqueSessionID();
    public:
        static void respondToLogin(const HttpRequest &request, int clientFd);
};





#endif