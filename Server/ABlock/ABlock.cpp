#include "ABlock.hpp"


ABlock::ABlock(): isGetAccepted(true), isPostAccepted(true), isDeleteAccepted(true), isAutoIndexOn(false), maxBodySize(0) {}

std::string ABlock::getRoot(void) const {
    return root;
}

size_t ABlock::getMaxBodySize(void) const {
    return maxBodySize;
}

bool ABlock::getIsGetAccepted(void) const {
    return isGetAccepted;
}

bool ABlock::getIsPostAccepted(void) const {
    return isPostAccepted;
}

bool ABlock::getIsDeleteAccepted(void) const {
    return isDeleteAccepted;
}

bool ABlock::getIsAutoIndexOn(void) const {
    return isAutoIndexOn;
}

// Setter methods
void ABlock::setRoot(const std::string &root) {
    this->root = root;
}

void ABlock::setMaxBodySize(size_t maxBodySize) {
    this->maxBodySize = maxBodySize;
}

ABlock::~ABlock() {}