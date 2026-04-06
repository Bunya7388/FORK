#pragma once

#include "common.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <chrono>

class SessionManager {
public:
    SessionManager();
    ~SessionManager();
    
    // Session operations
    uint64_t create_session(uint32_t client_addr, uint16_t client_port);
    bool get_session(uint64_t session_id, SessionInfo& info);
    bool update_session(uint64_t session_id);
    bool close_session(uint64_t session_id);
    
    // Cleanup
    void cleanup_expired_sessions();
    int get_session_timeout() const { return session_timeout_seconds_; }
    void set_session_timeout(int seconds) { session_timeout_seconds_ = seconds; }
    
    // Statistics
    int get_active_sessions() const;
    int get_total_sessions() const { return total_sessions_; }
    std::vector<SessionInfo> get_all_sessions() const;
    
private:
    // Session lookup
    uint64_t generate_session_id();
    bool is_session_expired(const SessionInfo& info) const;
    
    // Member variables
    std::unordered_map<uint64_t, SessionInfo> sessions_;
    mutable std::mutex sessions_mutex_;
    
    int session_timeout_seconds_;
    uint64_t next_session_id_;
    int total_sessions_;
};
