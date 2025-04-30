#include "../includes/clientHandler.h"
#include "../includes/RSAcrypto.h"

THREAD_RETURN client_handler(void* arg) {
    socket_t client_fd = *(socket_t*)arg;
    free(arg);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Get client address info
    if (getpeername(client_fd, (struct sockaddr*)&client_addr, &addr_len) == -1) {
        exit(EXIT_FAILURE);
    }

    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    char* clientKey = NULL;


    // Convert IP to string
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    client_port = ntohs(client_addr.sin_port);

    printf("Client connected: %s:%d\n", client_ip, client_port);

    char buffer[BUFFER_SIZE];


    while (1) {
        char* mess;
        size_t messSize;

        memset(buffer, 0, BUFFER_SIZE);

        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            break;
        }


        decryptBigServ((unsigned char*) buffer, bytes_received, &mess, &messSize);


        int type = 0;

        char* wholeMessage = (char*)malloc(sizeof(char)*messSize);

        char* restOfMessage = strchr(mess, '\n');
        if (restOfMessage) {
            type = atoi(mess);              // parse the integer at the start
            restOfMessage++;                // move past the '\n'
            strcpy(wholeMessage, restOfMessage); // copy the rest
        }
        
            

        if(type == 1){ //we get a public key
          
            char* Pseudo;
            int port = atoi(restOfMessage);
            char* restOfMessage = strchr(wholeMessage, '\n')+1;
            Pseudo = restOfMessage;
            restOfMessage = strchr(restOfMessage, '\n');
            strcpy(mess, restOfMessage+1);
            *restOfMessage = '\0';
            // printf("We get : \nType = %d\nPort= %d Pseudo = %s\n\nKey = %s\nEND of COMM\n", type, port, Pseudo, mess);
            if(checkFileContent(Pseudo, mess, client_ip, port) == 0){
                strcpy(buffer, "Ok");
            }else{
                strcpy(buffer, "No");
            }

            clientKey = loadKeyFromFile(Pseudo);
            size_t message_size;
            unsigned char * encMes;
            encryptFromCharKey(clientKey, buffer, 3, &encMes, &message_size);
            

            send(client_fd, (char *)encMes, message_size, 0);
            free(encMes);
        }else if(type == 2){
            //printf("%s\n", wholeMessage);
            //strcpy(buffer, "Bite");
            memset(buffer, 0, BUFFER_SIZE);
            listMatchingFiles(wholeMessage, buffer);
            if(strcmp(buffer, "") == 0){
                strcpy(buffer, "-1");
            }

            size_t message_size;
            unsigned char * encMes;

            //printf("%s\n", buffer);
            encryptBigMessageFromKey(clientKey, buffer, strlen(buffer), &encMes, &message_size);
            /*
            for (size_t i = 0; i < message_size; ++i) {
                printf("%02X", encMes[i]); // print each byte as two hex digits
                if (i < message_size - 1) printf(" "); // optional: add space between bytes
            }
            printf("\n");*/   
            

            send(client_fd, (char *)encMes, message_size, 0);
            free(encMes);

        }else if(type == 3){
            //printf("%s\n", wholeMessage);
            //strcpy(buffer, "Bite");
            memset(buffer, 0, BUFFER_SIZE);
            if(readFileContent(wholeMessage, buffer) != 0){
                strcpy(buffer, "NOk");
            }

            size_t message_size;
            unsigned char * encMes;

            //printf("%d\n%s\n", strlen(buffer), buffer);
            encryptBigMessageFromKey(clientKey, buffer, strlen(buffer), &encMes, &message_size);

            /*for (size_t i = 0; i < message_size; ++i) {
                printf("%02X", encMes[i]); // print each byte as two hex digits
                if (i < message_size - 1) printf(" "); // optional: add space between bytes
            }
            printf("\n");*/
            

            send(client_fd, (char *)encMes, message_size, 0);
            free(encMes);

        }
    }

    CLOSESOCKET(client_fd);
    if(clientKey)free(clientKey);

#ifdef __MINGW32__
    return 0;
#else
    pthread_exit(NULL);
#endif
}

ssize_t my_getline(char **lineptr, size_t *n, FILE *stream) {
    if (!lineptr || !n || !stream) return -1;

    char *buf = *lineptr;
    size_t size = *n;
    size_t len = 0;
    int c;

    if (buf == NULL || size == 0) {
        size = 128;
        buf = (char *)malloc(size);
        if (!buf) return -1;
    }

    while ((c = fgetc(stream)) != EOF) {
        if (len + 1 >= size) {
            size *= 2;
            char *newbuf = (char *)realloc(buf, size);
            if (!newbuf) {
                free(buf);
                return -1;
            }
            buf = newbuf;
        }
        buf[len++] = (char)c;
        if (c == '\n') break;
    }

    if (len == 0 && c == EOF) {
        free(buf);
        return -1;
    }

    buf[len] = '\0';
    *lineptr = buf;
    *n = size;

    return (ssize_t)len;
}


