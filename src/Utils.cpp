#include "ClusterForge.h"
#include <iomanip>
#include <sstream>
#ifndef NO_BOOST
#include <boost/log/utility/setup/common_attributes.hpp>
#endif

namespace ClusterForge {

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

void initializeLogging() {
#ifndef NO_BOOST
    // Initialize Boost.Log
    boost::log::add_common_attributes();
    
    // Add console sink
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%",
        boost::log::keywords::auto_flush = true
    );
    
    // Add file sink
    boost::log::add_file_log(
        boost::log::keywords::file_name = "clusterforge_%Y%m%d_%H%M%S.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024, // 10 MB
        boost::log::keywords::max_size = 100 * 1024 * 1024,     // 100 MB
        boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] [%ThreadID%] %Message%",
        boost::log::keywords::auto_flush = true
    );
    
    // Set log level
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= boost::log::trivial::info
    );
    
    BOOST_LOG_TRIVIAL(info) << "ClusterForge logging initialized";
#else
    // Fallback: simple std::cout logging
    std::cout << "[INFO] ClusterForge logging initialized (Boost.Log not available)" << std::endl;
#endif
}

void cleanupMPI() {
#ifndef NO_BOOST
    // Cleanup MPI resources
    BOOST_LOG_TRIVIAL(info) << "Cleaning up MPI resources";
#else
    std::cout << "[INFO] Cleaning up MPI resources" << std::endl;
#endif
}

} // namespace ClusterForge 