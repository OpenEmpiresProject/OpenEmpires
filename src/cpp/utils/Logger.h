#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
public:
    Logger();
    Logger(const std::string& filename);
    ~Logger();

    static void logInfo(const std::string& message);
    static void logWarning(const std::string& message);
    static void logError(const std::string& message);

private:
    static void log(const std::string& level, const std::string& message);
    static std::ofstream logFile;
};

#endif // LOGGER_H