#include "ClusterForge.h"

namespace ClusterForge {

HealthMonitor::HealthMonitor(std::shared_ptr<Cluster> cluster) : cluster_(cluster), is_running_(false) {}
HealthMonitor::~HealthMonitor() {}
void HealthMonitor::start() { is_running_ = true; }
void HealthMonitor::stop() { is_running_ = false; }

} // namespace ClusterForge 