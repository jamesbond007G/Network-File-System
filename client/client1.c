#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "10.42.0.1"
#define SERVER_PORT 1234

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    char message[100];

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

    while (1)
    {

        // Send a message to the server
        printf("Enter message to send: ");
        fgets(message, sizeof(message), stdin);
        send(client_socket, message, strlen(message), 0);

        // Receive a message from the server
        recv(client_socket, message, sizeof(message), 0);
        printf("Server says: %s", message);

        // Close the socket
    }
        close(client_socket);

    return 0;
}
