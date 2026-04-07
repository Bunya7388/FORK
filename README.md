# DNS-over-UDP Tunneling Server (dnstt-udp)

A high-performance, production-ready DNS tunneling server optimized for fast UDP-based DNS query forwarding with advanced features including worker pool configuration, session management, performance monitoring, and public key management.

## 🚀 Quick Start

```bash
# Build the project
./build.sh

# Generate 32-byte HEX PUBKEY
./build/dnstt-udp -g

# Start server with 8 workers on port 5300
./build/dnstt-udp -p 5300 -w 8

# Monitor with real-time statistics
# (displayed automatically during execution)
```

## ✨ Key Features

### Core Performance
- **✅ High-Speed UDP**: Optimized for fast DNS tunneling
- **✅ Worker Pool**: Configurable thread count via `-w` flag (1-256 workers)
- **✅ Concurrent Processing**: Non-blocking request handling
- **✅ Buffer Optimization**: Large socket buffers (4MB read/write)

### Session & Management
- **✅ Session Management**: Automatic session tracking and cleanup
- **✅ Performance Monitoring**: Real-time statistics (packets/sec, throughput)
- **✅ MTU Management**: Dynamic MTU size handling and packet fragmentation
- **✅ HEX 64 PUBKEY**: Generate and manage 32-byte public keys

### Compatibility & Deployment
- **✅ UDP GW Compatible**: Compatible with udpgw-server/badvpn-udpgw protocol
- **✅ Ready-to-Run**: Pre-built binaries included
- **✅ Systemd Integration**: Service file for system integration
- **✅ Production Ready**: Tested error handling and recovery

## 📋 Requirements

- **OS**: Linux/Unix system
- **Compiler**: C++17 compatible (GCC 7+, Clang 5+)
- **Libraries**: OpenSSL development libraries (libssl-dev)
- **Build Tools**: CMake 3.10+, Make
- **Runtime**: Pthreads library

### Ubuntu/Debian Installation

```bash
sudo apt-get update
sudo apt-get install -y \
    cmake \
    build-essential \
    libssl-dev \
    libpthread-stubs0-dev
```

### CentOS/RHEL Installation

```bash
sudo yum install -y \
    cmake3 \
    gcc \
    gcc-c++ \
    openssl-devel \
    make
```

## 🔨 Building

### Standard Build

```bash
chmod +x build.sh
./build.sh
```

### Manual Build

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### Build Output

```
Binary: ./build/dnstt-udp
Library: ./build/libdnstt-udp.a (if built)
Config: ./etc/dnstt-udp.conf
```

## 💻 Usage

### Display Help

```bash
./build/dnstt-udp -h
```

### Basic Server

```bash
# Default settings (port 5300, 4 workers, 1500 MTU)
./build/dnstt-udp

# Custom port and workers
./build/dnstt-udp -p 5353 -w 8

# With custom MTU for jumbo frames
./build/dnstt-udp -m 9000 -w 16
```

### Generate Public Key

```bash
# Display 32-byte HEX PUBKEY
./build/dnstt-udp -g

# Save to file for secure storage
./build/dnstt-udp -g -k /etc/dnstt-udp/pubkey.bin

# Load and display saved key
./build/dnstt-udp -s -k /etc/dnstt-udp/pubkey.bin
```

### Using Configuration File

```bash
./build/dnstt-udp -c /etc/dnstt-udp/dnstt-udp.conf
```

## 🎯 Command-Line Options

| Option | Description | Range | Default |
|--------|-------------|-------|---------|
| `-p, --port PORT` | Server listening port | 1-65535 | 5300 |
| `-w, --workers N` | Worker thread count | 1-256 | 4 |
| `-m, --mtu SIZE` | MTU size in bytes | 576-65535 | 1500 |
| `-b, --buffer-size SIZE` | Socket buffer size | 1MB-100MB | 4MB |
| `-c, --config FILE` | Load config file | - | - |
| `-g, --generate-key` | Generate new PUBKEY | - | - |
| `-k, --pubkey-file FILE` | PUBKEY file path | - | - |
| `-s, --show-key` | Display saved PUBKEY | - | - |
| `-h, --help` | Show help message | - | - |
| `-v, --version` | Show version | - | - |

## 📊 Performance Monitoring

The server displays real-time statistics:

```
======== Performance Statistics ========
Packets Received: 10245
Packets Sent: 10242
Bytes Received: 512250 bytes
Bytes Sent: 512100 bytes
Packets/sec: 2048.50
Throughput: 102.45 Mbps
Active Sessions: 15
Errors: 0
==========================================
```

## 🔐 Public Key Management

### Generate 32-Byte HEX Key

The server can generate cryptographically secure 32-byte keys:

```bash
$ ./build/dnstt-udp -g
Generated 32-Byte HEX PUBKEY:
========================================
HEX Key: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2g3h4i5j6k7l8m9n0o1p2q3r4s5t6u7v8
Fingerprint: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
========================================
```

### Key Operations

- **Generate random key**: `./build/dnstt-udp -g`
- **Generate from seed**: Uses internal seed mechanism
- **Save to binary**: `./build/dnstt-udp -g -k pubkey.bin`
- **Load and verify**: `./build/dnstt-udp -s -k pubkey.bin`
- **Fingerprint generation**: SHA256-based fingerprinting

## 🌐 MTU Management

### Available MTU Sizes

