#ifndef EXO_CHACHA_HPP
#define EXO_CHACHA_HPP

#include <godot_cpp/classes/ref_counted.hpp>


namespace godot {

    class ExoChaCha : public RefCounted{
        GDCLASS(ExoChaCha, RefCounted);
        protected:
        static void _bind_methods();
        
        public:
        ExoChaCha();
        ~ExoChaCha();
        PackedByteArray generate_key();
        PackedByteArray generate_nonce();
        PackedByteArray encrypt_data(PackedByteArray key, PackedByteArray nonce, String raw_data);
        PackedByteArray decrypt_data(PackedByteArray key, PackedByteArray nonce, PackedByteArray encrypted_data);
    };

}

#endif