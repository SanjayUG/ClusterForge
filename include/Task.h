#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>

namespace ClusterForge {

enum class TaskStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED
};

enum class TaskPriority {
    LOW = 1,
    NORMAL = 2,
    HIGH = 3,
    CRITICAL = 4
};

struct TaskRequirements {
    int cpu_cores;
    double memory_gb;
    double disk_gb;
    double network_mbps;
    int estimated_duration_ms;
    
    TaskRequirements() : cpu_cores(1), memory_gb(1.0), disk_gb(1.0), 
                        network_mbps(10.0), estimated_duration_ms(1000) {}
};

struct TaskDependency {
    int task_id;
    std::string dependency_type; // "data", "compute", "resource"
    
    TaskDependency(int id, const std::string& type) : task_id(id), dependency_type(type) {}
};

class Task {
private:
    int task_id_;
    std::string name_;
    std::string description_;
    TaskStatus status_;
    TaskPriority priority_;
    TaskRequirements requirements_;
    
    std::vector<TaskDependency> dependencies_;
    std::vector<int> dependent_tasks_; // Tasks that depend on this one
    
    std::chrono::system_clock::time_point created_time_;
    std::chrono::system_clock::time_point started_time_;
    std::chrono::system_clock::time_point completed_time_;
    
    int assigned_node_id_;
    std::atomic<bool> is_executing_;
    
    // Execution function
    std::function<bool()> execution_function_;
    
    // Callbacks
    std::function<void(int, TaskStatus)> status_change_callback_;
    std::function<void(int, int)> node_assignment_callback_;

public:
    Task(int id, const std::string& name, const TaskRequirements& reqs);
    ~Task() = default;
    
    // Getters
    int getId() const { return task_id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    TaskStatus getStatus() const { return status_; }
    TaskPriority getPriority() const { return priority_; }
    const TaskRequirements& getRequirements() const { return requirements_; }
    int getAssignedNodeId() const { return assigned_node_id_; }
    
    // Setters
    void setDescription(const std::string& desc) { description_ = desc; }
    void setPriority(TaskPriority priority) { priority_ = priority; }
    void setExecutionFunction(std::function<bool()> func) { execution_function_ = func; }
    
    // Status management
    void setStatus(TaskStatus status);
    bool start();
    bool complete();
    bool fail(const std::string& error_message = "");
    bool cancel();
    
    // Node assignment
    void assignToNode(int node_id);
    void unassignFromNode();
    
    // Dependencies
    void addDependency(int task_id, const std::string& type = "data");
    void removeDependency(int task_id);
    const std::vector<TaskDependency>& getDependencies() const { return dependencies_; }
    bool hasDependencies() const { return !dependencies_.empty(); }
    bool areDependenciesMet(const std::vector<int>& completed_tasks) const;
    
    void addDependentTask(int task_id);
    const std::vector<int>& getDependentTasks() const { return dependent_tasks_; }
    
    // Execution
    bool execute();
    bool isExecuting() const { return is_executing_; }
    
    // Timing
    std::chrono::milliseconds getExecutionTime() const;
    std::chrono::milliseconds getWaitTime() const;
    std::chrono::system_clock::time_point getCreatedTime() const { return created_time_; }
    std::chrono::system_clock::time_point getStartedTime() const { return started_time_; }
    std::chrono::system_clock::time_point getCompletedTime() const { return completed_time_; }
    
    // Callbacks
    void setStatusChangeCallback(std::function<void(int, TaskStatus)> callback);
    void setNodeAssignmentCallback(std::function<void(int, int)> callback);
    
    // Utility
    std::string getStatusString() const;
    std::string getPriorityString() const;
    bool isCompleted() const { return status_ == TaskStatus::COMPLETED; }
    bool isFailed() const { return status_ == TaskStatus::FAILED; }
    bool isCancelled() const { return status_ == TaskStatus::CANCELLED; }
    
    // Resource validation
    bool validateRequirements() const;
    double getResourceScore() const; // Combined resource requirement score

private:
    void notifyStatusChange(TaskStatus old_status, TaskStatus new_status);
    void notifyNodeAssignment(int old_node_id, int new_node_id);
};

// Task comparison for priority queues
struct TaskPriorityComparator {
    bool operator()(const std::shared_ptr<Task>& a, const std::shared_ptr<Task>& b) const {
        if (a->getPriority() != b->getPriority()) {
            return static_cast<int>(a->getPriority()) < static_cast<int>(b->getPriority());
        }
        return a->getId() > b->getId(); // FIFO for same priority
    }
};

} // namespace ClusterForge 