#include "ClusterForge.h"
#include <algorithm>
#include <random>
#include <iomanip>
#include <sstream>

namespace ClusterForge {

Node::Node(const NodeConfig& config) 
    : config_(config), status_(NodeStatus::ONLINE), metrics_index_(0), 
      failed_heartbeats_(0), failure_probability_(0.0), is_monitoring_(false) {
    last_heartbeat_ = std::chrono::system_clock::now();
    current_metrics_.timestamp = last_heartbeat_;
    
    // Initialize historical metrics
    for (int i = 0; i < 100; ++i) {
        historical_metrics_[i] = current_metrics_;
    }
}

Node::~Node() {
    stopMonitoring();
}

NodeStatus Node::getStatus() const {
    return status_;
}

const ResourceMetrics& Node::getCurrentMetrics() const {
    return current_metrics_;
}

bool Node::canAcceptTask(const std::shared_ptr<Task>& task) const {
    if (status_ != NodeStatus::ONLINE) {
        return false;
    }
    
    const auto& reqs = task->getRequirements();
    
    // Check CPU availability
    double available_cpu = config_.max_cpu_cores * (1.0 - current_metrics_.cpu_usage);
    if (reqs.cpu_cores > available_cpu) {
        return false;
    }
    
    // Check memory availability
    double available_memory = config_.max_memory_gb * (1.0 - current_metrics_.memory_usage);
    if (reqs.memory_gb > available_memory) {
        return false;
    }
    
    // Check disk availability (simplified)
    if (reqs.disk_gb > config_.max_disk_gb * 0.1) { // Allow 10% of disk
        return false;
    }
    
    return true;
}

bool Node::addTask(std::shared_ptr<Task> task) {
    if (!canAcceptTask(task)) {
        return false;
    }
    
    running_tasks_.push_back(task);
    task->assignToNode(config_.node_id);
    
    // Update metrics based on task requirements
    const auto& reqs = task->getRequirements();
    current_metrics_.cpu_usage += static_cast<double>(reqs.cpu_cores) / config_.max_cpu_cores;
    current_metrics_.memory_usage += reqs.memory_gb / config_.max_memory_gb;
    
    current_metrics_.cpu_usage = std::min(current_metrics_.cpu_usage, 1.0);
    current_metrics_.memory_usage = std::min(current_metrics_.memory_usage, 1.0);
    
    return true;
}

bool Node::removeTask(int task_id) {
    auto it = std::find_if(running_tasks_.begin(), running_tasks_.end(),
                          [task_id](const std::shared_ptr<Task>& task) {
                              return task->getId() == task_id;
                          });
    
    if (it == running_tasks_.end()) {
        return false;
    }
    
    // Update metrics
    const auto& reqs = (*it)->getRequirements();
    current_metrics_.cpu_usage -= static_cast<double>(reqs.cpu_cores) / config_.max_cpu_cores;
    current_metrics_.memory_usage -= reqs.memory_gb / config_.max_memory_gb;
    
    current_metrics_.cpu_usage = std::max(current_metrics_.cpu_usage, 0.0);
    current_metrics_.memory_usage = std::max(current_metrics_.memory_usage, 0.0);
    
    (*it)->unassignFromNode();
    running_tasks_.erase(it);
    
    return true;
}

void Node::updateMetrics(const ResourceMetrics& metrics) {
    // Store in historical buffer
    historical_metrics_[metrics_index_] = current_metrics_;
    metrics_index_ = (metrics_index_ + 1) % 100;
    
    current_metrics_ = metrics;
    current_metrics_.timestamp = std::chrono::system_clock::now();
    
    updateFailureProbability();
    notifyMetricsUpdate();
}

void Node::startMonitoring() {
    is_monitoring_ = true;
    updateHeartbeat();
}

void Node::stopMonitoring() {
    is_monitoring_ = false;
}

bool Node::isHealthy() const {
    auto now = std::chrono::system_clock::now();
    auto time_since_heartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_heartbeat_).count();
    
    return status_ == NodeStatus::ONLINE && 
           time_since_heartbeat < FAILOVER_TIMEOUT &&
           current_metrics_.cpu_usage < CPU_THRESHOLD &&
           current_metrics_.memory_usage < MEMORY_THRESHOLD;
}

void Node::setStatus(NodeStatus status) {
    NodeStatus old_status = status_;
    status_ = status;
    notifyStatusChange(old_status, status);
}

void Node::updateHeartbeat() {
    last_heartbeat_ = std::chrono::system_clock::now();
    failed_heartbeats_ = 0;
}

void Node::setStatusChangeCallback(std::function<void(int, NodeStatus)> callback) {
    status_change_callback_ = callback;
}

void Node::setMetricsUpdateCallback(std::function<void(int, const ResourceMetrics&)> callback) {
    metrics_update_callback_ = callback;
}

std::vector<ResourceMetrics> Node::getHistoricalMetrics(int count) const {
    std::vector<ResourceMetrics> result;
    count = std::min(count, 100);
    
    for (int i = 0; i < count; ++i) {
        int index = (metrics_index_ - i + 100) % 100;
        result.push_back(historical_metrics_[index]);
    }
    
    return result;
}

double Node::getAverageCPUUsage() const {
    auto metrics = getHistoricalMetrics(100);
    if (metrics.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& metric : metrics) {
        sum += metric.cpu_usage;
    }
    return sum / metrics.size();
}

double Node::getAverageMemoryUsage() const {
    auto metrics = getHistoricalMetrics(100);
    if (metrics.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& metric : metrics) {
        sum += metric.memory_usage;
    }
    return sum / metrics.size();
}

std::string Node::getStatusString() const {
    switch (status_) {
        case NodeStatus::ONLINE: return "ONLINE";
        case NodeStatus::OFFLINE: return "OFFLINE";
        case NodeStatus::DEGRADED: return "DEGRADED";
        case NodeStatus::FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

void Node::simulateWorkload(double cpu_load, double memory_load) {
    current_metrics_.cpu_usage = std::min(cpu_load, 1.0);
    current_metrics_.memory_usage = std::min(memory_load, 1.0);
    current_metrics_.timestamp = std::chrono::system_clock::now();
    
    // Simulate some I/O activity
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disk_dist(0.0, 100.0);
    std::uniform_real_distribution<> net_dist(0.0, 50.0);
    
    current_metrics_.disk_io = disk_dist(gen);
    current_metrics_.network_io = net_dist(gen);
    
    updateFailureProbability();
}

void Node::healthMonitoringLoop() {
    // No-op in single-threaded version
}

void Node::updateFailureProbability() {
    // Simple failure probability based on resource usage and health
    double cpu_factor = current_metrics_.cpu_usage > 0.9 ? 0.3 : 0.0;
    double memory_factor = current_metrics_.memory_usage > 0.9 ? 0.3 : 0.0;
    double health_factor = failed_heartbeats_ * 0.1;
    
    failure_probability_ = std::min(1.0, cpu_factor + memory_factor + health_factor);
}

void Node::notifyStatusChange(NodeStatus old_status, NodeStatus new_status) {
    if (status_change_callback_) {
        status_change_callback_(config_.node_id, new_status);
    }
}

void Node::notifyMetricsUpdate() {
    if (metrics_update_callback_) {
        metrics_update_callback_(config_.node_id, current_metrics_);
    }
}

} // namespace ClusterForge