#include "../includes/RSAcrypto.h"



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


int decryptServ(const unsigned char* encmes, size_t encSize, char** mess, size_t* messSize){

    char serverKey[] = "-----BEGIN PRIVATE KEY-----\n\
MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDvYwz7vFP+sVP/\
JX/WY8lPlWVq4CuuBon9DiyRYMgwlq4FBB8SOKjlELz77hL+FEp3RSEP1/DRJu4v\
JXhAaWIj4ShT5WqabWr7OVz7MjNfIHrjpQ2VflRURs9fpJpDlKdVTNSshkNXGCcE\
V6/eL8o2T2dT2U/az+3+AiJe5MBrpdyPHg5LXpW4+cufv+D8YoBMOPUi7kBmN/pN\
j7dS5KBAvnx8yftG63Poe0j+3IBzbwZnYZeM/c29ncvu1+PmohTu92vM1Y4TlcvB\
iDN7JkP0cTCiojCoIrLlgovWIkMAJniKI7YoUulba/HhOWj2/dM8V2J6IiGVo8eG\
UbzqznMhAgMBAAECggEAIMdFkRezwaFq4Lo0A7lkqIzKlwSMfpb/IIMnF53ys3KC\
cAXlMkPHXpw44F80Qw2ot6Wj1g1JuH3V6ec9zMYRTdvn/3rfqrcWScGMSYoRbjJe\
mVqxnjlu1x5eCVja1auecQfk7B2zTVLtjsLp/ii5jc+4FItiS5IioqAMsEUu0t5e\
pkMrxPFVsbY1R5aMyADeV/CSs1XQ+8Ss4Y3GaKyzU5WWxs6FKzoZBhwLxCofaeLm\
FhJjQbJnkNhNAwr3IEjHAAYEbCh3A7+DR6tItMUkxOKA5OKhZRfuGtxOMOaeqBt+\
xL6aH7bqbhb7h9V582QCL3BEz0/MBJMKUW1j+lSB9QKBgQD4lN2oC9wCpcE1GVKj\
9Te+BHbQ+gczWAAl93zyUjeoCCKXnsWiF3jM9nZl1hzBFXSMYT2NOisuZai21tpE\
Y79kp5dYCIqOeVZdhbjr6ivHiEoVN9F9dFvapt95qAE4WEoJgtGk9/14wH96dVQD\
yDha6qosUwusW1RTC7D2fbcApQKBgQD2h/BzI6vV+m0ZusXOY50dychbNNy+URqQ\
HOnpcOZ7OBc66rBmOs0n9K1jsuIQZk6FF/MtZypdscB+6hN8FkpmXo2zm/ySLbwl\
QwhRKD5UrsHN+ZhtqONCfNRvx9k8Yx6OpQhBby3VDQXEX3cHJST6eIhW75AP2BwC\
KZqCRG0DzQKBgGOw+BAL6YkaRpjv9eeRUXCgQ7Jdg4PA2BZh4bcg5/c4g/8AiswN\
08GHWkZPDysUWrBt0wyPdn/d/0KMee6RflF0sMp6am7nFI7fMSFTwYj9DJDj2N0S\
TEAtSlR030BkkBSuZTtexUWnFbjb0vzNUGSN1f9+sqDF49SGTMsd8cWNAoGAK2zW\
lfF6mz55aUbVAP3a9xxMBoTRPVTb8Pam3U/dyWaf8OjWGavR52/Z+u3PlEPxINCP\
LTg25johRjnFJN5oTI/rNfAMFVOpCIc+N8127UO7L6T/jJFrNpL2eJojUdfD0iyP\
v7btw9F/ao/GT6PjV5tTqLyaC7BtfpQFe1zCNyECgYB2FcLI7MaCPvMQqHHUM2CM\
EI5EzYL/yb6ZxwJkYLBe23+kGJtPUxNSrMXmZPH0CB6StbWLfPpiBzUmuAM2Egc+\
k8sBerPQweLUtCAMCegHcsLAO9b07yRzsdXoQO7lXjYmTzziwhW034dSgPvuhS/K\
N2JcBnMSiciSkHR3IS1Fvg==\n\
-----END PRIVATE KEY-----";

    int ret = 1;
    EVP_PKEY* privkey = NULL;
    BIO* bio = NULL;
    EVP_PKEY_CTX* ctx = NULL;

    // Create a memory BIO from the PEM string
    bio = BIO_new_mem_buf(serverKey, -1);
    if (!bio) goto cleanup;

    // Read the private key
    privkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    if (!privkey) goto cleanup;

    // Create decryption context
    ctx = EVP_PKEY_CTX_new(privkey, NULL);
    if (!ctx) goto cleanup;

    if (EVP_PKEY_decrypt_init(ctx) <= 0) goto cleanup;

    // Determine buffer length
    if (EVP_PKEY_decrypt(ctx, NULL, messSize, (const unsigned char*)encmes, encSize) <= 0) goto cleanup;

    // Allocate memory for decrypted message (+1 for null terminator)
    *mess = (char*)malloc(*messSize + 1);
    if (!*mess) goto cleanup;

    // Perform decryption
    if (EVP_PKEY_decrypt(ctx, (unsigned char*)*mess, messSize, (const unsigned char*)encmes, encSize) <= 0) goto cleanup;

    // Null-terminate the decrypted message
    (*mess)[*messSize] = '\0';

    ret = 0; // success

    cleanup:
    if (bio) BIO_free(bio);
    if (privkey) EVP_PKEY_free(privkey);
    if (ctx) EVP_PKEY_CTX_free(ctx);

    return ret;

}





int decryptBigServ(const unsigned char* encmes, size_t encSize, char** mess, size_t* messSize){

    if(encSize/ENCRYPTED_LENGTH != ceil(encSize/ENCRYPTED_LENGTH)){
        return 1;
    }
    *messSize = 1;
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

        decryptServ(enctmp, ENCRYPTED_LENGTH, &buff, &buffsize);
        //printf("%s\n", buff);

        *messSize += buffsize;
        
        char* tmp = (char*)malloc(sizeof(char)*(*messSize+buffsize-1));
        strcpy(tmp, *mess);
        strcat(tmp, buff);
        free(*mess);
        free(buff);
        *mess = tmp;

    }


    return 0;
}








    
/*
    decryptServ((unsigned char*) buffer, bytes_received, &mess, &messSize);

    char* restOfMessage = strchr(mess, '\n');
    if (restOfMessage) {
        type = atoi(mess);              // parse the integer at the start
        restOfMessage++;                // move past the '\n'
        strcpy(thisMessage, restOfMessage); // copy the rest
    }



    strcat(wholeMessage, thisMessage);

    if(type != 0){
        messagereceiving = 0;
    }
}*/



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