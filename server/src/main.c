#include "../includes/version.h"
#include "../includes/common.h"


#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string.h>

#define KEY_LENGTH 2048
#define PUB_EXP 65537
#define PADDING RSA_PKCS1_OAEP_PADDING

void handle_errors() {
    ERR_print_errors_fp(stderr);
    abort();
}

// Generate RSA Key Pair
void generate_keys(const char *pub_key_file, const char *priv_key_file) {
    RSA *rsa = RSA_new();
    BIGNUM *bn = BN_new();
    BN_set_word(bn, PUB_EXP);

    if (RSA_generate_key_ex(rsa, KEY_LENGTH, bn, NULL) != 1)
        handle_errors();

    FILE *pub = fopen(pub_key_file, "wb");
    PEM_write_RSA_PUBKEY(pub, rsa);
    fclose(pub);

    FILE *priv = fopen(priv_key_file, "wb");
    PEM_write_RSAPrivateKey(priv, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(priv);

    RSA_free(rsa);
    BN_free(bn);

    printf("Keys generated and saved.\n");
}

// Encrypt with Public Key
int encrypt_message(const char *pub_key_file, unsigned char *message, unsigned char *encrypted) {
    FILE *pub = fopen(pub_key_file, "rb");
    RSA *rsa = PEM_read_RSA_PUBKEY(pub, NULL, NULL, NULL);
    fclose(pub);

    int result = RSA_public_encrypt(strlen((char*)message), message, encrypted, rsa, PADDING);
    if (result == -1) handle_errors();
    RSA_free(rsa);
    return result;
}

// Decrypt with Private Key
int decrypt_message(const char *priv_key_file, unsigned char *encrypted, int encrypted_length, unsigned char *decrypted) {
    FILE *priv = fopen(priv_key_file, "rb");
    RSA *rsa = PEM_read_RSAPrivateKey(priv, NULL, NULL, NULL);
    fclose(priv);

    int result = RSA_private_decrypt(encrypted_length, encrypted, decrypted, rsa, PADDING);
    if (result == -1) handle_errors();
    RSA_free(rsa);
    return result;
}

int main(int argc, char *argv[]) {

    UNUSED(argc);
    UNUSED(argv);

    printf("CACA\n");

    const char *pub_key_file = "public.pem";
    const char *priv_key_file = "private.pem";

    generate_keys(pub_key_file, priv_key_file);

    unsigned char message[] = "Hello secure world!";
    unsigned char encrypted[KEY_LENGTH];
    unsigned char decrypted[KEY_LENGTH];

    int encrypted_length = encrypt_message(pub_key_file, message, encrypted);
    printf("Encrypted message length: %d\n", encrypted_length);

    int decrypted_length = decrypt_message(priv_key_file, encrypted, encrypted_length, decrypted);
    decrypted[decrypted_length] = '\0';
    printf("Decrypted message: %s\n", decrypted);

    return 0;
}
