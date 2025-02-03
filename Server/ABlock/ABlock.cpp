#include "ABlock.hpp"
#include <exception>
#include <map>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <string>
#include <utility>

ABlock::ABlock(): isGetAccepted(true), isPostAccepted(true), isDeleteAccepted(true), isAutoIndexOn(false), isLimited(false), maxBodySize(0), root(""), uploadStore(""), errorPages(), index() {}

ABlock::ABlock(const ABlock &other)
    : isGetAccepted(other.isGetAccepted),
      isPostAccepted(other.isPostAccepted),
      isDeleteAccepted(other.isDeleteAccepted),
      isAutoIndexOn(other.isAutoIndexOn),
      isLimited(other.isLimited),
      maxBodySize(other.maxBodySize),
      root(other.root),
      uploadStore(other.uploadStore),
      errorPages(other.errorPages),
      index(other.index)
      {}  

ABlock &ABlock::operator=(const ABlock &other) {
    if (this == &other)
        return *this;
    this->isGetAccepted = other.isGetAccepted;
    this->isPostAccepted = other.isPostAccepted;
    this->isDeleteAccepted = other.isDeleteAccepted;
    this->isAutoIndexOn = other.isAutoIndexOn;
    this->isLimited = other.isLimited;
    this->maxBodySize = other.maxBodySize;
    this->root = other.root;
    this->uploadStore = other.uploadStore;
    this->errorPages = other.errorPages;
    this->index = other.index;
    return *this;
}

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

bool ABlock::getIsLimited(void) const {return this->isLimited;}

void ABlock::setIsLimited(bool isLimited) {this->isLimited = isLimited;}

bool ABlock::isMethodAllowed(const std::string &method) const {
    if (method == "POST")
        return (this->isPostAccepted);
    if (method == "GET")
        return (this->isGetAccepted);
    if (method == "DELETE")
        return (this->isDeleteAccepted);
    throw std::runtime_error("Unkown method");
}


void ABlock::setErrorPagePath(const std::string &errorCode, const std::string &errorPage) {
    this->errorPages[errorCode] = errorPage;
}

std::string ABlock::getErrorPageHtml(int errorCode) const {
    std::stringstream ss;
    ss << errorCode;
    std::string errorPath("");
    std::map<std::string, std::string>::const_iterator it = this->errorPages.find(ss.str());
    if (it != this->errorPages.end()) {
        errorPath =  it->second;
    }
    std::ifstream file(errorPath.c_str());
    if (!file.is_open()) {
        return "";
    }
    ss.clear();
    ss << file.rdbuf();
    return ss.str();

}

std::map<std::string, std::string>::const_iterator ABlock::errorPagesCbegin(void) const {
    return this->errorPages.begin();
}

std::map<std::string, std::string>::const_iterator ABlock::errorPagesCend(void) const {
    return this->errorPages.end();
}

void ABlock::addIndex(const std::string &index) {
    this->index.push_back(index);
}

std::vector<std::string>::const_iterator ABlock::indexCbegin(void) const {
    return this->index.begin();
}

std::vector<std::string>::const_iterator ABlock::indexCend(void) const {
    return this->index.end();
}

void ABlock::setCgiInfo(const std::string &language, const std::string &path) {
    this->cgis[language] = path;
}


const std::string &ABlock::getCgiPath(const std::string &language) const { // will throw out_of_range if not found
    return this->cgis.at(language);
}


ABlock::~ABlock() {}