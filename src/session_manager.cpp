#include "session_manager.h"
#include <algorithm>
#include <iostream>

SessionManager::SessionManager()
    : session_timeout_seconds_(SESSION_TIMEOUT_SECONDS), 
      next_session_id_(1), total_sessions_(0) {
}

SessionManager::~SessionManager() {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    sessions_.clear();
}

uint64_t SessionManager::create_session(uint32_t client_addr, uint16_t client_port) {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    
    uint64_t session_id = generate_session_id();
    
    SessionInfo session;
    session.session_id = session_id;
    session.client_addr = client_addr;
    session.client_port = client_port;
    session.created_at = std::chrono::steady_clock::now();
    session.last_activity = std::chrono::steady_clock::now();
    session.packets_processed = 0;
    session.active = true;
    
    sessions_[session_id] = session;
    total_sessions_++;
    
    return session_id;
}

bool SessionManager::get_session(uint64_t session_id, SessionInfo& info) {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        info = it->second;
        return true;
    }
    
    return false;
}

bool SessionManager::update_session(uint64_t session_id) {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        it->second.last_activity = std::chrono::steady_clock::now();
        it->second.packets_processed++;
        return true;
    }
    
    return false;
}

bool SessionManager::close_session(uint64_t session_id) {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    
    auto it = sessions_.find(session_id);
    if (it != sessions_.end()) {
        it->second.active = false;
        sessions_.erase(it);
        return true;
    }
    
    return false;
}

void SessionManager::cleanup_expired_sessions() {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    
    auto it = sessions_.begin();
    while (it != sessions_.end()) {
        if (is_session_expired(it->second)) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

int SessionManager::get_active_sessions() const {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    return sessions_.size();
}

std::vector<SessionInfo> SessionManager::get_all_sessions() const {
    std::unique_lock<std::mutex> lock(sessions_mutex_);
    
    std::vector<SessionInfo> result;
    for (const auto& pair : sessions_) {
        result.push_back(pair.second);
    }
    
    return result;
}

uint64_t SessionManager::generate_session_id() {
    return next_session_id_++;
}

bool SessionManager::is_session_expired(const SessionInfo& info) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - info.last_activity).count();
    
    return elapsed > session_timeout_seconds_;
}
