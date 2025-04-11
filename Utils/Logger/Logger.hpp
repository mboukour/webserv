#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>

class Logger {
private:
    static std::string fileName;
    static std::ofstream logFile;
    static bool initialized;

public:
    static void init(const std::string& filename);
    static std::ofstream &getLogStream(void);
    static void close();
};

#endif
