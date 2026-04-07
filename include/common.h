#pragma once

#include <cstdint>
#include <chrono>
#include <string>
#include <vector>
#include <memory>

// Constants
constexpr int DEFAULT_WORKERS = 4;
constexpr int MIN_WORKERS = 1;
constexpr int MAX_WORKERS = 256;
constexpr int DEFAULT_PORT = 5300;
constexpr int DEFAULT_MTU = 1500;
constexpr int MAX_MTU = 65535;
constexpr int MIN_MTU = 576;
constexpr size_t BUFFER_SIZE = 4 * 1024 * 1024; // 4MB buffers
constexpr size_t UDP_PAYLOAD_MAX = 65507; // Max UDP payload size
constexpr int SESSION_TIMEOUT_SECONDS = 300; // 5 minutes
constexpr int STATS_UPDATE_INTERVAL_MS = 1000; // 1 second

// DNS-over-UDP packet structure
struct DNSPacket {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    std::vector<uint8_t> data;
    
    DNSPacket() : id(0), flags(0), qdcount(0), ancount(0), nscount(0), arcount(0) {}
};

// UDP GW Protocol structures (compatible with badvpn-udpgw)
struct UDPGWHeader {
    uint32_t header_version;
    uint32_t mtu;
};

struct UDPGWRequest {
    uint32_t id;
    uint32_t dest_addr;
    uint16_t dest_port;
    std::vector<uint8_t> data;
};

// Performance statistics
struct PerfStats {
    uint64_t packets_received = 0;
    uint64_t packets_sent = 0;
    uint64_t bytes_received = 0;
    uint64_t bytes_sent = 0;
    uint64_t errors = 0;
    uint64_t sessions_active = 0;
    double packets_per_sec = 0.0;
    double throughput_mbps = 0.0;
    std::chrono::steady_clock::time_point last_update;
};

// Session information
struct SessionInfo {
    uint64_t session_id;
    uint32_t client_addr;
    uint16_t client_port;
    std::chrono::steady_clock::time_point created_at;
    std::chrono::steady_clock::time_point last_activity;
    uint64_t packets_processed;
    bool active;
};

// PUBKEY structure (32 bytes / 64 hex chars)
struct PubKey {
    uint8_t key[32];
    std::string hex_string;
    std::string fingerprint;
};

// Logging utilities
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERR = 3
};

void log_message(LogLevel level, const std::string& module, const std::string& msg);

// Error codes
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_PACKET = 1,
    SESSION_NOT_FOUND = 2,
    BUFFER_OVERFLOW = 3,
    SEND_FAILED = 4,
    RECV_FAILED = 5,
    CONFIG_ERROR = 6,
    THREAD_ERROR = 7,
    TIMEOUT = 8
};
