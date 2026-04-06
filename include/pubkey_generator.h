#pragma once

#include "common.h"
#include <string>
#include <vector>
#include <openssl/evp.h>

class PubKeyGenerator {
public:
    PubKeyGenerator();
    ~PubKeyGenerator();
    
    // Key generation
    PubKey generate_key();
    PubKey generate_key_from_seed(const std::string& seed);
    
    // Key management
    bool save_key(const PubKey& key, const std::string& filepath);
    bool load_key(const std::string& filepath, PubKey& key);
    
    // Key operations
    std::string key_to_hex(const uint8_t* key, size_t len);
    bool hex_to_key(const std::string& hex, uint8_t* key, size_t max_len);
    
    // Fingerprint generation
    std::string generate_fingerprint(const uint8_t* key, size_t len);
    
    // Validation
    bool validate_key(const PubKey& key);
    bool validate_hex_key(const std::string& hex_key);
    
    // Constants
    static constexpr size_t KEY_SIZE = 64;
    static constexpr size_t HEX_KEY_SIZE = KEY_SIZE * 2;
    
private:
    // Key generation helper
    bool fill_random_bytes(uint8_t* buffer, size_t size);
    
    // Member variables
    EVP_PKEY_CTX* pkey_ctx_;
};
