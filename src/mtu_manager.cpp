#include "mtu_manager.h"
#include <algorithm>
#include <numeric>

MTUManager::MTUManager(int initial_mtu)
    : current_mtu_(initial_mtu), min_mtu_(MIN_MTU), max_mtu_(MAX_MTU),
      fragmented_packets_(0), reassembled_packets_(0), total_size_recorded_(0) {
    
    if (!is_valid_mtu(initial_mtu)) {
        current_mtu_ = DEFAULT_MTU;
    }
}

MTUManager::~MTUManager() {
    packet_sizes_.clear();
}

bool MTUManager::set_mtu(int mtu) {
    std::unique_lock<std::mutex> lock(mtu_mutex_);
    
    if (is_valid_mtu(mtu)) {
        current_mtu_ = mtu;
        return true;
    }
    
    return false;
}

int MTUManager::get_mtu() const {
    std::unique_lock<std::mutex> lock(mtu_mutex_);
    return current_mtu_;
}

void MTUManager::record_packet_size(size_t size) {
    std::unique_lock<std::mutex> lock(mtu_mutex_);
    
    packet_sizes_.push_back(size);
    total_size_recorded_ += size;
    
    // Keep only last 1000 measurements
    if (packet_sizes_.size() > 1000) {
        total_size_recorded_ -= packet_sizes_.front();
        packet_sizes_.erase(packet_sizes_.begin());
    }
}

int MTUManager::get_optimal_mtu() const {
    std::unique_lock<std::mutex> lock(mtu_mutex_);
    
    if (packet_sizes_.empty()) {
        return current_mtu_;
    }
    
    size_t max_size = *std::max_element(packet_sizes_.begin(), packet_sizes_.end());
    
    // Round up to common MTU sizes
    if (max_size <= 576) return 576;
    else if (max_size <= 1280) return 1280;
    else if (max_size <= 1500) return 1500;
    else if (max_size <= 9000) return 9000;
    else return MAX_MTU;
}

std::vector<std::vector<uint8_t>> MTUManager::fragment_packet(
    const std::vector<uint8_t>& data) {
    
    std::vector<std::vector<uint8_t>> fragments;
    
    if (data.size() <= static_cast<size_t>(current_mtu_)) {
        fragments.push_back(data);
        return fragments;
    }
    
    // Fragment the packet
    size_t offset = 0;
    size_t payload_size = static_cast<size_t>(current_mtu_) - 20; // Reserve 20 bytes for headers
    
    while (offset < data.size()) {
        size_t chunk_size = std::min(payload_size, data.size() - offset);
        fragments.push_back(
            std::vector<uint8_t>(data.begin() + offset, data.begin() + offset + chunk_size)
        );
        offset += chunk_size;
    }
    
    fragmented_packets_++;
    return fragments;
}

std::vector<uint8_t> MTUManager::reassemble_packet(
    const std::vector<std::vector<uint8_t>>& fragments) {
    
    std::vector<uint8_t> result;
    
    for (const auto& fragment : fragments) {
        result.insert(result.end(), fragment.begin(), fragment.end());
    }
    
    if (fragments.size() > 1) {
        reassembled_packets_++;
    }
    
    return result;
}

bool MTUManager::validate_mtu_for_udpgw() {
    // MTU must be at least 576 and must account for UDP/IP headers
    return current_mtu_ >= 576 && current_mtu_ <= MAX_MTU;
}

int MTUManager::get_udpgw_mtu() const {
    // Return MTU suitable for UDP GW protocol
    // Account for 28 bytes of IP+UDP headers
    int effective_mtu = current_mtu_ - 28;
    return std::max(effective_mtu, 548); // Minimum for DNS
}

size_t MTUManager::get_avg_packet_size() const {
    std::unique_lock<std::mutex> lock(mtu_mutex_);
    
    if (packet_sizes_.empty()) {
        return 0;
    }
    
    return total_size_recorded_ / packet_sizes_.size();
}

bool MTUManager::is_valid_mtu(int mtu) {
    return mtu >= min_mtu_ && mtu <= max_mtu_;
}
