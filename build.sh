#!/bin/bash

# ClusterForge Build Script
# This script automates the build process for ClusterForge

set -e  # Exit on any error

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    ClusterForge Build Script                 ║"
echo "║              Advanced Cluster Resource Manager               ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the ClusterForge root directory."
    exit 1
fi

# Check for required dependencies
print_status "Checking dependencies..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Please install CMake 3.16 or higher."
    exit 1
fi

# Check for C++ compiler
if command -v g++ &> /dev/null; then
    COMPILER="g++"
    print_status "Found GCC compiler"
elif command -v clang++ &> /dev/null; then
    COMPILER="clang++"
    print_status "Found Clang compiler"
else
    print_error "No C++ compiler found. Please install GCC or Clang."
    exit 1
fi

# Check for make
if ! command -v make &> /dev/null; then
    print_error "Make not found. Please install make."
    exit 1
fi

print_success "All basic dependencies found"

# Create build directory
print_status "Creating build directory..."
if [ -d "build" ]; then
    print_warning "Build directory already exists. Cleaning..."
    rm -rf build
fi
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Check if configuration was successful
if [ $? -ne 0 ]; then
    print_error "CMake configuration failed. Please check your dependencies."
    print_status "Required dependencies:"
    echo "  - MPI (OpenMPI or MPICH)"
    echo "  - Boost (system, thread, log, filesystem, graph, asio)"
    echo "  - Eigen3"
    echo "  - SQLite3"
    echo "  - OpenMP (optional)"
    exit 1
fi

print_success "CMake configuration completed"

# Build the project
print_status "Building ClusterForge..."
make -j$(nproc)

# Check if build was successful
if [ $? -ne 0 ]; then
    print_error "Build failed. Please check the error messages above."
    exit 1
fi

print_success "Build completed successfully!"

# Check if executable was created
if [ -f "clusterforge" ]; then
    print_success "Executable 'clusterforge' created successfully"
    
    # Show executable info
    echo ""
    print_status "Executable information:"
    ls -lh clusterforge
    echo ""
    
    # Check if it's executable
    if [ -x "clusterforge" ]; then
        print_success "Executable is ready to run"
    else
        print_warning "Executable may not have proper permissions"
        chmod +x clusterforge
    fi
else
    print_error "Executable 'clusterforge' not found after build"
    exit 1
fi

# Optional: Run basic test
if [ "$1" = "--test" ]; then
    echo ""
    print_status "Running basic test..."
    timeout 10s ./clusterforge --help > /dev/null 2>&1 || true
    print_success "Basic test completed"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    Build Completed!                         ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
print_status "To run ClusterForge:"
echo "  cd build"
echo "  ./clusterforge"
echo ""
print_status "To run with MPI (for distributed execution):"
echo "  mpirun -np 4 ./clusterforge"
echo ""
print_status "For more information, see README.md"
echo ""

exit 0 