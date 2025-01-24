#ifndef ABLOCK_HPP
#define ABLOCK_HPP

#include <string>
#include <unordered_map>
#include <vector>


// This is an abstract class that defines functionality different blocks might have
class ABlock {
    protected:
        ABlock();
        bool isGetAccepted;
        bool isPostAccepted;
        bool isDeleteAccepted;
        bool isAutoIndexOn;
        std::string root;
        size_t maxBodySize;
        std::unordered_map<int, std::string> errorPages;
        std::string uploadStore;


    public:
        std::string getRoot(void) const;
        size_t getMaxBodySize(void) const;
        bool getIsGetAccepted(void) const;
        bool getIsPostAccepted(void) const;
        bool getIsDeleteAccepted(void) const;
        bool getIsAutoIndexOn(void) const;
        void setRoot(const std::string &root);
        void setMaxBodySize(size_t maxBodySize);
        virtual void startServer(void) = 0;
        virtual ~ABlock();
};




#endif