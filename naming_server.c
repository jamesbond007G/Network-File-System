
 #include "naming_server.h"
int server_socket;
struct sockaddr_in server_address;
int server_socket2;
struct sockaddr_in server_address2;
pthread_t customer_intialize_thread;
pthread_t stserver_intialize_thread;
// pthread_t customer_thread[100];
int port_number_for_client[100];

void *customer_handler(void *arg)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int client_socket = *(int *)arg;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    while (1)
    {
        while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
        {
            // Process received data (you can replace this with your own logic)
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("Received data: %s", buffer);

            // Send a response back to the client (optional)
            send(client_socket, "Message received successfully\n", strlen("Message received successfully\n"), 0);
        }
    }
    close(client_socket);

    return NULL;
}
void *customer_intialize(void *arg)
{
    while (1)
    {
        char buffer[BUFFER_SIZE];
        // Accept a connection
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1)
        {
            perror("Error accepting connection");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        printf("Connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        pthread_t customer_thread;
        pthread_create(&customer_thread, NULL, customer_handler, (void *)&client_socket);
        // Handle data from the client

        // Close the client socket
    }
    return NULL;
}
void *stserver_handler(void *arg)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int stserver_socket = *(int *)arg;
    struct sockaddr_in stserver_address;
    socklen_t stserver_address_len = sizeof(stserver_address);
    My_info my_info;
    if ((recv(stserver_socket, &my_info, sizeof(my_info), 0)) > 0)
    {
        
    }
    printf("Received IP : %s port_num : %d port_for_customer : %d server_num : %d\n",my_info.ip,my_info.port, my_info.client_port, my_info.server_num);
    printf("%s\n", my_info.fullpath);

    close(stserver_socket);
    return NULL;
}
void *stserver_intialize(void *arg)
{
    while (1)
    {
        char buffer[BUFFER_SIZE];
        // Accept a connection
        int stserver_socket;
        struct sockaddr_in stserver_address;
        socklen_t stserver_address_len = sizeof(stserver_address);

        if ((stserver_socket = accept(server_socket2, (struct sockaddr *)&stserver_address, &stserver_address_len)) == -1)
        {
            perror("Error accepting connection");
            close(server_socket2);
            exit(EXIT_FAILURE);
        }

        printf("Connection from %s:%d\n", inet_ntoa(stserver_address.sin_addr), ntohs(stserver_address.sin_port));
        pthread_t stserver_thread;
        pthread_create(&stserver_thread, NULL, stserver_handler, (void *)&stserver_socket);
        // Handle data from the client

        // Close the client socket
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
    server_address.sin_port = htons(PORT1);

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
    server_address2.sin_port = htons(PORT2);

    // Bind the socket to the specified address and port
    if (bind(server_socket2, (struct sockaddr *)&server_address2, sizeof(server_address2)) == -1)
    {
        perror("Error binding socket");
        close(server_socket2);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket2, 5) == -1)
    {
        perror("Error listening for connections");
        close(server_socket2);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d for customers\n", PORT1);
    printf("Server is listening on port %d for storage servers\n", PORT2);
    pthread_create(&customer_intialize_thread, NULL, customer_intialize, NULL);
    pthread_create(&stserver_intialize_thread, NULL, stserver_intialize, NULL);
    pthread_join(customer_intialize_thread, NULL);
    pthread_join(stserver_intialize_thread, NULL);

    // Close the server socket
    close(server_socket);

    return 0;
}
