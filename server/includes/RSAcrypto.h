#ifndef __MORDROREUR_RSARYPTO_H__
#define __MORDROREUR_RSARYPTO_H__

#include "common.h"

#include <openssl/core_names.h>  // for OSSL_PKEY_PARAM_RSA_*


#define MAX_MESSAGE_LENGTH 200
#define ENCRYPTED_LENGTH 256


int RSAencrypt(EVP_PKEY*, const char*, unsigned char**, size_t*);


int decryptServ(const unsigned char*, size_t, char**, size_t*);

int decryptBigServ(const unsigned char*, size_t, char**, size_t*);

int encryptFromCharKey(const char*,const char*, size_t, unsigned char**, size_t*);

int encryptBigMessageFromKey(const char*, const char*, size_t, unsigned char**, size_t*);







#endif /*__MORDROREUR_RSARYPTO_H__ */