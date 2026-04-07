# DNS-over-UDP Tunneling Server

High-performance DNS tunneling server with optimized UDP transport for fast and reliable DNS query forwarding.

## Features

✅ **High-Speed UDP**: Optimized for fast DNS tunneling  
✅ **Worker Pool**: Configurable thread count via `-w` flag (1-256 workers)  
✅ **Session Management**: Automatic session tracking and cleanup  
✅ **Performance Monitoring**: Real-time statistics (packets/sec, throughput)  
✅ **Buffer Optimization**: Large socket buffers (4MB read/write)  
✅ **Ready-to-Run**: Pre-built binaries included  
✅ **Concurrent Processing**: Non-blocking request handling  
✅ **HEX 64 PUBKEY**: Generate and manage 32-byte public keys
✅ **MTU Management**: Dynamic MTU size handling and packet fragmentation  
✅ **UDP GW Compatible**: Compatible with udpgw-server/badvpn-udpgw protocol  

## Requirements

- Linux/Unix system
- C++17 compatible compiler
- OpenSSL development libraries
- CMake 3.10+
- Pthreads

## Building

### Quick Build

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Build with optimization

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### Building tests

```bash
cd build
ctest --output-on-failure
```

## Installation

### From source

```bash
sudo make install
```

Binary will be installed to `/usr/local/bin/dnstt-udp`

### Creating runtime directory

```bash
sudo mkdir -p /etc/dnstt-udp
sudo chmod 755 /etc/dnstt-udp
```

## Usage

### Basic startup

```bash
dnstt-udp -p 5300 -w 4 -m 1500
```

### Generate 32-byte HEX PUBKEY

```bash
# Generate and display key
dnstt-udp -g

# Generate and save to file
dnstt-udp -g -k /etc/dnstt-udp/pubkey.bin

# Display saved key information
dnstt-udp -s -k /etc/dnstt-udp/pubkey.bin
```

### Using configuration file

```bash
dnstt-udp -c /etc/dnstt-udp/dnstt-udp.conf
```

### Command-line options

```
Options:
  -p, --port PORT              listening port (default: 5300)
  -w, --workers WORKERS        number of worker threads 1-256 (default: 4)
  -m, --mtu MTU                MTU size in bytes (default: 1500)
  -b, --buffer-size SIZE       socket buffer size in bytes (default: 4MB)
  -c, --config FILE            load configuration file
  -g, --generate-key           generate and save 32-byte hex PUBKEY
  -k, --pubkey-file FILE       save generated key to file
  -s, --show-key               display current pubkey information
  -h, --help                   show this help message
  -v, --version                show version information
```

## Configuration File

### Default location

`/etc/dnstt-udp/dnstt-udp.conf`

### Example configuration

```ini
# DNS-over-UDP Server Configuration

# Server port
port = 5300

# Number of worker threads (1-256)
worker_count = 8

# MTU size in bytes (576-65535)
mtu_size = 1500

# Socket buffer size in bytes
buffer_size = 4194304

# Session timeout in seconds
session_timeout = 300

# Log level (DEBUG, INFO, WARN, ERR)
log_level = INFO

# Bind address
bind_address = 0.0.0.0

# Path to public key file
pubkey_path = /etc/dnstt-udp/pubkey

# Enable performance statistics
enable_stats = true

# Statistics update interval in milliseconds
stats_interval = 5000
```

## Performance Monitoring

The server displays real-time statistics showing:
- Packets received/sent
- Bytes received/sent
- Packets per second
- Throughput in Mbps
- Active sessions count
- Error count

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

## MTU Management

The server automatically manages packet fragmentation:
- Configurable MTU size (576-65535 bytes)
- Automatic packet fragmentation for large payloads
- UDP GW protocol compatibility (accounts for IP+UDP headers)
- Packet size tracking and optimal MTU detection

```bash
# Set custom MTU
dnstt-udp -m 9000 -w 8

# Default MTU (1500 bytes standard Ethernet)
dnstt-udp -m 1500
```

## Public Key Management

### Generate new 32-byte HEX PUBKEY

```bash
$ dnstt-udp -g
Generated 32-Byte HEX PUBKEY:
========================================
HEX Key: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2g3h4i5j6k7l8m9n0o1p2q3r4s5t6u7v8w9
Fingerprint: a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6
========================================
```

### Save key to file for secure storage

```bash
dnstt-udp -g -k /etc/dnstt-udp/pubkey.bin
```

### UDP GW Protocol Support

The server is compatible with badvpn-udpgw protocol:
- Automatic protocol detection
- Transparent packet forwarding
- Session-based routing
- Address and port preservation

## Systemd Service

Create `/etc/systemd/system/dnstt-udp.service`:

```ini
[Unit]
Description=DNS-over-UDP Tunneling Server
After=network.target

[Service]
Type=simple
User=root
ExecStart=/usr/local/bin/dnstt-udp -c /etc/dnstt-udp/dnstt-udp.conf
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable dnstt-udp
sudo systemctl start dnstt-udp
sudo systemctl status dnstt-udp
```

## Performance Tuning

### Worker Pool Optimization

For high-throughput scenarios:
```bash
# Multi-core system with 16 cores
dnstt-udp -w 16 -m 1500 -p 5300
```

### Buffer Size Optimization

For extreme throughput:
```bash
# Increase buffer size to 8MB
dnstt-udp -b 8388608 -w 32
```

### MTU Optimization for Jumbo Frames

For networks supporting jumbo frames:
```bash
# Use 9000 byte MTU
dnstt-udp -m 9000 -w 16
```

## Troubleshooting

### Server won't bind to port

```bash
# Check if port is already in use
sudo netstat -tlnp | grep 5300

# Use different port
dnstt-udp -p 5301
```

### High CPU usage

- Reduce worker count with `-w` flag
- Check for error conditions in logs
- Monitor packet loss rate

### Low throughput

- Increase MTU size appropriate for your network
- Increase worker pool size
- Increase socket buffer size

## Architecture

### Worker Pool

Non-blocking concurrent request handling with configurable worker threads:
- Min: 1 worker
- Default: 4 workers
- Max: 256 workers

### Session Management

Automatic session tracking and cleanup:
- Session timeout: 5 minutes (configurable)
- Automatic cleanup of expired sessions
- Per-session packet statistics

### Performance Monitoring

Real-time performance metrics:
- Packets per second tracking
- Throughput calculation (bits/sec)
- Active session monitoring
- Error tracking

### Buffer Management

Large, optimized socket buffers:
- 4MB read buffer
- 4MB write buffer
- Configurable via `-b` flag

## License

See LICENSE file for details.

## Support

For issues and feature requests, please refer to the project documentation.

---

**DNS-over-UDP Tunneling Server v1.0.0**  
High-Speed DNS Tunneling for Modern Networks
