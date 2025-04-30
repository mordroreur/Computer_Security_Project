#ifndef __MORDROREUR_COMMON_H__
#define __MORDROREUR_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <sodium.h>
#include <SDL3/SDL.h>

#ifdef __MINGW32__
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include <bcrypt.h>
    typedef SOCKET socket_t;
    #define CLOSESOCKET closesocket
    #define THREAD_RETURN DWORD WINAPI
    #define THREAD_HANDLE HANDLE
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define CLOSESOCKET close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define THREAD_RETURN void*
    #define THREAD_HANDLE pthread_t
#endif


#define MAX_USERNAME_SIZE 246
#define MAX_IP_LENGTH 64
#define MAX_INPUT_LENGTH 16384
#define AES_KEY_LEN 32

#define UNUSED(x) (void)(x)



#define DEBUG 1


typedef struct game_parameter_t{
  char* pseudo;
  int window_width;
  int window_height;
  // TODO : remove int fps_cap;
  
} game_parameter_t;


typedef struct message_t{
  char *message;
  unsigned char *cypher;
  int cypherLength;
  int who;
  int time;

  struct message_t* next;
}message_t;

typedef struct messaged_personne{
  char* pseudo;
  char* ip;
  int port;
  char* key;
  struct sockaddr_in other_addr;
  socket_t other_server;

  unsigned char send_K1[AES_KEY_LEN];
  unsigned char send_K2[AES_KEY_LEN];
  unsigned char send_K3[AES_KEY_LEN];
  int send_nonce;

  unsigned char rcv_K1[AES_KEY_LEN];
  unsigned char rcv_K2[AES_KEY_LEN];
  unsigned char rcv_K3[AES_KEY_LEN];
  int receive_nonce;

  message_t *mess_list;
  char typing[MAX_INPUT_LENGTH];
  int typingLength;

  struct messaged_personne* next;


} messaged_personne_t;



typedef struct {
  // is GUI
  char GUI;
  char MainServ;
  int listeningPORT;

  int still_messaging;
  //int list_person_size;
  messaged_personne_t* messaged_list;
  
  // window basis
  SDL_Window *window;
  SDL_Renderer *renderer;
  int game_still_running;


  int width;
  int height;
  
  char search[MAX_USERNAME_SIZE];
  int searchSize;

  char *searched;
  int canrem;



  game_parameter_t gmprm;
  EVP_PKEY* pub_key;
  EVP_PKEY* priv_key;

} AppState;

typedef struct combined_messaged_t{
  AppState* as;
  messaged_personne_t* pers;
}combined_messaged_t;


#define TCK_TO_GET 10


#endif /* __MORDROREUR_COMMON_H__ */
