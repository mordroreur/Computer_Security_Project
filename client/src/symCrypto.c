#include "../includes/symCrypto.h"

int get_secure_random_bytes(unsigned char* buffer, size_t size) {
#ifdef __MINGW32__
    HCRYPTPROV hProvider = 0;
    if (!CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return -1;
    }

    if (!CryptGenRandom(hProvider, (DWORD)size, buffer)) {
        CryptReleaseContext(hProvider, 0);
        return -1;
    }

    CryptReleaseContext(hProvider, 0);
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;

    ssize_t result = read(fd, buffer, size);
    close(fd);

    if (result != (ssize_t)size) return -1;
#endif
    return 0;
}

int generate_random_keys(messaged_personne_t* np) {


    if (get_secure_random_bytes(np->send_K1, AES_KEY_LEN) != 0 ||
        get_secure_random_bytes(np->send_K2, AES_KEY_LEN) != 0 ||
        get_secure_random_bytes(np->send_K3, AES_KEY_LEN) != 0) {
        return -1;
    }
    return 0;
}


int dualEncrypt(const char* plaintext, size_t len,
    messaged_personne_t* per,
    unsigned char** encrypted, size_t* encrypted_size) {
    
    int lon = 0;
    if (len > 12) {
        len += 8;
        lon = 1;
    }

    size_t padded_len = ((len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

    unsigned char nonce[CHACHA20_NONCE_LEN];
    memset(nonce, 0, CHACHA20_NONCE_LEN);
    sprintf((char*)nonce, "this:%d", per->send_nonce);

    // Prepare and pad plaintext
    unsigned char *padded_plain = malloc(padded_len);
    if (!padded_plain) return -1;
    memset(padded_plain, 0, padded_len);
    if (lon == 1) {
        memcpy(padded_plain, plaintext, 12);
        memcpy(padded_plain + 16, plaintext + 12, len - 12);
    } else {
        memcpy(padded_plain, plaintext, len);
    }

    // Step 1: ChaCha20 encrypt
    unsigned char *chacha_out = malloc(padded_len);
    if (!chacha_out) return -1;
    crypto_stream_chacha20_xor(chacha_out, padded_plain, padded_len, nonce, per->send_K1);

    // Step 2: AES-CBC encrypt ChaCha20 output
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    unsigned char *aes_out = malloc(padded_len + AES_BLOCK_SIZE);
    if (!aes_out) return -1;
    memset(aes_out, 0, padded_len + AES_BLOCK_SIZE);

    int outlen = 0, tmplen = 0;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, per->send_K2, nonce);
    EVP_EncryptUpdate(ctx, aes_out, &outlen, chacha_out, padded_len);
    EVP_EncryptFinal_ex(ctx, aes_out + outlen, &tmplen);
    outlen += tmplen;

    // Step 3: Append 4-byte original length after AES output (before XOR)
    *encrypted_size = outlen + 4;
    unsigned char *aes_with_len = malloc(*encrypted_size);
    if (!aes_with_len) return -1;
    memcpy(aes_with_len, aes_out, outlen);

    uint32_t len_net = htonl(len);
    memcpy(aes_with_len + outlen, &len_net, 4);

    // Step 4: XOR the combined AES output + len
    unsigned char *keystream = malloc(*encrypted_size);
    if (!keystream) return -1;
    crypto_stream_chacha20(keystream, *encrypted_size, nonce, per->send_K3);

    *encrypted = malloc(*encrypted_size);
    if (!(*encrypted)) return -1;
    for (size_t i = 0; i < *encrypted_size; i++) {
        (*encrypted)[i] = aes_with_len[i] ^ keystream[i];
    }

    per->send_nonce += strlen(plaintext);

    EVP_CIPHER_CTX_free(ctx);
    free(padded_plain);
    free(chacha_out);
    free(aes_out);
    free(aes_with_len);
    free(keystream);

    return 0;
}



int dualDecrypt(const unsigned char* ciphertext, size_t len,
    messaged_personne_t* per,
    char** plaintext) {

    if (len < 4) return -1;

    unsigned char nonce[CHACHA20_NONCE_LEN];
    memset(nonce, 0, CHACHA20_NONCE_LEN);
    sprintf((char *)nonce, "this:%d", per->receive_nonce);

    // Step 1: XOR the entire buffer to undo outermost encryption
    unsigned char *keystream = malloc(len);
    unsigned char *full_decrypted = malloc(len);
    if (!keystream || !full_decrypted) return -1;

    crypto_stream_chacha20(keystream, len, nonce, per->rcv_K3);
    for (size_t i = 0; i < len; i++) {
        full_decrypted[i] = ciphertext[i] ^ keystream[i];
    }

    // Step 2: Extract the original message length from the last 4 bytes
    if (len < 4) return -1;
    uint32_t len_net;
    memcpy(&len_net, full_decrypted + len - 4, 4);
    size_t original_len = ntohl(len_net);

    size_t aes_len = len - 4;

    // Step 3: AES decrypt
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char *chacha_out = malloc(aes_len);
    if (!ctx || !chacha_out) return -1;

    int outlen = 0, tmplen = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, per->rcv_K2, nonce);
    EVP_DecryptUpdate(ctx, chacha_out, &outlen, full_decrypted, aes_len);
    EVP_DecryptFinal_ex(ctx, chacha_out + outlen, &tmplen);
    outlen += tmplen;

    // Step 4: ChaCha20 decryption
    unsigned char *full_plain = malloc(outlen);
    if (!full_plain) return -1;
    crypto_stream_chacha20_xor(full_plain, chacha_out, outlen, nonce, per->rcv_K1);

    // Step 5: Recover original message
    if (original_len > 16) {
        *plaintext = malloc(original_len - 4);
        if (!*plaintext) return -1;
        memcpy(*plaintext, full_plain, 12);
        memcpy(*plaintext + 12, full_plain + 16, original_len - 12);
        (*plaintext)[original_len - 4] = '\0';
    } else {
        *plaintext = malloc(original_len + 1);
        if (!*plaintext) return -1;
        memcpy(*plaintext, full_plain, original_len);
        (*plaintext)[original_len] = '\0';
    }

    per->receive_nonce += strlen(*plaintext);

    EVP_CIPHER_CTX_free(ctx);
    free(keystream);
    free(full_decrypted);
    free(chacha_out);
    free(full_plain);

    return 0;
}
