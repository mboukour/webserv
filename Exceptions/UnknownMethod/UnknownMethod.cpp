#include "UnknownMethod.hpp"

UnknownMethod::UnknownMethod(): message("Unkown method") {}

UnknownMethod::UnknownMethod(const std::string &method): message("Unkown method: " + method) {}

const char *UnknownMethod::what() const throw() { return message.c_str(); }
