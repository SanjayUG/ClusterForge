cmake_minimum_required(VERSION 3.16)
project(ClusterForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages (make some optional for initial build)
find_package(MPI QUIET)
if(NOT MPI_FOUND)
    message(WARNING "MPI not found - distributed features will be disabled")
    add_definitions(-DNO_MPI)
endif()

# Try to find Boost components (make some optional)
find_package(Boost QUIET COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(WARNING "Boost not found - some features will be disabled")
    add_definitions(-DNO_BOOST)
endif()

# Try to find Eigen3
find_package(Eigen3 QUIET)
if(NOT EIGEN3_FOUND)
    message(WARNING "Eigen3 not found - ML features will be disabled")
    add_definitions(-DNO_EIGEN)
endif()

# Try to find OpenMP
find_package(OpenMP QUIET)

# Try to find SQLite3
find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND)
    message(WARNING "SQLite3 not found - logging will be disabled")
    add_definitions(-DNO_SQLITE)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
if(MPI_FOUND)
    include_directories(${MPI_INCLUDE_PATH})
endif()
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
if(EIGEN3_FOUND)
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(clusterforge ${SOURCES})

# Link libraries
if(MPI_FOUND)
    target_link_libraries(clusterforge ${MPI_LIBRARIES})
endif()
if(Boost_FOUND)
    target_link_libraries(clusterforge ${Boost_LIBRARIES})
endif()
if(EIGEN3_FOUND)
    target_link_libraries(clusterforge Eigen3::Eigen)
endif()
if(SQLite3_FOUND)
    target_link_libraries(clusterforge SQLite::SQLite3)
endif()
if(OpenMP_CXX_FOUND)
    target_link_libraries(clusterforge OpenMP::OpenMP_CXX)
endif()

# Installation
install(TARGETS clusterforge DESTINATION bin) 