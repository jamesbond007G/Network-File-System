void copy_and_paste_server(int indx, int indx2,char argument[], char extra_argument[])
{

    int client_socket1;
    struct sockaddr_in server_addr;
    client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket1 == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(query_port_for_ss[idx]);
    if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket1, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    char message1[1024];

    between_ss foridx1;
    between_ss foridx2;
    strcpy(foridx1.ip, ip_for_ss[idx2]);
    strcpy(foridx1.path, argument);
    foridx1.role = 1;
    foridx1.port = OK_PORT;
    strcpy(message1, "copy");
    printf("SENT TO 1ST SS\n");
    send(client_socket1, &message1, sizeof(message1), 0);
    send(client_socket1, &foridx1, sizeof(foridx1), 0);

    // send(client_socket1, &foridx1, sizeof(foridx1), 0);

    strcpy(foridx2.ip, ip_for_ss[idx]);
    strcpy(foridx2.path, extra_argument);
    foridx2.role = -1;
    foridx2.port = OK_PORT;

    int client_socket2;
    client_socket2 = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket2 == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr2;
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_port = htons(query_port_for_ss[idx2]);
    printf("sending to %d %s\n", query_port_for_ss[idx2], ip_for_ss[idx2]);

    if (inet_pton(AF_INET, ip_for_ss[idx2], &server_addr2.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // Connect to server
    if (connect(client_socket2, (struct sockaddr *)&server_addr2, sizeof(server_addr2)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    char message2[1024];
    strcpy(message2, "paste");
    printf("SENT TO 2nd SS\n");

    send(client_socket2, &message2, sizeof(message2), 0);
    send(client_socket2, &foridx2, sizeof(foridx2), 0);
    char check1[1024];
    char check2[20000];

    recv(client_socket1, &check1, sizeof(check1), 0);
    recv(client_socket2, &check2, sizeof(check2), 0);
    printf("check2 : %s", check2);
    // trie_add(idx2, check2);

    close(client_socket1);
    close(client_socket2);
    // printf("message from SS%d = %s\n message from SS%d = %s\n ", idx, check1, idx2, check2);
}
// void paste_server()