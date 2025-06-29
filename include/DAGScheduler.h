#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <unordered_map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <Eigen/Dense>

namespace ClusterForge {

class Node;
class Task;
class Cluster;

struct DAGNode {
    int task_id;
    TaskRequirements requirements;
    std::vector<int> dependencies;
    std::vector<int> dependents;
    double memory_pressure;
    double execution_priority;
    int depth;
    int height;
    
    DAGNode() : task_id(-1), memory_pressure(0.0), execution_priority(0.0), depth(0), height(0) {}
};

struct DAGEdge {
    std::string dependency_type;  // "data", "compute", "resource"
    double data_size_gb;
    double transfer_time_ms;
    double memory_overlap;
    
    DAGEdge() : data_size_gb(0.0), transfer_time_ms(0.0), memory_overlap(0.0) {}
};

struct MemoryProfile {
    double peak_memory_gb;
    double average_memory_gb;
    double memory_variance;
    std::vector<double> memory_timeline;
    double memory_pressure_score;
    
    MemoryProfile() : peak_memory_gb(0.0), average_memory_gb(0.0), 
                      memory_variance(0.0), memory_pressure_score(0.0) {}
};

struct SchedulingDecision {
    int task_id;
    int target_node_id;
    double memory_score;
    double cpu_score;
    double overall_score;
    std::string reasoning;
    std::vector<int> alternative_nodes;
    
    SchedulingDecision() : task_id(-1), target_node_id(-1), memory_score(0.0),
                          cpu_score(0.0), overall_score(0.0) {}
};

// Boost Graph types
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, 
                             DAGNode, DAGEdge> DAGGraph;
typedef boost::graph_traits<DAGGraph>::vertex_descriptor DAGVertex;
typedef boost::graph_traits<DAGGraph>::edge_descriptor DAGEdgeDesc;
typedef boost::graph_traits<DAGGraph>::vertex_iterator DAGVertexIter;
typedef boost::graph_traits<DAGGraph>::edge_iterator DAGEdgeIter;

class DAGAnalyzer {
private:
    DAGGraph dag_;
    std::unordered_map<int, DAGVertex> task_to_vertex_;
    std::unordered_map<DAGVertex, int> vertex_to_task_;
    
    std::vector<int> topological_order_;
    std::unordered_map<int, int> task_depths_;
    std::unordered_map<int, int> task_heights_;
    std::unordered_map<int, MemoryProfile> memory_profiles_;

public:
    DAGAnalyzer();
    
    // Graph construction
    void addTask(int task_id, const TaskRequirements& requirements);
    void addDependency(int from_task, int to_task, const DAGEdge& edge);
    void removeTask(int task_id);
    void clear();
    
    // Graph analysis
    std::vector<int> getTopologicalOrder();
    std::vector<int> getCriticalPath();
    int getTaskDepth(int task_id) const;
    int getTaskHeight(int task_id) const;
    double getTaskPriority(int task_id) const;
    
    // Memory analysis
    MemoryProfile analyzeMemoryUsage(int task_id);
    double calculateMemoryPressure(int task_id);
    std::vector<int> getMemoryCriticalPath();
    double estimatePeakMemory();
    
    // Dependency analysis
    std::vector<int> getDependencies(int task_id) const;
    std::vector<int> getDependents(int task_id) const;
    bool hasCycles() const;
    std::vector<std::vector<int>> getConnectedComponents();
    
    // Utility
    int getVertexCount() const;
    int getEdgeCount() const;
    bool containsTask(int task_id) const;
    std::string getGraphDescription() const;
};

class MemoryAwareScheduler {
private:
    std::shared_ptr<Cluster> cluster_;
    std::shared_ptr<DAGAnalyzer> dag_analyzer_;
    
    std::queue<int> ready_tasks_;
    std::unordered_map<int, SchedulingDecision> scheduling_decisions_;
    std::unordered_map<int, MemoryProfile> node_memory_profiles_;
    
    // Configuration
    double memory_weight_;
    double cpu_weight_;
    double network_weight_;
    bool enable_memory_optimization_;
    bool enable_dynamic_rescheduling_;
    
    // Performance tracking
    std::vector<double> memory_efficiency_scores_;
    std::vector<double> scheduling_times_;
    int total_scheduled_tasks_;
    int memory_optimized_tasks_;

public:
    explicit MemoryAwareScheduler(std::shared_ptr<Cluster> cluster);
    
    // Core scheduling
    SchedulingDecision scheduleTask(int task_id);
    std::vector<SchedulingDecision> scheduleReadyTasks();
    bool executeSchedulingDecision(const SchedulingDecision& decision);
    void updateTaskStatus(int task_id, TaskStatus status);
    
    // Memory-aware optimization
    int selectOptimalNode(int task_id, const std::vector<int>& candidate_nodes);
    double calculateMemoryScore(int task_id, int node_id);
    double calculateCPUScore(int task_id, int node_id);
    double calculateOverallScore(int task_id, int node_id);
    
