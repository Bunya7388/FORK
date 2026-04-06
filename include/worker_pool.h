#pragma once

#include "common.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <functional>
#include <arpa/inet.h>
#include <atomic>

struct WorkItem {
    std::vector<uint8_t> data;
    sockaddr_in client_addr;
    std::function<void(const std::vector<uint8_t>&, const sockaddr_in&)> callback;
};

class WorkerPool {
public:
    explicit WorkerPool(int num_workers);
    ~WorkerPool();
    
    // Pool management
    void start();
    void stop();
    void resize(int new_size);
    
    // Task submission
    void enqueue_task(const WorkItem& item);
    
    // Statistics
    int get_active_workers() const;
    int get_queue_size() const;
    int get_max_workers() const { return max_workers_; }
    
private:
    // Worker thread function
    void worker_thread();
    
    // Member variables
    int num_workers_;
    int max_workers_;
    std::atomic<bool> running_{false};
    
    std::queue<WorkItem> task_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    std::vector<std::unique_ptr<std::thread>> workers_;
    std::atomic<int> active_workers_{0};
};
