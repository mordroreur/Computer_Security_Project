#ifndef __MORDROREUR_COMMON_H__
#define __MORDROREUR_COMMON_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <openssl/core_names.h>  // for OSSL_PKEY_PARAM_RSA_*

#ifdef __MINGW32__
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <bcrypt.h>
    typedef SOCKET socket_t;
    #define CLOSESOCKET closesocket
    #define THREAD_RETURN DWORD WINAPI
    #define THREAD_HANDLE HANDLE
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <dirent.h>
    typedef int socket_t;
    #define CLOSESOCKET close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define THREAD_RETURN void*
    #define THREAD_HANDLE pthread_t
#endif



#define PORT 42429
#define BUFFER_SIZE 4096


#define UNUSED(x) (void)(x)



#define DEBUG 1



#endif /* __MORDROREUR_COMMON_H__ */
