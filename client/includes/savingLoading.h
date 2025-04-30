#ifndef __MORDROREUR_SAVINGLOADING_H__
#define __MORDROREUR_SAVINGLOADING_H__

#include "common.h"




#define PARAM_NAME "game_parameter.txt"
#define PRIVATE_KEY_NAME "private_key.pem"
#define PUBLIC_KEY_NAME "public_key.pem"
#define ALL_FILE_FOLDER "./savedFiles/"
#define MAX_LENGTH_FILE_NAME 128
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480


#ifdef __MINGW32__
    #define FILE_MODE "rt"  // Windows: 't' enforces text mode
#else
    #define FILE_MODE "r"   // Unix: no 't' needed
#endif



int loadParameter(AppState*);
int writeParameterFile(game_parameter_t* );
int readParameterFile(game_parameter_t *);

int loadPublicKeyAsChar(char**, size_t*);
int loadPrivateKeyFromFile(AppState*);
int writePrivateKeyToFile(AppState*);

/*
int areParameterSaved();
int createParameterFile(game_parameter_t *);


*/






#endif /*__MORDROREUR_SAVINGLOADING_H__*/