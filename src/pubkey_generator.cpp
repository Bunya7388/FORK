#include "pubkey_generator.h"
#include <openssl/rand.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <iostream>

PubKeyGenerator::PubKeyGenerator() : pkey_ctx_(nullptr) {
    OpenSSL_add_all_algorithms();
}

PubKeyGenerator::~PubKeyGenerator() {
    if (pkey_ctx_) {
        EVP_PKEY_CTX_free(pkey_ctx_);
    }
}

PubKey PubKeyGenerator::generate_key() {
    PubKey key;
    
    if (!fill_random_bytes(key.key, KEY_SIZE)) {
        return key;
    }
    
    key.hex_string = key_to_hex(key.key, KEY_SIZE);
    key.fingerprint = generate_fingerprint(key.key, KEY_SIZE);
    
    return key;
}

PubKey PubKeyGenerator::generate_key_from_seed(const std::string& seed) {
    PubKey key;
    
    // Use SHA-256 of seed as the key (32 bytes)
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, seed.data(), seed.length());
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    // Copy hash directly into key buffer
    std::memcpy(key.key, hash, std::min(size_t(hash_len), KEY_SIZE));
    if (hash_len < KEY_SIZE) {
        // Extra safety if SHA-256 output is shorter than expected (should not happen)
        std::memset(key.key + hash_len, 0, KEY_SIZE - hash_len);
    }
    
    key.hex_string = key_to_hex(key.key, KEY_SIZE);
    key.fingerprint = generate_fingerprint(key.key, KEY_SIZE);
    
    return key;
}

bool PubKeyGenerator::save_key(const PubKey& key, const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(key.key), KEY_SIZE);
    file.close();
    
    return true;
}

bool PubKeyGenerator::load_key(const std::string& filepath, PubKey& key) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.read(reinterpret_cast<char*>(key.key), KEY_SIZE);
    if (file.gcount() != static_cast<std::streamsize>(KEY_SIZE)) {
        return false;
    }
    
    file.close();
    
    key.hex_string = key_to_hex(key.key, KEY_SIZE);
    key.fingerprint = generate_fingerprint(key.key, KEY_SIZE);
    
    return true;
}

std::string PubKeyGenerator::key_to_hex(const uint8_t* key, size_t len) {
    std::stringstream ss;
    for (size_t i = 0; i < len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
    }
    return ss.str();
}

bool PubKeyGenerator::hex_to_key(const std::string& hex, uint8_t* key, size_t max_len) {
    if (hex.length() < HEX_KEY_SIZE || max_len < KEY_SIZE) {
        return false;
    }
    
    for (size_t i = 0; i < KEY_SIZE; ++i) {
        std::string byte_str = hex.substr(i * 2, 2);
        try {
            int byte_val = std::stoi(byte_str, nullptr, 16);
            key[i] = static_cast<uint8_t>(byte_val);
        } catch (...) {
            return false;
        }
    }
    
    return true;
}

std::string PubKeyGenerator::generate_fingerprint(const uint8_t* key, size_t len) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, key, len);
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    // Convert first 16 bytes to hex fingerprint
    std::stringstream ss;
    for (unsigned int i = 0; i < 16; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool PubKeyGenerator::validate_key(const PubKey& key) {
    // Basic validation - non-zero key
    for (size_t i = 0; i < KEY_SIZE; ++i) {
        if (key.key[i] != 0) {
            return true;
        }
    }
    return false;
}

bool PubKeyGenerator::validate_hex_key(const std::string& hex_key) {
    if (hex_key.length() != HEX_KEY_SIZE) {
        return false;
    }
    
    for (char c : hex_key) {
        if (!std::isxdigit(c)) {
            return false;
        }
    }
    
    return true;
}

bool PubKeyGenerator::fill_random_bytes(uint8_t* buffer, size_t size) {
    return RAND_bytes(buffer, size) == 1;
}
