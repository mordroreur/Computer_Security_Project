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

  char file_name[MAX_LENGTH_FILE_NAME];
  sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PARAM_NAME);
  
  FILE *param = fopen(file_name, FILE_MODE);
  if(param == NULL){
    ap->gmprm.pseudo = NULL;
    ap->gmprm.window_width = WINDOW_WIDTH;
    ap->gmprm.window_height = WINDOW_HEIGHT;
    //TODO : remove ap->gmprm.fps_cap = FPS_TO_GET;
    if(writeParameterFile(&(ap->gmprm))){return 1;}
  }else{
    fclose(param);
    if(readParameterFile(&(ap->gmprm))){return 1;}
  }

  return 0;
}



int writeParameterFile(game_parameter_t* gp){
  char file_name[MAX_LENGTH_FILE_NAME];
  sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PARAM_NAME);
  FILE* param = fopen(file_name, "w");
  if(param == NULL){
    return 1;
  }else{
    fprintf(param, "%s\n", gp->pseudo);
    fprintf(param, "%d\n", gp->window_width);
    fprintf(param, "%d\n", gp->window_height);
    // TODO : remove fprintf(param, "%d\n", gp.fps_cap);
    fclose(param);
  }
  return 0;
}
/*
char* key = strchr(result, '\n')+1;
    char* line = strtok(result, "\n");
    if (!line) {
        free(result);
        return 1;
    }

    // Split ip and port
    char* colon = strchr(line, ':');
    if (!colon) {
        free(result);
        return 1;
    }

  FILE* param = fopen(file_name, "rt");  // 't' is optional on Linux but explicit for Windows
    if (param == NULL) {
        return 1;
    }

    char username[MAX_USERNAME_SIZE];
    if (fgets(username, MAX_USERNAME_SIZE, param) == NULL) {
        fclose(param);
        return 1;
    }

    // Remove both \r and \n for Windows line endings
    username[strcspn(username, "\r\n")] = '\0';

    if (fscanf(param, "%d\n%d\n", &(gp->window_width), &(gp->window_height)) != 2) {
        fclose(param);
        return 1;
    }

    fclose(param);

    if (strcmp(username, "(null)") != 0) {
        size_t len = strlen(username);
        gp->pseudo = (char*)malloc(len + 1);
        if (gp->pseudo == NULL) return 1; // malloc failed
        strcpy(gp->pseudo, username);
    }


*/
int readParameterFile(game_parameter_t *gp) {
  char file_name[MAX_LENGTH_FILE_NAME];
  snprintf(file_name, MAX_LENGTH_FILE_NAME, "%s%s", ALL_FILE_FOLDER, PARAM_NAME);

  FILE* param = fopen(file_name, FILE_MODE);
  if (param == NULL) {
      return 1;
  }

  char username[MAX_USERNAME_SIZE];
  if (fgets(username, MAX_USERNAME_SIZE, param) == NULL) {
      fclose(param);
      return 1;
  }

  // Remove both '\r' and '\n'
  username[strcspn(username, "\r\n")] = '\0';

  if (fscanf(param, "%d\n%d\n", &(gp->window_width), &(gp->window_height)) != 2) {
      fclose(param);
      return 1;
  }

  fclose(param);

  if (strcmp(username, "(null)") != 0 && username[0] != '\0') {
      size_t len = strlen(username);
      gp->pseudo = (char*)malloc(len + 1);
      if (!gp->pseudo) return 1;
      strcpy(gp->pseudo, username);
  } else {
      gp->pseudo = NULL;  // handle explicitly if needed
  }

  return 0;
}


int loadPrivateKeyFromFile(AppState* ap) {
    char file_name[MAX_LENGTH_FILE_NAME];

    // Load private key
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PRIVATE_KEY_NAME);
    FILE* param = fopen(file_name, "rb");
    if (param == NULL) {
        return 1;
    }
    ap->priv_key = PEM_read_PrivateKey(param, NULL, NULL, NULL);
    fclose(param);
    if (ap->priv_key == NULL) {
        return 1;
    }

    // Load public key
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PUBLIC_KEY_NAME);
    param = fopen(file_name, "rb");
    if (param == NULL) {
        EVP_PKEY_free(ap->priv_key);
        ap->priv_key = NULL;
        return 1;
    }
    ap->pub_key = PEM_read_PUBKEY(param, NULL, NULL, NULL);
    fclose(param);
    if (ap->pub_key == NULL) {
        EVP_PKEY_free(ap->priv_key);
        ap->priv_key = NULL;
        return 1;
    }

    return 0;
}

int loadPublicKeyAsChar(char** key, size_t* keySize) {
  char file_name[MAX_LENGTH_FILE_NAME];

  // Load public key
  sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PUBLIC_KEY_NAME);
  FILE* file = fopen(file_name, "rb");
  if (file == NULL) {
      exit(EXIT_FAILURE);
  }

  // Move to end to get file size
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  rewind(file);

  if (fileSize < 0) {
    perror("Failed to get file size");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  // Allocate buffer (+1 for null-terminator)
  *key = (char*)malloc(fileSize + 1);
  if (!(*key)) {
      perror("Failed to allocate buffer");
      fclose(file);
      exit(EXIT_FAILURE);
  }

  // Read the whole file
  size_t bytesRead = fread(*key, 1, fileSize, file);
  /*if ((unsigned char)(*key[0]) == 0xEF &&
      (unsigned char)(*key[1]) == 0xBB &&
      (unsigned char)(*key[2]) == 0xBF) {
      // Skip the BOM
      memmove(*key, *key + 3, fileSize - 2);
      (*key)[fileSize - 3] = '\0';
      *keySize -= 3;
  }*/
  fclose(file);

  if ((int)(bytesRead) != fileSize) {
      fprintf(stderr, "Failed to read entire file\n");
      free(*key);
      exit(EXIT_FAILURE);
  }

  // Null-terminate
  (*key)[fileSize] = '\0';
  *keySize = fileSize;

  return 0;
}


int writePrivateKeyToFile(AppState* ap) {
    char file_name[MAX_LENGTH_FILE_NAME];

    // Write public key
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PUBLIC_KEY_NAME);
    FILE* pub = fopen(file_name, "wb");
    if (!pub) return 1;
    if (!PEM_write_PUBKEY(pub, ap->pub_key)) {
        fclose(pub);
        return 1;
    }
    fclose(pub);

    // Write private key
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, PRIVATE_KEY_NAME);
    FILE* priv = fopen(file_name, "wb");
    if (!priv) return 1;
    if (!PEM_write_PrivateKey(priv, ap->priv_key, NULL, NULL, 0, NULL, NULL)) {
        fclose(priv);
        return 1;
    }
    fclose(priv);

    return 0;
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





*/