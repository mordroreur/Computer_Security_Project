#ifndef __MORDROREUR_SYMCRYPTO_H__
#define __MORDROREUR_SYMCRYPTO_H__

#include "common.h"

#include <openssl/aes.h>


#define CHACHA20_NONCE_LEN 12       // Standard ChaCha20 nonce size

int generate_random_keys(messaged_personne_t*);
int dualEncrypt(const char*, size_t, messaged_personne_t*, unsigned char**, size_t*);

int dualDecrypt(const unsigned char*, size_t, messaged_personne_t*, char**);





#endif /*__MORDROREUR_SYMCRYPTO_H__ */