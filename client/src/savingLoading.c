#include "../includes/savingLoading.h"


int loadParameter(AppState* ap){
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

  return isfolder;
}






/*
int areParameterSaved(){
  FILE *param = fopen(PARAM_NAME, "r");
  if(param == NULL){
    return 1;
  }else{
    fclose(param);
  }
  return 0;
}


int createParameterFile(game_parameter_t * gp){
  gp->window_width = WINDOW_WIDTH;
  gp->window_height = WINDOW_HEIGHT;
  gp->fps_cap = FPS_TO_GET;
  return writeParameterFile(gp);
}


int writeParameterFile(game_parameter_t *gp){
  FILE *param = fopen(PARAM_NAME, "w");
  if(param == NULL){
    return 1;
  }else{
    fprintf(param, "%d\n", gp->window_width);
    fprintf(param, "%d\n", gp->window_height);
    fprintf(param, "%d\n", gp->fps_cap);
    fclose(param);
  }
  return 0;
}


int readParameterFile(game_parameter_t *gp){
  FILE *param = fopen(PARAM_NAME, "r");
  if(param == NULL){
    return 1;
  }else{
    fscanf(param, "%d\n%d\n%d\n", &(gp->window_width), &(gp->window_height), &(gp->fps_cap));
    fclose(param);
  }
  return 0;
}
*/