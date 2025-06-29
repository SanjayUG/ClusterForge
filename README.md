# ClusterForge v1.0.0

**Advanced Cluster Resource Manager with Explainable DRL Scheduling, Predictive Failover, and Memory-Aware DAG Optimization**

## 🚀 Overview

ClusterForge is a cutting-edge C++-based Cluster Resource Manager that revolutionizes distributed computing through three innovative features:

### 🧠 **Explainable DRL Scheduling with Meta-Learning**
- **Deep Reinforcement Learning** scheduler that adapts to new clusters via meta-learning
- **Human-readable explanations** for task assignments (unlike opaque DRL systems)
- **Attention mechanisms** and SHAP values for decision transparency
- **Meta-learning** enables rapid adaptation to new cluster configurations

### 🔮 **Predictive Failover with Proactive Task Migration**
- **LSTM-based anomaly detection** to predict node failures before they occur
- **Proactive task migration** instead of reactive failover
- **Time-series analysis** of CPU, memory, I/O, temperature, and power patterns
- **Failure probability scoring** with confidence intervals

### 🧩 **Dynamic Memory-Aware DAG Scheduling**
- **Graph-based task dependency** management using Boost.Graph
- **Memory-aware topological sorting** with resource constraints
- **Dynamic DAG reconstruction** based on real-time resource availability
- **Memory pressure optimization** for complex workflows

## 🏗️ Architecture

```
ClusterForge/
├── Core Components
│   ├── Node Manager          # Individual node representation
│   ├── Task Scheduler        # Job distribution and resource allocation
│   ├── Health Monitor        # Node status checking and failure detection
│   ├── Failover Handler      # Automatic task reassignment
│   └── Performance Logger    # Statistics and execution tracking
│
├── Innovative Features
│   ├── DRL Scheduler         # Explainable deep reinforcement learning
│   ├── Predictive Failover   # LSTM-based failure prediction
│   └── DAG Scheduler         # Memory-aware directed acyclic graph
│
└── Infrastructure
    ├── MPI Communication     # Distributed node communication
    ├── Boost Libraries       # Graph algorithms, logging, async I/O
    ├── Eigen                 # Machine learning operations
    └── SQLite                # Performance logging and persistence
```

## 🛠️ Technical Stack

- **Language**: C++17/20 with modern features
- **Distributed Computing**: MPI (Message Passing Interface)
- **Machine Learning**: Eigen library for neural networks
- **Graph Algorithms**: Boost.Graph for DAG management
- **Async I/O**: Boost.Asio for non-blocking operations
- **Logging**: Boost.Log with structured logging
- **Database**: SQLite for performance metrics
- **Parallel Processing**: OpenMP for multi-threading

## 📋 Prerequisites

### System Requirements
- **OS**: Linux, macOS, or Windows with WSL
- **Compiler**: GCC 8+, Clang 7+, or MSVC 2019+
- **Memory**: 4GB RAM minimum, 8GB recommended
- **Storage**: 2GB free space

### Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install libboost-all-dev libeigen3-dev libsqlite3-dev libopenmpi-dev

# CentOS/RHEL
sudo yum install boost-devel eigen3-devel sqlite-devel openmpi-devel

# macOS
brew install boost eigen sqlite open-mpi

# Windows (vcpkg)
vcpkg install boost eigen3 sqlite3 openmpi
```

## 🔧 Installation

### 1. Clone the Repository
```bash
git clone https://github.com/your-username/ClusterForge.git
cd ClusterForge
```

### 2. Build the Project
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. Run the Demo
```bash
./clusterforge
```

## 🎯 Usage Examples

### Basic Cluster Setup
```cpp
#include "ClusterForge.h"

// Create cluster configuration
ClusterConfig config;
config.cluster_id = 1;
config.name = "My Cluster";
config.max_nodes = 10;
config.enable_auto_scaling = true;
config.enable_failover = true;

