#include "udp_server.h"
#include "config.h"
#include "pubkey_generator.h"
#include <iostream>
#include <iomanip>
#include <csignal>
#include <atomic>
#include <thread>
#include <cstring>

static std::atomic<bool> shutdown_signal(false);

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        shutdown_signal = true;
    }
}

void print_usage(const char* program) {
    std::cout << "DNS-over-UDP Tunneling Server v1.0.0\n\n";
    std::cout << "Usage: " << program << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -p, --port PORT              listening port (default: 5300)\n";
    std::cout << "  -w, --workers WORKERS        number of worker threads 1-256 (default: 4)\n";
    std::cout << "  -m, --mtu MTU                MTU size in bytes (default: 1500)\n";
    std::cout << "  -b, --buffer-size SIZE       socket buffer size in bytes (default: 4MB)\n";
    std::cout << "  -c, --config FILE            load configuration file\n";
    std::cout << "  -g, --generate-key           generate and save 32-byte hex PUBKEY\n";
    std::cout << "  -k, --pubkey-file FILE       save generated key to file\n";
    std::cout << "  -s, --show-key               display current pubkey information\n";
    std::cout << "  -h, --help                   show this help message\n";
    std::cout << "  -v, --version                show version information\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program << " -p 5300 -w 8\n";
    std::cout << "  " << program << " -c /etc/dnstt-udp.conf\n";
    std::cout << "  " << program << " -g -k /etc/dnstt-udp/pubkey\n";
}

void print_version() {
    std::cout << "DNS-over-UDP Tunneling Server v1.0.0\n";
    std::cout << "Copyright 2025 - High-Speed DNS Tunneling\n";
}

void display_statistics(UDPServer& server) {
    PerfStats stats = server.get_stats();
    
    std::cout << "\n======== Performance Statistics ========\n";
    std::cout << std::setw(30) << "Packets Received: " << stats.packets_received << "\n";
    std::cout << std::setw(30) << "Packets Sent: " << stats.packets_sent << "\n";
    std::cout << std::setw(30) << "Bytes Received: " << stats.bytes_received << " bytes\n";
    std::cout << std::setw(30) << "Bytes Sent: " << stats.bytes_sent << " bytes\n";
    std::cout << std::setw(30) << "Packets/sec: " << std::fixed << std::setprecision(2) 
              << stats.packets_per_sec << "\n";
    std::cout << std::setw(30) << "Throughput: " << stats.throughput_mbps << " Mbps\n";
    std::cout << std::setw(30) << "Active Sessions: " << stats.sessions_active << "\n";
    std::cout << std::setw(30) << "Errors: " << stats.errors << "\n";
    std::cout << "==========================================\n";
}

void statistics_thread_func(UDPServer& server, int interval_ms) {
    while (!shutdown_signal.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        if (shutdown_signal.load()) break;
        display_statistics(server);
    }
}

int main(int argc, char* argv[]) {
    Config config;
    std::string config_file;
    bool generate_key = false;
    std::string pubkey_file;
    bool show_key = false;
    
    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            print_version();
            return 0;
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                config.set_int(Config::PORT, std::atoi(argv[++i]));
            }
        } else if (arg == "-w" || arg == "--workers") {
            if (i + 1 < argc) {
                int workers = std::atoi(argv[++i]);
                if (workers < MIN_WORKERS || workers > MAX_WORKERS) {
                    std::cerr << "Error: worker count must be between " << MIN_WORKERS 
                              << " and " << MAX_WORKERS << "\n";
                    return 1;
                }
                config.set_int(Config::WORKERS, workers);
            }
        } else if (arg == "-m" || arg == "--mtu") {
            if (i + 1 < argc) {
                int mtu = std::atoi(argv[++i]);
                if (mtu < MIN_MTU || mtu > MAX_MTU) {
                    std::cerr << "Error: MTU must be between " << MIN_MTU 
                              << " and " << MAX_MTU << "\n";
                    return 1;
                }
                config.set_int(Config::MTU, mtu);
            }
        } else if (arg == "-b" || arg == "--buffer-size") {
            if (i + 1 < argc) {
                config.set_int(Config::BUFFER_SIZE_KEY, std::atoi(argv[++i]));
            }
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
                if (!config.load_from_file(config_file)) {
                    std::cerr << "Error: failed to load configuration file: " << config_file << "\n";
                    return 1;
                }
            }
        } else if (arg == "-g" || arg == "--generate-key") {
            generate_key = true;
        } else if (arg == "-k" || arg == "--pubkey-file") {
            if (i + 1 < argc) {
                pubkey_file = argv[++i];
            }
        } else if (arg == "-s" || arg == "--show-key") {
            show_key = true;
        }
    }
    
    // Validate configuration
    if (!config.validate()) {
        std::cerr << "Configuration validation failed:\n";
        for (const auto& error : config.get_errors()) {
            std::cerr << "  - " << error << "\n";
        }
        return 1;
    }
    
    // Handle key generation
    if (generate_key) {
        PubKeyGenerator keygen;
        PubKey key = keygen.generate_key();
        
        std::cout << "Generated 32-Byte HEX PUBKEY:\n";
        std::cout << "========================================\n";
        std::cout << "HEX Key: " << key.hex_string << "\n";
        std::cout << "Fingerprint: " << key.fingerprint << "\n";
        std::cout << "========================================\n";
        
        if (!pubkey_file.empty()) {
            if (keygen.save_key(key, pubkey_file)) {
                std::cout << "Key saved to: " << pubkey_file << "\n";
            } else {
                std::cerr << "Error: failed to save key to " << pubkey_file << "\n";
                return 1;
            }
        }
        return 0;
    }
    
    // Handle key display
    if (show_key && !pubkey_file.empty()) {
        PubKeyGenerator keygen;
        PubKey key;
        
        if (keygen.load_key(pubkey_file, key)) {
            std::cout << "Loaded PUBKEY Information:\n";
            std::cout << "========================================\n";
            std::cout << "HEX Key: " << key.hex_string << "\n";
            std::cout << "Fingerprint: " << key.fingerprint << "\n";
            std::cout << "========================================\n";
        } else {
            std::cerr << "Error: failed to load key from " << pubkey_file << "\n";
            return 1;
        }
        return 0;
    }
    
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Get configuration values
    int port = config.get_int(Config::PORT);
    int workers = config.get_int(Config::WORKERS);
    int mtu = config.get_int(Config::MTU);
    
    // Create and initialize server
    std::cout << "Starting DNS-over-UDP Tunneling Server\n";
    std::cout << "Port: " << port << "\n";
    std::cout << "Workers: " << workers << "\n";
    std::cout << "MTU: " << mtu << " bytes\n";
    std::cout << "========================================\n\n";
    
    UDPServer server(port, mtu, workers);
    
    if (!server.initialize()) {
        std::cerr << "Fatal error: failed to initialize server\n";
        return 1;
    }
    
    if (!server.start()) {
        std::cerr << "Fatal error: failed to start server\n";
        return 1;
    }
    
    // Start statistics display thread
    int stats_interval = config.get_int(Config::STATS_INTERVAL, 5000);
    std::thread stats_thread(statistics_thread_func, std::ref(server), stats_interval);
    
    // Main loop - wait for shutdown signal
    while (!shutdown_signal.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\nShutting down server...\n";
    
    // Display final statistics
    display_statistics(server);
    
    server.stop();
    
    if (stats_thread.joinable()) {
        stats_thread.join();
    }
    
    std::cout << "Server stopped gracefully\n";
    return 0;
}
