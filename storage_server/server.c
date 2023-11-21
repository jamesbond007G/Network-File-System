#include "server2.h"
#include "semaphore.h"
typedef struct
{
    char ip[32];
    int port;
    int client_port;
    int server_num;
    int query_port;
    char fullpath[1024];

} My_info;
int server_socket;
sem_t seama;
sem_t seama1;

struct sockaddr_in server_address;
int server_socket2;
struct sockaddr_in server_address2;
// int l = 0;
int getRandomNumber(int min, int max)
{
    return min + rand() % (max - min + 1);
}
int PERSONAL_PORT;
int CLIENT_PORT;
void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void createTar_self(const char *srcFolder, const char *tarFile)
{
    char command[1024];

    // Use the "tar" command to create a tar archive from the source folder
    // snprintf(command, sizeof(command), "tar -cf %s -C %s .", tarFile, srcFolder);
    snprintf(command, sizeof(command), "tar -cf %s %s", tarFile, srcFolder);
    // if (system(command) == -1)
    // {
    //     error("Tar creation command failed");
    // }
}

void extractTar_self(const char *tarFile, const char *destFolder)
{
    char command[1024];

    // Use the "tar" command to extract the tar archive to the destination folder
    snprintf(command, sizeof(command), "tar -xf %s -C %s", tarFile, destFolder);

    // if (system(command) == -1)
    // {
    //     error("Tar extraction command failed");
    // }
}

void copyself(const char *srcFolder, const char *destFolder)
{
    const char *tarFileName = "archive.tar";
    createTar_self(srcFolder, tarFileName);
    extractTar_self(tarFileName, destFolder);
    system("rm ./archive.tar");
}
// void error(const char *msg)
// {
//     perror(msg);
//     exit(EXIT_FAILURE);
// }

void untarFile(char destination[])
{
    char command[2000];
    // snprintf(command, sizeof(command), "tar -xf new.tar");
    snprintf(command, sizeof(command), "tar -xf new.tar -C %s", destination);
    if (system(command) == -1)
    {
        error("Untar command failed");
    }
}

