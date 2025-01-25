#include "UnknownMethodException.hpp"

UnknownMethodException::UnknownMethodException(): message("Unkown method") {}

UnknownMethodException::UnknownMethodException(const std::string &method): message("Unkown method: " + method) {}

const char *UnknownMethodException::what() const throw() { return message.c_str(); }
