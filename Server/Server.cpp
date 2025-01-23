#include "Server.hpp"

AcceptedMethods::AcceptedMethods(): isGetAccepted(false), isPostAccepted(false), isDeleteAccepted(false) {};

void AcceptedMethods::setAllMethodsAccepted() {isGetAccepted = true; isPostAccepted = true; isDeleteAccepted = true;}

