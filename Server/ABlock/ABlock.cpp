#include "ABlock.hpp"
#include <exception>
#include "../../Exceptions/UnknownMethodException/UnknownMethodException.hpp"

ABlock::ABlock(): isGetAccepted(true), isPostaccepted(true), isDeleteAccepted(true), isAutoIndexOn(false), isLimited(false), maxBodySize(0), root(""), uploadStore("") {}

ABlock::ABlock(const ABlock &other)
    : isGetAccepted(other.isGetAccepted),
      isPostaccepted(other.isPostaccepted),
      isDeleteAccepted(other.isDeleteAccepted),
      isAutoIndexOn(other.isAutoIndexOn),
      isLimited(other.isLimited),
      maxBodySize(other.maxBodySize),
      root(other.root),
      uploadStore(other.uploadStore) {}

std::string ABlock::getRoot(void) const {
    return root;
}

size_t ABlock::getMaxBodySize(void) const {
    return maxBodySize;
}


// Getter methods
bool ABlock::getMethod(int method) const {
    switch (method) {
        case GET:
            return this->isGetAccepted;
        case POST:
            return this->isPostaccepted;
        case DELETE:
            return this->isDeleteAccepted;
        default:
            return false; 
    }
}

bool ABlock::getIsAutoIndexOn(void) const {
    return this->isAutoIndexOn;
}

// Setter methods
void ABlock::setRoot(const std::string &root) {
    this->root = root;
}

void ABlock::setMaxBodySize(size_t maxBodySize) {
    this->maxBodySize = maxBodySize;
}


void ABlock::setMethod(int method, bool toSet) {
    switch (method) {
        case GET:
            this->isGetAccepted = toSet;
            break;
        case POST:
            this->isPostaccepted = toSet;
            break;
        case DELETE:
            this->isDeleteAccepted = toSet;
            break;
        default:
            throw std::logic_error("Wrong method received, please refer to the HttpMethods enum");
            break;
    }
}


void ABlock::setAllMethodsAsDenied(void) {
    this->isGetAccepted = false;
    this->isPostaccepted = false;
    this->isDeleteAccepted = false;
}

void ABlock::setAutoIndex(bool toSet) {
    this->isAutoIndexOn = toSet;
}

bool ABlock::getIsLimited(void) const {return this->isLimited;}

void ABlock::setIsLimited(bool isLimited) {this->isLimited = isLimited;}

bool ABlock::isMethodAllowed(const std::string &method) const {
    if (method == "POST")
        return (this->isPostaccepted);
    if (method == "GET")
        return (this->isGetAccepted);
    if (method == "DELETE")
        return (this->isDeleteAccepted);
    throw UnknownMethodException(method);
}


ABlock::~ABlock() {}