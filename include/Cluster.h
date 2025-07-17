#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <string>
#include <thread>
#include <mutex>
#ifndef NO_BOOST
#include <boost/thread.hpp>
#endif

namespace ClusterForge {

class Node;
class Task;
class Scheduler;
class HealthMonitor;
class FailoverHandler;
class Logger;

struct ClusterConfig {
    int cluster_id;
    std::string name;
    int max_nodes;
    bool enable_auto_scaling;
    bool enable_failover;
    int health_check_interval_ms;
    int failover_timeout_ms;
    
    ClusterConfig() : cluster_id(0), max_nodes(100), enable_auto_scaling(true),
                      enable_failover(true), health_check_interval_ms(5000), 
                      failover_timeout_ms(10000) {}
};

struct ClusterMetrics {
    int total_nodes;
    int online_nodes;
    int failed_nodes;
    int total_tasks;
    int running_tasks;
    int completed_tasks;
    int failed_tasks;
    double average_cpu_usage;
    double average_memory_usage;
    std::chrono::system_clock::time_point timestamp;
    
    ClusterMetrics() : total_nodes(0), online_nodes(0), failed_nodes(0),
                       total_tasks(0), running_tasks(0), completed_tasks(0),
                       failed_tasks(0), average_cpu_usage(0.0), average_memory_usage(0.0) {}
};

class Cluster : public std::enable_shared_from_this<Cluster> {
private:
    ClusterConfig config_;
    std::vector<std::shared_ptr<Node>> nodes_;
    std::unordered_map<int, std::shared_ptr<Node>> node_map_;
    
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<HealthMonitor> health_monitor_;
    std::shared_ptr<FailoverHandler> failover_handler_;
    std::shared_ptr<Logger> logger_;
    
    std::atomic<bool> is_running_;
    
    // Statistics
    ClusterMetrics current_metrics_;
    std::vector<ClusterMetrics> historical_metrics_;
    
    // Callbacks
    std::function<void(const ClusterMetrics&)> metrics_update_callback_;
    std::function<void(int, const std::string&)> event_callback_;

#ifndef NO_BOOST
    std::vector<boost::thread> task_threads_;
    boost::mutex task_mutex_;
#endif
    
public:
    explicit Cluster(const ClusterConfig& config);
    ~Cluster();
    
    // Initialization
    void initialize();
    
    // Cluster management
    bool start();
    void stop();
    bool isRunning() const { return is_running_; }
    
    // Node management
    bool addNode(const NodeConfig& node_config);
    bool removeNode(int node_id);
    std::shared_ptr<Node> getNode(int node_id) const;
    std::vector<std::shared_ptr<Node>> getAllNodes() const;
    std::vector<std::shared_ptr<Node>> getOnlineNodes() const;
    std::vector<std::shared_ptr<Node>> getHealthyNodes() const;
    
    // Task management
    bool submitTask(std::shared_ptr<Task> task);
    bool cancelTask(int task_id);
    std::shared_ptr<Task> getTask(int task_id) const;
    std::vector<std::shared_ptr<Task>> getAllTasks() const;
    std::vector<std::shared_ptr<Task>> getRunningTasks() const;
    std::vector<std::shared_ptr<Task>> getCompletedTasks() const;
    
    // Resource management
    bool allocateResources(int node_id, const TaskRequirements& requirements);
    void deallocateResources(int node_id, const TaskRequirements& requirements);
    double getClusterUtilization() const;
    std::vector<int> getAvailableNodes(const TaskRequirements& requirements) const;
    
    // Load balancing
    int findBestNode(const TaskRequirements& requirements) const;
    void rebalanceLoad();
    double getLoadBalanceScore() const;
    
    // Health monitoring
    void updateNodeHealth(int node_id, NodeStatus status);
    void handleNodeFailure(int node_id);
    bool isNodeHealthy(int node_id) const;
    
    // Failover
    void initiateFailover(int failed_node_id);
    bool migrateTasks(int from_node_id, int to_node_id);
    std::vector<int> getFailoverCandidates(int failed_node_id) const;
    
    // Metrics and monitoring
    ClusterMetrics getCurrentMetrics() const;
    std::vector<ClusterMetrics> getHistoricalMetrics(int count = 100) const;
    void updateMetrics();
    
    // Configuration
    const ClusterConfig& getConfig() const { return config_; }
    void updateConfig(const ClusterConfig& config);
    
    // Callbacks
    void setMetricsUpdateCallback(std::function<void(const ClusterMetrics&)> callback);
    void setEventCallback(std::function<void(int, const std::string&)> callback);
    
    // Utility
    std::string getStatusReport() const;
    void simulateWorkload(double cpu_load, double memory_load);
    void clearCompletedTasks();
    
    // Component access
    std::shared_ptr<Scheduler> getScheduler() const { return scheduler_; }
    std::shared_ptr<HealthMonitor> getHealthMonitor() const { return health_monitor_; }
    std::shared_ptr<FailoverHandler> getFailoverHandler() const { return failover_handler_; }
    std::shared_ptr<Logger> getLogger() const { return logger_; }

private:
    void clusterManagementLoop();
    void initializeComponents();
    void cleanupComponents();
    void notifyMetricsUpdate();
    void notifyEvent(int event_type, const std::string& message);
    void updateNodeMap();
    void validateClusterState();
    void executeTask(std::shared_ptr<Task> task);
};

} // namespace ClusterForge 