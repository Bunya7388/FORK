#include "worker_pool.h"
#include <iostream>
#include <cassert>
#include <chrono>

int main() {
    std::cout << "Testing Worker Pool...\n";
    
    // Test 1: Create and start pool
    WorkerPool pool(4);
    pool.start();
    
    assert(pool.get_active_workers() == 0);
    assert(pool.get_max_workers() == 4);
    
    std::cout << "✓ Worker pool created and started\n";
    
    // Test 2: Enqueue tasks
    int completed_tasks = 0;
    
    for (int i = 0; i < 10; ++i) {
        WorkItem item;
        item.data.push_back(static_cast<uint8_t>(i));
        item.callback = [&completed_tasks](const std::vector<uint8_t>&, const sockaddr_in&) {
            completed_tasks++;
        };
        pool.enqueue_task(item);
    }
    
    // Wait for tasks
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    assert(pool.get_queue_size() == 0);
    std::cout << "✓ Tasks enqueued and processed\n";
    
    // Test 3: Resize pool
    pool.resize(8);
    assert(pool.get_max_workers() == 8);
    
    std::cout << "✓ Worker pool resized\n";
    
    // Cleanup
    pool.stop();
    
    std::cout << "All worker pool tests passed!\n";
    return 0;
}
