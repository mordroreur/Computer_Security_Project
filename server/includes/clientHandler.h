#ifndef __MORDROREUR_CLIENTHANDLER_H__
#define __MORDROREUR_CLIENTHANDLER_H__

#include "common.h"

#define MAX_RESULT_SIZE 8192       
#define MAX_LENGTH_FILE_NAME 128
#define ALL_FILE_FOLDER "./savedFiles/"

THREAD_RETURN client_handler(void*);


int checkFileContent(const char*, const char*, const char*, const int);
char* loadKeyFromFile(const char*);
int listMatchingFiles(const char*, char*);
int readFileContent(const char*, char*);




#endif /*__MORDROREUR_CLIENTHANDLER_H__ */