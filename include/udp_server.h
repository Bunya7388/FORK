#pragma once

#include "common.h"
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>

class WorkerPool;
class SessionManager;
class PerformanceMonitor;
class MTUManager;

class UDPServer {
public:
    UDPServer(int port, int mtu_size, int worker_count);
    ~UDPServer();

    // Server lifecycle
    bool initialize();
    bool start();
    void stop();
    
    // Configuration
    void set_worker_count(int count);
    void set_mtu_size(int mtu);
    void set_buffer_size(size_t size);
    
    // Statistics
    PerfStats get_stats() const;
    void reset_stats();
    
    // Socket management
    int get_socket() const { return server_socket_; }
    int get_port() const { return port_; }
    
    // Shutdown signal
    bool is_running() const { return running_.load(); }
    
private:
    // Socket handling
    bool create_socket();
    bool bind_socket();
    bool configure_socket();
    bool close_socket();
    
    // Packet processing
    void process_packet(const std::vector<uint8_t>& data, 
                       const sockaddr_in& client_addr);
    
    // Main receive loop
    void receive_loop();
    
    // Member variables
    int port_;
    int mtu_size_;
    int worker_count_;
    size_t buffer_size_;
    int server_socket_;
    
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> receive_thread_;
    
    std::unique_ptr<WorkerPool> worker_pool_;
    std::unique_ptr<SessionManager> session_manager_;
    std::unique_ptr<PerformanceMonitor> perf_monitor_;
    std::unique_ptr<MTUManager> mtu_manager_;
    
    // Socket buffers
    std::vector<uint8_t> receive_buffer_;
    std::vector<uint8_t> send_buffer_;
};
