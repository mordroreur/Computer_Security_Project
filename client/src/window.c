#include "../includes/window.h"
#include "../includes/version.h"
#include "../includes/TCPServer.h"

TTF_Font* font;


int Initialize_sdl_window(AppState * as){

    if (!SDL_SetAppMetadata("skyche", VERSION_STRING, "com.mordroreur.skyche")) {
        return SDL_APP_FAILURE;
    }

    long unsigned int i;
    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init()) {
        return SDL_APP_FAILURE;
    }


    if (!SDL_CreateWindowAndRenderer("Skyche", as->gmprm.window_width, as->gmprm.window_height, 0, &as->window, &as->renderer)) {
        return SDL_APP_FAILURE;
    }
    
    
    // TODO : read and understand all sets
    SDL_SetRenderVSync(as->renderer, false);
    SDL_SetWindowRelativeMouseMode(as->window, false);
    SDL_SetHintWithPriority(SDL_HINT_WINDOWS_RAW_KEYBOARD, "1", SDL_HINT_OVERRIDE);


    // initialise all important value for the game
    as->game_still_running = 1;
    //as->target_ns_per_frame = 1000000000/gp->fps_cap;
    //as->target_ns_per_ticks = 1000000/TCK_TO_GET;
    
    //as->debug_fps_string[0] = 0;
    as->width = as->gmprm.window_width;
    as->height = as->gmprm.window_height;
    
    // TODO : why?
    //SDL_snprintf(as->debug_version_string, sizeof(as->debug_version_string), "%s", VERSION_STRING);

    // Load the font
    font = TTF_OpenFont("Roboto-Black.ttf", 90);  // Adjust font size (24 here)
    if (!font) {
        printf("Failed to load font: %s\n", SDL_GetError());
        return 1;
    }  

    

    return 0;
}


