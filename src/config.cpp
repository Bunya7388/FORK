#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

Config::Config() {
    load_from_defaults();
}

Config::~Config() {
    config_map_.clear();
    errors_.clear();
}

bool Config::load_from_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        errors_.push_back("Cannot open configuration file: " + filepath);
        return false;
    }
    
    std::string line;
    int line_num = 0;
    
    while (std::getline(file, line)) {
        line_num++;
        
        // Trim whitespace
        line = trim_string(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Find separator
        size_t sep_pos = line.find('=');
        if (sep_pos == std::string::npos) {
            errors_.push_back("Invalid line at " + std::to_string(line_num));
            continue;
        }
        
        std::string key = trim_string(line.substr(0, sep_pos));
        std::string value = trim_string(line.substr(sep_pos + 1));
        
        if (key.empty()) {
            errors_.push_back("Empty key at line " + std::to_string(line_num));
            continue;
        }
        
        config_map_[key] = value;
    }
    
    file.close();
    return true;
}

bool Config::load_from_defaults() {
    config_map_[PORT] = std::to_string(DEFAULT_PORT);
    config_map_[WORKERS] = std::to_string(DEFAULT_WORKERS);
    config_map_[MTU] = std::to_string(DEFAULT_MTU);
    config_map_[BUFFER_SIZE_KEY] = std::to_string(BUFFER_SIZE);
    config_map_[SESSION_TIMEOUT] = std::to_string(SESSION_TIMEOUT_SECONDS);
    config_map_[LOG_LEVEL] = "INFO";
    config_map_[BIND_ADDRESS] = "0.0.0.0";
    config_map_[PUBKEY_PATH] = "/etc/dnstt-udp/pubkey";
    config_map_[ENABLE_STATS] = "true";
    config_map_[STATS_INTERVAL] = "5000"; // 5 seconds
    
    return true;
}

int Config::get_int(const std::string& key, int default_val) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return default_val;
        }
    }
    return default_val;
}

std::string Config::get_string(const std::string& key, const std::string& default_val) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        return it->second;
    }
    return default_val;
}

bool Config::get_bool(const std::string& key, bool default_val) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        
        if (value == "true" || value == "1" || value == "yes" || value == "on") {
            return true;
        } else if (value == "false" || value == "0" || value == "no" || value == "off") {
            return false;
        }
    }
    return default_val;
}

void Config::set_int(const std::string& key, int value) {
    config_map_[key] = std::to_string(value);
}

void Config::set_string(const std::string& key, const std::string& value) {
    config_map_[key] = value;
}

void Config::set_bool(const std::string& key, bool value) {
    config_map_[key] = value ? "true" : "false";
}

bool Config::save_to_file(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        errors_.push_back("Cannot open file for writing: " + filepath);
        return false;
    }
    
    file << "# DNS-over-UDP Tunneling Server Configuration\n\n";
    
    for (const auto& pair : config_map_) {
        file << pair.first << " = " << pair.second << "\n";
    }
    
    file.close();
    return true;
}

bool Config::validate() {
    errors_.clear();
    
    int port = get_int(PORT);
    if (port < 1 || port > 65535) {
        errors_.push_back("Invalid port: " + std::to_string(port));
    }
    
    int workers = get_int(WORKERS);
    if (workers < MIN_WORKERS || workers > MAX_WORKERS) {
        errors_.push_back("Worker count must be between " + std::to_string(MIN_WORKERS) + 
                         " and " + std::to_string(MAX_WORKERS));
    }
    
    int mtu = get_int(MTU);
    if (mtu < MIN_MTU || mtu > MAX_MTU) {
        errors_.push_back("MTU must be between " + std::to_string(MIN_MTU) + 
                         " and " + std::to_string(MAX_MTU));
    }
    
    int timeout = get_int(SESSION_TIMEOUT);
    if (timeout < 10 || timeout > 3600) {
        errors_.push_back("Session timeout must be between 10 and 3600 seconds");
    }
    
    return errors_.empty();
}

std::vector<std::string> Config::split_string(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    
    return result;
}

std::string Config::trim_string(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }
    
    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    
    return std::string(start, end + 1);
}
