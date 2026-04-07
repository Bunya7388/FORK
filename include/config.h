#pragma once

#include "common.h"
#include <string>
#include <map>
#include <vector>

class Config {
public:
    Config();
    ~Config();
    
    // Configuration loading
    bool load_from_file(const std::string& filepath);
    bool load_from_defaults();
    
    // Configuration access
    int get_int(const std::string& key, int default_val = 0) const;
    std::string get_string(const std::string& key, const std::string& default_val = "") const;
    bool get_bool(const std::string& key, bool default_val = false) const;
    
    // Configuration setting
    void set_int(const std::string& key, int value);
    void set_string(const std::string& key, const std::string& value);
    void set_bool(const std::string& key, bool value);
    
    // Save configuration
    bool save_to_file(const std::string& filepath);
    
    // Validation
    bool validate();
    std::vector<std::string> get_errors() const { return errors_; }
    
    // Predefined keys
    static constexpr const char* PORT = "port";
    static constexpr const char* WORKERS = "worker_count";
    static constexpr const char* MTU = "mtu_size";
    static constexpr const char* BUFFER_SIZE_KEY = "buffer_size";
    static constexpr const char* SESSION_TIMEOUT = "session_timeout";
    static constexpr const char* LOG_LEVEL = "log_level";
    static constexpr const char* BIND_ADDRESS = "bind_address";
    static constexpr const char* PUBKEY_PATH = "pubkey_path";
    static constexpr const char* ENABLE_STATS = "enable_stats";
    static constexpr const char* STATS_INTERVAL = "stats_interval";
    
private:
    // Configuration storage
    std::map<std::string, std::string> config_map_;
    std::vector<std::string> errors_;
    
    // Parsing utilities
    std::vector<std::string> split_string(const std::string& str, char delimiter);
    std::string trim_string(const std::string& str);
};
