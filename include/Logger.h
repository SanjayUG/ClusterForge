#pragma once

#include <string>
#include <memory>

namespace ClusterForge {

class Logger {
public:
    Logger();
    ~Logger() = default;
    
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
    void logDebug(const std::string& message);
    
    void setLogLevel(int level) { log_level_ = level; }
    int getLogLevel() const { return log_level_; }
    
private:
    int log_level_;
    std::string getTimestamp();
};

} // namespace ClusterForge 