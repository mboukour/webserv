#ifndef ABLOCK_HPP
#define ABLOCK_HPP

#include <string>

#include <utility>
#include <vector>
#include <iostream>
#include <map>

enum HttpMethod {
    GET,
    POST,
    DELETE
};

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
        std::vector<std::string> index;
        std::map<std::string, std::string> cgis;
        std::string uploadPath;
    public:
        ABlock &operator=(const ABlock &other);

        std::string getRoot(void) const;
        size_t getMaxBodySize(void) const;

        bool getMethod(int method) const;
        void setRoot(const std::string &root);
        void setMaxBodySize(size_t maxBodySize);

        void setAutoIndex(bool toSet);
        bool getIsAutoIndexOn(void) const;

        bool isMethodAllowed(const std::string& method) const;
        void setMethod(int method, bool toSet);
        void setAllMethodsAsDenied(void);

        bool getIsLimited(void) const;
        void setIsLimited(bool isLimited);

        void setCgiInfo(const std::string &language, const std::string &path);
        const std::string &getCgiPath(const std::string &language) const;
        std::map<std::string, std::string>::const_iterator cgisCbegin(void) const;
        std::map<std::string, std::string>::const_iterator cgisCend(void) const;

        void setErrorPagePath(const std::string &errorCode, const std::string &errorPage);
        std::string getErrorPageHtml(int errorCode) const;
        std::map<std::string, std::string>::const_iterator errorPagesCbegin(void) const;
        std::map<std::string, std::string>::const_iterator errorPagesCend(void) const;

        void addIndex(const std::string &index);
        std::vector<std::string> getIndexes(void) const;
        std::vector<std::string>::const_iterator indexCbegin(void) const;
        std::vector<std::string>::const_iterator indexCend(void) const;

        void setUploadPath(const std::string &uploadPath);
        std::string getUploadPath(void) const;

        virtual void startServer(void) = 0;
        virtual ~ABlock();
};




#endif