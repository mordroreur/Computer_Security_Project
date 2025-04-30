#include "../includes/asymCrypto.h"


int loadPrivatePublicKeys(AppState* ap){
    char isKeyNotok = loadPrivateKeyFromFile(ap);

    if(isKeyNotok == 0){
        isKeyNotok = testRSAKey(ap);
    }
    if(isKeyNotok){
        RSAkeyGenerate(ap);
    }
    return 0;
}


int RSAencrypt(EVP_PKEY* key, const char* dec, unsigned char** enc, size_t* enc_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) return 1;

    if (EVP_PKEY_encrypt_init(ctx) <= 0) return 1;

    // Determine buffer length
    if (EVP_PKEY_encrypt(ctx, NULL, enc_len, (unsigned char*)dec, strlen(dec)) <= 0) return 1;

    *enc = (unsigned char*)malloc(*enc_len);
    if (!*enc) return 1;

    // Encrypt the message
    if (EVP_PKEY_encrypt(ctx, *enc, enc_len, (unsigned char*)dec, strlen(dec)) <= 0) return 1;

    EVP_PKEY_CTX_free(ctx);
    return 0;
}

int RSAdecrypt(EVP_PKEY* key, const unsigned char* enc, size_t enc_len, char** dec, size_t* dec_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) return 1;

    if (EVP_PKEY_decrypt_init(ctx) <= 0) return 1;

    // Determine buffer length
    if (EVP_PKEY_decrypt(ctx, NULL, dec_len, enc, enc_len) <= 0) return 1;

    *dec = (char*)malloc(*dec_len + 1); // +1 for null-terminator
    if (!*dec) return 1;

    // Decrypt the message
    if (EVP_PKEY_decrypt(ctx, (unsigned char*)*dec, dec_len, enc, enc_len) <= 0) return 1;

    (*dec)[*dec_len] = '\0'; // Null-terminate the string

    EVP_PKEY_CTX_free(ctx);
    return 0;
}

int testRSAKey(AppState* as){

    char* test = "Ceci est un text test";
    unsigned char* encrypted = NULL;
    size_t encrypted_len = 0;

    char* decrypted = NULL;
    size_t decrypted_len = 0;

    if (RSAencrypt(as->pub_key, test, &encrypted, &encrypted_len) != 0) {
        return 1;
    }

    if (RSAdecrypt(as->priv_key, encrypted, encrypted_len, &decrypted, &decrypted_len) != 0) {
        free(encrypted);
        return 1;
    }

    /*
    printf("%s\n", test);
    for (size_t i = 0; i < encrypted_len; ++i) {
        printf("%02X", encrypted[i]); // print each byte as two hex digits
        if (i < encrypted_len - 1) printf(" "); // optional: add space between bytes
    }
    printf("\n");
    printf("%s\n", decrypted);*/

    int result = strcmp(test, decrypted);

    free(encrypted);
    free(decrypted);

    return result;
}



int RSAkeyGenerate(AppState* as) {
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;

    // Create a context for key generation
    ctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
    if (!ctx) return 1;

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 1;
    }

    // Set the key size
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, KEY_LENGTH) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 1;
    }

    // Optional: Set the public exponent to 65537
    unsigned long pub_exp = 65537;
    OSSL_PARAM params[] = {
        OSSL_PARAM_ulong(OSSL_PKEY_PARAM_RSA_E, &pub_exp),
        OSSL_PARAM_END
    };
    if (EVP_PKEY_CTX_set_params(ctx, params) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 1;
    }

    // Generate the key
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return 1;
    }

    as->priv_key = pkey;

    // Also extract public key reference
    as->pub_key = EVP_PKEY_dup(pkey);  // safe duplicate

    writePrivateKeyToFile(as);

    EVP_PKEY_CTX_free(ctx);
    if(as->GUI == 0)
        printf("Keys generated successfully.\n");
    return 0;
}








