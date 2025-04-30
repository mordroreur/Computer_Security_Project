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
    if(len > 12){
        len += 8;
        lon = 1;
    }

    size_t padded_len = ((len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;

    unsigned char nonce[CHACHA20_NONCE_LEN];
    memset(nonce, 0, CHACHA20_NONCE_LEN);
    sprintf((char *)nonce, "this:%d", per->send_nonce);
    //sprintf((char *)nonce, "this 0");


    // Step 0: Pad plaintext
    unsigned char *padded_plain = malloc(padded_len);
    if (!padded_plain) return -1;
    memset(padded_plain, 0, padded_len);
    if(lon == 1){
        memcpy(padded_plain, plaintext, 12);
        memcpy(padded_plain+16, plaintext+12, len-12);
    }else{
       memcpy(padded_plain, (unsigned char*)plaintext, len); 
    }

    // Debugging step to print the content of the encrypted buffer
    /*for (int i = 0; i < padded_len; i++) {
        printf("Encrypted buffer[%d]: %02x\n", i, (padded_plain)[i]);
    }*/

    


    // Step 1: ChaCha20 encrypt plaintext
    unsigned char *chacha_out = malloc(padded_len);
    if (!chacha_out) return -1;
    crypto_stream_chacha20_xor(chacha_out, padded_plain, padded_len, nonce, per->send_K1);

    // Step 2: AES-CBC encrypt the ChaCha20 output
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    unsigned char *aes_out = malloc(padded_len + AES_BLOCK_SIZE);
    if (!aes_out) return -1;
    memset(aes_out, 0, padded_len + AES_BLOCK_SIZE);

    int outlen = 0, tmplen = 0;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, per->send_K2, nonce);
    EVP_EncryptUpdate(ctx, aes_out, &outlen, chacha_out, padded_len);
    EVP_EncryptFinal_ex(ctx, aes_out + outlen, &tmplen);

    //printf("outlen after EVP_EncryptUpdate: %d\n", outlen);
    //printf("outlen after EVP_EncryptFinal_ex: %d\n", outlen + tmplen);

    outlen += tmplen;


    // Step 3: Generate ChaCha20 keystream with K3
    unsigned char *keystream = malloc(outlen);
    if (!keystream) return -1;
    crypto_stream_chacha20(keystream, outlen, nonce, per->send_K3);

    // Step 4: Allocate output buffer with 4-byte size prefix
    *encrypted_size = outlen + 4;
    *encrypted = malloc(sizeof(char)*(*encrypted_size));
    if (!(*encrypted)) return -1;
    memset(*encrypted, 0, *encrypted_size);

    uint32_t len_net = htonl(len); // Store original message length
    memcpy(*encrypted, &len_net, 4);
    //printf("Allocated encrypted buffer with size: %d\n", *encrypted_size);
    //printf("outlen after AES encryption: %d\n", outlen);

    for (int i = 0; i < outlen; i++) {
        (*encrypted)[4 + i] = aes_out[i] ^ keystream[i];
    }

    // Debugging step to print the content of the encrypted buffer
    /*for (int i = 0; i < padded_len; i++) {
        printf("Encrypted buffer[%d]: %02x\n", i, (padded_plain)[i]);
    }*/

    per->send_nonce += strlen(plaintext);

    // Cleanup
    EVP_CIPHER_CTX_free(ctx);
    free(chacha_out);
    free(padded_plain);
    free(aes_out);
    free(keystream);

    return 0;
}


int dualDecrypt(const unsigned char* ciphertext, size_t len,
    messaged_personne_t* per,
    char** plaintext) {

    if (len < 4) return -1; // Not enough data for even the length header

    // Step 0: Extract original length from first 4 bytes
    uint32_t original_len_net;
    memcpy(&original_len_net, ciphertext, 4);
    size_t original_len = ntohl(original_len_net);

    size_t encrypted_len = len - 4;

    unsigned char nonce[CHACHA20_NONCE_LEN];
    memset(nonce, 0, CHACHA20_NONCE_LEN);
    sprintf((char *)nonce, "this:%d", per->receive_nonce);
    //sprintf((char *)nonce, "this 0");

    // Step 1: Reconstruct keystream
    unsigned char *keystream = malloc(encrypted_len);
    unsigned char *aes_out = malloc(encrypted_len);
    if (!keystream || !aes_out) return -1;

    crypto_stream_chacha20(keystream, encrypted_len, nonce, per->rcv_K3);

    // Step 2: Undo XOR to get AES ciphertext
    for (size_t i = 0; i < encrypted_len; i++) {
        aes_out[i] = ciphertext[4 + i] ^ keystream[i];
    }

    // Step 3: AES decrypt
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char *chacha_out = malloc(encrypted_len);
    if (!ctx || !chacha_out) return -1;


    int outlen = 0, tmplen = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, per->rcv_K2, nonce);
    EVP_DecryptUpdate(ctx, chacha_out, &outlen, aes_out, encrypted_len);
    EVP_DecryptFinal_ex(ctx, chacha_out + outlen, &tmplen);
    outlen += tmplen;

    // Debugging step to print the content of the encrypted buffer

    


    // Step 4: ChaCha20 decrypt full buffer, then truncate to original_len
    unsigned char *full_plain = malloc(outlen);
    if (!full_plain) return -1;

    crypto_stream_chacha20_xor(full_plain, chacha_out, outlen, nonce, per->rcv_K1);




    if(original_len > 16){
        *plaintext = malloc(original_len -4);
        if (!*plaintext) return -1;
        memcpy(*plaintext, full_plain, 12);
        memcpy(*plaintext+12, full_plain+16, original_len-12);
        (*plaintext)[original_len-3] = '\0'; // Null-terminate
    }else{
        *plaintext = malloc(sizeof(char)*(original_len+1));
        if (!*plaintext) return -1;
        memcpy(*plaintext, full_plain, original_len);
        (*plaintext)[original_len] = '\0'; // Null-terminate
    }

    

    per->receive_nonce += strlen(*plaintext);

    // Cleanup
    EVP_CIPHER_CTX_free(ctx);
    free(keystream);
    free(aes_out);
    free(chacha_out);
    free(full_plain);

    return 0;
}
