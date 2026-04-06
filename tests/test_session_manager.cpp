#include "session_manager.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Testing Session Manager...\n";
    
    SessionManager mgr;
    
    // Test 1: Create session
    uint64_t session1 = mgr.create_session(0xC0A80001, 12345); // 192.168.0.1:12345
    assert(session1 > 0);
    
    std::cout << "✓ Session created with ID: " << session1 << "\n";
    
    // Test 2: Get session
    SessionInfo info;
    assert(mgr.get_session(session1, info));
    assert(info.client_port == 12345);
    assert(info.active);
    
    std::cout << "✓ Session retrieved successfully\n";
    
    // Test 3: Update session
    assert(mgr.update_session(session1));
    assert(mgr.get_session(session1, info));
    assert(info.packets_processed == 1);
    
    std::cout << "✓ Session updated successfully\n";
    
    // Test 4: Active sessions count
    uint64_t session2 = mgr.create_session(0xC0A80002, 12346);
    assert(mgr.get_active_sessions() == 2);
    
    std::cout << "✓ Active sessions count: " << mgr.get_active_sessions() << "\n";
    
    // Close session2
    mgr.close_session(session2);
    
    // Test 5: Close session
    assert(mgr.close_session(session1));
    assert(mgr.get_active_sessions() == 1);
    
    std::cout << "✓ Session closed successfully\n";
    
    // Test 6: Session timeout handling
    mgr.set_session_timeout(1);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    mgr.cleanup_expired_sessions();
    
    std::cout << "✓ Expired sessions cleaned up\n";
    
    std::cout << "All session manager tests passed!\n";
    return 0;
}
