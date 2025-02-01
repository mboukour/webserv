#ifndef ABLOCK_HPP
#define ABLOCK_HPP

#include <string>
// #include <unordered_map>
#include <vector>
#include <iostream>
#include <map>

enum HttpMethod {
    GET,
    POST,
    DELETE
};

// This is an abstract class that defines functionality different blocks might have
class ABlock {
    protected:
        ABlock();
        ABlock(const ABlock& other);
        bool isGetAccepted;
        bool isPostAccepted;
        bool isDeleteAccepted;
        bool isAutoIndexOn;
        bool isLimited;
        size_t maxBodySize;
        std::string root;
        std::string uploadStore;
        std::map<std::string, std::string> errorPages;

    public:
        std::string getRoot(void) const;
        size_t getMaxBodySize(void) const;
        ABlock& operator=(const ABlock& other);
        bool getMethod(int method) const;
        bool getIsAutoIndexOn(void) const;
        void setRoot(const std::string &root);
        void setMaxBodySize(size_t maxBodySize);
        void setAllMethodsAsDenied(void);
        void setMethod(int method, bool toSet);
        void setAutoIndex(bool toSet);
        void setIsLimited(bool isLimited);
        bool getIsLimited(void) const;
        void setErrorPagePath(const std::string &errorCode, const std::string &errorPage);
        std::string getErrorPageHtml(int errorCode) const;
        bool isMethodAllowed(const std::string& method) const;
        std::map<std::string, std::string>::const_iterator errorPagesCbegin(void) const;
        std::map<std::string, std::string>::const_iterator errorPagesCend(void) const;

        virtual void startServer(void) = 0;
        virtual ~ABlock();
};




#endif