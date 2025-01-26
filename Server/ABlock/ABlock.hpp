#ifndef ABLOCK_HPP
#define ABLOCK_HPP

#include <string>
// #include <unordered_map>
#include <vector>
#include <iostream>

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
        bool isPostAllowed;
        bool isDeleteAccepted;
        bool isAutoIndexOn;
        bool isLimited;
        size_t maxBodySize;
        std::string root;
        std::string uploadStore;


    public:
        std::string getRoot(void) const;
        size_t getMaxBodySize(void) const;
        bool getMethod(int method) const;
        bool getIsAutoIndexOn(void) const;
        void setRoot(const std::string &root);
        void setMaxBodySize(size_t maxBodySize);
        void setAllMethodsAsDenied(void);
        void setMethod(int method, bool toSet);
        void setAutoIndex(bool toSet);
        void setIsLimited(bool isLimited);
        bool getIsLimited(void) const;
        bool isMethodAllowed(const std::string& method) const;
        virtual void startServer(void) = 0;
        virtual ~ABlock();
};




#endif