void drawLoading(AppState *as) {
    // Set the drawing color to white and clear the screen
    SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderClear(as->renderer);

    
    
    
    // Create a surface with the text rendered
    SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };  // Black text color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Loading", 7, textColor);
    
    if (!textSurface) {
        printf("Unable to render text surface: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    // Create a texture from the surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(as->renderer, textSurface);


    if (!textTexture) {
        printf("Unable to create texture: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    // Get the dimensions of the text
    SDL_FRect textRect;
    textRect.x = (as->width - (as->width/3)) / 2;  // Center text horizontally
    textRect.y = (as->height - (as->height/3)) / 2;  // Center text vertically
    textRect.w = as->width/3;
    textRect.h = as->height/3;

 

    // Render the text on the screen
    SDL_RenderTexture(as->renderer, textTexture, NULL, &textRect);

    SDL_DestroySurface(textSurface);
    SDL_DestroyTexture(textTexture);

    // Present the renderer
    SDL_RenderPresent(as->renderer);
}




void getMainIPSDL(AppState *as, char *newIP, int* messLen){

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
    //        running = 0;
            SDL_StopTextInput(as->window);
            as->still_messaging = 0;
            as->game_still_running = 0;
            as->MainServ = 1;
            return; // user closed window
        }
        if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.scancode == SDL_SCANCODE_RETURN ) {
    //            running = 0; // User pressed Enter
                break;
            } else if (e.key.scancode == SDL_SCANCODE_BACKSPACE && *messLen > 0) {
                // Handle backspace
                newIP[--(*messLen)] = '\0';
            }
        }
        if (e.type == SDL_EVENT_TEXT_INPUT) {
            if ((int)(*messLen + strlen(e.text.text)) < MAX_IP_LENGTH - 1) {
                strcat(newIP, e.text.text);
                *messLen += strlen(e.text.text);
            }
        }
    }

    // Set the drawing color to white and clear the screen
    SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderClear(as->renderer);

    SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };  // Black text color
    drawString(as, "Server unreachable!!!!", 23, (as->width / 2.0), (as->height / 8.0), as->width/1.2, (as->height/8.0), textColor);
    drawString(as, "New Ip : ", 10, (as->width / 2.0), (as->height / 5.0), as->width/1.2, (as->height/8.0), textColor);
    
    drawString(as, newIP, *messLen, (as->width / 2.0), (as->height / 3.6), as->width/1.2, (as->height/8.0), textColor);

    float posx, posy;
    SDL_MouseButtonFlags mousestate = SDL_GetMouseState(&posx, &posy);
    SDL_FRect rect;
    rect.x = (as->width / 5.0)*1.0;
    rect.y = (as->height / 5.0)*3.5;
    rect.w = (as->width / 5.0)*1.0;
    rect.h = (as->height / 5.0);
    if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
        SDL_SetRenderDrawColor(as->renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        if(mousestate & SDL_BUTTON_LMASK){
            as->MainServ = 1;
            SDL_StopTextInput(as->window);
            as->game_still_running = 0;
        }
    }else{
        SDL_SetRenderDrawColor(as->renderer, 255, 125, 125, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(as->renderer, &rect);

    rect.x = (as->width / 5.0)*3.0;
    rect.y = (as->height / 5.0)*3.5;
    rect.w = (as->width / 5.0)*1.0;
    rect.h = (as->height / 5.0);
    if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
        SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        if(mousestate & SDL_BUTTON_LMASK){
            SDL_StopTextInput(as->window);
            as->game_still_running = 0;
        }
    }else{
        SDL_SetRenderDrawColor(as->renderer, 125, 255, 125, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(as->renderer, &rect);


    drawString(as, "Try this Ip", 12, (as->width / 5.0)*3.5, (as->height / 5.0)*3.75, as->width/5.0, (as->height/12.0), textColor);
    drawString(as, "Mode no CA", 11, (as->width / 5.0)*1.5, (as->height / 5.0)*3.75, as->width/5.0, (as->height/12.0), textColor);
    
    

    // Present the renderer
    SDL_RenderPresent(as->renderer);


}


void getPseudo(AppState *as, char *newIP, int* messLen){

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
    //        running = 0;
            SDL_StopTextInput(as->window);
            as->still_messaging = 0;
            as->game_still_running = 0;
            as->MainServ = 1;
            return; // user closed window
        }
        if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.scancode == SDL_SCANCODE_RETURN ) {
    //            running = 0; // User pressed Enter
                break;
            } else if (e.key.scancode == SDL_SCANCODE_BACKSPACE && *messLen > 0) {
                // Handle backspace
                newIP[--(*messLen)] = '\0';
            }
        }
        if (e.type == SDL_EVENT_TEXT_INPUT) {
            if ((int)(*messLen + strlen(e.text.text)) < MAX_USERNAME_SIZE - 1) {
                strcat(newIP, e.text.text);
                *messLen += strlen(e.text.text);
            }
        }
    }

    // Set the drawing color to white and clear the screen
    SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderClear(as->renderer);

    SDL_Color textColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };  // Black text color
    drawString(as, "Enter your pseudo", 18, (as->width / 2.0), (as->height / 8.0), as->width/1.2, (as->height/8.0), textColor);
    
    drawString(as, newIP, *messLen, (as->width / 2.0), (as->height / 3.6), as->width/1.2, (as->height/8.0), textColor);

    float posx, posy;
    SDL_MouseButtonFlags mousestate = SDL_GetMouseState(&posx, &posy);
    SDL_FRect rect;
    rect.x = (as->width / 5.0)*2.0;
    rect.y = (as->height / 5.0)*3.5;
    rect.w = (as->width / 5.0)*1.0;
    rect.h = (as->height / 5.0);
    if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
        SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        if(mousestate & SDL_BUTTON_LMASK){
            SDL_StopTextInput(as->window);
            as->game_still_running = 0;
        }
    }else{
        SDL_SetRenderDrawColor(as->renderer, 125, 255, 125, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(as->renderer, &rect);



    drawString(as, "Confirm", 8, (as->width / 5.0)*2.5, (as->height / 5.0)*4.0, as->width/5.0, (as->height/12.0), textColor);

    
    

    // Present the renderer
    SDL_RenderPresent(as->renderer);

}


void messagePersonSDL(AppState *as){
    as->canrem = 0;
    messaged_personne_t *person_see = as->messaged_list;
    if(person_see == NULL){
        as->game_still_running = 0;
    }
    for (int i = 1; i < as->game_still_running; i++) {
        person_see = person_see->next;
        if(person_see == NULL){
            as->game_still_running = 0;
        }
    }
    

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
    //        running = 0;
            //SDL_StopTextInput(as->window);
            as->still_messaging = 0;
            return; // user closed window
        }
        if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.scancode == SDL_SCANCODE_RETURN ) {
    //            running = 0; // User pressed Enter
                if(as->game_still_running == 0){
                    if(!as->MainServ){
                        getFromServer(as, as->search);
                        memset(as->search, 0, MAX_USERNAME_SIZE);
                        as->searchSize = 0;
                        free(as->searched);
                        searchServer(as, "", &as->searched);
                    }else{
                        getFromFile(as, as->search);
                        memset(as->search, 0, MAX_USERNAME_SIZE);
                        as->searchSize = 0;
                    }
                }else{
                    sendMessagePersonne(as, as->game_still_running, person_see->typing);
                    memset(person_see->typing, 0, MAX_INPUT_LENGTH);
                    person_see->typingLength = 0;
                }
                break;
            } else if (e.key.scancode == SDL_SCANCODE_BACKSPACE) {
                // Handle backspace
                if(as->game_still_running == 0){
                    if(as->searchSize > 0){
                        if(!as->MainServ){
                            as->search[--(as->searchSize)] = '\0';
                            free(as->searched);
                            searchServer(as, as->search, &as->searched);
                        }else{
                            as->search[--(as->searchSize)] = '\0';
                        }

                    }
                }else{
                    if(person_see->typingLength > 0){
                        person_see->typing[--(person_see->typingLength)] = '\0';
                    }
                }
            }
        }
        if (e.type == SDL_EVENT_TEXT_INPUT) {
            if(as->game_still_running == 0){
                if ((int)(as->searchSize + strlen(e.text.text)) < MAX_USERNAME_SIZE - 1) {
                    strcat(as->search, e.text.text);
                    as->searchSize += strlen(e.text.text);
                    if(!as->MainServ){
                        free(as->searched);
                        searchServer(as, as->search, &as->searched);
                    }
                }
            }else{
                if ((int)(person_see->typingLength + strlen(e.text.text)) < MAX_INPUT_LENGTH - 1) {
                    strcat(person_see->typing, e.text.text);
                    person_see->typingLength += strlen(e.text.text);
                }
            }
        }
    }
    SDL_Color blackColor = { 0, 0, 0, SDL_ALPHA_OPAQUE };  // Black text color
    SDL_Color grayColor = { 125, 125, 125, SDL_ALPHA_OPAQUE };  // Black text color
    SDL_Color blueColor = { 125, 125, 255, SDL_ALPHA_OPAQUE };
    SDL_Color greenColor = { 125, 255, 125, SDL_ALPHA_OPAQUE };

    float posx, posy;
    SDL_MouseButtonFlags mousestate = SDL_GetMouseState(&posx, &posy);

    // Set the drawing color to white and clear the screen
    SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);  /* white, full alpha */
    SDL_RenderClear(as->renderer);
    
    
    SDL_FRect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = as->width;
    rect.h = (as->height / 8.0);
    SDL_SetRenderDrawColor(as->renderer, 125, 125, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(as->renderer, &rect);
    char welcomString[MAX_USERNAME_SIZE_WELCOME];
    sprintf(welcomString, "Welcome %s", as->gmprm.pseudo);
    drawString(as, welcomString, strlen(welcomString), (as->width / 5.0)*3.0, (as->height / 16.0), (as->width/5.0)*4.0, (as->height/12.0), blackColor);


    rect.y = 0;
    rect.w = as->width/5.0;
    if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
        SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
        if(mousestate & SDL_BUTTON_LMASK){
            memset(as->search, 0, MAX_USERNAME_SIZE);
            as->searchSize = 0;
            searchServer(as, "", &as->searched);
            as->game_still_running = 0;
        }
    }else{
        SDL_SetRenderDrawColor(as->renderer, 125, 255, 125, SDL_ALPHA_OPAQUE);
    }
    SDL_RenderFillRect(as->renderer, &rect);
    SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(as->renderer, &rect);
    drawString(as, "Connect to new person", 22, (as->width / 10.0), (as->height / 33.0), (as->width/5.0), (as->height/16.0), blackColor);

    messaged_personne_t *messaged_list = as->messaged_list;
    int i = 0;
    while ( messaged_list != NULL) {
        i++;
        rect.y = (as->height / 8.0)*i;
        if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
            SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
            if(mousestate & SDL_BUTTON_LMASK){
                //SDL_StopTextInput(as->window);
                as->game_still_running = i;
            }
        }else{
            SDL_SetRenderDrawColor(as->renderer, 125, 255, 125, SDL_ALPHA_OPAQUE);
        }
        SDL_RenderFillRect(as->renderer, &rect);
        SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(as->renderer, &rect);
        drawString(as, messaged_list->pseudo, strlen(messaged_list->pseudo), (as->width / 10.0), (as->height / 33.0) + (as->height / 8.0)*i, (as->width/5.0), (as->height/16.0), blackColor);


        char thisIP[MAX_USERNAME_SIZE_WELCOME];
        sprintf(thisIP, "%s:%d", messaged_list->ip, messaged_list->port);
        drawString(as, thisIP, strlen(thisIP), (as->width / 10.0), (as->height / 33.0) + (as->height/16.0) + (as->height / 8.0)*i, (as->width/5.0), (as->height/23.0), blackColor);
        messaged_list = messaged_list->next;
    }

        if(as->game_still_running == 0){

           if(!as->MainServ){

                drawString(as, "Connect to : ", 14, (as->width / 5.0)*3.0, (as->height / 5.0)*0.8, (as->width/5.0)*4.0, (as->height/12.0), blackColor);
                drawStringNorth(as, as->searched, strlen(as->searched), (as->width / 5.0)*1.5, (as->height / 5.0)*1.8, (as->width/5.0)*4.0, (as->height/18.0), grayColor);
                drawStringNorth(as, "Pseudo existing : ", 19, (as->width / 5.0)*1.5, (as->height / 5.0)*1.6, (as->width/5.0)*4.0, (as->height/16.0), blackColor);
            
            }else{
                drawString(as, "file name of ip:port and key : ", 32, (as->width / 5.0)*3.0, (as->height / 5.0)*0.8, (as->width/5.0)*4.0, (as->height/12.0), blackColor);
            }


        drawString(as, as->search, strlen(as->search), (as->width / 5.0)*3.0, (as->height / 5.0)*1.1, (as->width/5.0)*4.0, (as->height/9.0), blackColor);
 


        rect.x = (as->width / 5.0)*2.5;
        rect.y = (as->height / 5.0)*3.5;
        rect.w = (as->width / 5.0)*1.0;
        rect.h = (as->height / 5.0);
        if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
            SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
            if(mousestate & SDL_BUTTON_LMASK){
                if(!as->MainServ){
                    getFromServer(as, as->search);
                    memset(as->search, 0, MAX_USERNAME_SIZE);
                    as->searchSize = 0;
                    free(as->searched);
                    searchServer(as, "", &as->searched);
                }else{
                    getFromFile(as, as->search);
                    memset(as->search, 0, MAX_USERNAME_SIZE);
                    as->searchSize = 0;
                }
            }
        }else{
            SDL_SetRenderDrawColor(as->renderer, 125, 255, 125, SDL_ALPHA_OPAQUE);
        }
        SDL_RenderFillRect(as->renderer, &rect);
        drawString(as, "Connect", 8, (as->width / 5.0)*3.0, (as->height / 5.0)*4.0, (as->width/5.0)*4.0, (as->height/12.0), blackColor);


    }else{

        float lineNB = 0;
        float ratio;
        float hsize = 0;
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "a", 2, blackColor);
        hsize = textSurface->h;
        ratio = as->height / (textSurface->h*16.0);

        //drawString(as, person_see->typing, person_see->typingLength, (as->width / 10.0)*5, (as->height / 8.0)*6, (as->width/5.0)*4.0, (as->height/12.0), blackColor);
        if(person_see->typingLength != 0){
            int start = 0;
            int linenb = 0;
            int ends[100];
            ends[0] = 0;
            
            SDL_DestroySurface(textSurface);
            while(start < person_see->typingLength){
                int end = 0;
                int lastSpace = 0;
                int maxPast = 0;
        
                while(end + start < person_see->typingLength && maxPast == 0){
                    end++;
                    SDL_Surface* textSurface = TTF_RenderText_Solid(font, person_see->typing+start, end, blackColor);
                    int size = textSurface->w*ratio;
                    if(size >= ((as->width/5.0)*4.0 - (as->width / 10.0)*1.2)){
                        maxPast = 1;
                        end--;
                    }else if(*(person_see->typing+start+end) == ' '){
                        lastSpace = end+1;
                    }else if(*(person_see->typing+start+end) == '\n'){
                        maxPast = 1;
                        lastSpace = end;
                    }
                    
                    SDL_DestroySurface(textSurface);
                }
                if(lastSpace != 0 && maxPast == 1){
                    end = lastSpace;
                }
                ends[++linenb] = end+start;
                start += end;
            }
            
            for(int i = 0; i < linenb; i++){
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, person_see->typing+ends[i], ends[i+1]-ends[i], blackColor);

                if (!textSurface) {
                    printf("Unable to render text surface: %s\n", SDL_GetError());
                    TTF_CloseFont(font);
                    return;
                }

                // Create a texture from the surface
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(as->renderer, textSurface);

                if (!textTexture) {
                    printf("Unable to create texture: %s\n", SDL_GetError());
                    TTF_CloseFont(font);
                    return;
                }

                // Get the dimensions of the text
                SDL_FRect textRect;
                textRect.x = as->width/5.0 + (as->width / 10.0)*0.1;  // Center text horizontally
                textRect.y = as->height - ((textTexture->h*ratio)/2.0) - ((textTexture->h*ratio)*(linenb-i));  // Center text vertically
                textRect.w = textTexture->w * ratio; //as->width/MAX_IP_LENGTH * (*messLen);
                textRect.h = textTexture->h * ratio; //as->height/8;

                // Render the text on the screen
                SDL_RenderTexture(as->renderer, textTexture, NULL, &textRect);

                SDL_DestroySurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
            lineNB = linenb;
        }

        rect.x = (as->width / 10.0)*9;
        rect.y = (as->height) - ((hsize*ratio)*(((lineNB<1)?1:lineNB)+1));
        rect.w = (as->width / 10.0)*1.0;
        rect.h = ((hsize*ratio)*(((lineNB<1)?1:lineNB)+1));
        if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
            SDL_SetRenderDrawColor(as->renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
            if(mousestate & SDL_BUTTON_LMASK){
                sendMessagePersonne(as, as->game_still_running, person_see->typing);
                memset(person_see->typing, 0, MAX_INPUT_LENGTH);
                person_see->typingLength = 0;
            }
        }else{
            SDL_SetRenderDrawColor(as->renderer, 125, 255, 125, SDL_ALPHA_OPAQUE);
        }
        SDL_RenderFillRect(as->renderer, &rect);
        drawString(as, "Send", 5, (as->width / 10.0)*9.5, (as->height) - (((hsize*ratio)*(((lineNB<1)?1:lineNB)+1))/2), (as->width/10.0), (as->height/15.0), blackColor);

        rect.x = (as->width / 5.0);
        rect.w = (as->width / 5.0)*4.0 - (as->width / 10.0);
        SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(as->renderer, &rect);

        if(lineNB == 0){lineNB++;}
        lineNB += 1;


        message_t *mess = person_see->mess_list;
        while(mess != NULL){


            int len = strlen(mess->message);

            int start = 0;
            int linenb = 0;
            int ends[100];
            ends[0] = 0;
            
            SDL_DestroySurface(textSurface);
            while(start < len){
                int end = 0;
                int lastSpace = 0;
                int maxPast = 0;
        
                while(end + start < len && maxPast == 0){
                    end++;
                    SDL_Surface* textSurface = TTF_RenderText_Solid(font, mess->message+start, end, blackColor);
                    int size = textSurface->w*ratio;
                    if(size >= ((as->width/5.0)*4.0 - (as->width / 10.0)*0.2)){
                        maxPast = 1;
                        end--;
                    }else if(*(mess->message+start+end) == ' '){
                        lastSpace = end+1;
                    }else if(*(mess->message+start+end) == '\n'){
                        maxPast = 1;
                        lastSpace = end;
                    }
                    
                    SDL_DestroySurface(textSurface);
                }
                if(lastSpace != 0 && maxPast == 1){
                    end = lastSpace;
                }
                ends[++linenb] = end+start;
                start += end;
            }
            
            for(int i = 0; i < linenb; i++){
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, mess->message+ends[i], ends[i+1]-ends[i], (mess->who)?blueColor:greenColor);

                if (!textSurface) {
                    printf("Unable to render text surface: %s\n", SDL_GetError());
                    TTF_CloseFont(font);
                    return;
                }

                // Create a texture from the surface
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(as->renderer, textSurface);

                if (!textTexture) {
                    printf("Unable to create texture: %s\n", SDL_GetError());
                    TTF_CloseFont(font);
                    return;
                }

                // Get the dimensions of the text
                SDL_FRect textRect;
                textRect.x = as->width/5.0 + (as->width / 10.0)*0.1;  // Center text horizontally
                textRect.y = as->height - ((textTexture->h*ratio)/2.0) - ((textTexture->h*ratio)*(lineNB+linenb-i));  // Center text vertically
                textRect.w = textTexture->w * ratio; //as->width/MAX_IP_LENGTH * (*messLen);
                textRect.h = textTexture->h * ratio; //as->height/8;

                // Render the text on the screen
                SDL_RenderTexture(as->renderer, textTexture, NULL, &textRect);

                SDL_DestroySurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
            lineNB += linenb;
            lineNB += 0.5;

            rect.x = (as->width / 5.0);
            rect.w = (as->width / 5.0)*4.0;
            rect.y = (as->height) - ((hsize*ratio)*(lineNB+0.5));
            rect.h = ((hsize*ratio)*(linenb+1));
            if(posx >= rect.x && posx <= rect.x+rect.w && posy >= rect.y && posy <= rect.y+rect.h){
                if(mess->time > 50){
                    SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
                    SDL_RenderFillRect(as->renderer, &rect);
                    char* cyph = (char*)malloc(sizeof(char)*(3*mess->cypherLength));
                    char* ptr = cyph;
                    for (int k = 0; k < mess->cypherLength; k++) {
                        ptr += sprintf(ptr, "%02x", mess->cypher[k]);
                        if (k < mess->cypherLength - 1) {
                            *ptr++ = ' ';
                        }
                    }
                    *ptr = '\0';
                    drawString(as, cyph, 3*mess->cypherLength-1, (as->width / 5.0)*3, (as->height) - ((hsize*ratio)*(lineNB+1)) + (((hsize*ratio)*(linenb+1))/2), (as->width/5.0)*3.9, (as->height/30.0), blackColor);
                    free(cyph);
                }else{
                    mess->time++;
                }
                SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                SDL_RenderRect(as->renderer, &rect);
            }else{
                mess->time = 0;
            }
            



            mess = mess->next;
        }










    }
    as->canrem = 1;



    // Present the renderer
    SDL_RenderPresent(as->renderer);



}



