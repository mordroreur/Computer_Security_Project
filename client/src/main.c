#include "../includes/common.h"
//#include "../includes/window.h"
#include "../includes/version.h"
#include "../includes/savingLoading.h"

int main(int argc, char *argv[]) {


  printf("Welcome to the skyche, the message app by Mordroreur.\nYou are running the version %s.\n", VERSION_STRING);

  // initialise the state of the app
  AppState *as = (AppState*)(malloc(sizeof(AppState)));
  if (!as) { return SDL_APP_FAILURE;}

  as->GUI = 1;
  if(argc > 1){
    if(!strcmp(argv[1], "noGUI")){
      as->GUI = 0;
    }else{
      printf("You can use this program with \"noGUI\" to use the comande line version\n");
    }
  }

  loadParameter(as);

  // TODO : Open listening port for everyone + get port (if no port saved) 

  if (as->GUI) {
    // TODO : init windows + send it to other thread in loading
  }

  
  // TODO : try to connect with the server

  // TODO : while true

  // TODO : options : new destinataire, send message


  








  return 0;
}


  

/*

UNUSED(argc);
UNUSED(argv);

// initialise window part of the game
AppState *as;

game_parameter_t game_param;



// read for a save of the game parameters
if(areParameterSaved()){
  if(createParameterFile(&game_param)){
fprintf(stderr, "Error creating parameter file\n");
    return 1;
  }
}else{
  if(readParameterFile(&game_param)){
fprintf(stderr, "Error reading parameter file\n");
    return 1;
  }
}

// Create the window
Initialize_sdl_window(&as, &game_param); 


// Main loop
#ifdef __EMSCRIPTEN__
emscripten_set_main_loop_arg(mainloop, as, 60, 1);
#else

// TODO : update thread
// Uint64 last_tick = 0;


long unsigned int fps_count = 0;
Uint64 second_count = SDL_GetTicksNS()-1000000000;


Uint64 last_frame = SDL_GetTicksNS();
Uint64 now = 0;

while(as->game_still_running){
    
    now = SDL_GetTicksNS();


    if(now - last_frame > as->target_ns_per_frame){

        // TODO rename main Draw
        mainloop(as);



        //printf("%ld     %ld \n", now, last_frame);


        last_frame += as->target_ns_per_frame;
        fps_count++;

    }else{
        SDL_DelayNS((as->target_ns_per_frame - (now - last_frame - 10)));
    }
    
    
    
    if (now - second_count > 999999999) {
        second_count += 1000000000;
        SDL_snprintf(as->debug_fps_string, sizeof(as->debug_fps_string), "%" SDL_PRIu64 " fps", fps_count);
        fps_count = 0;
    }
    

    
}


#endif






// Cleanup
SDL_free(as);


return 0;






//////////////////////client////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define SERVER_IP "127.0.0.1"

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char send_char, recv_char;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect"); exit(1);
    }

    printf("Enter a character to send: ");
    scanf(" %c", &send_char);

    send(sock, &send_char, 1, 0);
    recv(sock, &recv_char, 1, 0);

    printf("Received: %c\n", recv_char);

    close(sock);
    return 0;
}


////////////////////// serv //////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 12345

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);  // free the allocated memory for client socket

    char buffer;
    while (1) {
        int bytes_read = recv(client_sock, &buffer, 1, 0);
        if (bytes_read <= 0) break;
        printf("Received: %c\n", buffer);
        buffer += 1; // simple response: next char
        send(client_sock, &buffer, 1, 0);
    }
    printf("Client disconnected.\n");
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) { perror("socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen"); exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_sock = malloc(sizeof(int));  // allocate memory for each client socket
        *client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (*client_sock < 0) {
            perror("accept");
            free(client_sock);
            continue;
        }

        printf("Client connected.\n");

        // Create a new thread to handle the client
        if (pthread_create(&tid, NULL, handle_client, client_sock) != 0) {
            perror("pthread_create");
            close(*client_sock);
            free(client_sock);
            continue;
        }

        pthread_detach(tid);  // detach the thread so it cleans itself up when done
    }

    close(server_sock);
    return 0;
}




*/
