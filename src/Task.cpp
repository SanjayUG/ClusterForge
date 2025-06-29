#include "ClusterForge.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace ClusterForge {

Task::Task(int id, const std::string& name, const TaskRequirements& reqs)
    : task_id_(id), name_(name), requirements_(reqs), status_(TaskStatus::PENDING),
      priority_(TaskPriority::NORMAL), assigned_node_id_(-1), is_executing_(false) {
    created_time_ = std::chrono::system_clock::now();
}

void Task::setStatus(TaskStatus status) {
    TaskStatus old_status = status_;
    status_ = status;
    
    switch (status) {
        case TaskStatus::RUNNING:
            if (started_time_.time_since_epoch().count() == 0) {
                started_time_ = std::chrono::system_clock::now();
            }
            break;
        case TaskStatus::COMPLETED:
        case TaskStatus::FAILED:
        case TaskStatus::CANCELLED:
            if (completed_time_.time_since_epoch().count() == 0) {
                completed_time_ = std::chrono::system_clock::now();
            }
            break;
        default:
            break;
    }
    
    notifyStatusChange(old_status, status);
}

bool Task::start() {
    if (status_ != TaskStatus::PENDING) {
        return false;
    }
    
    if (assigned_node_id_ == -1) {
        return false;
    }
    
    setStatus(TaskStatus::RUNNING);
    return true;
}

bool Task::complete() {
    if (status_ != TaskStatus::RUNNING) {
        return false;
    }
    
    setStatus(TaskStatus::COMPLETED);
    return true;
}

bool Task::fail(const std::string& error_message) {
    if (status_ == TaskStatus::COMPLETED || status_ == TaskStatus::CANCELLED) {
        return false;
    }
    
    setStatus(TaskStatus::FAILED);
    return true;
}

bool Task::cancel() {
    if (status_ == TaskStatus::COMPLETED || status_ == TaskStatus::FAILED) {
        return false;
    }
    
    setStatus(TaskStatus::CANCELLED);
    return true;
}

void Task::assignToNode(int node_id) {
    int old_node_id = assigned_node_id_;
    assigned_node_id_ = node_id;
    notifyNodeAssignment(old_node_id, node_id);
}

void Task::unassignFromNode() {
    int old_node_id = assigned_node_id_;
    assigned_node_id_ = -1;
    notifyNodeAssignment(old_node_id, -1);
}

void Task::addDependency(int task_id, const std::string& type) {
    // Check if dependency already exists
    auto it = std::find_if(dependencies_.begin(), dependencies_.end(),
                          [task_id](const TaskDependency& dep) {
                              return dep.task_id == task_id;
                          });
    
    if (it == dependencies_.end()) {
        dependencies_.emplace_back(task_id, type);
    }
}

void Task::removeDependency(int task_id) {
    dependencies_.erase(
        std::remove_if(dependencies_.begin(), dependencies_.end(),
                      [task_id](const TaskDependency& dep) {
                          return dep.task_id == task_id;
                      }),
        dependencies_.end()
    );
}

bool Task::areDependenciesMet(const std::vector<int>& completed_tasks) const {
    for (const auto& dep : dependencies_) {
        auto it = std::find(completed_tasks.begin(), completed_tasks.end(), dep.task_id);
        if (it == completed_tasks.end()) {
            return false;
        }
    }
    return true;
}

void Task::addDependentTask(int task_id) {
    auto it = std::find(dependent_tasks_.begin(), dependent_tasks_.end(), task_id);
    if (it == dependent_tasks_.end()) {
        dependent_tasks_.push_back(task_id);
    }
}

bool Task::execute() {
    if (status_ != TaskStatus::RUNNING) {
        return false;
    }
    
    if (is_executing_.exchange(true)) {
        return false; // Already executing
    }
    
    bool success = false;
    
    if (execution_function_) {
        success = execution_function_();
    } else {
        // Default execution simulation (no sleep in single-threaded build)
        success = true;
    }
    
    is_executing_ = false;
    
    if (success) {
        complete();
    } else {
        fail("Execution failed");
    }
    
    return success;
}

std::chrono::milliseconds Task::getExecutionTime() const {
    if (started_time_.time_since_epoch().count() == 0 || 
        completed_time_.time_since_epoch().count() == 0) {
        return std::chrono::milliseconds(0);
    }
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        completed_time_ - started_time_);
}

std::chrono::milliseconds Task::getWaitTime() const {
    if (started_time_.time_since_epoch().count() == 0) {
        return std::chrono::milliseconds(0);
    }
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        started_time_ - created_time_);
}

void Task::setStatusChangeCallback(std::function<void(int, TaskStatus)> callback) {
    status_change_callback_ = callback;
}

void Task::setNodeAssignmentCallback(std::function<void(int, int)> callback) {
    node_assignment_callback_ = callback;
}

std::string Task::getStatusString() const {
    switch (status_) {
        case TaskStatus::PENDING: return "PENDING";
        case TaskStatus::RUNNING: return "RUNNING";
        case TaskStatus::COMPLETED: return "COMPLETED";
        case TaskStatus::FAILED: return "FAILED";
        case TaskStatus::CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

std::string Task::getPriorityString() const {
    switch (priority_) {
        case TaskPriority::LOW: return "LOW";
        case TaskPriority::NORMAL: return "NORMAL";
        case TaskPriority::HIGH: return "HIGH";
        case TaskPriority::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

bool Task::validateRequirements() const {
    return requirements_.cpu_cores > 0 &&
           requirements_.memory_gb > 0.0 &&
           requirements_.disk_gb > 0.0 &&
           requirements_.estimated_duration_ms > 0;
}

double Task::getResourceScore() const {
    // Combined resource requirement score (higher = more resource intensive)
    double cpu_score = static_cast<double>(requirements_.cpu_cores) / 16.0; // Normalize to 16 cores
    double memory_score = requirements_.memory_gb / 32.0; // Normalize to 32GB
    double disk_score = requirements_.disk_gb / 1000.0; // Normalize to 1TB
    
    return (cpu_score + memory_score + disk_score) / 3.0;
}

void Task::notifyStatusChange(TaskStatus old_status, TaskStatus new_status) {
    if (status_change_callback_) {
        status_change_callback_(task_id_, new_status);
    }
}

void Task::notifyNodeAssignment(int old_node_id, int new_node_id) {
    if (node_assignment_callback_) {
        node_assignment_callback_(task_id_, new_node_id);
    }
}

} // namespace ClusterForge 