#include "../includes/TCPServer.h"
#include "../includes/asymCrypto.h"
#include "../includes/savingLoading.h"
#include "../includes/symCrypto.h"
#include "../includes/common.h"


socket_t mainServerSock;


void print_key(const unsigned char* key) {
    for (size_t i = 0; i < AES_KEY_LEN; ++i) {
        printf("%02x", key[i]);
        if(i < AES_KEY_LEN-1){printf(" ");}
    }
    printf("\n\n");
}

void print_All_key(messaged_personne_t* mp) {
    print_key(mp->rcv_K1);
    print_key(mp->rcv_K2);
    print_key(mp->rcv_K3);
    print_key(mp->send_K1);
    print_key(mp->send_K2);
    print_key(mp->send_K3);
}




THREAD_RETURN listen_to_person(void* arg)
{
    combined_messaged_t* whole_pers = (combined_messaged_t*)arg;
    messaged_personne_t* pers = whole_pers->pers;
    AppState *as = whole_pers->as;
    
    char buffer[BUFFER_SIZE];
    pers->send_nonce = 0;
    pers->receive_nonce = 0;

    /*printf("Keys : ");
    print_All_key(pers);*/

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(pers->other_server, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Connection closed by peer (%s:%d).\n", pers->ip, pers->port);
            } else {
                perror("recv failed");
            }
            break;
        }
        
        message_t * mes = (message_t*)malloc(sizeof(message_t));
        mes->cypher = (unsigned char*)malloc(sizeof(char)*(bytes_received+1));
        strcpy((char*)mes->cypher, buffer);
        mes->cypher[bytes_received] = '\0';
        mes->cypherLength = bytes_received;

        dualDecrypt((unsigned char*)buffer, bytes_received, pers, &mes->message);


        printf("[From %s (%s:%d)] : %s\n", pers->pseudo, pers->ip, pers->port, mes->message);

        
        mes->who = 1;
        mes->next = pers->mess_list;
        pers->mess_list = mes;

    }

    

    messaged_personne_t **messaa = &(as->messaged_list);
    while(*messaa != pers){
        (messaa) = &((*messaa)->next);
    }
    int useless = 0;
    while(as->canrem == 0){
        useless++;
    }
    (*messaa) = pers->next;

    free(pers->ip);
    free(pers->key);
    free(pers->pseudo);
    message_t *mess = pers->mess_list;
    while(mess != NULL){
        free(mess->cypher);
        free(mess->message);
        message_t *nmess = mess->next;
        free(mess);
        mess = nmess;
    }
    // Close the connection
    CLOSESOCKET(pers->other_server);
    free(pers);
    free(whole_pers);

    

    
    return 0;
}




