#include "exo_chaha.hpp"
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

//#include <iostream>
#include <sodium.h>
//#include <string>
//#include <vector>
//#include <sstream>
//#include <iomanip>

using namespace godot;

void ExoChaCha::_bind_methods() {
    UtilityFunctions::print("ExoChaCha");
    ClassDB::bind_method(D_METHOD("generate_key"), &ExoChaCha::generate_key);
    ClassDB::bind_method(D_METHOD("generate_nonce"), &ExoChaCha::generate_nonce);
    ClassDB::bind_method(D_METHOD("encrypt_data", "key", "nonce", "raw_data"), &ExoChaCha::encrypt_data);
    ClassDB::bind_method(D_METHOD("decrypt_data", "key", "nonce", "encrypted_data"), &ExoChaCha::decrypt_data);

}

ExoChaCha::ExoChaCha() {
    if(sodium_init() == -1){
        UtilityFunctions::print("sodium_init() failed");
        return;
    }
    UtilityFunctions::print("ExoChaCha Working!");
}

ExoChaCha::~ExoChaCha() {}

PackedByteArray ExoChaCha::generate_key() {
    PackedByteArray byte_array;
    int size = crypto_aead_chacha20poly1305_KEYBYTES;
    unsigned char key[size];
    crypto_aead_chacha20poly1305_keygen(key);
    byte_array.resize(size);
    memcpy(byte_array.ptrw(), key, size);
    return byte_array;
}

PackedByteArray ExoChaCha::generate_nonce() {
    PackedByteArray byte_array;
    int size = crypto_aead_chacha20poly1305_NPUBBYTES;
    unsigned char nonce[size];
    randombytes_buf(nonce, size);
    byte_array.resize(size);
    memcpy(byte_array.ptrw(), nonce, size);
    return byte_array;
}

PackedByteArray ExoChaCha::encrypt_data(PackedByteArray key, PackedByteArray nonce, String raw_data) {
    PackedByteArray byte_array;
    int64_t plaintext_len = raw_data.length();
    int64_t ciphertext_len = plaintext_len + crypto_aead_chacha20poly1305_ABYTES;
    byte_array.resize(ciphertext_len);

    if (crypto_aead_chacha20poly1305_encrypt(byte_array.ptrw(), nullptr,
                                            (const unsigned char*)raw_data.utf8().get_data(), plaintext_len,
                                            nullptr, 0,
                                            nullptr, // nsec
                                            nonce.ptr(), key.ptr())) {
        UtilityFunctions::print("crypto_aead_chacha20poly1305_encrypt() failed");
        return byte_array;
    }
    return byte_array;
}

PackedByteArray ExoChaCha::decrypt_data(PackedByteArray key, PackedByteArray nonce, PackedByteArray encrypted_data) {
    int encrypted_size = encrypted_data.size();
    int tag_size = crypto_aead_chacha20poly1305_ABYTES;

    // Check if the encrypted data is long enough to contain the tag
    if (encrypted_size < tag_size) {
        UtilityFunctions::print("Error: Encrypted data is too short to contain the authentication tag.");
        return PackedByteArray(); // Return an empty PackedByteArray on error
    }

    int decrypted_size = encrypted_size - tag_size;
    PackedByteArray decrypted_byte_array;
    decrypted_byte_array.resize(decrypted_size);

    if (crypto_aead_chacha20poly1305_decrypt(decrypted_byte_array.ptrw(), nullptr, nullptr,
                                            encrypted_data.ptr(), encrypted_size,
                                            nullptr, 0,
                                            nonce.ptr(), key.ptr()) != 0) {
        UtilityFunctions::print("crypto_aead_chacha20poly1305_decrypt() failed");
        return PackedByteArray(); // Return an empty PackedByteArray on failure
    }

    return decrypted_byte_array;
}