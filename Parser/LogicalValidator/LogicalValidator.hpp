#ifndef LOGICALVALIDATOR_HPP
#define LOGICALVALIDATOR_HPP

#include <vector>
#include <string>
#include "../Parser.hpp"

class LogicalValidator{
    private:        
        static bool isValidErrorCode(const std::string &code);
        static bool isValidDirective(const std::string &directive);
        static bool isAcceptedSubBlock(const std::string &directive);
        LogicalValidator();
    public:
        static void checkLogicError(const std::vector<Block> &serverBlocks);
};



#endif