#pragma once

// Standard library includes
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <random>
#include <string>
#include <fstream>
#include <sstream>

// MPI includes (conditional)
#ifndef NO_MPI
#include <mpi.h>
#endif

// Boost includes (conditional)
#ifndef NO_BOOST
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/filesystem.hpp>
#endif

// Eigen includes (conditional)
#ifndef NO_EIGEN
#include <Eigen/Dense>
#include <Eigen/Core>
#endif

// SQLite includes (conditional)
#ifndef NO_SQLITE
#include <sqlite3.h>
#endif

// OpenMP includes
#ifdef _OPENMP
#include <omp.h>
#endif

// Project includes
#include "Node.h"
#include "Task.h"
#include "Cluster.h"
#include "Scheduler.h"
#include "HealthMonitor.h"
#include "FailoverHandler.h"
#include "Logger.h"

// Conditional includes for advanced features
#ifndef NO_EIGEN
#include "DRLScheduler.h"
#include "PredictiveFailover.h"
#endif

#ifndef NO_BOOST
#include "DAGScheduler.h"
#endif

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

namespace ClusterForge {
    // Global constants
    constexpr int MAX_NODES = 100;
    constexpr int MAX_TASKS = 1000;
    constexpr double CPU_THRESHOLD = 0.8;
    constexpr double MEMORY_THRESHOLD = 0.85;
    constexpr int HEALTH_CHECK_INTERVAL = 5000; // milliseconds
    constexpr int FAILOVER_TIMEOUT = 10000; // milliseconds
    
    // Utility functions
    std::string getCurrentTimestamp();
    void initializeLogging();
    void cleanupMPI();
} 