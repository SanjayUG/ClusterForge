#include "ClusterForge.h"

namespace ClusterForge {

Scheduler::Scheduler(std::shared_ptr<Cluster> cluster) : cluster_(cluster) {
}

} // namespace ClusterForge 