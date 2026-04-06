#pragma once

#include "common.h"
#include <vector>
#include <mutex>

class MTUManager {
public:
    explicit MTUManager(int initial_mtu = DEFAULT_MTU);
    ~MTUManager();
    
    // MTU management
    bool set_mtu(int mtu);
    int get_mtu() const;
    
    // MTU discovery
    void record_packet_size(size_t size);
    int get_optimal_mtu() const;
    
    // Fragment handling
    std::vector<std::vector<uint8_t>> fragment_packet(const std::vector<uint8_t>& data);
    std::vector<uint8_t> reassemble_packet(const std::vector<std::vector<uint8_t>>& fragments);
    
    // UDP GW compatibility
    bool validate_mtu_for_udpgw();
    int get_udpgw_mtu() const;
    
    // Statistics
    size_t get_avg_packet_size() const;
    int get_fragmented_packets() const { return fragmented_packets_; }
    int get_reassembled_packets() const { return reassembled_packets_; }
    
private:
    // MTU validation
    bool is_valid_mtu(int mtu);
    
    // Member variables
    int current_mtu_;
    int min_mtu_;
    int max_mtu_;
    
    std::vector<size_t> packet_sizes_;
    mutable std::mutex mtu_mutex_;
    
    int fragmented_packets_;
    int reassembled_packets_;
    size_t total_size_recorded_;
};
