#ifndef __MORDROREUR_ASYMCRYPTO_H__
#define __MORDROREUR_ASYMCRYPTO_H__

#include "common.h"
#include "savingLoading.h"

#include <openssl/core_names.h>  // for OSSL_PKEY_PARAM_RSA_*

#define KEY_LENGTH 2048
#define PUB_EXP 65537

#define MAX_MESSAGE_LENGTH 120
#define ENCRYPTED_LENGTH 256



int loadPrivatePublicKeys(AppState*);
int RSAencrypt(EVP_PKEY*, const char*, unsigned char**, size_t*);
int RSAdecrypt(EVP_PKEY*, const unsigned char*,size_t, char**, size_t*);
int BigRSAdecrypt(EVP_PKEY*, const unsigned char*,size_t, char**, size_t*);


int encryptToServ(const char*, size_t, unsigned char**, size_t*);

int encryptBigMessageServ(const char*, size_t, unsigned char**, size_t*);

int encryptFromCharKey(const char*,const char*, size_t, unsigned char**, size_t*);

int encryptBigMessageFromKey(const char*, const char*, size_t, unsigned char**, size_t*);

// private function
int testRSAKey(AppState*);
int RSAkeyGenerate(AppState*);





#endif /*__MORDROREUR_ASYMCRYPTO_H__ */