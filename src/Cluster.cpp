#include "ClusterForge.h"
#include <algorithm>
#include <sstream>

namespace ClusterForge {

Cluster::Cluster(const ClusterConfig& config) 
    : config_(config), is_running_(false) {
    // Don't call initializeComponents() here - it will be called by initialize()
}

Cluster::~Cluster() {
    stop();
    cleanupComponents();
}

void Cluster::initialize() {
    initializeComponents();
}

void Cluster::initializeComponents() {
    scheduler_ = std::make_shared<SimpleScheduler>(shared_from_this());
    health_monitor_ = std::make_shared<HealthMonitor>(shared_from_this());
    failover_handler_ = std::make_shared<FailoverHandler>(shared_from_this());
    logger_ = std::make_shared<Logger>();
}

void Cluster::cleanupComponents() {
    scheduler_.reset();
    health_monitor_.reset();
    failover_handler_.reset();
    logger_.reset();
}

bool Cluster::start() {
    if (is_running_.exchange(true)) {
        return false; // Already running
    }
    logger_->logInfo("Starting ClusterForge cluster: " + config_.name);
    if (health_monitor_) {
        health_monitor_->start();
    }
    logger_->logInfo("Cluster started successfully");
    return true;
}

void Cluster::stop() {
    if (!is_running_.exchange(false)) {
        return; // Not running
    }
    logger_->logInfo("Stopping ClusterForge cluster");
    if (health_monitor_) {
        health_monitor_->stop();
    }
    logger_->logInfo("Cluster stopped");
}

bool Cluster::addNode(const NodeConfig& node_config) {
    if (nodes_.size() >= config_.max_nodes) {
        logger_->logWarning("Cannot add node: maximum nodes reached");
        return false;
    }
    auto node = std::make_shared<Node>(node_config);
    nodes_.push_back(node);
    node_map_[node_config.node_id] = node;
    node->setStatusChangeCallback([this](int node_id, NodeStatus status) {
        this->updateNodeHealth(node_id, status);
    });
    node->setMetricsUpdateCallback([this](int node_id, const ResourceMetrics& metrics) {
        this->updateMetrics();
    });
    logger_->logInfo("Added node: " + node_config.hostname + " (ID: " + std::to_string(node_config.node_id) + ")");
    return true;
}

bool Cluster::removeNode(int node_id) {
    auto it = std::find_if(nodes_.begin(), nodes_.end(),
                          [node_id](const std::shared_ptr<Node>& node) {
                              return node->getId() == node_id;
                          });
    if (it == nodes_.end()) {
        return false;
    }
    nodes_.erase(it);
    node_map_.erase(node_id);
    logger_->logInfo("Removed node: " + std::to_string(node_id));
    return true;
}

std::shared_ptr<Node> Cluster::getNode(int node_id) const {
    auto it = node_map_.find(node_id);
    return (it != node_map_.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Node>> Cluster::getAllNodes() const {
    return nodes_;
}

std::vector<std::shared_ptr<Node>> Cluster::getOnlineNodes() const {
    std::vector<std::shared_ptr<Node>> online_nodes;
    for (const auto& node : nodes_) {
        if (node->getStatus() == NodeStatus::ONLINE) {
            online_nodes.push_back(node);
        }
    }
    return online_nodes;
}

std::vector<std::shared_ptr<Node>> Cluster::getHealthyNodes() const {
    std::vector<std::shared_ptr<Node>> healthy_nodes;
    for (const auto& node : nodes_) {
        if (node->isHealthy()) {
            healthy_nodes.push_back(node);
        }
    }
    return healthy_nodes;
}

bool Cluster::submitTask(std::shared_ptr<Task> task) {
    int best_node_id = findBestNode(task->getRequirements());
    if (best_node_id == -1) {
        logger_->logWarning("No suitable node found for task: " + task->getName());
        return false;
    }
    auto node = getNode(best_node_id);
    if (!node || !node->addTask(task)) {
        logger_->logWarning("Failed to assign task to node: " + std::to_string(best_node_id));
        return false;
    }
    logger_->logInfo("Task submitted: " + task->getName() + " -> Node " + std::to_string(best_node_id));
    return true;
}

bool Cluster::cancelTask(int task_id) {
    for (auto& node : nodes_) {
        if (node->removeTask(task_id)) {
            logger_->logInfo("Task cancelled: " + std::to_string(task_id));
            return true;
        }
    }
    return false;
}

int Cluster::findBestNode(const TaskRequirements& requirements) const {
    int best_node_id = -1;
    double best_score = -1.0;
    for (const auto& node : nodes_) {
        if (!node->isHealthy()) continue;
        if (node->canAcceptTask(std::make_shared<Task>(0, "", requirements))) {
            const auto& metrics = node->getCurrentMetrics();
            double score = 1.0 - (metrics.cpu_usage + metrics.memory_usage) / 2.0;
            if (score > best_score) {
                best_score = score;
                best_node_id = node->getId();
            }
        }
    }
    return best_node_id;
}

void Cluster::updateNodeHealth(int node_id, NodeStatus status) {
    logger_->logInfo("Node " + std::to_string(node_id) + " status changed to: " + 
                    (status == NodeStatus::ONLINE ? "ONLINE" : 
                     status == NodeStatus::OFFLINE ? "OFFLINE" : 
                     status == NodeStatus::DEGRADED ? "DEGRADED" : "FAILED"));
    if (status == NodeStatus::FAILED && config_.enable_failover) {
        handleNodeFailure(node_id);
    }
}

void Cluster::handleNodeFailure(int node_id) {
    logger_->logWarning("Handling failure of node: " + std::to_string(node_id));
    if (failover_handler_) {
        failover_handler_->handleNodeFailure(node_id);
    }
}

bool Cluster::isNodeHealthy(int node_id) const {
    auto node = getNode(node_id);
    return node && node->isHealthy();
}

ClusterMetrics Cluster::getCurrentMetrics() const {
    ClusterMetrics metrics;
    metrics.total_nodes = nodes_.size();
    metrics.timestamp = std::chrono::system_clock::now();
    double total_cpu = 0.0;
    double total_memory = 0.0;
    int online_count = 0;
    int failed_count = 0;
    for (const auto& node : nodes_) {
        if (node->getStatus() == NodeStatus::ONLINE) {
            online_count++;
            const auto& node_metrics = node->getCurrentMetrics();
            total_cpu += node_metrics.cpu_usage;
            total_memory += node_metrics.memory_usage;
        } else if (node->getStatus() == NodeStatus::FAILED) {
            failed_count++;
        }
    }
    metrics.online_nodes = online_count;
    metrics.failed_nodes = failed_count;
    metrics.average_cpu_usage = online_count > 0 ? total_cpu / online_count : 0.0;
    metrics.average_memory_usage = online_count > 0 ? total_memory / online_count : 0.0;
    return metrics;
}

void Cluster::updateMetrics() {
    current_metrics_ = getCurrentMetrics();
    historical_metrics_.push_back(current_metrics_);
    if (historical_metrics_.size() > 100) {
        historical_metrics_.erase(historical_metrics_.begin());
    }
    notifyMetricsUpdate();
}

std::string Cluster::getStatusReport() const {
    std::stringstream ss;
    ss << "Cluster Status Report\n";
    ss << "=====================\n";
    ss << "Name: " << config_.name << "\n";
    ss << "Total Nodes: " << nodes_.size() << "\n";
    auto metrics = getCurrentMetrics();
    ss << "Online Nodes: " << metrics.online_nodes << "\n";
    ss << "Failed Nodes: " << metrics.failed_nodes << "\n";
    ss << "Average CPU Usage: " << (metrics.average_cpu_usage * 100) << "%\n";
    ss << "Average Memory Usage: " << (metrics.average_memory_usage * 100) << "%\n";
    ss << "\nNode Details:\n";
    for (const auto& node : nodes_) {
        ss << "  Node " << node->getId() << " (" << node->getHostname() << "): "
           << node->getStatusString() << "\n";
    }
    return ss.str();
}

void Cluster::clusterManagementLoop() {
    // No-op in single-threaded version
}

void Cluster::notifyMetricsUpdate() {
    if (metrics_update_callback_) {
        metrics_update_callback_(current_metrics_);
    }
}

void Cluster::notifyEvent(int event_type, const std::string& message) {
    if (event_callback_) {
        event_callback_(event_type, message);
    }
}

} // namespace ClusterForge 