int readFileContent(const char* fileName, char* content) {
    char file_name[MAX_LENGTH_FILE_NAME];
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, fileName);
    //printf("%s\n", file_name);
    FILE* file = fopen(file_name, "r");  // Open in read mode
    if (!file) {
        //printf("Failed to open file: %s\n", fileName);
        return 1;
    }

    // Go to the end of the file to get the size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);  // Go back to the start


    // Read the entire file into memory
    size_t bytesRead = fread(content, 1, fileSize, file);
    content[bytesRead] = '\0';  // Null-terminate the string

    fclose(file);
    return 0;
}



int listMatchingFiles(const char* pattern, char* result) {


#ifdef __MINGW32__
    WIN32_FIND_DATAA findFileData;
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*", ALL_FILE_FOLDER);

    HANDLE hFind = FindFirstFileA(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Failed to open folder: %s\n", ALL_FILE_FOLDER);
        return 1;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (strstr(findFileData.cFileName, pattern)) {
                strncat(result, findFileData.cFileName, MAX_RESULT_SIZE - strlen(result) - 2);
                strncat(result, "\n", MAX_RESULT_SIZE - strlen(result) - 1);
            }
        }
    } while (FindNextFileA(hFind, &findFileData));

    FindClose(hFind);

#else  // Linux / Unix
    DIR* dir;
    struct dirent* entry;


    dir = opendir(ALL_FILE_FOLDER);
    if (!dir) {
        printf("Failed to open folder: %s\n", ALL_FILE_FOLDER);
        return 1;
    }


    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_REG) {  // regular file
            if (strstr(entry->d_name, pattern)) {
                strncat(result, entry->d_name, BUFFER_SIZE - strlen(result) - 2);
                strncat(result, "\n", BUFFER_SIZE - strlen(result) - 1);
            }
        }
    }


    closedir(dir);
#endif

    return 0;
}


int checkFileContent(const char* wholeMessage, const char* thisMessage, const char* client_ip, const int client_port) {
    char file_name[MAX_LENGTH_FILE_NAME];
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, wholeMessage);

    FILE* file = fopen(file_name, "r");
    if (!file) {
        // Create new file
        file = fopen(file_name, "w");
        if (!file) {
            printf("Failed to create file '%s'\n", file_name);
            return 1;
        }
        fprintf(file, "%s:%d\n%s", client_ip, client_port, thisMessage);
        fclose(file);
        return 0;  // New file created
    }

    // Read the first line (IP:port)
    char* line = NULL;
    size_t len = 0;
    ssize_t read = my_getline(&line, &len, file);
    if (read == -1) {
        fclose(file);
        free(line);
        printf("Failed to read from file '%s'\n", file_name);
        return 1;
    }

    // Parse the IP and port from the first line
    char file_ip[INET_ADDRSTRLEN];
    char file_port[16];
    if (sscanf(line, "%[^:]:%s", file_ip, file_port) != 2) {
        fclose(file);
        free(line);
        printf("Invalid file format in '%s'\n", file_name);
        return 1;
    }

    // Read the message content
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, read, SEEK_SET);

    char* buffer = (char*)malloc(fileSize - read + 1);
    if (!buffer) {
        fclose(file);
        free(line);
        printf("Memory allocation failed.\n");
        return 1;
    }
    fread(buffer, 1, fileSize - read, file);
    buffer[fileSize - read] = '\0';

    fclose(file);
    free(line);

    int needUpdate = 0;

    // Check IP
    if (strcmp(file_ip, client_ip) != 0) {
        needUpdate = 1;  // IP changed — full rewrite
    }
    // Check message
    else if (strcmp(buffer, thisMessage) != 0) {
        needUpdate = 1;  // Message changed
    }
    // Check port
    else if (atoi(file_port) != client_port) {
        needUpdate = 2;  // Only port changed
    }

    // If updates needed
    if (needUpdate == 2) {
        file = fopen(file_name, "w");
        if (!file) {
            printf("Failed to open file '%s' for writing\n", file_name);
            free(buffer);
            return 1;
        }
        fprintf(file, "%s:%d\n%s", client_ip, client_port, thisMessage);
        fclose(file);
        needUpdate = 0;
    }

    free(buffer);
    return needUpdate;  // 0 = ok, 1 = Wrong, 2 = port updated
}




char* loadKeyFromFile(const char* wholeMessage) {
    char file_name[MAX_LENGTH_FILE_NAME];
    sprintf(file_name, "%s%s", ALL_FILE_FOLDER, wholeMessage);

    FILE* file = fopen(file_name, "r");
    if (!file) {
        printf("File '%s' not found.\n", file_name);
        return NULL;
    }

    // Skip the first line (IP:port)
    char* line = NULL;
    size_t len = 0;
    ssize_t read = my_getline(&line, &len, file);
    if (read == -1) {
        printf("Failed to read header line in '%s'\n", file_name);
        fclose(file);
        free(line);
        return NULL;
    }
    free(line);

    // Read the rest as the key
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, read, SEEK_SET);  // start reading after the header line

    char* keyBuffer = (char*)malloc(fileSize - read + 1);
    if (!keyBuffer) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    fread(keyBuffer, 1, fileSize - read, file);
    keyBuffer[fileSize - read] = '\0';  // Null-terminate

    fclose(file);
    return keyBuffer;  // Caller must free this
}