| Size | Type | Use Case |
|------|------|----------|
| 576 | Minimum | Dialup, Satellite links |
| 1280 | IPv6 default | IPv6 networks |
| 1500 | Ethernet | Standard networks |
| 9000 | Jumbo frames | High-speed LAN |

### Dynamic MTU Configuration

```bash
# Standard Ethernet
./build/dnstt-udp -m 1500 -w 8

# Jumbo frames for high bandwidth
./build/dnstt-udp -m 9000 -w 16 -b 8388608

# Conservative for unstable links
./build/dnstt-udp -m 1280 -w 4
```

### UDP GW Protocol Support

Compatible with badvpn-udpgw:
- Automatic protocol detection
- MTU accounting (28 bytes for IP+UDP headers)
- Session-based routing
- Address and port preservation

## 📝 Configuration File

### Default Location

```
/etc/dnstt-udp/dnstt-udp.conf
```

### Sample Configuration

```ini
# Server Settings
port = 5300
worker_count = 8
mtu_size = 1500
buffer_size = 4194304

# Session Management
session_timeout = 300

# Logging
log_level = INFO

# Performance
enable_stats = true
stats_interval = 5000

# Security
pubkey_path = /etc/dnstt-udp/pubkey
enable_udpgw_compat = true
max_sessions = 10000
```

## 🐧 Systemd Integration

### Install Service

```bash
sudo cp dnstt-udp.service /etc/systemd/system/
sudo systemctl daemon-reload
```

### Manage Service

```bash
# Start server
sudo systemctl start dnstt-udp

# Stop server
sudo systemctl stop dnstt-udp

# Check status
sudo systemctl status dnstt-udp

# Enable on boot
sudo systemctl enable dnstt-udp

# View logs
sudo journalctl -u dnstt-udp -f
```

## 🏗️ Project Structure

```
.
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── README_SERVER.md         # Detailed server documentation
├── build.sh                 # Automated build script
├── dnstt-udp.service        # Systemd service file
├── LICENSE                  # License information
│
├── include/                 # Header files
│   ├── common.h             # Common definitions
│   ├── udp_server.h         # UDP server class
│   ├── worker_pool.h        # Concurrent worker management
│   ├── session_manager.h    # Session tracking
│   ├── performance_monitor.h # Performance metrics
│   ├── pubkey_generator.h   # Key management
│   ├── mtu_manager.h        # MTU handling
│   └── config.h             # Configuration system
│
├── src/                     # Implementation files
│   ├── main.cpp             # Application entry point
│   ├── udp_server.cpp       # UDP server implementation
│   ├── worker_pool.cpp      # Worker pool implementation
│   ├── session_manager.cpp  # Session management
│   ├── performance_monitor.cpp # Performance tracking
│   ├── pubkey_generator.cpp # Key generation
│   ├── mtu_manager.cpp      # MTU management
│   └── config.cpp           # Configuration handling
│
├── tests/                   # Test suite
│   ├── CMakeLists.txt       # Test configuration
│   ├── test_worker_pool.cpp # Worker pool tests
│   ├── test_session_manager.cpp # Session tests
│   └── test_pubkey.cpp      # Key generation tests
│
├── etc/                     # Configuration files
│   └── dnstt-udp.conf       # Default configuration
│
└── dist/                    # Distribution package (generated)
    ├── bin/dnstt-udp        # Binary executable
    └── etc/dnstt-udp.conf   # Configuration
```

## 📈 Performance Tuning

### For High Throughput

```bash
# Multi-core system (16 cores)
./build/dnstt-udp -w 16 -m 9000 -b 8388608
```

### For Low Latency

```bash
# Optimize for response time
./build/dnstt-udp -w 8 -m 1500
```

### For Stable Networks

```bash
# Standard configuration
./build/dnstt-udp -w 4 -m 1500 -p 5300
```

## 🔍 Testing

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Test Results

```
✓ WorkerPoolTest - Concurrent processing
✓ SessionManagerTest - Session lifecycle
✓ PubKeyTest - Key generation and management
```

## 📊 Architecture Overview

### Worker Pool
- 1-256 configurable worker threads
- Non-blocking task queue
- Dynamic resizing capability
- Load balancing

### Session Management
- Automatic session creation on first packet
- Timeout-based cleanup (default 5 minutes)
- Per-session statistics tracking
- Memory-efficient storage

### Performance Monitoring
- Real-time packet counting
- Throughput calculation (Mbps)
- Error tracking
- Session statistics

### MTU Manager
- Dynamic MTU configuration
- Packet fragmentation support
- MTU discovery mechanisms
- UDP GW compatibility

## 🐛 Troubleshooting

### Port Already in Use

```bash
# Check which process is using the port
sudo netstat -tlnp | grep 5300

# Use different port
./build/dnstt-udp -p 5301
```

### High CPU Usage

```bash
# Reduce worker count
./build/dnstt-udp -w 4

# Or increase MTU to reduce fragmentation
./build/dnstt-udp -m 9000
```

### Permission Denied

```bash
# Run as root for privileged ports (<1024)
sudo ./build/dnstt-udp -p 53

# Or use non-privileged port (>1024)
./build/dnstt-udp -p 5300
```

## 📜 License

See [LICENSE](LICENSE) file for details.

---

**DNS-over-UDP Tunneling Server v1.0.0**  
High-Performance DNS Tunneling for Modern Networks