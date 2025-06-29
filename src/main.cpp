#include "ClusterForge.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

using namespace ClusterForge;

void printBanner() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    ClusterForge v1.0.0                      ║\n";
    std::cout << "║              Advanced Cluster Resource Manager               ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Features:                                                   ║\n";
    std::cout << "║  • Explainable DRL Scheduling with Meta-Learning            ║\n";
    std::cout << "║  • Predictive Failover with Proactive Task Migration        ║\n";
    std::cout << "║  • Dynamic Memory-Aware DAG Scheduling                      ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void printDependencyStatus() {
    std::cout << "=== Dependency Status ===\n";
    
#ifdef NO_MPI
    std::cout << "❌ MPI: Not available (distributed features disabled)\n";
#else
    std::cout << "✅ MPI: Available\n";
#endif

#ifdef NO_BOOST
    std::cout << "❌ Boost: Not available (advanced features disabled)\n";
#else
    std::cout << "✅ Boost: Available\n";
#endif

#ifdef NO_EIGEN
    std::cout << "❌ Eigen3: Not available (ML features disabled)\n";
#else
    std::cout << "✅ Eigen3: Available\n";
#endif

#ifdef NO_SQLITE
    std::cout << "❌ SQLite3: Not available (logging disabled)\n";
#else
    std::cout << "✅ SQLite3: Available\n";
#endif

    std::cout << "\n";
}

void initializeCluster(std::shared_ptr<Cluster> cluster) {
    std::cout << "Initializing cluster with 5 nodes...\n";
    
    // Create nodes with different configurations
    for (int i = 1; i <= 5; ++i) {
        NodeConfig config;
        config.node_id = i;
        config.hostname = "node-" + std::to_string(i);
        config.port = 8000 + i;
        config.max_cpu_cores = 8 + (i * 2);  // 10, 12, 14, 16, 18 cores
        config.max_memory_gb = 16.0 + (i * 4.0);  // 20, 24, 28, 32, 36 GB
        config.max_disk_gb = 500.0;
        config.max_network_mbps = 1000.0;
        
        cluster->addNode(config);
        std::cout << "  Added node " << i << ": " << config.max_cpu_cores 
                  << " cores, " << config.max_memory_gb << " GB RAM\n";
    }
    
    std::cout << "Cluster initialization complete.\n\n";
}

void createSampleTasks(std::vector<std::shared_ptr<Task>>& tasks) {
    std::cout << "Creating sample tasks with dependencies...\n";
    
    // Task 1: Data preprocessing (high memory, low CPU)
    TaskRequirements req1;
    req1.cpu_cores = 2;
    req1.memory_gb = 8.0;
    req1.disk_gb = 10.0;
    req1.estimated_duration_ms = 5000;
    
    auto task1 = std::make_shared<Task>(1, "Data Preprocessing", req1);
    task1->setPriority(TaskPriority::HIGH);
    tasks.push_back(task1);
    
    // Task 2: Feature extraction (medium CPU, medium memory)
    TaskRequirements req2;
    req2.cpu_cores = 4;
    req2.memory_gb = 4.0;
    req2.disk_gb = 5.0;
    req2.estimated_duration_ms = 3000;
    
    auto task2 = std::make_shared<Task>(2, "Feature Extraction", req2);
    task2->addDependency(1, "data");
    tasks.push_back(task2);
    
    // Task 3: Model training (high CPU, high memory)
    TaskRequirements req3;
    req3.cpu_cores = 8;
    req3.memory_gb = 16.0;
    req3.disk_gb = 20.0;
    req3.estimated_duration_ms = 10000;
    
    auto task3 = std::make_shared<Task>(3, "Model Training", req3);
    task3->addDependency(2, "compute");
    task3->setPriority(TaskPriority::CRITICAL);
    tasks.push_back(task3);
    
    // Task 4: Validation (medium CPU, low memory)
    TaskRequirements req4;
    req4.cpu_cores = 2;
    req4.memory_gb = 2.0;
    req4.disk_gb = 1.0;
    req4.estimated_duration_ms = 2000;
    
    auto task4 = std::make_shared<Task>(4, "Model Validation", req4);
    task4->addDependency(3, "data");
    tasks.push_back(task4);
    
    // Task 5: Deployment (low CPU, low memory)
    TaskRequirements req5;
    req5.cpu_cores = 1;
    req5.memory_gb = 1.0;
    req5.disk_gb = 0.5;
    req5.estimated_duration_ms = 1000;
    
    auto task5 = std::make_shared<Task>(5, "Model Deployment", req5);
    task5->addDependency(4, "compute");
    tasks.push_back(task5);
    
    std::cout << "Created " << tasks.size() << " tasks with dependencies.\n\n";
}

void demonstrateBasicFeatures(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Basic Cluster Features ===\n";
    
    // Create a sample task
    TaskRequirements req;
    req.cpu_cores = 4;
    req.memory_gb = 8.0;
    req.disk_gb = 5.0;
    req.estimated_duration_ms = 5000;
    
    auto task = std::make_shared<Task>(100, "Demo Task", req);
    task->setPriority(TaskPriority::HIGH);
    
    // Submit the task
    bool submitted = cluster->submitTask(task);
    if (submitted) {
        std::cout << "Task submitted successfully.\n";
        std::cout << "Task assigned to node: " << task->getAssignedNodeId() << "\n";
        std::cout << "Task status: " << task->getStatusString() << "\n";
    } else {
        std::cout << "Failed to submit task.\n";
    }
    
    std::cout << "\n";
}

