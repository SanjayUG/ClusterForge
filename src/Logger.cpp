#include "ClusterForge.h"
#include <iostream>

namespace ClusterForge {

Logger::Logger() : log_level_(0) {}

void Logger::logInfo(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}
void Logger::logWarning(const std::string& message) {
    std::cout << "[WARNING] " << message << std::endl;
}
void Logger::logError(const std::string& message) {
    std::cout << "[ERROR] " << message << std::endl;
}
void Logger::logDebug(const std::string& message) {
    std::cout << "[DEBUG] " << message << std::endl;
}

} // namespace ClusterForge 