// Initialize cluster
auto cluster = std::make_shared<Cluster>(config);

// Add nodes
NodeConfig node_config;
node_config.node_id = 1;
node_config.hostname = "compute-01";
node_config.max_cpu_cores = 16;
node_config.max_memory_gb = 32.0;
cluster->addNode(node_config);

// Start cluster
cluster->start();
```

### DRL Scheduling
```cpp
// Create DRL scheduler
auto drl_scheduler = std::make_shared<DRLScheduler>(cluster);
drl_scheduler->start();

// Submit task with explanation
TaskRequirements req;
req.cpu_cores = 4;
req.memory_gb = 8.0;
auto task = std::make_shared<Task>(1, "ML Training", req);

bool scheduled = drl_scheduler->scheduleTask(task);
if (scheduled) {
    std::string explanation = drl_scheduler->explainDecision(task, task->getAssignedNodeId());
    std::cout << "DRL Explanation: " << explanation << std::endl;
}
```

### Predictive Failover
```cpp
// Initialize predictive failover
auto failover = std::make_shared<PredictiveFailover>(cluster);
failover->start();

// Monitor nodes for anomalies
failover->monitorNodes();

// Get failure predictions
auto predictions = failover->getFailurePredictions();
for (const auto& pred : predictions) {
    if (pred.failure_probability > 0.3) {
        std::cout << "Node " << pred.node_id << " at risk: " 
                  << (pred.failure_probability * 100) << "%" << std::endl;
    }
}
```

### DAG Scheduling
```cpp
// Create DAG scheduler
auto dag_scheduler = std::make_shared<DAGScheduler>(cluster);

// Build task dependencies
std::vector<std::shared_ptr<Task>> tasks;
// ... create tasks ...
std::vector<std::pair<int, int>> dependencies = {{1, 2}, {2, 3}};

dag_scheduler->buildDAG(tasks, dependencies);

// Schedule with memory optimization
auto decisions = dag_scheduler->scheduleAllTasks();
dag_scheduler->optimizeMemoryUsage();
```

## 📊 Performance Metrics

ClusterForge provides comprehensive performance monitoring:

### DRL Scheduler Metrics
- **Decision Accuracy**: Percentage of optimal task assignments
- **Average Reward**: Reinforcement learning performance
- **Training Loss**: Neural network convergence
- **Explanation Quality**: Human interpretability scores

### Predictive Failover Metrics
- **Prediction Accuracy**: True positive rate for failure prediction
- **Migration Success Rate**: Successful task migrations
- **False Positive Rate**: Unnecessary migrations
- **Prediction Horizon**: Time ahead of actual failures

### DAG Scheduler Metrics
- **Memory Efficiency**: Peak memory usage optimization
- **Execution Time**: Total workflow completion time
- **Resource Utilization**: CPU and memory usage patterns
- **Load Balance**: Distribution across cluster nodes

## 🔬 Research Contributions

### Novel Features (2025)
1. **Explainable DRL**: Unlike SLURM/YARN, provides human-readable explanations
2. **Proactive Failover**: Predicts failures instead of reacting to them
3. **Memory-Aware DAG**: Addresses memory optimization gap in existing systems

### Technical Innovations
- **Meta-Learning Integration**: Rapid adaptation to new cluster configurations
- **LSTM Anomaly Detection**: Advanced time-series analysis for failure prediction
- **Dynamic Graph Optimization**: Real-time DAG reconstruction based on resource availability

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

### Development Setup
```bash
# Install development dependencies
sudo apt-get install clang-format cppcheck valgrind

# Run tests
make test

# Code formatting
make format

# Static analysis
make analyze
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Boost Libraries** for robust C++ infrastructure
- **Eigen** for efficient linear algebra operations
- **MPI Forum** for distributed computing standards
- **Research Community** for inspiration and feedback



**ClusterForge** - Shaping the future of cluster resource management through intelligent, explainable, and predictive systems. 🚀 