#include "performance_monitor.h"
#include <cmath>

PerformanceMonitor::PerformanceMonitor()
    : monitoring_(false) {
    start_time_ = std::chrono::steady_clock::now();
    last_update_ = start_time_;
}

PerformanceMonitor::~PerformanceMonitor() {
    stop_monitoring();
}

void PerformanceMonitor::record_packet_received(size_t bytes) {
    packets_received_++;
    bytes_received_ += bytes;
}

void PerformanceMonitor::record_packet_sent(size_t bytes) {
    packets_sent_++;
    bytes_sent_ += bytes;
}

void PerformanceMonitor::record_error() {
    error_count_++;
}

void PerformanceMonitor::record_session_active(int count) {
    active_sessions_ = count;
}

PerfStats PerformanceMonitor::get_stats() {
    std::unique_lock<std::mutex> lock(stats_mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_update_).count();
    
    if (elapsed_ms >= STATS_UPDATE_INTERVAL_MS) {
        update_calculations();
        last_update_ = now;
    }
    
    cached_stats_.packets_received = packets_received_.load();
    cached_stats_.packets_sent = packets_sent_.load();
    cached_stats_.bytes_received = bytes_received_.load();
    cached_stats_.bytes_sent = bytes_sent_.load();
    cached_stats_.errors = error_count_.load();
    cached_stats_.sessions_active = active_sessions_.load();
    cached_stats_.last_update = now;
    
    return cached_stats_;
}

void PerformanceMonitor::update_calculations() {
    auto now = std::chrono::steady_clock::now();
    auto total_elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - start_time_).count();
    
    if (total_elapsed_ms > 0) {
        uint64_t total_packets = packets_received_.load() + packets_sent_.load();
        double pps = (total_packets * 1000.0) / total_elapsed_ms;
        cached_stats_.packets_per_sec = pps;
        
        uint64_t total_bytes = bytes_received_.load() + bytes_sent_.load();
        double mbps = (total_bytes * 8.0) / (total_elapsed_ms * 1000.0);
        cached_stats_.throughput_mbps = mbps;
    }
}

void PerformanceMonitor::reset_stats() {
    packets_received_ = 0;
    packets_sent_ = 0;
    bytes_received_ = 0;
    bytes_sent_ = 0;
    error_count_ = 0;
    active_sessions_ = 0;
    start_time_ = std::chrono::steady_clock::now();
    last_update_ = start_time_;
}

void PerformanceMonitor::start_monitoring() {
    monitoring_ = true;
}

void PerformanceMonitor::stop_monitoring() {
    monitoring_ = false;
}

bool PerformanceMonitor::is_monitoring() const {
    return monitoring_.load();
}

double PerformanceMonitor::get_packets_per_sec() const {
    return cached_stats_.packets_per_sec;
}

double PerformanceMonitor::get_throughput_mbps() const {
    return cached_stats_.throughput_mbps;
}

uint64_t PerformanceMonitor::get_total_packets() const {
    return packets_received_.load() + packets_sent_.load();
}

uint64_t PerformanceMonitor::get_total_bytes() const {
    return bytes_received_.load() + bytes_sent_.load();
}
