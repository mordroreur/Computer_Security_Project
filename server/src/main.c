#include "../includes/version.h"
#include "../includes/common.h"
#include "../includes/clientHandler.h"



int main() {
#ifdef __MINGW32__
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
char isfolder;
#ifdef __MINGW32__
  DWORD attrs = GetFileAttributesA(ALL_FILE_FOLDER);
  isfolder = (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat st;
  isfolder =  (stat(ALL_FILE_FOLDER, &st) == 0 && S_ISDIR(st.st_mode));
#endif

  if(!isfolder){
#ifdef __MINGW32__
    CreateDirectoryA(ALL_FILE_FOLDER, NULL);
#else
    mkdir(ALL_FILE_FOLDER, 0755);
#endif
  }

  socket_t server_fd;    
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("bind failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }


    if (listen(server_fd, 5) == SOCKET_ERROR) {
        perror("listen failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }


    char ip_str[INET_ADDRSTRLEN];
    struct sockaddr_in actual_addr;
    socklen_t actual_len = sizeof(actual_addr);

    if (getsockname(server_fd, (struct sockaddr*)&actual_addr, &actual_len) == -1) {
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

#ifdef __MINGW32__
    if (InetNtop(AF_INET, &actual_addr.sin_addr, ip_str, INET_ADDRSTRLEN) == NULL) {
        perror("InetNtop failed");
        exit(EXIT_FAILURE);
    }
#else
    if (inet_ntop(AF_INET, &actual_addr.sin_addr, ip_str, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop failed");
        exit(EXIT_FAILURE);
    }
#endif

    printf("Server listening on %s:%d\n", ip_str, PORT);



    while (1) {
        socket_t* client_fd = malloc(sizeof(socket_t));
        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (*client_fd == INVALID_SOCKET) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

#ifdef __MINGW32__
        THREAD_HANDLE thread = CreateThread(NULL, 0, client_handler, client_fd, 0, NULL);
        if (thread == NULL) {
            perror("CreateThread failed");
            CLOSESOCKET(*client_fd);
            free(client_fd);
        } else {
            CloseHandle(thread); // detach
        }
#else
        THREAD_HANDLE thread;
        if (pthread_create(&thread, NULL, client_handler, client_fd) != 0) {
            perror("pthread_create failed");
            CLOSESOCKET(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(thread); // detach
        }
#endif
    }

    CLOSESOCKET(server_fd);

#ifdef __MINGW32__
    WSACleanup();
#endif

    return 0;
}





















/*
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
}*/
/*
int main(int argc, char *argv[]) {

    UNUSED(argc);
    UNUSED(argv);

    printf("CACA\n");

   

    return 0;
}
*/

/*
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 5

void* run(void* arg) {
    size_t job = *(size_t*)arg;
    printf("Hello from thread %zu!\n", job);
    fflush(stdout);
    free(arg);
    return NULL;
}

int main() {
    pthread_t threads[N];

    printf("Starting threads...\n");
    fflush(stdout);
    
    for (size_t i = 0; i < N; i++) {
        size_t* job = malloc(sizeof(size_t));
        *job = i;
        if (pthread_create(&threads[i], NULL, run, job) != 0) {
            perror("pthread_create failed");
            free(job);
        }
    }
    
    for (size_t i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Done!\n");
    fflush(stdout);
    return 0;
}
*/
/*
#include <stdio.h>
#include <stdlib.h>


#ifdef __MINGW32__
    #include <windows.h>
#else
    #include <pthread.h>
#endif



#define N 5


#ifdef __MINGW32__
DWORD WINAPI run(LPVOID arg) {
#else
void* run(void* arg) {
#endif

    size_t job = *(size_t*)arg;
    printf("Job %zu\n", job);
    return 0;
}


int main() {

    int job = 42;
    printf("Starting threads...\n");

    
    for (size_t i = 0; i < N; i++) {
        size_t* job = malloc(sizeof(size_t));
        *job = i;
        if (pthread_create(&threads[i], NULL, run, job) != 0) {
            perror("pthread_create failed");
            free(job);
        }
    }

    for (size_t i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

#ifdef __MINGW32__
    HANDLE thread = CreateThread(NULL, 0, run, &job, 0, NULL);
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, run, &job);
    pthread_join(thread, NULL);
#endif

    printf("Done!\n");

    return 0;
}
*/
