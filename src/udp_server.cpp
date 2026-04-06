#include "udp_server.h"
#include "worker_pool.h"
#include "session_manager.h"
#include "performance_monitor.h"
#include "mtu_manager.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

UDPServer::UDPServer(int port, int mtu_size, int worker_count)
    : port_(port), mtu_size_(mtu_size), worker_count_(worker_count),
      buffer_size_(BUFFER_SIZE), server_socket_(-1) {
    receive_buffer_.resize(buffer_size_);
    send_buffer_.resize(buffer_size_);
    
    worker_pool_ = std::make_unique<WorkerPool>(worker_count);
    session_manager_ = std::make_unique<SessionManager>();
    perf_monitor_ = std::make_unique<PerformanceMonitor>();
    mtu_manager_ = std::make_unique<MTUManager>(mtu_size);
}

UDPServer::~UDPServer() {
    stop();
    close_socket();
}

bool UDPServer::initialize() {
    log_message(LogLevel::INFO, "UDPServer", "Initializing DNS-over-UDP server on port " + std::to_string(port_));
    
    if (!create_socket()) {
        log_message(LogLevel::ERR, "UDPServer", "Failed to create socket");
        return false;
    }
    
    if (!configure_socket()) {
        log_message(LogLevel::ERR, "UDPServer", "Failed to configure socket");
        close_socket();
        return false;
    }
    
    if (!bind_socket()) {
        log_message(LogLevel::ERR, "UDPServer", "Failed to bind socket");
        close_socket();
        return false;
    }
    
    log_message(LogLevel::INFO, "UDPServer", "Server initialized successfully");
    return true;
}

bool UDPServer::create_socket() {
    server_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket_ < 0) {
        return false;
    }
    return true;
}

bool UDPServer::configure_socket() {
    // Enable address reuse
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return false;
    }
    
    // Set large socket buffers (4MB read/write)
    int buffer_size = 4 * 1024 * 1024;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        return false;
    }
    if (setsockopt(server_socket_, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        return false;
    }
    
    // Set non-blocking mode
    int flags = fcntl(server_socket_, F_GETFL, 0);
    if (fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK) < 0) {
        return false;
    }
    
    // Disable IP fragmentation if possible
    int mtu_discover = IP_PMTUDISC_DO;
    setsockopt(server_socket_, IPPROTO_IP, IP_MTU_DISCOVER, &mtu_discover, sizeof(mtu_discover));
    
    return true;
}

bool UDPServer::bind_socket() {
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        return false;
    }
    
    return true;
}

bool UDPServer::close_socket() {
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
    return true;
}

bool UDPServer::start() {
    if (running_.load()) {
        return false;
    }
    
    running_ = true;
    worker_pool_->start();
    perf_monitor_->start_monitoring();
    
    receive_thread_ = std::make_unique<std::thread>(&UDPServer::receive_loop, this);
    
    log_message(LogLevel::INFO, "UDPServer", "Server started with " + std::to_string(worker_count_) + " workers");
    return true;
}

void UDPServer::stop() {
    running_ = false;
    
    if (receive_thread_ && receive_thread_->joinable()) {
        receive_thread_->join();
    }
    
    worker_pool_->stop();
    perf_monitor_->stop_monitoring();
    session_manager_->cleanup_expired_sessions();
    
    log_message(LogLevel::INFO, "UDPServer", "Server stopped");
}

void UDPServer::receive_loop() {
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    
    log_message(LogLevel::INFO, "UDPServer", "Receive loop started");
    
    while (running_.load()) {
        std::memset(&client_addr, 0, sizeof(client_addr));
        
        ssize_t recv_len = recvfrom(server_socket_, receive_buffer_.data(), 
                                    receive_buffer_.size(), MSG_DONTWAIT,
                                    reinterpret_cast<sockaddr*>(&client_addr),
                                    &client_addr_len);
        
        if (recv_len > 0) {
            std::vector<uint8_t> packet_data(receive_buffer_.begin(), 
                                            receive_buffer_.begin() + recv_len);
            
            perf_monitor_->record_packet_received(recv_len);
            mtu_manager_->record_packet_size(recv_len);
            
            process_packet(packet_data, client_addr);
        } else if (recv_len < 0) {
            // Would block or error
            usleep(100);
        }
    }
    
    log_message(LogLevel::INFO, "UDPServer", "Receive loop stopped");
}

void UDPServer::process_packet(const std::vector<uint8_t>& data,
                               const sockaddr_in& client_addr) {
    // Create session if needed
    uint32_t client_ip = ntohl(client_addr.sin_addr.s_addr);
    uint16_t client_port = ntohs(client_addr.sin_port);
    
    uint64_t session_id = session_manager_->create_session(client_ip, client_port);
    session_manager_->update_session(session_id);
    
    // Enqueue work to worker pool
    WorkItem work;
    work.data = data;
    work.client_addr = client_addr;
    work.callback = [this](const std::vector<uint8_t>& response, const sockaddr_in& addr) {
        ssize_t sent = sendto(server_socket_, response.data(), response.size(), 0,
                             reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        if (sent > 0) {
            perf_monitor_->record_packet_sent(sent);
        }
    };
    
    worker_pool_->enqueue_task(work);
}

void UDPServer::set_worker_count(int count) {
    if (count < MIN_WORKERS || count > MAX_WORKERS) {
        log_message(LogLevel::WARN, "UDPServer", "Invalid worker count: " + std::to_string(count));
        return;
    }
    worker_count_ = count;
    worker_pool_->resize(count);
}

void UDPServer::set_mtu_size(int mtu) {
    if (mtu_manager_->set_mtu(mtu)) {
        mtu_size_ = mtu;
    }
}

void UDPServer::set_buffer_size(size_t size) {
    if (size > 0 && size <= 100 * 1024 * 1024) { // Max 100MB
        buffer_size_ = size;
        receive_buffer_.resize(size);
        send_buffer_.resize(size);
    }
}

PerfStats UDPServer::get_stats() const {
    return perf_monitor_->get_stats();
}

void UDPServer::reset_stats() {
    perf_monitor_->reset_stats();
}

void log_message(LogLevel level, const std::string& module, const std::string& msg) {
    static const char* level_names[] = {"DEBUG", "INFO", "WARN", "ERR"};
    
    (void)std::chrono::system_clock::now(); // Use now for future timestamp logging
    
    std::cout << "[" << level_names[static_cast<int>(level)] << "] "
              << "[" << module << "] " << msg << std::endl;
}
