#include "naming_server.h"
int server_socket, client_socket;
struct sockaddr_in server_address, client_address;
socklen_t client_address_len = sizeof(client_address);
pthread_t customer_intialize_thread;

void *customer_intialize(void *arg)
{
    while (1)
    {
        printf("1");
        char buffer[BUFFER_SIZE];
        // Accept a connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1)
        {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        printf("Connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Handle data from the client
        ssize_t bytes_received;
        while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
        {
            // Process received data (you can replace this with your own logic)
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("Received data: %s", buffer);

            // Send a response back to the client (optional)
            send(client_socket, "Message received successfully", strlen("Message received successfully"), 0);
        }

        // Close the client socket
        close(client_socket);
    }
    return NULL;
}

int main()
{
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error binding socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1)
    {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);
    pthread_create(&customer_intialize_thread, NULL, customer_intialize, NULL);
    pthread_join(customer_intialize_thread, NULL);

    // Close the server socket
    close(server_socket);

    return 0;
}
