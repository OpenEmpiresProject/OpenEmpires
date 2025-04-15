#include "Logger.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

std::ofstream Logger::logFile;

// Add default constructor as well
Logger::Logger() {
    logFile.open("default_log.txt", std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open default log file." << std::endl;
    }
}

// Constructor with filename parameter

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::logInfo(const std::string& message) {
    log("INFO", message);
}

void Logger::logWarning(const std::string& message) {
    log("WARNING", message);
}

void Logger::logError(const std::string& message) {
    log("ERROR", message);
}

void Logger::log(const std::string& level, const std::string& message) {
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        logFile << std::ctime(&now) << " [" << level << "] " << message << std::endl;
    }
}