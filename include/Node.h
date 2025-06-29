#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>

namespace ClusterForge {

enum class NodeStatus {
    ONLINE,
    OFFLINE,
    DEGRADED,
    FAILED
};

struct ResourceMetrics {
    double cpu_usage;
    double memory_usage;
    double disk_io;
    double network_io;
    std::chrono::system_clock::time_point timestamp;
    
    ResourceMetrics() : cpu_usage(0.0), memory_usage(0.0), disk_io(0.0), network_io(0.0) {}
};

struct NodeConfig {
    int node_id;
    std::string hostname;
    int port;
    int max_cpu_cores;
    double max_memory_gb;
    double max_disk_gb;
    double max_network_mbps;
    
    NodeConfig() : node_id(0), port(0), max_cpu_cores(0), 
                   max_memory_gb(0.0), max_disk_gb(0.0), max_network_mbps(0.0) {}
};

class Task;

class Node {
private:
    NodeConfig config_;
    NodeStatus status_;
    ResourceMetrics current_metrics_;
    ResourceMetrics historical_metrics_[100]; // Circular buffer
    int metrics_index_;
    
    std::vector<std::shared_ptr<Task>> running_tasks_;
    std::atomic<bool> is_monitoring_;
    
    // Performance tracking
    std::chrono::system_clock::time_point last_heartbeat_;
    int failed_heartbeats_;
    double failure_probability_;
    
    // Callbacks
    std::function<void(int, NodeStatus)> status_change_callback_;
    std::function<void(int, const ResourceMetrics&)> metrics_update_callback_;

public:
    explicit Node(const NodeConfig& config);
    ~Node();
    
    // Getters
    int getId() const { return config_.node_id; }
    const std::string& getHostname() const { return config_.hostname; }
    NodeStatus getStatus() const;
    const ResourceMetrics& getCurrentMetrics() const;
    const NodeConfig& getConfig() const { return config_; }
    
    // Resource management
    bool canAcceptTask(const std::shared_ptr<Task>& task) const;
    bool addTask(std::shared_ptr<Task> task);
    bool removeTask(int task_id);
    void updateMetrics(const ResourceMetrics& metrics);
    
    // Health monitoring
    void startMonitoring();
    void stopMonitoring();
    bool isHealthy() const;
    double getFailureProbability() const { return failure_probability_; }
    
    // Status management
    void setStatus(NodeStatus status);
    void updateHeartbeat();
    int getFailedHeartbeats() const { return failed_heartbeats_; }
    
    // Callbacks
    void setStatusChangeCallback(std::function<void(int, NodeStatus)> callback);
    void setMetricsUpdateCallback(std::function<void(int, const ResourceMetrics&)> callback);
    
    // Performance analysis
    std::vector<ResourceMetrics> getHistoricalMetrics(int count = 100) const;
    double getAverageCPUUsage() const;
    double getAverageMemoryUsage() const;
    
    // Utility
    std::string getStatusString() const;
    void simulateWorkload(double cpu_load, double memory_load);

private:
    void healthMonitoringLoop();
    void updateFailureProbability();
    void notifyStatusChange(NodeStatus old_status, NodeStatus new_status);
    void notifyMetricsUpdate();
};

} // namespace ClusterForge 