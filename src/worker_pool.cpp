#include "worker_pool.h"
#include <iostream>

WorkerPool::WorkerPool(int num_workers)
    : num_workers_(num_workers), max_workers_(num_workers) {
    if (num_workers < MIN_WORKERS || num_workers > MAX_WORKERS) {
        num_workers_ = DEFAULT_WORKERS;
    }
}

WorkerPool::~WorkerPool() {
    stop();
}

void WorkerPool::start() {
    if (running_.load()) {
        return;
    }
    
    running_ = true;
    
    for (int i = 0; i < num_workers_; ++i) {
        workers_.push_back(std::make_unique<std::thread>(&WorkerPool::worker_thread, this));
    }
}

void WorkerPool::stop() {
    running_ = false;
    queue_cv_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker && worker->joinable()) {
            worker->join();
        }
    }
    
    workers_.clear();
}

void WorkerPool::enqueue_task(const WorkItem& item) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        task_queue_.push(item);
    }
    queue_cv_.notify_one();
}

void WorkerPool::worker_thread() {
    while (running_.load()) {
        WorkItem work;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !task_queue_.empty() || !running_.load(); });
            
            if (!running_.load() && task_queue_.empty()) {
                break;
            }
            
            if (!task_queue_.empty()) {
                work = task_queue_.front();
                task_queue_.pop();
                active_workers_++;
            }
        }
        
        if (!work.data.empty() && work.callback) {
            // Process the packet
            // For DNS tunneling, we would:
            // 1. Parse DNS query
            // 2. Forward to upstream DNS or process locally
            // 3. Send response via callback
            
            // Example: Echo response for now
            std::vector<uint8_t> response = work.data;
            work.callback(response, work.client_addr);
        }
        
        active_workers_--;
    }
}

void WorkerPool::resize(int new_size) {
    if (new_size < MIN_WORKERS || new_size > MAX_WORKERS) {
        return;
    }
    
    if (new_size < num_workers_) {
        // Reduce workers
        num_workers_ = new_size;
    } else if (new_size > num_workers_) {
        // Add workers
        int to_add = new_size - num_workers_;
        for (int i = 0; i < to_add; ++i) {
            workers_.push_back(std::make_unique<std::thread>(&WorkerPool::worker_thread, this));
        }
        num_workers_ = new_size;
    }
}

int WorkerPool::get_active_workers() const {
    return active_workers_.load();
}

int WorkerPool::get_queue_size() const {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return task_queue_.size();
}
