#pragma once

#include <memory>
#include <atomic>

namespace ClusterForge {

class Cluster;

class HealthMonitor {
public:
    explicit HealthMonitor(std::shared_ptr<Cluster> cluster);
    ~HealthMonitor();
    
    void start();
    void stop();
    bool isRunning() const { return is_running_; }
    
private:
    std::shared_ptr<Cluster> cluster_;
    std::atomic<bool> is_running_;
    
    void monitoringLoop();
};

} // namespace ClusterForge 