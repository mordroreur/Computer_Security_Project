#ifndef __MORDROREUR_WINDOW_H__
#define __MORDROREUR_WINDOW_H__

#include "common.h"

static const struct {
    const char *key;
    const char *value;
} extended_metadata[] = {
    { SDL_PROP_APP_METADATA_URL_STRING, "https://....com" },
    { SDL_PROP_APP_METADATA_CREATOR_STRING, "Mordroreur" },
    { SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "None" },
    { SDL_PROP_APP_METADATA_TYPE_STRING, "game" }
};






/// @brief This function create a SDL3 window and store
/// @param  AppState : a uninitialized variable in wich the function store the renderer and window created
/// @return 0 if everithing worked well, 1 else
int Initialize_sdl_window(AppState **, game_parameter_t *);



void mainloop(AppState *);


#endif /* __MORDROREUR_WINDOW_H__ */
