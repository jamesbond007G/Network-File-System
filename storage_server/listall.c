#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

#define SERVER_IP "10.42.0.1"
#define SERVER_PORT 5432


void listFilesRecursivelyAndSend(FILE *outputFile, const char *basePath, int socket)
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
        // Skip hidden files and directories (those starting with a dot)
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        // Construct the full path
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, entry->d_name);

        // Send the absolute path of the current entry to the server
        // fullPath[strlen(fullPath)-1]='\n';
        send(socket, fullPath, strlen(fullPath), 0);
        send(socket, "\n", 1, 0);
        // If it's a directory, recursively list its contents and send to the server
        if (entry->d_type == DT_DIR)
        {
            listFilesRecursivelyAndSend(outputFile, fullPath, socket);
        }
    }

    closedir(dir);
}

void sendFile(const char *filename, int socket)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    send(socket, &file_size, sizeof(file_size), 0);

    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        send(socket, buffer, bytesRead, 0);
    }

    fclose(file);
}

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Open a file for appending the output
    FILE *outputFile = fopen("output.txt", "a");
    if (outputFile == NULL)
    {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Specify the starting directory path
    const char *startPath = "/home/gopal/2ndYearSem1/OSN/final-project-026";

    // List files and directories recursively starting from the specified path and send to the server
    listFilesRecursivelyAndSend(outputFile, startPath, client_socket);
    char message[1024];
    recv(client_socket, message, sizeof(message), 0);
    printf("Server says: %s\n", message);
    // Close the output file
    fclose(outputFile);

    // Close the socket
    close(client_socket);

    return 0;
}