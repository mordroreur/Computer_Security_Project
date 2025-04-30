#include "../includes/common.h"
#include "../includes/window.h"
#include "../includes/version.h"
#include "../includes/savingLoading.h"
#include "../includes/asymCrypto.h"
#include "../includes/TCPServer.h"
#include "../includes/symCrypto.h"


void safeFlushStdin() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}


int main(int argc, char *argv[]) {

  // initialise the state of the app
  AppState *as = (AppState*)(malloc(sizeof(AppState)));
  if (!as) { return SDL_APP_FAILURE;}

  as->still_messaging = 1;

  as->GUI = 1;
  if(argc > 1){
    if(!strcmp(argv[1], "noGUI")){
      as->GUI = 0;
    }else{
      printf("You can use this program with \"noGUI\" to use the comande line version\n");
    }
  }

  if(as->GUI == 0){
    printf("Welcome to the skyche, the message app by Mordroreur.\nYou are running the version %s.\n", VERSION_STRING);
  }

  loadParameter(as);
  as->canrem = 1;
  if (as->GUI) {
    Initialize_sdl_window(as);
    drawLoading(as);
  }

  loadPrivatePublicKeys(as);

  as->MainServ = 2;
  char serv_ip[MAX_IP_LENGTH];
  strncpy(serv_ip, SERVER_IP, MAX_IP_LENGTH - 1);
  serv_ip[MAX_IP_LENGTH - 1] = '\0';
  int iplen = strlen(serv_ip);
  do{
    connectToMainServer(as, serv_ip);
    if(as->MainServ == 2){
      if (as->GUI) {
        as->game_still_running = 1;
        SDL_StartTextInput(as->window);
        while(as->game_still_running){
          getMainIPSDL(as, serv_ip, &iplen);
          msleep(10);
        }
      }else{
        printf("Server unreachable!!!!\n Try an other ip (Y = yes)?");
        char res;
        if (scanf(" %c", &res) != 1) {
          printf("Invalid input.\n");
          safeFlushStdin();
          continue;
        }
        safeFlushStdin(); // clear any extra chars
        if (res == 'Y' || res == 'y') {
          printf("Enter the new IP: ");
          if (fgets(serv_ip, MAX_IP_LENGTH, stdin) == NULL) {
              printf("Error reading IP.\n");
              continue;
          }
          // Remove trailing newline
          serv_ip[strcspn(serv_ip, "\n")] = '\0';
        }else{
          as->MainServ = 1;
        }
      }
    }
  }while (as->MainServ == 2);
  
  as->listeningPORT = -1;

#ifdef __MINGW32__
  THREAD_HANDLE thread = CreateThread(NULL, 0, serverHandler, as, 0, NULL);
  if (thread == NULL) {
      exit(EXIT_FAILURE);
  } else {
      CloseHandle(thread); // detach
  }
#else
  THREAD_HANDLE thread;
  if (pthread_create(&thread, NULL, serverHandler, as) != 0) {
    exit(EXIT_FAILURE);
  } else {
    pthread_detach(thread); // detach
  }
#endif
  

char isPseudoGood = 0;
  do{
    if(as->gmprm.pseudo == NULL){
      char username[MAX_USERNAME_SIZE] = "";
      int pseudoSize = 0;
      while (!isPseudoGood) {
        if(as->GUI) {
          as->game_still_running = 1;
          SDL_StartTextInput(as->window);
          while(as->game_still_running){
            getPseudo(as, username, &pseudoSize);
            msleep(10);
          }
        }else{
          pseudoSize = 10;
          printf("You need a pseudo : ");
          if (fgets(username, sizeof(username), stdin) != NULL) {
            size_t len = strlen(username);
            if (len > 0 && username[len - 1] == '\n') {
                username[len - 1] = '\0';
            }
          }
        }
        if(as->MainServ == 0){
          if(pseudoSize != 0){
            isPseudoGood = !checkUsernameAndKey(as, username);
            if(isPseudoGood == 0){
              memset(username, 0, MAX_USERNAME_SIZE);
              pseudoSize = 0;
            }
          }
        }else{
          isPseudoGood = 1;
        }
      }
      if(as->still_messaging && isPseudoGood != 0){
        size_t len = strlen(username);
        as->gmprm.pseudo = (char*)malloc(sizeof(char)*len);
        strcpy(as->gmprm.pseudo, username);
        writeParameterFile(&(as->gmprm));
      }
    }else{
      if(!as->MainServ){
        isPseudoGood = !checkUsernameAndKey(as, as->gmprm.pseudo);
        if(isPseudoGood == 0){
          free(as->gmprm.pseudo);
          as->gmprm.pseudo = NULL;
        }
      }
    }
  }while(as->MainServ == 0 && !isPseudoGood);

  if(!as->GUI){
    printf("Welcome %s\n", as->gmprm.pseudo);
  }
 
  as->messaged_list = NULL;
  char input[MAX_INPUT_LENGTH];

  if(as->GUI && as->MainServ == 0){
    memset(as->search, 0, MAX_USERNAME_SIZE);
    as->searchSize = 0;
    searchServer(as, "", &as->searched);
    SDL_StartTextInput(as->window);
  }

  as->game_still_running = 0;
  while (as->still_messaging)
  {
    if(as->GUI){

      messagePersonSDL(as);
      msleep(2);

    }else{
      printf("Available commands: search(or s), connect(or c), message(or m), exit(or q)\n");
      printf("\n> ");
      if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL) {
        printf("Error reading input.\n");
        continue;
      }
      input[strcspn(input, "\n")] = '\0';
      if (strcmp(input, "exit") == 0 || strcmp(input, "q") == 0|| strcmp(input, "Q") == 0) {
        as->still_messaging = 0;
      }else if (strcmp(input, "search") == 0|| strcmp(input, "s") == 0|| strcmp(input, "S") == 0) {
        if(!as->MainServ){
          char word[MAX_INPUT_LENGTH];
          char *result;
          printf("Enter word to search: ");
          if (fgets(word, MAX_INPUT_LENGTH, stdin) == NULL) {
              printf("Error reading word.\n");
              continue;
          }
          word[strcspn(word, "\n")] = '\0';
          searchServer(as, word, &result);
          printf("%s\n", result);
          free(result);
        }else{
          printf("This option only exist if a CA is connected");
        }
      }
      else if (strcmp(input, "connect") == 0|| strcmp(input, "c") == 0|| strcmp(input, "C") == 0) {
        if(!as->MainServ){
          char word[MAX_INPUT_LENGTH];
          printf("Enter connection target: ");
          if (fgets(word, MAX_INPUT_LENGTH, stdin) == NULL) {
              printf("Error reading word.\n");
              continue;
          }
          word[strcspn(word, "\n")] = '\0';
          if(getFromServer(as, word) == 0){
            printf("%s as been added\n", word);
          }
        }else{
          char word[MAX_INPUT_LENGTH];
          printf("Enter file name: ");
          if (fgets(word, MAX_INPUT_LENGTH, stdin) == NULL) {
              printf("Error reading word.\n");
              continue;
          }
          word[strcspn(word, "\n")] = '\0';
          if(getFromFile(as, word) == 0){
            printf("%s as been added\n", word);
          }
        }
      }
      else if (strcmp(input, "message") == 0|| strcmp(input, "m") == 0|| strcmp(input, "M") == 0) {
          
        
        char numStr[MAX_INPUT_LENGTH];
          int number;


          messaged_personne_t *messaged_list = as->messaged_list;
          printf("Select a number in the list (");
          int i = 0;
          while (messaged_list != NULL) {
              printf("%d : %s", i++, messaged_list->pseudo);
              messaged_list = messaged_list->next;
              if (messaged_list != NULL) printf(", ");
          }
          printf("): ");

          if (fgets(numStr, MAX_INPUT_LENGTH, stdin) == NULL) {
              printf("Error reading number.\n");
              continue;
          }

          number = atoi(numStr);

          
          if (number > i || number < 1) {
              printf("Invalid number selected.\n");
              continue;
          }
          

          char message[MAX_MESSAGE_LENGTH];
          printf("Enter your message: ");
          if (fgets(message, MAX_MESSAGE_LENGTH, stdin) == NULL) {
              printf("Error reading message.\n");
              continue;
          }
          message[strcspn(message, "\n")] = '\0';

          sendMessagePersonne(as, number, message);
      }
      else {
          printf("Unknown command.\n");
      }
    }

  }
  

  
  

  
  if(as->GUI){
    cleanSDL(as);
  }



  // TODO : Close all conncetions
  clearConnexionToMainServer(as);



  return 0;
}

