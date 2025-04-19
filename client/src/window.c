#include "../includes/window.h"
#include "../includes/version.h"



int Initialize_sdl_window(AppState ** appstate, game_parameter_t *gp){

    if (!SDL_SetAppMetadata("3d renderer", VERSION_STRING, "com.mordroreur.3drenderer")) {
        return SDL_APP_FAILURE;
    }

    long unsigned int i;
    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            return SDL_APP_FAILURE;
        }
    }

    AppState *as = (AppState*)(SDL_calloc(1, sizeof(AppState)));
    if (!as) {
        return SDL_APP_FAILURE;
    }else{
        *appstate = as;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }


    if (!SDL_CreateWindowAndRenderer("3d renderer", gp->window_width, gp->window_height, 0, &as->window, &as->renderer)) {
        return SDL_APP_FAILURE;
    }
    
    
    // TODO : read and understand all sets
    SDL_SetRenderVSync(as->renderer, false);
    SDL_SetWindowRelativeMouseMode(as->window, false);
    SDL_SetHintWithPriority(SDL_HINT_WINDOWS_RAW_KEYBOARD, "1", SDL_HINT_OVERRIDE);


    // initialise all important value for the game
    as->game_still_running = 1;
    as->target_ns_per_frame = 1000000000/gp->fps_cap;
    as->target_ns_per_ticks = 1000000/TCK_TO_GET;
    
    as->debug_fps_string[0] = 0;
    as->width = gp->window_width;
    as->height = gp->window_height;
    // TODO : why?
    SDL_snprintf(as->debug_version_string, sizeof(as->debug_version_string), "%s", VERSION_STRING);

    

    return 0;
}



// TODO remove this from here
void mainloop(AppState *as){
    #ifdef __EMSCRIPTEN__
    //main_ticks
    if(!as->game_still_running){
        emscripten_cancel_main_loop();
    }
    #endif
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            as->game_still_running = 0;
        }else if(event.type == SDL_EVENT_KEY_UP){
            if(event.key.key == SDLK_ESCAPE){
                as->game_still_running = 0;
            }
        }
    }

    SDL_FRect rects[16];
        const Uint64 now = SDL_GetTicks();
        int i;

        /* we'll have the rectangles grow and shrink over a few seconds. */
        const float direction = ((now % 2000) >= 1000) ? 1.0f : -1.0f;
        const float scale = ((float) (((int) (now % 1000)) - 500) / 500.0f) * direction;

        /* as you can see from this, rendering draws over whatever was drawn before it. */
        SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
        SDL_RenderClear(as->renderer);  /* start with a blank canvas. */

        /* Rectangles are comprised of set of X and Y coordinates, plus width and
        height. (0, 0) is the top left of the window, and larger numbers go
        down and to the right. This isn't how geometry works, but this is
        pretty standard in 2D graphics. */

        /* Let's draw a single rectangle (square, really). */
        rects[0].x = rects[0].y = 100;
        rects[0].w = rects[0].h = 100 + (100 * scale);
        SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);  /* red, full alpha */
        SDL_RenderRect(as->renderer, &rects[0]);

        /* Now let's draw several rectangles with one function call. */
        for (i = 0; i < 3; i++) {
            const float size = (i+1) * 50.0f;
            rects[i].w = rects[i].h = size + (size * scale);
            rects[i].x = (as->width - rects[i].w) / 2;  /* center it. */
            rects[i].y = (as->height - rects[i].h) / 2;  /* center it. */
        }
        SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);  /* green, full alpha */
        SDL_RenderRects(as->renderer, rects, 3);  /* draw three rectangles at once */

        /* those were rectangle _outlines_, really. You can also draw _filled_ rectangles! */
        rects[0].x = 400;
        rects[0].y = 50;
        rects[0].w = 100 + (100 * scale);
        rects[0].h = 50 + (50 * scale);
        SDL_SetRenderDrawColor(as->renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);  /* blue, full alpha */
        SDL_RenderFillRect(as->renderer, &rects[0]);

        /* ...and also fill a bunch of rectangles at once... *//*
        for (i = 0; i < SDL_arraysize(rects); i++) {
	  const float w = (float) ((float)(as->width) / SDL_arraysize(rects));
            const float h = i * 8.0f;
            rects[i].x = i * w;
            rects[i].y = as->height - h;
            rects[i].w = w;
            rects[i].h = h;
        }*/
        SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* white, full alpha */
        SDL_RenderFillRects(as->renderer, rects, SDL_arraysize(rects));


        SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(as->renderer, 0, 0, as->debug_fps_string);

        SDL_SetRenderDrawColor(as->renderer, 0, 0, 255, 255);
        SDL_RenderDebugText(as->renderer, 0, as->height-10, as->debug_version_string);

        SDL_RenderPresent(as->renderer);  /* put it all on the screen! */

}
