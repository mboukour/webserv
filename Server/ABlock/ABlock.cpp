#include "ABlock.hpp"
#include <exception>

ABlock::ABlock(): isGetAccepted(true), isPostAccepted(true), isDeleteAccepted(true), isAutoIndexOn(false), maxBodySize(0), root(""), uploadStore("") {}

ABlock::ABlock(const ABlock &other)
    : isGetAccepted(other.isGetAccepted),
      isPostAccepted(other.isPostAccepted),
      isDeleteAccepted(other.isDeleteAccepted),
      isAutoIndexOn(other.isAutoIndexOn),
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
            return this->isPostAccepted;
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
            this->isPostAccepted = toSet;
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
    this->isPostAccepted = false;
    this->isDeleteAccepted = false;
}

void ABlock::setAutoIndex(bool toSet) {
    this->isAutoIndexOn = toSet;
}

ABlock::~ABlock() {}