void uncompress(int clientSocket, char destination[])
{
    int tarFile = open("new.tar", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (tarFile == -1)
    {
        error("Unable to create tar file");
    }

    char buffer[2000];
    ssize_t bytesRead;

    // Receive and write the tar file
    if ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (write(tarFile, buffer, bytesRead) == -1)
        {
            error("Error writing to tar file");
        }
    }

    // Close sockets and tar file
    close(tarFile);

    // Untar the received file
    untarFile(destination);
}
void tarFolder(const char folderPath[])
{
    char excludePattern[] = "SS*";

    char command[BUFFER_SIZE + 50]; // Increased buffer size to accommodate the command
    snprintf(command, sizeof(command), "tar --exclude='%s' -cf - %s", excludePattern, folderPath);

    int tarPipe[2];
    if (pipe(tarPipe) == -1)
    {
        error("Pipe creation failed");
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        error("Fork failed");
    }

    if (pid == 0)
    {
        // Child process (tar)
        close(tarPipe[0]);               // Close read end
        dup2(tarPipe[1], STDOUT_FILENO); // Redirect stdout to pipe
        execlp("tar", "tar", "--exclude", excludePattern, "-cf", "-", folderPath, (char *)NULL);
        error("Exec failed");
    }
    else
    {
        // Parent process
        close(tarPipe[1]); // Close write end
        int tarFile = open("output.tar", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (tarFile == -1)
        {
            error("Unable to create tar file");
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;

        while ((bytesRead = read(tarPipe[0], buffer, sizeof(buffer))) > 0)
        {
            if (write(tarFile, buffer, bytesRead) == -1)
            {
                error("Error writing to tar file");
            }
        }

        close(tarPipe[0]);
        close(tarFile);
    }
}


void compress(char folder[], int clientSocket)
{

    tarFolder(folder);
    int tarFile = open("output.tar", O_RDONLY);
    if (tarFile == -1)
    {
        error("Unable to open tar file");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(tarFile, buffer, sizeof(buffer))) > 0)
    {
        if (send(clientSocket, buffer, bytesRead, 0) == -1)
        {
            error("Error sending tar file");
        }
    }
    close(tarFile);
}

void receive_and_extract_tar()
{
    // Receive tar data from the socket
    char buffer[BUFFER_SIZE];
    FILE *tar_file = fopen("copy.tar", "wb");
    if (tar_file == NULL)
    {
        perror("Error opening tar file for writing");
        exit(EXIT_FAILURE);
    }

    // ssize_t bytesRead;
    // while ((bytesRead = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
    //     fwrite(buffer, 1, bytesRead, tar_file);
    // }

    fclose(tar_file);

    // Extract tar archive using system command

    const char *extract_command = "tar -xf .tar";
    int result = system(extract_command);

    if (result == 0)
    {
        printf("Tar archive extracted successfully.\n");
    }
    else
    {
        fprintf(stderr, "Error extracting tar archive.\n");
    }

    // Cleanup: Remove the received tar file
    remove("copy.tar");
}
int create_file(char path[], char name[])
{
    FILE *file;
    char fullPath[256]; // Adjust the size as needed

    // Concatenate path and filename to create the full path
    sprintf(fullPath, "%s/%s", path, name);
    printf("ok%sok", fullPath);
    // Create the file
    file = fopen(fullPath, "w");

    if (file == NULL)
    {
        printf("Unable to create file.\n");
        return 0;
    }

    // Optionally, you can write to the file
    fprintf(file, "Hello, this is your file content.\n");

    // Close the file
    fclose(file);

    printf("File created at: %s\n", fullPath);
    return 1;
}
int create_directory(char path[], char name[])
{
    char fullPath[256]; // Adjust the size as needed

    // Concatenate path and directory name to create the full path
    // sprintf(fullPath, "%s/%s", path, name);
    strcpy(fullPath, path);
    strcat(fullPath, "/");
    strcat(fullPath, name);
    // Create the directory
    printf("%s\n", fullPath);
    int status = mkdir(fullPath, 0777); // 0777 gives read, write, and execute permissions to owner, group, and others

    if (status == 0)
    {
        printf("Directory created at: %s\n", fullPath);
        return 1;
    }
    else
    {
        printf("Unable to create directory.\n");
        return 0;
    }
}
int delete_file(char path[])
{
    char fullPath[256]; // Adjust the size as needed

    // Concatenate path and filename to create the full path
    // sprintf(fullPath, "%s%s", path, name);
    strcpy(fullPath, path);
    // Delete the file
    int status = remove(fullPath);

    if (status == 0)
    {
        printf("File deleted: %s\n", fullPath);
        return 1;
    }
    else
    {
        printf("Unable to delete file.\n");
        return 0;
    }
}

// void delete_directory(char path[], char name[])
// {
//     char fullPath[256]; // Adjust the size as needed

//     // Concatenate path and directory name to create the full path
//     sprintf(fullPath, "%s/%s", path, name);

//     // Delete the directory
//     int status = rmdir(fullPath);

//     if (status == 0)
//     {
//         printf("Directory deleted: %s\n", fullPath);
//     }
//     else
//     {
//         printf("Unable to delete directory.\n");
//     }
// }

char *getIpAddress()
{
    FILE *fp;
    char *command = "ifconfig | grep 'inet ' | grep -v '127.0.0.1' | awk '{print $2}'";
    char *result = malloc(16); // Assuming IPv4 address length

    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("Failed to run command");
        exit(EXIT_FAILURE);
    }

    if (fgets(result, 16, fp) == NULL)
    {
        perror("Failed to read output");
        exit(EXIT_FAILURE);
    }

    pclose(fp);

    // Remove newline character from the result
    result[strcspn(result, "\n")] = 0;

    return result;
}
void listFilesRecursivelyAndSend(const char *basePath, char actualpath[])
{
    DIR *dir = opendir(basePath);
    if (dir == NULL)
    {
        // perror("Error opening directory");
        // exit(EXIT_FAILURE);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        char fullPath[1024];
        if (strlen(entry->d_name) >= 2 && entry->d_name[0] == 'S' && entry->d_name[1] == 'S')
        {
            return;
        }
        snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, entry->d_name);
        strcat(actualpath, fullPath);
        strcat(actualpath, "\n");

        if (entry->d_type == DT_DIR)
        {
            listFilesRecursivelyAndSend(fullPath, actualpath);
        }
    }

    closedir(dir);
}

// int deleteDirectory(const char *path) {
//     if (path == NULL) {
//         fprintf(stderr, "Invalid path\n");
//         return -1;
//     }

//     if (rmdir(path) == 0) {
//         printf("Directory '%s' deleted successfully\n", path);
//         return 0;
//     } else {
//         perror("Error deleting directory");
//         return -1;
//     }
// }

// void deleteDirectory(const char *path);

int deleteFile(const char path[])
{
    if (remove(path) != 0)
    {
        perror("Error deleting file");
        return 0;
    }
    else
    {
        printf("Removed file: %s\n", path);
        return 1;
    }
}

