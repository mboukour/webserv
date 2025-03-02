#include "Logger.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <iomanip>

std::ofstream Logger::logFile;
bool Logger::initialized = false;

void Logger::init(const std::string& filename) {
    if (!initialized) {
        logFile.open(filename.c_str(), std::ios::out | std::ios::trunc);
        logFile.clear();
        initialized = true;
    }
}

void Logger::log(const std::string& message) {
    if (!initialized) {
        init("webserv.log");
    }
    
    std::time_t now = std::time(NULL);
    std::string timestamp = std::ctime(&now);
    timestamp = timestamp.substr(0, timestamp.size() - 1);

    logFile << "[" << timestamp << "] " << message << std::endl;
    std::cout << "[" << timestamp << "] " << message << std::endl;
    logFile.flush();
}

std::ofstream &Logger::getLogStream(void) {
    return logFile;
}

void Logger::close() {
    if (initialized) {
        logFile.close();
        initialized = false;
    }
}