int encryptToServ(const char* mes, size_t mesSize, unsigned char** encmess, size_t* encSize){

    char serverKey[] = "-----BEGIN PUBLIC KEY-----\n\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA72MM+7xT/rFT/yV/1mPJ\
T5VlauArrgaJ/Q4skWDIMJauBQQfEjio5RC8++4S/hRKd0UhD9fw0SbuLyV4QGli\
I+EoU+Vqmm1q+zlc+zIzXyB646UNlX5UVEbPX6SaQ5SnVUzUrIZDVxgnBFev3i/K\
Nk9nU9lP2s/t/gIiXuTAa6Xcjx4OS16VuPnLn7/g/GKATDj1Iu5AZjf6TY+3UuSg\
QL58fMn7Rutz6HtI/tyAc28GZ2GXjP3NvZ3L7tfj5qIU7vdrzNWOE5XLwYgzeyZD\
9HEwoqIwqCKy5YKL1iJDACZ4iiO2KFLpW2vx4Tlo9v3TPFdieiIhlaPHhlG86s5z\
IQIDAQAB\n\
-----END PUBLIC KEY-----";

    
    int ret = 1;
    EVP_PKEY* pubkey = NULL;
    BIO* bio = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    size_t outlen = 0;

    if (!mes || mesSize == 0 || !encmess || !encSize) {
        printf("Invalid input.\n");
        return 1;
    }

    *encmess = NULL;
    *encSize = 0;

    // Create a memory BIO from the PEM string
    bio = BIO_new_mem_buf(serverKey, -1);
    if (!bio) {
        printf("Failed to create BIO.\n");
        goto cleanup;
    }

    // Read the public key
    pubkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (!pubkey) {
        printf("Failed to read public key.\n");
        goto cleanup;
    }

    // Create encryption context
    ctx = EVP_PKEY_CTX_new(pubkey, NULL);
    if (!ctx) {
        printf("Failed to create context.\n");
        goto cleanup;
    }

    // Initialize encryption
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        printf("Failed to init encryption.\n");
        goto cleanup;
    }

    // Determine encrypted size
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (const unsigned char*)mes, mesSize) <= 0) {
        printf("Failed to determine encrypted size.\n");
        goto cleanup;
    }

    // Allocate memory
    *encmess = (unsigned char*)malloc(outlen);
    if (!*encmess) {
        printf("Failed to allocate memory.\n");
        goto cleanup;
    }


    // Perform encryption
    if (EVP_PKEY_encrypt(ctx, *encmess, &outlen, (const unsigned char*)mes, mesSize) <= 0) {
        printf("Encryption failed.\n");
        free(*encmess);
        *encmess = NULL;
        goto cleanup;
    }

    *encSize = outlen;
    /*
    printf("Encryption succeeded!\n");
    printf("Encrypted (%zu bytes):\n", *encSize);
    for (size_t i = 0; i < *encSize; ++i) {
        printf("%02X ", (*encmess)[i]);
    }
    printf("\n");*/

    ret = 0;

    cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pubkey) EVP_PKEY_free(pubkey);
    if (bio) BIO_free(bio);

    return ret;
}



int encryptBigMessageServ(const char* mes, size_t mesSize, unsigned char** encmess, size_t* encSize){

    int nbToEnc = ceil((float)(mesSize)/(MAX_MESSAGE_LENGTH-1));
    *encSize = nbToEnc*ENCRYPTED_LENGTH;
    *encmess = (unsigned char*)malloc(sizeof(unsigned char)*(*encSize));

    memset(*encmess, 0, *encSize);

    for(int i = 0; i < nbToEnc; i++){
        char partmessage[MAX_MESSAGE_LENGTH];
        int partmessagelength = (i < nbToEnc-1)?MAX_MESSAGE_LENGTH-1:mesSize-(i*(MAX_MESSAGE_LENGTH-1));

        for(int j = 0; j < partmessagelength; j++){
            partmessage[j] = mes[j+(i*(MAX_MESSAGE_LENGTH-1))];
        }
        partmessage[partmessagelength] = '\0';

        unsigned char* encmesstmp;
        size_t encSizetmp = 0;

        if(encryptToServ(partmessage, partmessagelength, (unsigned char **)&encmesstmp, &encSizetmp)!=0){
            exit(EXIT_FAILURE);
        }

        memcpy((*encmess) + (i * ENCRYPTED_LENGTH), encmesstmp, encSizetmp);


        free(encmesstmp);

    }

    return 0;

}