void deleteDirectoryRecursive(const char path[], int *return_value)
{
    DIR *dir = opendir(path);
    struct dirent *entry;

    if (!dir)
    {
        *return_value = 0;
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char *fullPath = malloc(strlen(path) + strlen(entry->d_name) + 2);
            if (fullPath)
            {
                sprintf(fullPath, "%s/%s", path, entry->d_name);

                if (entry->d_type == DT_DIR)
                {
                    deleteDirectoryRecursive(fullPath, return_value);
                }
                else
                {
                    if (!deleteFile(fullPath))
                    {
                        *return_value = 0;
                    }
                }

                free(fullPath);
            }
            else
            {
                perror("Memory allocation error");
                closedir(dir);
                *return_value = 0;
                return;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) != 0)
    {
        *return_value = 0;
        perror("Error deleting directory");
    }
    else
    {
        printf("Removed directory: %s\n", path);
    }
}
void copysame(char sourcePath[], char destPath[])
{
    char chdirCommand[1024];
    char createCommand[1024];
    char copyCommand[1024];
    char extractCommand[10240];
    struct stat fileStat;
    char d[1024];
    int idx;
    char dir_name[1024];
    char copy_name_of_source[1024];
    strcpy(copy_name_of_source, sourcePath);
    if (stat(sourcePath, &fileStat) == 0)
    {
        if (S_ISDIR(fileStat.st_mode))
        {
            for (int i = strlen(sourcePath) - 1; i >= 0; i--)
            {
                if (sourcePath[i] == '/')
                {
                    idx = i;
                    break;
                }
            }
            // printf()
            for (int i = idx; i < strlen(sourcePath); i++)
            {
                dir_name[i - idx] = copy_name_of_source[i];
                // printf("%c %d hello\n",dir_name[i-idx],i-idx);
            }
            // printf("%s\n",dir_name);
            snprintf(createCommand, sizeof(createCommand), "tar -czf  copysame.tar -C %s .", sourcePath);
            // if (system(createCommand) == -1)
            // {
            //     perror("Tar creation command failed");
            //     exit(EXIT_FAILURE);
            // }
            snprintf(copyCommand, sizeof(copyCommand), "cp copysame.tar %s", destPath);
            // if (system(copyCommand) == -1)
            // {
            //     perror("Copy command failed");
            //     exit(EXIT_FAILURE);
            // }

            char new_dir[1024];
            strcpy(new_dir, destPath);
            strcat(new_dir, dir_name);
            mkdir(new_dir, 0777);
            printf("%s\n", new_dir);
            snprintf(extractCommand, sizeof(extractCommand), "tar -xf copysame.tar -C %s", new_dir);
            // if (system(extractCommand) == -1)
            // {
            //     perror("Tar extraction command failed");
            //     exit(EXIT_FAILURE);
            // }
            snprintf(copyCommand, sizeof(copyCommand), "rm copysame.tar ");
            // if (system(copyCommand) == -1)
            // {
            //     perror("Copy command failed");
            //     exit(EXIT_FAILURE);
            // }
        }
        else if (S_ISREG(fileStat.st_mode))
        {
            snprintf(copyCommand, sizeof(copyCommand), "cp %s %s", sourcePath, destPath);
            // if (system(copyCommand) == -1)
            // {
            //     perror("Copy command failed");
            //     exit(EXIT_FAILURE);
            // }
        }
    }
}
pthread_t temporary_thread;
void *listen_thread1(void *arg)
{
    between_ss r1 = *(between_ss *)arg;
    int index_f_last_slash = -1;
    for (int lof = strlen(r1.path) - 1; lof >= 0; lof--)
    {
        if (r1.path[lof] == '/')
        {
            index_f_last_slash = lof;
            break;
        }
    }
    char go[1024];
    int k = 0;
    for (int lof1 = index_f_last_slash; lof1 < strlen(r1.path); lof1++)
    {
        go[lof1 - index_f_last_slash] = r1.path[lof1];
        k++;
        /* code */
    }
    go[k] = '\0';
    // printf("fileorfoldernameg%sg", go);

    int serverSocket1, clientSocket1;
    struct sockaddr_in serverAddr1, clientAddr1;
    socklen_t addrSize1 = sizeof(struct sockaddr_in);

    // Create socket
    if ((serverSocket1 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        error("Socket creation failed");
    }

    // Configure server address struct
    serverAddr1.sin_family = AF_INET;
    serverAddr1.sin_addr.s_addr = INADDR_ANY;
    serverAddr1.sin_port = htons(r1.port);

    // Bind socket
    if (bind(serverSocket1, (struct sockaddr *)&serverAddr1, sizeof(serverAddr1)) == -1)
    {
        error("Socket bind failed");
    }

    // Listen for incoming connections
    if (listen(serverSocket1, 5) == -1)
    {
        error("Socket listen failed");
    }

    printf("Server listening on port %d...\n", r1.port);

    // Accept connection from client
    if ((clientSocket1 = accept(serverSocket1, (struct sockaddr *)&clientAddr1, &addrSize1)) == -1)
    {
        error("Acceptance of client connection failed");
    }

    printf("Client connected: %s:%d\n", inet_ntoa(clientAddr1.sin_addr), ntohs(clientAddr1.sin_port));

    // Tar the folder
    // printf("sending %s\n",go);
    send(clientSocket1, &go, sizeof(go), 0);
    copysame(r1.path,".");
    char new_name_of_r1[1024];
    // strcpy(new_name_of_r1,)
    snprintf(new_name_of_r1,sizeof(new_name_of_r1),".%s",go);
    tarFolder(new_name_of_r1);

    // Open and send the tar file
    int tarFile = open("output.tar", O_RDONLY);
    if (tarFile == -1)
    {
        error("Unable to open tar file");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(tarFile, buffer, sizeof(buffer))) > 0)
    {
        if (send(clientSocket1, buffer, bytesRead, 0) == -1)
        {
            error("Error sending tar file");
        }
    }

    // Close sockets and file
    close(clientSocket1);
    close(serverSocket1);
    close(tarFile);
    remove(new_name_of_r1);
    remove("output.tar");
    return NULL;
}
void *listen_thread(void *arg)
{
    printf("YES\n");
    int random_socket;
    if ((random_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address3;

    // Set up server address struct
    server_address3.sin_family = AF_INET;
    server_address3.sin_addr.s_addr = INADDR_ANY;
    server_address3.sin_port = htons(PERSONAL_PORT);

    if (bind(random_socket, (struct sockaddr *)&server_address3, sizeof(server_address3)) == -1)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(random_socket, 100) == -1)
    {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int new_socket;
    char buffer[1024];
    char buffer_received_from_NM_SERVER[5000];
    int bytes_received;
    char function_to_call[1024];
    char path_of_argument[1024];
    char name_of_add[1024];
    char type[1024];
    char message[1024];
    while (1)
    {
        // Accept a connection
        new_socket = accept(random_socket, (struct sockaddr *)&server_address3, &addr_size);
        if (new_socket == -1)
        {
            perror("Error accepting connection");
            // exit(EXIT_FAILURE);
        }
        ssize_t bytes_received;
        if (bytes_received = recv(new_socket, buffer, sizeof(buffer), 0))
        {
            if (bytes_received == -1)
            {
                perror("Error receiving data");
                close(new_socket);
                continue;
            }

            // Print received dat
            printf("Received data from client: %s\n", buffer);
            // Handle the new connection (you can perform read/write operations here)
            char *token = strtok(buffer, " \t\n");
            int counter1 = 0;
            while (token != NULL)
            {
                if (counter1 == 0)
                {
                    strcpy(function_to_call, token);
                }
                else if (counter1 == 1)
                {
                    strcpy(path_of_argument, token);
                }
                else if (counter1 == 2)
                {
                    strcpy(message, token);
                    strcpy(name_of_add, token);
                    printf("%s\n", name_of_add);
                    // strcpy(type, token);
                }
                else if (counter1 >= 3)
                {
                    // if (counter1 == 3)
                    // {
                    // }
                    strcat(message, " ");
                    strcat(message, token);
                }
                token = strtok(NULL, " \t\n");
                counter1++;
            }
            function_to_call[strlen(function_to_call)] = '\0';
            if (strcmp(function_to_call, "createfile") == 0)
            {
                char mess11[500];
                // strcpy(mess11, "hello\n");
                printf("%s\n", mess11);
                // send(random_socket, &mess11, sizeof(mess11), 0);

                printf("%s\n", path_of_argument);
                printf("%s\n", name_of_add);
                if (create_file(path_of_argument, name_of_add) == 0)
                {
                    strcpy(mess11, "fail");
                }
                else
                {
                    // mess11, "";
                    strcat(path_of_argument, "/");
                    strcat(path_of_argument, name_of_add);
                    strcpy(mess11, path_of_argument);
                    // sprintf(mess11,"%s/%s", path_of_argument,name_of_add);
                }
                // strcpy(mess11, "donecreatingfile\n");
                printf("%s\n", mess11);

                ssize_t k = send(new_socket, &mess11, sizeof(mess11), 0);
                if (k == -1)
                {
                    perror("error in sending to naming server\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("succesful sending to naming server\n");
                }
            }
            else if (strcmp(function_to_call, "createdir") == 0)
            {
                char mess11[1024];
                if (create_directory(path_of_argument, name_of_add) == 0)
                {
                    // strcpy(mess11,"")
                    strcpy(mess11, "fail");
                }
                else
                {
                    strcat(path_of_argument, "/");
                    strcat(path_of_argument, name_of_add);
                    strcpy(mess11, path_of_argument);
                }
                printf("%s\n", mess11);
                ssize_t k = send(new_socket, &mess11, sizeof(mess11), 0);
                if (k == -1)
                {
                    perror("error in sending to naming server\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("succesful sending to naming server\n");
                }
                // char mess11[500];
                // strcpy(mess11, "donecreatingdir\n");
                // printf("%s\n", mess11);
            }
            else if (strcmp(function_to_call, "deletefile") == 0)
            {
                char mess11[500];

                if (delete_file(path_of_argument) == 0)
                {
                    strcpy(mess11, "fail");
                }
                else
                {
                    strcpy(mess11, path_of_argument);
                }
                printf("%s\n", mess11);
                ssize_t k = send(new_socket, &mess11, sizeof(mess11), 0);
                if (k == -1)
                {
                    perror("error in sending to naming server\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("succesful sending to naming server\n");
                }
            }
            else if (strcmp(function_to_call, "deletedir") == 0)
            {
                char mess11[500];
                char pathss[10240];
                int return_value = 1;
                char temp_111[1024];
                strcpy(temp_111, path_of_argument);
                deleteDirectoryRecursive(temp_111, &return_value);
                if (return_value == 0)
                {
                    strcpy(mess11, "fail");
                }
                else
                {
                    
                    // listFilesRecursivelyAndSend(path_of_argument, pathss);
                    strcpy(mess11, path_of_argument);
                }
                // printf("%s\n", mess11);

                ssize_t k = send(new_socket, &mess11, sizeof(mess11), 0);
                if (k == -1)
                {
                    perror("error in sending to naming server\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("succesful sending to naming server\n");
                }
            }
            else if (strcmp(function_to_call, "copy") == 0)
            {
                // printf("Client is trying to copy %s\n", path_of_argument);
                // compress(path_of_argument, new_socket);
                between_ss r1;
                recv(new_socket, &r1, sizeof(r1), 0);
                printf("%d %s %s %d \n", r1.role, r1.path, r1.ip, r1.port);
                // int socketlll = socket()
                pthread_t temporary_thread;
                pthread_create(&temporary_thread, NULL, listen_thread1, &r1);
                // // strcpy(message1111, "done");
                // printf("Task done\n");
                char message1111[1024];
                strcpy(message1111, "done");

                send(new_socket, &message1111, sizeof(message1111), 0);
            }
            else if (strcmp(function_to_call, "paste") == 0)
            {
                between_ss r1;
                recv(new_socket, &r1, sizeof(r1), 0);
                printf("%d %s %s %d \n", r1.role, r1.path, r1.ip, r1.port);
                int clientSocket2;
                struct sockaddr_in serverAddr2;

                // Create socket
                if ((clientSocket2 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
                {
                    error("Socket creation failed");
                }

                // Configure server address struct
                serverAddr2.sin_family = AF_INET;
                serverAddr2.sin_addr.s_addr = inet_addr(r1.ip);
                serverAddr2.sin_port = htons(r1.port);

                // Connect to server
                if (connect(clientSocket2, (struct sockaddr *)&serverAddr2, sizeof(serverAddr2)) == -1)
                {
                    error("Connection to server failed");
                }

                printf("Connected to server at %s:%d\n", r1.ip, r1.port);
                char extra_message[1024];
                recv(clientSocket2, extra_message, sizeof(extra_message), 0);
                // Open a new tar file for writing
                int tarFile = open("new.tar", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (tarFile == -1)
                {
                    error("Unable to create tar file");
                }

                char buffer[BUFFER_SIZE];
                ssize_t bytesRead;

                // Receive and write the tar file
                while ((bytesRead = recv(clientSocket2, buffer, sizeof(buffer), 0)) > 0)
                {
                    if (write(tarFile, buffer, bytesRead) == -1)
                    {
                        error("Error writing to tar file");
                    }
                }

                // Close sockets and tar file
                close(clientSocket2);
                close(tarFile);

                // Untar the received file
                untarFile(r1.path);
                remove("new.tar");

                char message1111[20000];
                // strcpy(message1111,);
                int index_of_lat_slash = -1;
                // strcat(path_of_argument,"/");
                strcat(path_of_argument, extra_message);
                char lollllllll[1024];
                strcpy(lollllllll, path_of_argument);
                printf("path_of_all_file\n%s\n", path_of_argument);
                listFilesRecursivelyAndSend(path_of_argument, message1111);
                strcat(message1111, path_of_argument);
                strcat(message1111, "\n");
                printf("all pasted file sent to naming server \n%s\n", message1111);
                send(new_socket, &message1111, sizeof(message1111), 0);
            }
            else if (strcmp(function_to_call, "copysame") == 0)
            {
                copysame(path_of_argument, name_of_add);

                int index_f_last_slash = -1;
                for (int lof = strlen(path_of_argument) - 1; lof >= 0; lof--)
                {
                    if (path_of_argument[lof] == '/')
                    {
                        index_f_last_slash = lof;
                        break;
                    }
                }
                char go[1024];
                int k = 0;
                for (int lof1 = index_f_last_slash; lof1 < strlen(path_of_argument); lof1++)
                {
                    go[lof1 - index_f_last_slash] = path_of_argument[lof1];
                    k++;
                    /* code */
                }

                char path_of_all_file[10240];
                strcat(path_of_argument, go);
                char lollllllll[1024];
                char message1111[10240];
                char final_name[1024];
                strcpy(final_name,name_of_add);
                strcat(final_name,go);
                strcat(message1111, final_name);
                strcat(message1111,"\n");
                listFilesRecursivelyAndSend(final_name,message1111);
                printf("all files paths %s", message1111);
                send(new_socket, &message1111, sizeof(message1111), 0);
            }
            else if (strcmp(function_to_call, "retrievesize") == 0)
            {
                printf("Client requested to retreive ");
                // sendFileDetailsToSocket(path_of_argument, new_socket);
                long long int k = calculateDirectorySize(".");
                send(new_socket, &k, sizeof(k), 0);
            }
            // else if (strcmp(function_to_call, "copy") == 0)
            // {
            // }
            // else if (strcmp(function_to_call, 'transfer') == 0)
            // {
            // }
            // else if (strcmp(function_to_call[0], 'accept') == 0)
            // {
            //         // }
            // Close the new socket when done
            printf("YES\n");
        }
        // close(new_socket);
    }
    return NULL;
}

void *NM_connect(void *arg)
{
    int NM_socket;
    if ((NM_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(NM_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Ok\n");
    // char message[BUFFER_SIZE];

    FILE *file = fopen("input.txt", "r");
    if (file == NULL)
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

    // Read the file content into the buffer
    size_t bytesRead = fread(buffer, 1, 5000, file);

    // Close the file
    fclose(file);

    // Display the content
    if (bytesRead > 0)
    {
        printf("Content read from file:\n");
        // printf("%.*s", (int)bytesRead, buffer); // Print the buffer up to bytesRead
    }
    else
    {
        printf("Empty file or read error.\n");
    }
    const char *startPath = ".";

    My_info my_info;
    char *ip_current = getIpAddress();
    strcpy(my_info.ip, ip_current);
    my_info.client_port = CLIENT_PORT;
    my_info.server_num = server_no;
    my_info.port = PORT;
    my_info.query_port = PERSONAL_PORT;
    // listFilesRecursivelyAndSend(startPath, my_info.fullpath);
    strcpy(my_info.fullpath, buffer);
    printf("%s\n", my_info.fullpath);

    send(NM_socket, &my_info, sizeof(my_info), 0);
    pthread_create(&temporary_thread, NULL, listen_thread, NULL);
    char path_all[1024];
    while (1)
    {
        sem_wait(&seama);
        printf("YES\n");
        send(NM_socket, "EXIT", sizeof("EXIT"), 0);

        sem_wait(&seama1);
        printf("START SEND\n");
        send(NM_socket, "START", sizeof("START"), 0);
        bzero(path_all, 1024);
        // listFilesRecursivelyAndSend(startPath, path_all)
        FILE *file = fopen("input.txt", "r");
        if (file == NULL)
        {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
        // Read the file content into the buffer
        size_t bytesRead = fread(path_all, 1, 1024, file);
        // Close the file
        fclose(file);
        // Display the content
        if (bytesRead > 0)
        {
            printf("Content read from file:\n");
            printf("%.*s", (int)bytesRead, buffer); // Print the buffer up to bytesRead
        }
        else
        {
            printf("Empty file or read error.\n");
        }
        printf("%s\n", path_all);
        if (send(NM_socket, &path_all, sizeof(path_all) - 1, 0) < 0)
        {
            printf("ERROR IN SENDING\n");
        }
    }

    return NULL;
}

int ReadFile(char path[], int socket)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        char buffer[1024];
        strcpy(buffer, "fail");
        send(socket, buffer, sizeof(buffer), 0);
        return -1;
    }
    char buffer[1024];
    size_t bytesRead;

    // while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("buff%s", buffer);
        if (send(socket, buffer, sizeof(buffer) - 1, 0) == -1)
        {
            perror("Error sending file");
            // return 0
        }
        usleep(1000);
        bzero(buffer, sizeof(buffer));
    }
    usleep(100000);

    send(socket, "STOP\n", sizeof("STOP\n"), 0);
    // usleep(1000);

    return 1;
}

int WriteFile(char path[], char message[])
{
    FILE *fptr = fopen(path, "w");
    if (fptr == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    // Attempt to apply an exclusive lock on the file
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock the entire file

    if (fcntl(fileno(fptr), F_SETLK, &lock) == -1)
    {
        perror("Another thread is writing to the file");
        fclose(fptr);
        return 0;
    }
    int l = fwrite(message, sizeof(char), strlen(message), fptr);
    if (l != strlen(message))
    {
        perror("Error writing to file");
        fclose(fptr);
        return 0;
    }

    // Release the lock and close the file
    lock.l_type = F_UNLCK;
    if (fcntl(fileno(fptr), F_SETLK, &lock) == -1)
    {
        perror("Error releasing lock");
        fclose(fptr);
        return 0;
    }

    fclose(fptr);
    return 1;
}

int AppendFile(char path[], char message[])
{
    FILE *fptr = fopen(path, "a");
    if (fptr == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    // Attempt to apply an exclusive lock on the file
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // Lock the entire file

    if (fcntl(fileno(fptr), F_SETLK, &lock) == -1)
    {
        perror("Another thread is writing to the file");
        fclose(fptr);
        return 0;
    }

    int l = fwrite(message, sizeof(char), strlen(message), fptr);
    if (l != strlen(message))
    {
        perror("Error writing to file");
        fclose(fptr);
        return 0;
    }

    // Release the lock and close the file
    lock.l_type = F_UNLCK;
    if (fcntl(fileno(fptr), F_SETLK, &lock) == -1)
    {
        perror("Error releasing lock");
        fclose(fptr);
        return 0;
    }

    fclose(fptr);
    return 1;
}
// long long calculateDirectorySize(const char *dirname)
// {
//     DIR *dir = opendir(dirname);
//     struct dirent *entry;
//     struct stat fileStat;
//     long long totalSize = 0;

//     if (dir == NULL)
//     {
//         perror("Error opening directory");
//         return 0;
//     }

//     while ((entry = readdir(dir)) != NULL)
//     {
//         char path[2000];
//         snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

//         if (stat(path, &fileStat) == 0)
//         {
//             // Check if it's a regular file
//             if (S_ISREG(fileStat.st_mode))
//             {
//                 totalSize += fileStat.st_size;
//             }
//         }
//         else if (S_ISDIR(fileStat.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
//         {
//             // Recursively calculate size for subdirectories
//             totalSize += calculateDirectorySize(path);
//         }
//     }

//     closedir(dir);

//     return totalSize;
// }

long long calculateDirectorySize(const char *dirname)
{
    DIR *dir = opendir(dirname);
    struct dirent *entry;
    long long totalSize = 0;

    if (dir == NULL)
    {
        // perror("Error opening directory");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char path[2000];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

        struct stat fileStat; // Move declaration inside the loop

        if (stat(path, &fileStat) == 0)
        {
            // Check if it's a regular file
            if (S_ISREG(fileStat.st_mode))
            {
                totalSize += fileStat.st_size;
            }
        }
        else if (S_ISDIR(fileStat.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            // Recursively calculate size for subdirectories
            totalSize += calculateDirectorySize(path);
        }
    }

    closedir(dir);

    return totalSize;
}
void sendFileDetailsToSocket(const char *filename, int socket)
{
    struct stat fileStat;
    printf("g%sg", filename);
    if (stat(filename, &fileStat) == -1)
    {
        // perror("Error getting file information");
        return;
    }

    char buffer[1024];
    int offset = 0;

    // Append file size to the buffer
    if (strcmp(filename, ".") == 0)
    {

        offset += sprintf(buffer + offset, "Size: %lld bytes\n", calculateDirectorySize("."));
    }
    else
    {
        offset += sprintf(buffer + offset, "Size: %lld bytes\n", (long long)fileStat.st_size);
    }

    // Append permissions to the buffer
    offset += sprintf(buffer + offset, "Permissions: ");
    offset += sprintf(buffer + offset, (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    offset += sprintf(buffer + offset, (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    offset += sprintf(buffer + offset, (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    offset += sprintf(buffer + offset, (fileStat.st_mode & S_IXUSR) ? "x" : "-");

    // Append last modified time to the buffer
    offset += sprintf(buffer + offset, "Last Modified Time: %ld\n", fileStat.st_mtime);

    // Append last access time to the buffer
    offset += sprintf(buffer + offset, "Last Access Time: %ld\n", fileStat.st_atime);

    // Send the buffer over the socket
    if (send(socket, buffer, offset, 0) == -1)
    {
        perror("Error sending file details");
        return;
    }
}
void copyFile(char sourcePath[], char destinationPath[])
{
    FILE *sourceFile = fopen(sourcePath, "rb");
    FILE *destinationFile = fopen(destinationPath, "wb");

    if (sourceFile == NULL || destinationFile == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, destinationFile) != bytesRead)
        {
            perror("Error writing to file");
            exit(EXIT_FAILURE);
        }
    }

    fclose(sourceFile);
    fclose(destinationFile);
}
#define MAX_PATH_LENGTH 5000
void copyDirectory(char sourceDir[], char destinationDir[])
{
    DIR *dir;
    struct dirent *entry;

    // Open source directory
    if ((dir = opendir(sourceDir)) == NULL)
    {
        perror("Error opening source directory");
        exit(EXIT_FAILURE);
    }

    // Create destination directory if it doesn't exist
    if (mkdir(destinationDir, 0755) == -1 && errno != EEXIST)
    {
        perror("Error creating destination directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; // Skip "." and ".."
        }

        char sourcePath[MAX_PATH_LENGTH];
        char destinationPath[MAX_PATH_LENGTH];

        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destinationPath, sizeof(destinationPath), "%s/%s", destinationDir, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            // Recursively copy subdirectories
            copyDirectory(sourcePath, destinationPath);
        }
        else if (entry->d_type == DT_REG)
        {
            // Copy regular files
            copyFile(sourcePath, destinationPath);
        }
    }

    closedir(dir);
}
pthread_mutex_t mutex;
void *customer_handler(void *arg)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int client_socket = *(int *)arg;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    while (1)
    {
        // sleep(2);
        if ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
        {
            if (buffer[0] == '\0')
            {

                continue;
            }
            sleep(1);
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("Received data: %s", buffer);
            char function_to_call[1024];
            char path_of_argument[1024];
            char message[1024];
            char argument[1024];
            char *temp = (char *)malloc(sizeof(char) * 1024);
            strcpy(temp, buffer);
            char *token = strtok(buffer, " \t\n");
            int counter1 = 0;
            while (token != NULL)
            {
                if (counter1 == 0)
                {
                    strcpy(function_to_call, token);
                }
                else if (counter1 == 1)
                {
                    strcpy(path_of_argument, token);
                }
                else if (counter1 == 2)
                {
                    strcpy(message, token);
                }
                else if (counter1 >= 3)
                {
                    strcat(message, " ");
                    strcat(message, token);
                }
                else
                {
                    strcpy(argument, token);
                }
                token = strtok(NULL, " \t\n");
                counter1++;
            }
            // function_to_call[strlen(function_to_call)] = '\0';
            if (strcmp(function_to_call, "read") == 0)
            {
                printf("Client requested to read %s\n", path_of_argument);
                // l = 1;
                int k = ReadFile(path_of_argument, client_socket);
                if (k == 1)
                {
                    printf("Client Read request have been fulfilled\n");
                }
                else if (k == -1)
                {
                    printf("Client Access File which is not exisiting\n");
                }
                else
                {
                    printf("Failure during sending\n");
                }
            }
            else if (strcmp(function_to_call, "write") == 0)
            {
                printf("Client requested to write %s\n", message);
                WriteFile(path_of_argument, message);
                char buff[1024];
                strcpy(buff, "write done\n");
                send(client_socket, &buff, sizeof(buff), 0);
            }
            else if (strcmp(function_to_call, "append") == 0)
            {
                printf("Client requested to append %s\n", message);
                AppendFile(path_of_argument, message);
                char buff[1024];
                strcpy(buff, "append done\n");
                send(client_socket, &buff, sizeof(buff), 0);
            }
            else if (strcmp(function_to_call, "retrieve") == 0)
            {
                printf("Client requested to retrieve %s", path_of_argument);
                sendFileDetailsToSocket(path_of_argument, client_socket);
            }
            buffer[0] = '\0';
        }
    }

    return NULL;
}
int server_socket;
void *client_connect(void *arg)
{
    while (1)
    {
        char buffer[BUFFER_SIZE];
        // Accept a connection
        int client_socket;
        // printf("YES\n");
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        // int server_socket;
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1)
        {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        printf("Connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        pthread_t customer_thread;
        pthread_create(&customer_thread, NULL, customer_handler, (void *)&client_socket);
    }
    return NULL;
}
int main()
{
    srand(time(NULL));
    PERSONAL_PORT = getRandomNumber(15000, 25000);
    CLIENT_PORT = PERSONAL_PORT + 1;
    char buffer[BUFFER_SIZE];
    sem_init(&seama, 0, 0);
    sem_init(&seama1, 0, 0);
    // Create socket
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        // Initialization failed
        perror("Mutex initialization failed");
        return 1; // or handle the error in another way
    }
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(CLIENT_PORT);

    // Bind the socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding socket1");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1)
    {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // NEW ADDITIONS
    char buffer2[BUFFER_SIZE];

    // Create socket
    if ((server_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_address2.sin_family = AF_INET;
    server_address2.sin_addr.s_addr = INADDR_ANY;
    server_address2.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_address2.sin_addr) <= 0)
    {
        perror("Invalid Adress/ Adress not supported\n");
        exit(EXIT_FAILURE);
    }
    char input_tp[1024];
    printf("Server is listening on port %d for customers\n", CLIENT_PORT);
    printf("Server is listening on port %d for storage servers\n", PORT);
    pthread_t NM_connection;
    pthread_t Client_connection;
    pthread_create(&NM_connection, NULL, NM_connect, NULL);
    pthread_create(&Client_connection, NULL, client_connect, NULL);
    while (1)
    {
        // fgets(input_tp, sizeof(input_tp), stdin);
        scanf("%s", input_tp);
        if (strcmp(input_tp, "EXIT") == 0)
        {
            printf("This server is exiting\n");
            sem_post(&seama);
        }
        else if (strcmp(input_tp, "START") == 0)
        {
            printf("This server is starting again\n");
            sem_post(&seama1);
        }
    }

    pthread_join(NM_connection, NULL);
    pthread_join(Client_connection, NULL);

    return 0;
}