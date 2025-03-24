#include "AllUtils.hpp"



void AllUtils::removeLeadingSpaces(std::string &str) {
    size_t firstNonSpace = str.find_first_not_of(" \t");
    if (firstNonSpace != std::string::npos) 
        str.erase(0, firstNonSpace);
}