int connectToMainServer(AppState* as, char* serverIP){

#ifdef __MINGW32__
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    
    struct sockaddr_in server_addr;

    // Create socket
    if ((mainServerSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, serverIP, &server_addr.sin_addr) <= 0) {
        //perror("Invalid address/ Address not supported");
        //CLOSESOCKET(mainServerSock);
        //exit(EXIT_FAILURE);
        return 1;
    }

    // Connect to server
    if (connect(mainServerSock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        //perror("Connection failed bananananan");
        CLOSESOCKET(mainServerSock);
        return 1;
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);
    as->MainServ = 0;
    return 0;

}


int checkUsernameAndKey(AppState* as, char* username){

    char* publicKey;
    size_t sizeOfKey;
    loadPublicKeyAsChar(&publicKey, &sizeOfKey);

    size_t messageSize = snprintf(NULL, 0, "1\n%d\n%s\n%s", as->listeningPORT, username, publicKey);
    char* wholemessage = (char*)malloc(sizeof(char)*(messageSize+1));

    unsigned char* encmess;
    size_t encSize;
    while(as->listeningPORT == -1){
        msleep(10);
    }

    sprintf(wholemessage, "1\n%d\n%s\n%s", as->listeningPORT, username, publicKey);

    encryptBigMessageServ(wholemessage, messageSize, &encmess, &encSize);


    send(mainServerSock, (char *)encmess, encSize, 0);

    memset(wholemessage, 0, messageSize);
    size_t bytes_received = recv(mainServerSock, wholemessage, messageSize, 0);

    free(publicKey);
    size_t resultSize;
    BigRSAdecrypt(as->priv_key, (unsigned char*)wholemessage, bytes_received, &(publicKey), &resultSize);


    //printf("%s\n", publicKey);

    int ret = strcmp(publicKey, "Ok");

    free(publicKey);
    free(wholemessage);
    free(encmess);

    return ret;
}

int searchServer(AppState* as, char* word, char** result){


    size_t messageSize = strlen(word)+3;
    char wholemessage[BUFFER_SIZE];

    unsigned char* encmess;
    size_t encSize;

    sprintf(wholemessage, "2\n%s", word);

    encryptBigMessageServ(wholemessage, messageSize, &encmess, &encSize);


    send(mainServerSock, (char *)encmess, encSize, 0);

    memset(wholemessage, 0, BUFFER_SIZE);
    size_t bytes_received = recv(mainServerSock, wholemessage, BUFFER_SIZE, 0);

    /*
    for (size_t i = 0; i < bytes_received; ++i) {
        printf("%02X", (unsigned char)wholemessage[i]); // print each byte as two hex digits
        if (i < messageSize - 1) printf(" "); // optional: add space between bytes
    }
    printf("\n");*/

    size_t resultSize;
    BigRSAdecrypt(as->priv_key, (unsigned char*)wholemessage, bytes_received, result, &resultSize);

    if(strcmp(*result, "-1") == 0){
        strcpy(*result, "");
    }

    free(encmess);

    return 0;

}

int is_valid_ip_port_line(const char* line) {
    int a, b, c, d, port;
    if (sscanf(line, "%d.%d.%d.%d:%d", &a, &b, &c, &d, &port) == 5) {
        if ((a | b | c | d) < 0 || a > 255 || b > 255 || c > 255 || d > 255)
            return 0;
        if (port < 1 || port > 65535)
            return 0;
        return 1;
    }
    return 0;
}

int getFromFile(AppState* as, char* word){

    FILE* f = fopen(word, "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    // Allocate memory (+1 for null terminator)
    char* result = (char*)malloc(size + 1);
    if (!result) {
        fclose(f);
        return 1;
    }

    fread(result, 1, size, f);
    result[size] = '\0';  // Null-terminate
    fclose(f);
    printf("%s\n", result);

    const char* newline = strchr(result, '\n');
    if (!newline) return 0;

    size_t first_line_len = newline - result;
    char* first_line = (char*)malloc(first_line_len + 1);
    strncpy(first_line, result, first_line_len);
    first_line[first_line_len] = '\0';


    if (is_valid_ip_port_line(first_line)) {
        // Check for key delimiters
        if (!(strstr(result, "-----BEGIN PUBLIC KEY-----") &&
            strstr(result, "-----END PUBLIC KEY-----"))) {
            free(first_line);
            return 1;
        }
    }else{
        free(first_line);
        return 1;
    }

    free(first_line);
    
    printf("%s\n", result);

    messaged_personne_t* new_pers = (messaged_personne_t*)malloc(sizeof(messaged_personne_t));



    // Get the first line (ip:port)
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
    *colon = '\0';
    new_pers->ip = (char*)malloc(sizeof(char)*(colon-line+1));
    
    strncpy(new_pers->ip, line, colon-line+1);

    new_pers->port = atoi(colon+1);

    //printf("%s:%d\n", new_pers->ip, new_pers->port);

    // Collect the rest as the public key
    //printf("%d, %s", strlen(key), key);
    new_pers->key = (char*)malloc(sizeof(char)*(strlen(key)+1));


    strncpy(new_pers->key, key, strlen(key));
    new_pers->key[strlen(key)] = '\0';

    new_pers->pseudo = (char*)malloc(sizeof(char)*(strlen(word)+1));
    strcpy(new_pers->pseudo, word);


    if(tryConnectSomeone(as, new_pers) == 1){
        free(new_pers->ip);
        free(new_pers->key);
        free(new_pers->pseudo);
        free(new_pers);
        return 1;
    }
    memset(new_pers->typing, 0, MAX_INPUT_LENGTH);
    new_pers->typingLength = 0;
    new_pers->mess_list = NULL;
    new_pers->next = NULL;

    messaged_personne_t** pers_append = &(as->messaged_list);
    while(*pers_append != NULL){
        pers_append = &((*pers_append)->next);
    }
    *pers_append = new_pers;

    free(result);

    return 0;
}

int getFromServer(AppState* as, char* word){

    char* result;
    size_t messageSize = strlen(word)+3;
    char wholemessage[BUFFER_SIZE];

    unsigned char* encmess;
    size_t encSize;

    sprintf(wholemessage, "3\n%s", word);

    encryptBigMessageServ(wholemessage, messageSize, &encmess, &encSize);


    send(mainServerSock, (char *)encmess, encSize, 0);

    memset(wholemessage, 0, BUFFER_SIZE);

    size_t bytes_received = recv(mainServerSock, wholemessage, BUFFER_SIZE, 0);


    /*for (size_t i = 0; i < bytes_received; ++i) {
        printf("%02X", (unsigned char)wholemessage[i]); // print each byte as two hex digits
        if (i < messageSize - 1) printf(" "); // optional: add space between bytes
    }
    printf("\n");*/

    size_t resultSize;
    BigRSAdecrypt(as->priv_key, (unsigned char*)wholemessage, bytes_received, &result, &resultSize);

    free(encmess);

    if(strcmp(result, "NOk") == 0){
        free(result);
        return 1;
    }

    //printf("%s\n", result);

    messaged_personne_t* new_pers = (messaged_personne_t*)malloc(sizeof(messaged_personne_t));



    // Get the first line (ip:port)
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
    *colon = '\0';
    new_pers->ip = (char*)malloc(sizeof(char)*(colon-line+1));
    
    strncpy(new_pers->ip, line, colon-line+1);

    new_pers->port = atoi(colon+1);

    //printf("%s:%d\n", new_pers->ip, new_pers->port);

    // Collect the rest as the public key
    //printf("%d, %s", strlen(key), key);
    new_pers->key = (char*)malloc(sizeof(char)*(strlen(key)+1));


    strncpy(new_pers->key, key, strlen(key));
    new_pers->key[strlen(key)] = '\0';

    new_pers->pseudo = (char*)malloc(sizeof(char)*(strlen(word)+1));
    strcpy(new_pers->pseudo, word);


    if(tryConnectSomeone(as, new_pers) == 1){
        free(new_pers->ip);
        free(new_pers->key);
        free(new_pers->pseudo);
        free(new_pers);
        return 1;
    }
    memset(new_pers->typing, 0, MAX_INPUT_LENGTH);
    new_pers->typingLength = 0;
    new_pers->mess_list = NULL;
    new_pers->next = NULL;

    messaged_personne_t** pers_append = &(as->messaged_list);
    while(*pers_append != NULL){
        pers_append = &((*pers_append)->next);
    }
    msleep(2);
    *pers_append = new_pers;


    free(result);

    return 0;

}


int clearConnexionToMainServer(AppState* as){
    CLOSESOCKET(mainServerSock);

#ifdef __MINGW32__
    WSACleanup();
#endif 

    messaged_personne_t* new_pers = as->messaged_list;

    while(new_pers != NULL){
        free(new_pers->ip);
        free(new_pers->key);
        free(new_pers->pseudo);
        message_t *mess = new_pers->mess_list;
        while(mess != NULL){
            free(mess->cypher);
            free(mess->message);
            message_t *nmess = mess->next;
            free(mess);
            mess = nmess;
        }
        // Close the connection
        CLOSESOCKET(new_pers->other_server);
        messaged_personne_t* next = new_pers->next;
        free(new_pers);
        new_pers = next;
    }


    return 0;
}


void msleep(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000); // usleep takes microseconds
    #endif
}






int tryConnectSomeone(AppState* as, messaged_personne_t* new_pers){

    
        // Create socket
        if ((new_pers->other_server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
    
        // Server address setup
        new_pers->other_addr.sin_family = AF_INET;
        new_pers->other_addr.sin_port = htons(new_pers->port);
    
        if (inet_pton(AF_INET, new_pers->ip, &(new_pers->other_addr.sin_addr)) <= 0) {
            perror("Invalid address/ Address not supported");
            CLOSESOCKET(new_pers->other_server);
            exit(EXIT_FAILURE);
        }
    
        // Connect to server
        if (connect(new_pers->other_server, (struct sockaddr*)&new_pers->other_addr, sizeof(new_pers->other_addr)) == SOCKET_ERROR) {
            //perror("Connection failed bananananan");
            CLOSESOCKET(new_pers->other_server);
            return 1;
        }


        char* publicKey;
        size_t sizeOfKey;
        loadPublicKeyAsChar(&publicKey, &sizeOfKey);

        //size_t messageSize = strlen(as->gmprm.pseudo)+sizeOfKey+5;
        size_t messageSize = snprintf(NULL, 0, "%s\n%s", as->gmprm.pseudo, publicKey);
        char* wholemessage = (char*)malloc(sizeof(char)*(messageSize+1));

        unsigned char* encmess;
        size_t encSize;

        sprintf(wholemessage, "%s\n%s", as->gmprm.pseudo, publicKey);
        

        encryptBigMessageFromKey(new_pers->key, wholemessage, messageSize, &encmess, &encSize);


        send(new_pers->other_server, (char *)encmess, encSize, 0);

        memset(wholemessage, 0, messageSize);
        size_t bytes_received = recv(new_pers->other_server, wholemessage, messageSize, 0);

        free(publicKey);
        size_t resultSize;
        
        BigRSAdecrypt(as->priv_key, (unsigned char*)wholemessage, bytes_received, &(publicKey), &resultSize);

        /*printf("Decrypted : \n");
        for (size_t i = 0; i < resultSize; ++i) {
            printf("%02x", (unsigned char)publicKey[i]);
            if(i < resultSize-1){printf(" ");}
        }
        printf("\n");*/

       
        memcpy(new_pers->rcv_K1, publicKey, AES_KEY_LEN);
        memcpy(new_pers->rcv_K2, publicKey + AES_KEY_LEN+1, AES_KEY_LEN);
        memcpy(new_pers->rcv_K3, publicKey + 2 * AES_KEY_LEN+2, AES_KEY_LEN);

        generate_random_keys(new_pers);


        size_t total_len = AES_KEY_LEN * 3 + 2;
        char* message = (char*)malloc(sizeof(char)*(total_len));
        //sprintf(message, "%s\n%s\n%s", new_pers->send_K1, new_pers->send_K2, new_pers->send_K3);
        memcpy(message, new_pers->send_K1, AES_KEY_LEN);
        message[AES_KEY_LEN] = '\n';

        memcpy(message + AES_KEY_LEN + 1, new_pers->send_K2, AES_KEY_LEN);
        message[2 * AES_KEY_LEN + 1] = '\n';

        memcpy(message + 2 * AES_KEY_LEN + 2, new_pers->send_K3, AES_KEY_LEN);

        free(encmess);

        encryptBigMessageFromKey(new_pers->key, message, total_len, &encmess, &encSize);

        send(new_pers->other_server, (char *)encmess, encSize, 0);




        free(publicKey);
        free(wholemessage);
        free(encmess);

        combined_messaged_t *perper = (combined_messaged_t*)malloc(sizeof(combined_messaged_t));
        perper->as = as;
        perper->pers = new_pers;

        #ifdef __MINGW32__
            HANDLE hThread = CreateThread(NULL, 0, listen_to_person, (void*)perper, 0, NULL);
            if (hThread == NULL) {
                fprintf(stderr, "CreateThread failed\n");
                CLOSESOCKET(new_pers->other_server);
                free(new_pers);
                return 1;
            }
            CloseHandle(hThread); // Detach
        #else
            pthread_t tid;
            if (pthread_create(&tid, NULL, listen_to_person, (void*)perper) != 0) {
                perror("pthread_create failed");
                CLOSESOCKET(new_pers->other_server);
                free(new_pers);
                return 1;
            }
            pthread_detach(tid);
        #endif


        return 0;
    }
















    

THREAD_RETURN serverHandler(void* args) {
    AppState* as = (AppState*) args;

#ifdef __MINGW32__
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    char isfolder;
#ifdef __MINGW32__
    DWORD attrs = GetFileAttributesA(ALL_FILE_FOLDER);
    isfolder = (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    isfolder = (stat(ALL_FILE_FOLDER, &st) == 0 && S_ISDIR(st.st_mode));
#endif

    if (!isfolder) {
#ifdef __MINGW32__
        CreateDirectoryA(ALL_FILE_FOLDER, NULL);
#else
        mkdir(ALL_FILE_FOLDER, 0755);
#endif
    }

    socket_t server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = 0;  // let the system pick a random port

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("bind failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == SOCKET_ERROR) {
        perror("listen failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }

    // Get actual IP and port
    char ip_str[INET_ADDRSTRLEN];
    struct sockaddr_in actual_addr;
    socklen_t actual_len = sizeof(actual_addr);

    if (getsockname(server_fd, (struct sockaddr*)&actual_addr, &actual_len) == -1) {
        perror("getsockname failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }

#ifdef __MINGW32__
    if (InetNtop(AF_INET, &actual_addr.sin_addr, ip_str, INET_ADDRSTRLEN) == NULL) {
        perror("InetNtop failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }
#else
    if (inet_ntop(AF_INET, &actual_addr.sin_addr, ip_str, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop failed");
        CLOSESOCKET(server_fd);
        exit(EXIT_FAILURE);
    }
#endif

    as->listeningPORT = ntohs(actual_addr.sin_port);

    printf("Server listening on %s:%d\n", ip_str, ntohs(actual_addr.sin_port));

    while (1) {
        messaged_personne_t* new_pers = (messaged_personne_t*)malloc(sizeof(messaged_personne_t));

        new_pers->other_server = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (new_pers->other_server == INVALID_SOCKET) {
            perror("accept failed");
            free(new_pers);
            continue;
        }

        // Get and print client IP and port
    char client_ip[INET_ADDRSTRLEN];
    uint16_t client_port = ntohs(client_addr.sin_port);


    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop failed");
    } else {

        
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);


        int bytes_received = recv(new_pers->other_server, buffer, BUFFER_SIZE, 0);

        char* result;
        size_t resultSize;
   
        BigRSAdecrypt(as->priv_key, (unsigned char*)buffer, bytes_received, &(result), &resultSize);

        

        char* key = strchr(result, '\n');
        *key = '\0';
        key++;


        new_pers->ip = (char*)malloc(sizeof(char)*(strlen(client_ip)+1));
        
        strncpy(new_pers->ip, client_ip, strlen(client_ip));
        new_pers->ip[strlen(client_ip)] = '\0';

        new_pers->port = client_port;

        new_pers->key = (char*)malloc(sizeof(char)*(strlen(key)+1));


        strncpy(new_pers->key, key, strlen(key));
        new_pers->key[strlen(key)] = '\0';


        new_pers->pseudo = (char*)malloc(sizeof(char)*strlen(result)+1);
        strcpy(new_pers->pseudo, result);

        printf("\rConnection from %s:%d by %s\n>", client_ip, client_port, new_pers->pseudo);

        free(result);


        generate_random_keys(new_pers);

        size_t total_len = AES_KEY_LEN * 3 + 2;
        char* message = (char*)malloc(sizeof(char)*(total_len));

        memcpy(message, new_pers->send_K1, AES_KEY_LEN);
        message[AES_KEY_LEN] = '\n';

        memcpy(message + AES_KEY_LEN + 1, new_pers->send_K2, AES_KEY_LEN);
        message[2 * AES_KEY_LEN + 1] = '\n';

        memcpy(message + 2 * AES_KEY_LEN + 2, new_pers->send_K3, AES_KEY_LEN);


        unsigned char* encmess;
        size_t encSize;

  
        encryptBigMessageFromKey(new_pers->key, message, total_len, &encmess, &encSize);

        send(new_pers->other_server, (char *)encmess, encSize, 0);

        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(new_pers->other_server, buffer, BUFFER_SIZE, 0);

        free(message);
        BigRSAdecrypt(as->priv_key, (unsigned char*)buffer, bytes_received, &(message), &resultSize);


        
        memcpy(new_pers->rcv_K1, message, AES_KEY_LEN);
        memcpy(new_pers->rcv_K2, message + AES_KEY_LEN+1, AES_KEY_LEN);
        memcpy(new_pers->rcv_K3, message + 2 * AES_KEY_LEN+2, AES_KEY_LEN);


        messaged_personne_t** pers_append = &(as->messaged_list);
        while(*pers_append != NULL){
            pers_append = &((*pers_append)->next);
        }

        memset(new_pers->typing, 0, MAX_INPUT_LENGTH);
        new_pers->typingLength = 0;
        new_pers->mess_list = NULL;
        new_pers->next = NULL;

        *pers_append = new_pers;
        free(message);
        free(encmess);

        combined_messaged_t *perper = (combined_messaged_t*)malloc(sizeof(combined_messaged_t));
        perper->as = as;
        perper->pers = new_pers;

        #ifdef __MINGW32__
            HANDLE hThread = CreateThread(NULL, 0, listen_to_person, (void*)perper, 0, NULL);
            if (hThread == NULL) {
                fprintf(stderr, "CreateThread failed\n");
                CLOSESOCKET(new_pers->other_server);
                free(new_pers);
                continue;
            }
            CloseHandle(hThread); // Detach
        #else
            pthread_t tid;
            if (pthread_create(&tid, NULL, listen_to_person, (void*)perper) != 0) {
                perror("pthread_create failed");
                CLOSESOCKET(new_pers->other_server);
                free(new_pers);
                continue;
            }
            pthread_detach(tid);
        #endif
        } 
    }

    CLOSESOCKET(server_fd);

#ifdef __MINGW32__
    WSACleanup();
#endif

    return 0;
}








int sendMessagePersonne(AppState* as, int number, char* message){

    messaged_personne_t* pers_send = as->messaged_list;
    for(int i = 1; i < number; i++){
        pers_send = (pers_send->next);
    }

    message_t * mes = (message_t*)malloc(sizeof(message_t));
    mes->message = (char*)malloc(sizeof(char)*(strlen(message)+1));
    strcpy(mes->message, message);
    mes->message[strlen(message)] = '\0';
    mes->who = 0;


    dualEncrypt(message, strlen(message), pers_send, &mes->cypher, (size_t*)&mes->cypherLength);



    send(pers_send->other_server, (char *)mes->cypher, mes->cypherLength, 0);
    mes->next = pers_send->mess_list;
    pers_send->mess_list = mes;


    return 0;
}