int cleanSDL(AppState* as){

    TTF_CloseFont(font);

    TTF_Quit();
    SDL_DestroyRenderer(as->renderer);
    SDL_DestroyWindow(as->window);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        msleep(10);
    }


    return 0;

}


void drawString(AppState* as, char* word, int wordlen, int x, int y, int maxWidth,float subrat, SDL_Color textColor){
    // Create a surface with the text rendered
    if(wordlen == 0){
        return;
    }

    int start = 0;
    int linenb = 0;

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "a", 2, textColor);
    float ratio = subrat / textSurface->h;
    SDL_DestroySurface(textSurface);
    
    while(start < wordlen){
        int end = 0;
        int lastSpace = 0;
        int maxPast = 0;

        while(end + start < wordlen && maxPast == 0){
            end++;
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, word+start, end, textColor);
            int size = textSurface->w*ratio;
            if(*(word+start+end) == ' '){
                lastSpace = end;
            }else if(*(word+start+end) == '\n'){
                maxPast = 1;
                lastSpace = end;
            }
            if(size > maxWidth){
                maxPast = 1;
                end--;
            }
            SDL_DestroySurface(textSurface);
        }
        if(lastSpace != 0 && maxPast == 1){
            end = lastSpace;
        }
        
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, word+start, end, textColor);

        if (!textSurface) {
            printf("Unable to render text surface: %s\n", SDL_GetError());
            TTF_CloseFont(font);
            return;
        }

        // Create a texture from the surface
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(as->renderer, textSurface);

        if (!textTexture) {
            printf("Unable to create texture: %s\n", SDL_GetError());
            TTF_CloseFont(font);
            return;
        }

        // Get the dimensions of the text
        SDL_FRect textRect;
        textRect.x = x - ((textTexture->w*ratio)/2.0);  // Center text horizontally
        textRect.y = y - ((textTexture->h*ratio)/2.0) + ((textTexture->h*ratio)*linenb);  // Center text vertically
        textRect.w = textTexture->w * ratio; //as->width/MAX_IP_LENGTH * (*messLen);
        textRect.h = textTexture->h * ratio; //as->height/8;

        // Render the text on the screen
        SDL_RenderTexture(as->renderer, textTexture, NULL, &textRect);

        SDL_DestroySurface(textSurface);
        SDL_DestroyTexture(textTexture);
        start += end;
        linenb++;
    }
}


