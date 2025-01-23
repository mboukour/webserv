#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <unordered_map>
#include <vector>

class AcceptedMethods {
    public:
        bool isGetAccepted;
        bool isPostAccepted;
        bool isDeleteAccepted;

        AcceptedMethods();
        void setAllMethodsAccepted();
};

class Server {
    private:
        int port;
        std::string serverName;
        std::string root;
        std::unordered_map<std::vector<int>, std::string> errorPages;
        size_t maxBodySize;
        AcceptedMethods methods;

};




#endif