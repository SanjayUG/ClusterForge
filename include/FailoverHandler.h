#pragma once

#include <memory>
#include <vector>

namespace ClusterForge {

class Cluster;

class FailoverHandler {
public:
    explicit FailoverHandler(std::shared_ptr<Cluster> cluster);
    ~FailoverHandler() = default;
    
    void handleNodeFailure(int node_id);
    bool migrateTasks(int from_node_id, int to_node_id);
    std::vector<int> getFailoverCandidates(int failed_node_id) const;
    
private:
    std::shared_ptr<Cluster> cluster_;
};

} // namespace ClusterForge 