void drawStringNorth(AppState* as, char* word, int wordlen, int x, int y, int maxWidth,float subrat, SDL_Color textColor){
    // Create a surface with the text rendered
    if(wordlen == 0){
        return;
    }

    int start = 0;
    int linenb = 0;

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "a", 2, textColor);
    float ratio = subrat / textSurface->h;
    SDL_DestroySurface(textSurface);
    
    while(start < wordlen){
        int end = 0;
        int lastSpace = 0;
        int maxPast = 0;

        while(end + start < wordlen && maxPast == 0){
            end++;
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, word+start, end, textColor);
            int size = textSurface->w*ratio;
            if(*(word+start+end) == ' '){
                lastSpace = end;
            }else if(*(word+start+end) == '\n'){
                maxPast = 1;
                lastSpace = end+1;
            }
            if(size > maxWidth){
                maxPast = 1;
                end--;
            }
            SDL_DestroySurface(textSurface);
        }
        if(lastSpace != 0 && maxPast == 1){
            end = lastSpace;
        }
        
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, word+start, end, textColor);

        if (!textSurface) {
            printf("Unable to render text surface: %s\n", SDL_GetError());
            TTF_CloseFont(font);
            return;
        }

        // Create a texture from the surface
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(as->renderer, textSurface);

        if (!textTexture) {
            printf("Unable to create texture: %s\n", SDL_GetError());
            TTF_CloseFont(font);
            return;
        }

        // Get the dimensions of the text
        SDL_FRect textRect;
        textRect.x = x;  // Center text horizontally
        textRect.y = y - ((textTexture->h*ratio)/2.0) + ((textTexture->h*ratio)*linenb);  // Center text vertically
        textRect.w = textTexture->w * ratio; //as->width/MAX_IP_LENGTH * (*messLen);
        textRect.h = textTexture->h * ratio; //as->height/8;

        // Render the text on the screen
        SDL_RenderTexture(as->renderer, textTexture, NULL, &textRect);

        SDL_DestroySurface(textSurface);
        SDL_DestroyTexture(textTexture);
        start += end;
        linenb++;
    }
}


