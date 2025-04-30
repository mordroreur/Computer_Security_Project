#ifndef __MORDROREUR_TCPServer_H__
#define __MORDROREUR_TCPServer_H__

#include "common.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 42429
#define BUFFER_SIZE 4096




int connectToMainServer(AppState*, char*);

int checkUsernameAndKey(AppState*, char*);

int searchServer(AppState*, char*, char**);
int getFromServer(AppState*, char*);
int getFromFile(AppState*, char*);

int clearConnexionToMainServer(AppState*);

void msleep(int);

THREAD_RETURN serverHandler(void*);

int tryConnectSomeone(AppState*, messaged_personne_t*);

int sendMessagePersonne(AppState*, int, char*);



#endif /*__MORDROREUR_TCPServer_H__ */