    // Dynamic rescheduling
    void checkMemoryPressure();
    bool shouldReschedule(int task_id, int current_node);
    std::vector<int> findBetterNodes(int task_id, int current_node);
    bool rescheduleTask(int task_id, int new_node);
    
    // Memory profiling
    void updateNodeMemoryProfile(int node_id, const MemoryProfile& profile);
    MemoryProfile getNodeMemoryProfile(int node_id) const;
    double getClusterMemoryEfficiency() const;
    
    // Configuration
    void setMemoryWeight(double weight) { memory_weight_ = weight; }
    void setCPUWeight(double weight) { cpu_weight_ = weight; }
    void setNetworkWeight(double weight) { network_weight_ = weight; }
    void enableMemoryOptimization(bool enable) { enable_memory_optimization_ = enable; }
    void enableDynamicRescheduling(bool enable) { enable_dynamic_rescheduling_ = enable; }
    
    // Performance monitoring
    double getAverageMemoryEfficiency() const;
    double getAverageSchedulingTime() const;
    int getTotalScheduledTasks() const { return total_scheduled_tasks_; }
    int getMemoryOptimizedTasks() const { return memory_optimized_tasks_; }
    
    // Utility
    std::string getSchedulingReport() const;
    void clearHistory();

private:
    void updateReadyTasks();
    std::vector<int> getCandidateNodes(int task_id);
    double estimateMemoryOverlap(int task_id, int node_id);
    void logSchedulingDecision(const SchedulingDecision& decision);
};

class DAGScheduler {
private:
    std::shared_ptr<Cluster> cluster_;
    std::shared_ptr<DAGAnalyzer> dag_analyzer_;
    std::shared_ptr<MemoryAwareScheduler> memory_scheduler_;
    
    std::vector<std::shared_ptr<Task>> dag_tasks_;
    std::unordered_map<int, std::shared_ptr<Task>> task_map_;
    
    std::queue<int> execution_queue_;
    std::unordered_map<int, TaskStatus> task_statuses_;
    std::vector<int> completed_tasks_;
    
    // Configuration
    bool enable_parallel_execution_;
    bool enable_memory_optimization_;
    int max_parallel_tasks_;
    double memory_threshold_;
    
    // Performance tracking
    std::chrono::system_clock::time_point dag_start_time_;
    std::chrono::system_clock::time_point dag_completion_time_;
    std::vector<double> execution_times_;
    double total_memory_usage_;

public:
    explicit DAGScheduler(std::shared_ptr<Cluster> cluster);
    
    // DAG management
    void addTask(std::shared_ptr<Task> task);
    void addDependency(int from_task, int to_task, const DAGEdge& edge);
    void buildDAG(const std::vector<std::shared_ptr<Task>>& tasks, 
                  const std::vector<std::pair<int, int>>& dependencies);
    void clearDAG();
    
    // Execution management
    bool startExecution();
    void stopExecution();
    bool isExecuting() const;
    void executeNextTask();
    void executeParallelTasks();
    
    // Scheduling
    std::vector<SchedulingDecision> scheduleAllTasks();
    SchedulingDecision scheduleTask(int task_id);
    bool executeSchedulingDecision(const SchedulingDecision& decision);
    
    // Memory optimization
    void optimizeMemoryUsage();
    std::vector<int> getMemoryCriticalTasks();
    double estimatePeakMemoryUsage();
    void redistributeMemoryLoad();
    
    // Monitoring
    double getExecutionProgress() const;
    std::chrono::milliseconds getExecutionTime() const;
    double getMemoryEfficiency() const;
    std::string getExecutionStatus() const;
    
    // Configuration
    void setMaxParallelTasks(int max_tasks) { max_parallel_tasks_ = max_tasks; }
    void setMemoryThreshold(double threshold) { memory_threshold_ = threshold; }
    void enableParallelExecution(bool enable) { enable_parallel_execution_ = enable; }
    void enableMemoryOptimization(bool enable) { enable_memory_optimization_ = enable; }
    
    // Utility
    std::string getDAGDescription() const;
    std::vector<int> getReadyTasks() const;
    std::vector<int> getRunningTasks() const;
    std::vector<int> getCompletedTasks() const { return completed_tasks_; }
    
    // Performance analysis
    double getAverageExecutionTime() const;
    double getTotalMemoryUsage() const { return total_memory_usage_; }
    std::vector<double> getExecutionTimes() const { return execution_times_; }
    std::string getPerformanceReport() const;

private:
    void updateExecutionQueue();
    void updateTaskStatus(int task_id, TaskStatus status);
    bool canExecuteTask(int task_id) const;
    void logExecutionMetrics(int task_id, double execution_time, double memory_usage);
    void handleTaskCompletion(int task_id);
};

} // namespace ClusterForge 