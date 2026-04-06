#pragma once

#include "common.h"
#include <atomic>
#include <chrono>
#include <mutex>

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    // Recording metrics
    void record_packet_received(size_t bytes);
    void record_packet_sent(size_t bytes);
    void record_error();
    void record_session_active(int count);
    
    // Statistics retrieval
    PerfStats get_stats();
    void reset_stats();
    
    // Real-time monitoring
    void start_monitoring();
    void stop_monitoring();
    bool is_monitoring() const;
    
    // Detailed metrics
    double get_packets_per_sec() const;
    double get_throughput_mbps() const;
    uint64_t get_total_packets() const;
    uint64_t get_total_bytes() const;
    
private:
    // Statistics update
    void update_calculations();
    
    // Member variables
    std::atomic<uint64_t> packets_received_{0};
    std::atomic<uint64_t> packets_sent_{0};
    std::atomic<uint64_t> bytes_received_{0};
    std::atomic<uint64_t> bytes_sent_{0};
    std::atomic<uint64_t> error_count_{0};
    std::atomic<int> active_sessions_{0};
    
    std::atomic<bool> monitoring_{false};
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point last_update_;
    
    mutable std::mutex stats_mutex_;
    PerfStats cached_stats_;
};
