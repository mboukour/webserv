#include "Logger.hpp"
#include <ctime>
#include <fstream>
#include <iostream>

std::string Logger::fileName;
std::ofstream Logger::logFile;
bool Logger::initialized = false;

void Logger::init(const std::string& filename) {
    if (!initialized) {
        fileName = filename;
        logFile.open(filename.c_str(), std::ios::out | std::ios::trunc);
        logFile.clear();
        initialized = true;
    }
}

std::ofstream &Logger::getLogStream(void) {
    if (!initialized) {
        init(fileName);
    }
    return logFile;
}

void Logger::close() {
    if (initialized) {
        logFile.close();
        initialized = false;
    }
}
