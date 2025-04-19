#ifndef __MORDROREUR_COMMON_H__
#define __MORDROREUR_COMMON_H__

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __MINGW32__
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
//    #include <pthread.h>
#endif


#define UNUSED(x) (void)(x)



#define DEBUG 1


typedef struct {
  // is GUI
  char GUI;

  // window basis
  SDL_Window *window;
  SDL_Renderer *renderer;

  int game_still_running;
  int width;
  int height;

    // fps ticks base
  Uint64 target_ns_per_frame;
  Uint64 target_ns_per_ticks;
  char debug_fps_string[32];
  char debug_version_string[32];

} AppState;



typedef struct game_parameter_t{
  int window_width;
  int window_height;
  int fps_cap;
  char* pseudo;
} game_parameter_t;


#define TCK_TO_GET 10


#endif /* __MORDROREUR_COMMON_H__ */
