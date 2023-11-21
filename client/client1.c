#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 5000

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"

typedef struct
{

    char ip[32];
    int port;
    int flag;
    char fullpath[BUFFER_SIZE];
} return_struct;
#define SERVER_IP "10.42.0.56"
#define SERVER_PORT 12346

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    char message[100];
    char write_message[100];
    char returned_message[5000];

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

    printf(ANSI_COLOR_CYAN);
    printf("**********************************************\n");
    printf("*                                            *\n");
    printf("*    Welcome to Our Wonderful NFS System!    *\n");
    printf("*                                            *\n");
    printf("**********************************************\n");
    printf(ANSI_COLOR_RESET);

    while (1)
    {
        char message1[1024];
        int ch = 0;
        printf("Enter your choice: \n");
        // printf("1. Reading, Writing, and Retrieving Information about Files\n2. Creating and Deleting Files and Folders: \n3. Copying Files/Directories Between Storage Servers: \n4. To Exit the server NFS \nEnter message to send: \n");
        printf(ANSI_COLOR_BLUE "1. Reading, Writing, and Retrieving Information about Files\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_GREEN "2. Creating and Deleting Files and Folders:\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_YELLOW "3. Copying Files/Directories Between Storage Servers:\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_MAGENTA "4. To Exit the server NFS\n" ANSI_COLOR_RESET);
        printf(ANSI_COLOR_CYAN "Enter message to send:\n" ANSI_COLOR_RESET);

        scanf("%d\n", &ch);
        while (fgets(message, sizeof(message), stdin) == 0)
        {
        }
        char message_check[1024];
        strcpy(message_check, message);
        int good = 0;
        char *token = strtok(message_check, " ");
        if (strcmp(token, "read") == 0 || strcmp(token, "write") == 0 || strcmp(token, "append") == 0 || strcmp(token, "deletefile") == 0 || strcmp(token, "createdir") == 0 || strcmp(token, "deletedir") == 0 || strcmp(token, "copy") == 0 || strcmp(token, "createfile") == 0 || strcmp(token,"retrieve")==0)
        {
            good = 1;
        }

        if (ch == 1)
        {
            if (good == 0)
            {
                printf("Invalid command\n");
                continue;
            }

            int write_flag = 0, read_flag = 0;
            strcpy(message1, message);
            printf("%s\n", message);
            send(client_socket, message, sizeof(message), 0);
            char *token = strtok(message, " ");
            if (strcmp(token, "write") == 0 || strcmp(token, "append") == 0)
            {
                write_flag = 1;
            }
            if (strcmp(token, "read") == 0)
            {
                read_flag = 1;
            }
            return_struct return_struct1;
            memset(&return_struct1, 0, sizeof(return_struct1));
            recv(client_socket, &return_struct1, sizeof(return_struct1), 0);

            // if (write_flag)
            // {
            //     send(client_socket, token, strlen(token), 0);
            // }
            if (return_struct1.port == -1)
            {
                printf("File not found\n");
                continue;
            }
            if (return_struct1.port == 0)
            {
                send(client_socket, message, sizeof(message), 0);
                memset(&return_struct1, 0, sizeof(return_struct1));
                recv(client_socket, &return_struct1, sizeof(return_struct1), 0);
            }
            printf("Server gave ip: %s and port number : %d\n", return_struct1.ip, return_struct1.port);
            int client_socket1;
            struct sockaddr_in server_addr1;
            client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
            if (client_socket1 == -1)
            {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
            }
            server_addr1.sin_family = AF_INET;
            server_addr1.sin_port = htons(return_struct1.port);
            if (inet_pton(AF_INET, return_struct1.ip, &server_addr1.sin_addr) <= 0)
            {
                perror("Invalid address/ Address not supported");
                exit(EXIT_FAILURE);
            }
            if (connect(client_socket1, (struct sockaddr *)&server_addr1, sizeof(server_addr1)) < 0)
            {
                perror("Connection failed");
                exit(EXIT_FAILURE);
            }
            printf("connnnect\n");
            char new[1024];
            bzero(new, sizeof(new));
            send(client_socket1, message1, strlen(message1), 0);
            if (read_flag)
            {
                while (recv(client_socket1, new, sizeof(new) - 1, 0) > 0)
                {
                    new[strlen(new)] = '\0';
                    printf("%s", new);
                    if (strncmp(new, "STOP", 4) == 0)
                    {
                        break;
                    }
                    char *token = strtok(new, " \n");
                    while (token != NULL)
                    {
                        token = strtok(NULL, " \n");
                    }
                    usleep(100);
                    bzero(new, sizeof(new));
                }
            }
            else
            {
                if (recv(client_socket1, new, sizeof(new) - 1, 0) > 0)
                {
                    printf("%s", new);
                }
                if (write_flag)
                {
                    send(client_socket, message1, strlen(message1), 0);
                }
            }
            close(client_socket1);
        }
        else if (ch == 2)
        {
            if (good == 0)
            {
                printf("Invalid command\n");
                continue;
            }
            strcpy(message1, message);
            send(client_socket, message, strlen(message), 0);
            if (recv(client_socket, returned_message, sizeof(returned_message), 0))
            {
                printf("%s", returned_message);
            }
        }
        else if (ch == 3)
        {
            if (good == 0)
            {
                printf("Invalid command\n");
                continue;
            }
            strcpy(message1, message);
            send(client_socket, message, strlen(message), 0);
            if (recv(client_socket, returned_message, sizeof(returned_message), 0))
            {
                printf("%s", returned_message);
            }
        }
        else if (ch == 4)
        {
            send(client_socket, "EXIT", sizeof("EXIT"), 0);
            close(client_socket);
            break;
        }
        else
        {
            printf("Invalid choice\n");
        }
    }
    return 0;
}
