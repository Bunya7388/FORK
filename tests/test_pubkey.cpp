#include "pubkey_generator.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <cstdio>

int main() {
    std::cout << "Testing PUBKEY Generator...\n";
    
    PubKeyGenerator keygen;
    
    // Test 1: Generate key
    PubKey key1 = keygen.generate_key();
    assert(keygen.validate_key(key1));
    
    std::cout << "✓ Generated 32-byte HEX PUBKEY\n";
    std::cout << "  HEX: " << key1.hex_string.substr(0, 32) << "..." << "\n";
    std::cout << "  Fingerprint: " << key1.fingerprint << "\n";
    
    assert(key1.hex_string.length() == 64); // 32 bytes = 64 hex chars
    assert(key1.fingerprint.length() == 32); // 16 bytes = 32 hex chars
    
    // Test 2: Generate key from seed
    PubKey key2 = keygen.generate_key_from_seed("test_seed_12345");
    assert(keygen.validate_key(key2));
    
    std::cout << "✓ Generated key from seed\n";
    
    // Test 3: Hex conversion
    std::string hex = keygen.key_to_hex(key1.key, 32);
    assert(hex.length() == 64);
    assert(keygen.validate_hex_key(hex));
    
    std::cout << "✓ Hex key conversion validated\n";
    
    // Test 4: Save and load key
    std::string keyfile = "/tmp/test_pubkey.bin";
    assert(keygen.save_key(key1, keyfile));
    
    PubKey key3;
    assert(keygen.load_key(keyfile, key3));
    
    // Verify loaded key matches original
    std::string hex1 = keygen.key_to_hex(key1.key, 32);
    std::string hex3 = keygen.key_to_hex(key3.key, 32);
    assert(hex1 == hex3);
    
    std::cout << "✓ Key save/load functionality working\n";
    
    // Cleanup
    std::remove(keyfile.c_str());
    
    // Test 5: Fingerprint generation
    std::string fp1 = keygen.generate_fingerprint(key1.key, 64);
    assert(fp1.length() == 32);
    
    std::cout << "✓ Fingerprint generation verified\n";
    
    std::cout << "All PUBKEY tests passed!\n";
    return 0;
}
