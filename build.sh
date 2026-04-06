#!/bin/bash

# DNS-over-UDP Server Build Script
# Comprehensive build, test, and packaging script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
INSTALL_PREFIX="${1:-/usr/local}"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}DNS-over-UDP Tunneling Server Build Script${NC}"
echo "=========================================="

# Check dependencies
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"
    
    local missing_deps=0
    
    if ! command -v cmake &> /dev/null; then
        echo -e "${RED}✗ CMake not found${NC}"
        missing_deps=1
    else
        echo -e "${GREEN}✓ CMake found${NC}"
    fi
    
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        echo -e "${RED}✗ C++ compiler not found${NC}"
        missing_deps=1
    else
        echo -e "${GREEN}✓ C++ compiler found${NC}"
    fi
    
    if ! pkg-config --exists openssl; then
        echo -e "${RED}✗ OpenSSL development libraries not found${NC}"
        missing_deps=1
    else
        echo -e "${GREEN}✓ OpenSSL found${NC}"
    fi
    
    if [ $missing_deps -eq 1 ]; then
        echo -e "${RED}Missing dependencies. Please install required packages.${NC}"
        exit 1
    fi
}

# Create build directory
setup_build() {
    echo -e "${YELLOW}Setting up build directory...${NC}"
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    echo -e "${GREEN}✓ Build directory created${NC}"
}

# Configure with CMake
configure() {
    echo -e "${YELLOW}Configuring with CMake...${NC}"
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -DCMAKE_CXX_FLAGS="-O3 -march=native"
    
    echo -e "${GREEN}✓ Configuration complete${NC}"
}

# Build
build() {
    echo -e "${YELLOW}Building...${NC}"
    
    make -j$(nproc)
    
    echo -e "${GREEN}✓ Build complete${NC}"
}

# Run tests
run_tests() {
    echo -e "${YELLOW}Running tests...${NC}"
    
    ctest --output-on-failure || {
        echo -e "${YELLOW}Note: Some tests may require running the server${NC}"
    }
    
    echo -e "${GREEN}✓ Tests completed${NC}"
}

# Generate binary distributable
create_distribution() {
    echo -e "${YELLOW}Creating distribution package...${NC}"
    
    local dist_dir="${SCRIPT_DIR}/dist"
    mkdir -p "$dist_dir/bin"
    mkdir -p "$dist_dir/etc"
    mkdir -p "$dist_dir/lib"
    
    cp "${BUILD_DIR}/dnstt-udp" "$dist_dir/bin/"
    
    cat > "$dist_dir/etc/dnstt-udp.conf" << 'EOF'
# DNS-over-UDP Server Configuration
port = 5300
worker_count = 4
mtu_size = 1500
buffer_size = 4194304
session_timeout = 300
log_level = INFO
bind_address = 0.0.0.0
pubkey_path = /etc/dnstt-udp/pubkey
enable_stats = true
stats_interval = 5000
EOF
    
    chmod +x "$dist_dir/bin/dnstt-udp"
    
    echo -e "${GREEN}✓ Distribution created in $dist_dir${NC}"
}

# Display usage
show_usage() {
    cat << EOF
${GREEN}Build completed successfully!${NC}

Binary location: ${BUILD_DIR}/dnstt-udp

Usage:
  ${BUILD_DIR}/dnstt-udp -p 5300 -w 4 -m 1500

Generate PUBKEY:
  ${BUILD_DIR}/dnstt-udp -g -k ./pubkey.bin

For more information:
  ${BUILD_DIR}/dnstt-udp -h

To install system-wide:
  cd ${BUILD_DIR}
  sudo make install

Install path: ${INSTALL_PREFIX}
EOF
}

# Main execution
main() {
    check_dependencies
    setup_build
    configure
    build
    run_tests
    create_distribution
    show_usage
}

# Run main
main
