#include "server1.h"

typedef struct
{
    char ip[32];
    int port;
    int client_port;
    int server_num;
    char fullpath[1024];

} My_info;

void listFilesRecursivelyAndSend(const char *basePath, char actualpath[])
{
    DIR *dir = opendir(basePath);
    if (dir == NULL)
    {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        char fullPath[1024];
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
void *
NM_connect(void *arg)
{
    int NM_socket;
    // Create socket
    if ((NM_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    // Set up server address struct
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    // server_address.sin_addr.s_addr = INADDR_ANY;
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
    char message[BUFFER_SIZE];

    // send(NM_socket, "Hello from storage server\n", strlen("Hello from storage server\n"), 0);
    // return NULL;
    // Open a file for appending the output
    FILE *outputFile = fopen("output.txt", "a");
    if (outputFile == NULL)
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    const char *startPath = ".";

    My_info my_info;

    strcpy(my_info.ip, SERVER_IP);
    my_info.client_port = PORT;
    my_info.server_num = server_no;
    // sleep(4);
    listFilesRecursivelyAndSend(startPath, my_info.fullpath);
    printf("%s\n", my_info.fullpath);
    fclose(outputFile);
    send(NM_socket, &my_info, sizeof(my_info), 0);

    // while (1)
    // {

    //     // Send a message to the server
    //     printf("Enter message to send: ");
    //     fgets(message, sizeof(message), stdin);
    //     send(NM_socket, message, strlen(message), 0);

    //     // Receive a message from the server
    //     recv(NM_socket, message, sizeof(message), 0);
    //     printf("Server says: %s", message);

    //     // Close the socket
    // }

    return NULL;
}
void Create_file(char path[], char name[])
{
   FILE *file;
    // const char *directoryPath,"/path/to/your/directory; // Replace this with the actual path

    // Create the full path to the file
    char filePath[100]; // Adjust the size according to your needs
    snprintf(filePath, sizeof(filePath), "%s/%s", path, name);

    // Open or create a file for writing in the specified directory
    file = fopen(filePath, "w");

    if (file == NULL) {
        perror("Error creating file");
        return 1;
    }

    // Write data to the file
    // fprintf(file, "Hello, this is a sample file in a specific directory.");

    // Close the file
    fclose(file);
}

void *client_connect(void *arg)
{

    return NULL;
}

int main()
{
    pthread_t NM_connection;
    pthread_t Client_connection;
    // char buffer[BUFFER_SIZE];

    pthread_create(&NM_connection, NULL, NM_connect, NULL);
    pthread_create(&Client_connection, NULL, client_connect, NULL);
    // Close the server socket

    pthread_join(NM_connection, NULL);
    pthread_join(Client_connection, NULL);

    return 0;
}