int BigRSAdecrypt(EVP_PKEY* key, const unsigned char* encmes, size_t encSize, char** mess, size_t* messSize){

    if(encSize/ENCRYPTED_LENGTH != ceil(encSize/ENCRYPTED_LENGTH)){
        return 1;
    }
    *messSize = 0;
    *mess = (char*)malloc(sizeof(char));
    strcpy(*mess, "");



    for(int i = 0; i < (int)(encSize/ENCRYPTED_LENGTH); i++){
        unsigned char enctmp[ENCRYPTED_LENGTH];
        memset(enctmp, 0, ENCRYPTED_LENGTH);
        for(int j = 0; j < ENCRYPTED_LENGTH; j++){
            enctmp[j] = encmes[j+(ENCRYPTED_LENGTH*i)];
        }

        char* buff;
        size_t buffsize;
        
        RSAdecrypt(key, enctmp, ENCRYPTED_LENGTH, &buff, &buffsize);

        char* tmp = (char*)malloc(sizeof(char)*(*messSize + buffsize + 1));
        memcpy(tmp, *mess, *messSize);
        memcpy(tmp + *messSize, buff, buffsize);
        tmp[*messSize + buffsize] = '\0';
        *messSize += buffsize;


        free(*mess);
        free(buff);
        *mess = tmp;

    }


    return 0;
}







int encryptFromCharKey(const char* charkey ,const char* mes, size_t mesSize, unsigned char** encmess, size_t* encSize){

    int ret = 1;
    EVP_PKEY* pubkey = NULL;
    BIO* bio = NULL;
    EVP_PKEY_CTX* ctx = NULL;
    *encmess = NULL;



    if (!mes || mesSize == 0 || !encmess || !encSize) {
        printf("Invalid input.\n");
        return 1;
    }

    // Create a memory BIO from the PEM string
    bio = BIO_new_mem_buf(charkey, -1);
    if (!bio) {
        printf("Failed to create BIO.\n");
        goto cleanup;
    }

    // Read the public key
    pubkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (!pubkey) {
        printf("Failed to read public key.\n");
        goto cleanup;
    }

    // Create encryption context
    ctx = EVP_PKEY_CTX_new(pubkey, NULL);
    if (!ctx) {
        printf("Failed to create context.\n");
        goto cleanup;
    }

    // Initialize encryption
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        printf("Failed to init encryption.\n");
        goto cleanup;
    }

    // Determine encrypted size
    if (EVP_PKEY_encrypt(ctx, NULL, encSize, (const unsigned char*)mes, mesSize) <= 0) {
        printf("Failed to determine encrypted size.\n");
        goto cleanup;
    }


    // Allocate memory
    *encmess = (unsigned char*)malloc(*encSize);
    if (!*encmess) {
        printf("Failed to allocate memory.\n");
        goto cleanup;
    }

    size_t keysize = EVP_PKEY_size(pubkey);
    if (mesSize > keysize - 11) {
        //printf("Message too large for RSA encryption! %ld over %ld\n", mesSize, keysize-11);
        free(*encmess);
        *encmess = NULL;
        goto cleanup;
    }

    // Perform encryption
    if (EVP_PKEY_encrypt(ctx, *encmess, encSize, (const unsigned char*)mes, mesSize) <= 0) {
        printf("Encryption failed.\n");
        free(*encmess);
        *encmess = NULL;
        goto cleanup;
    }


    ret = 0;

    cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (pubkey) EVP_PKEY_free(pubkey);
    if (bio) BIO_free(bio);

    return ret;
}


int encryptBigMessageFromKey(const char* key, const char* mes, size_t mesSize, unsigned char** encmess, size_t* encSize){

    int nbToEnc = ceil((float)(mesSize)/(MAX_MESSAGE_LENGTH-1));
    *encSize = nbToEnc*ENCRYPTED_LENGTH;
    *encmess = (unsigned char*)malloc(sizeof(unsigned char)*(*encSize));

    memset(*encmess, 0, *encSize);

    for(int i = 0; i < nbToEnc; i++){
        char partmessage[MAX_MESSAGE_LENGTH];
        int partmessagelength = (i < nbToEnc-1)?MAX_MESSAGE_LENGTH-1:mesSize-(i*(MAX_MESSAGE_LENGTH-1));

        for(int j = 0; j < partmessagelength; j++){
            partmessage[j] = mes[j+(i*(MAX_MESSAGE_LENGTH-1))];
        }
        partmessage[partmessagelength] = '\0';

        unsigned char* encmesstmp;
        size_t encSizetmp;

        if(encryptFromCharKey(key, partmessage, partmessagelength, (unsigned char **)&encmesstmp, &encSizetmp)!=0){
            exit(EXIT_FAILURE);
        }


        memcpy((*encmess) + (i * ENCRYPTED_LENGTH), encmesstmp, encSizetmp);


        free(encmesstmp);

    }

    return 0;

}