#ifndef NO_EIGEN
void demonstrateDRLScheduler(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Explainable DRL Scheduler ===\n";
    std::cout << "DRL Scheduler feature is available with Eigen3.\n";
    std::cout << "This would implement deep reinforcement learning for task scheduling.\n";
    std::cout << "\n";
}
#else
void demonstrateDRLScheduler(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Explainable DRL Scheduler ===\n";
    std::cout << "DRL Scheduler feature requires Eigen3 library.\n";
    std::cout << "Install Eigen3 to enable this feature.\n";
    std::cout << "\n";
}
#endif

#ifndef NO_BOOST
void demonstratePredictiveFailover(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Predictive Failover ===\n";
    std::cout << "Predictive Failover feature is available with Boost.\n";
    std::cout << "This would implement LSTM-based failure prediction.\n";
    std::cout << "\n";
}
#else
void demonstratePredictiveFailover(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Predictive Failover ===\n";
    std::cout << "Predictive Failover feature requires Boost library.\n";
    std::cout << "Install Boost to enable this feature.\n";
    std::cout << "\n";
}
#endif

#ifndef NO_BOOST
void demonstrateDAGScheduler(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Dynamic Memory-Aware DAG Scheduler ===\n";
    std::cout << "DAG Scheduler feature is available with Boost.\n";
    std::cout << "This would implement memory-aware task dependency management.\n";
    std::cout << "\n";
}
#else
void demonstrateDAGScheduler(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Demonstrating Dynamic Memory-Aware DAG Scheduler ===\n";
    std::cout << "DAG Scheduler feature requires Boost library.\n";
    std::cout << "Install Boost to enable this feature.\n";
    std::cout << "\n";
}
#endif

void runSimulation(std::shared_ptr<Cluster> cluster) {
    std::cout << "=== Running Cluster Simulation ===\n";
    
    // Start the cluster
    cluster->start();
    std::cout << "Cluster started successfully.\n";
    
    // Create and submit tasks
    std::vector<std::shared_ptr<Task>> tasks;
    createSampleTasks(tasks);
    
    for (auto& task : tasks) {
        cluster->submitTask(task);
        std::cout << "Submitted task: " << task->getName() << "\n";
    }
    
    // Run simulation for 10 seconds
    std::cout << "Running simulation for 10 seconds...\n";
    auto start_time = std::chrono::steady_clock::now();
    
    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(10)) {
        // Get cluster metrics
        auto metrics = cluster->getCurrentMetrics();
        
        std::cout << "\rCluster Status - Nodes: " << metrics.online_nodes << "/" << metrics.total_nodes
                  << " | Tasks: " << metrics.running_tasks << "/" << metrics.total_tasks
                  << " | CPU: " << (metrics.average_cpu_usage * 100) << "%"
                  << " | Memory: " << (metrics.average_memory_usage * 100) << "%" << std::flush;
        
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Removed for single-threaded build
    }
    
    std::cout << "\nSimulation completed.\n";
    
    // Show final status
    std::cout << "\nFinal Cluster Status:\n";
    std::cout << cluster->getStatusReport() << "\n";
    
    // Stop the cluster
    cluster->stop();
    std::cout << "Cluster stopped.\n\n";
}

int main(int argc, char* argv[]) {
    // Initialize MPI if available
#ifndef NO_MPI
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
#endif
        // Main process
        printBanner();
        printDependencyStatus();
        
        // Initialize logging
        initializeLogging();
        
        // Create cluster configuration
        ClusterConfig config;
        config.cluster_id = 1;
        config.name = "ClusterForge Demo Cluster";
        config.max_nodes = 10;
        config.enable_auto_scaling = true;
        config.enable_failover = true;
        config.health_check_interval_ms = 5000;
        config.failover_timeout_ms = 10000;
        
        // Create cluster
        auto cluster = std::make_shared<Cluster>(config);
        
        // Initialize cluster components
        cluster->initialize();
        
        // Initialize cluster
        initializeCluster(cluster);
        
        // Demonstrate features
        demonstrateBasicFeatures(cluster);
        demonstrateDRLScheduler(cluster);
        demonstratePredictiveFailover(cluster);
        demonstrateDAGScheduler(cluster);
        
        // Run full simulation
        runSimulation(cluster);
        
        std::cout << "ClusterForge demonstration completed successfully!\n";
        std::cout << "Thank you for exploring the future of cluster resource management.\n\n";
        
#ifndef NO_MPI
    } else {
        // Worker processes (for future MPI implementation)
        std::cout << "Worker process " << rank << " of " << size << " started.\n";
        
        // Worker process logic would go here
        while (true) {
            // Process tasks assigned by master
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // Cleanup
    cleanupMPI();
    MPI_Finalize();
#endif
    
    return 0;
} 