#pragma once

#include <memory>
#include <vector>

namespace ClusterForge {

class Cluster;
class Task;

class Scheduler {
public:
    explicit Scheduler(std::shared_ptr<Cluster> cluster);
    virtual ~Scheduler() = default;
    
    virtual bool scheduleTask(std::shared_ptr<Task> task) = 0;
    virtual int selectOptimalNode(const TaskRequirements& requirements) = 0;
    
protected:
    std::shared_ptr<Cluster> cluster_;
};

class SimpleScheduler : public Scheduler {
public:
    explicit SimpleScheduler(std::shared_ptr<Cluster> cluster) : Scheduler(cluster) {}
    bool scheduleTask(std::shared_ptr<Task> task) override {
        // Basic scheduling: always return true
        return true;
    }
    int selectOptimalNode(const TaskRequirements& requirements) override {
        // Basic node selection: always return -1 (no node)
        return -1;
    }
};

} // namespace ClusterForge 