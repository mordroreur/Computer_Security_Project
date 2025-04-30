#ifndef __MORDROREUR_WINDOW_H__
#define __MORDROREUR_WINDOW_H__

#include "common.h"
#include <SDL3_ttf/SDL_ttf.h>

static const struct {
    const char *key;
    const char *value;
} extended_metadata[] = {
    { SDL_PROP_APP_METADATA_URL_STRING, "https://....com" },
    { SDL_PROP_APP_METADATA_CREATOR_STRING, "Mordroreur" },
    { SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "None" },
    { SDL_PROP_APP_METADATA_TYPE_STRING, "communication" }
};


#define MAX_USERNAME_SIZE_WELCOME 256



/// @brief This function create a SDL3 window and store
/// @param  AppState : a uninitialized variable in wich the function store the renderer and window created
/// @return 0 if everithing worked well, 1 else
int Initialize_sdl_window(AppState *);


void drawLoading(AppState *);

void getMainIPSDL(AppState *, char *, int*);
void getPseudo(AppState *, char *, int*);

void messagePersonSDL(AppState *);



int cleanSDL(AppState*);


// private functions
void drawString(AppState*, char*, int, int, int, int, float, SDL_Color);
void drawStringNorth(AppState*, char*, int, int, int, int, float, SDL_Color);


#endif /* __MORDROREUR_WINDOW_H__ */
