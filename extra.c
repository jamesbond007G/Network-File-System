#include "naming_server.h"
#include "functions.h"
#include <semaphore.h>
#include <stdlib.h>
#define CACHE_SIZE 4
#define TOTAL_SERVER 100
int server_socket;
struct sockaddr_in server_address;
int server_socket2;
struct sockaddr_in server_address2;
int server_socket3;
struct sockaddr_in server_address3;
pthread_t customer_intialize_thread;
pthread_t stserver_intialize_thread;
int port_number_for_client[TOTAL_SERVER];
char *ip_for_ss[TOTAL_SERVER];
int port_number_for_ss[TOTAL_SERVER];
int query_port_for_ss[TOTAL_SERVER];
pthread_t cli_thread;
TrieNode *Trie_for_ss;
TrieNode *Trie_for_ss1[TOTAL_SERVER];
// LRUCache *cache;
int current_cache_size = 0;
int cache_time = 0;
int socket_for_client[TOTAL_SERVER];
int working_server[TOTAL_SERVER];
int currently_working_storage_servers = 0;
int arrived_till_now = 0;
int getRandomNumber(int min, int max)
{
    return min + rand() % (max - min + 1);
}

// cache

int mod = 100007;
int hash[100007];
int cache_size = 0;
int djb2Hash(const char *str)
{
    int hash = 5381;
    int c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
        hash %= mod;
    }
    // printf("%d\n", hash);
    return hash % mod;
}

typedef struct cache
{
    char path[1024];
    int server_num;
    struct cache *next;

} cache;

cache *head = NULL;

// Function to create a new cache node
cache *createCacheNode(const char *path, int server_num)
{
    cache *newNode = (cache *)malloc(sizeof(cache));
    if (newNode != NULL)
    {
        strncpy(newNode->path, path, 1024 - 1);
        newNode->path[1024 - 1] = '\0'; // Ensure null-terminated string
        newNode->server_num = server_num;
        newNode->next = NULL;
    }
    return newNode;
}

// Function to insert a new cache node at the beginning of the linked list
cache *insertCacheNode(const char *path, int server_num)
{
    cache *newNode = createCacheNode(path, server_num);
    if (newNode != NULL)
    {
        newNode->next = head;
        head = newNode;
    }
    cache_size++;
    return head;
}

cache *deleteLastElement()
{
    if (head == NULL)
    {
        // List is empty
        return NULL;
    }

    if (head->next == NULL)
    {
        // Only one element in the list
        free(head);
        return NULL;
    }

    cache *current = head;
    while (current->next->next != NULL)
    {
        current = current->next;
    }
    hash[djb2Hash(current->next->path)] = -1;
    cache_size--;
    free(current->next);
    current->next = NULL;
    return head;
}

cache *insert_cache(const char *path, int server_num)
{
    if (cache_size == CACHE_SIZE)
    {
        head = deleteLastElement(head);
    }
    head = insertCacheNode(path, server_num);
    hash[djb2Hash(path)] = server_num;
    return head;
}

cache *deleteNodeByPath(const char *pathToDelete)
{
    cache *current = head;
    cache *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->path, pathToDelete) == 0)
        {
            // Node with the specified path found

            if (prev == NULL)
            {
                // Node to delete is the head
                head = current->next;
            }
            else
            {
                // Update the previous node's next pointer
                prev->next = current->next;
            }

            // Free the node
            free(current);
            return head;
        }

        // Move to the next node
        prev = current;
        current = current->next;
    }

    // Node with the specified path not found
    return head;
}

void printCacheList()
{
    cache *current = head;
    while (current != NULL)
    {
        printf("Path: %s, Server Number: %d\n", current->path, current->server_num);
        current = current->next;
    }
}

int get_cache(char *path)
{
    if (hash[djb2Hash(path)] != -1)
    {
        printf("Cache hit\n");
        deleteNodeByPath(path);
        head = insertCacheNode(path, hash[djb2Hash(path)]);

        return hash[djb2Hash(path)];
    }
    else
    {
        printf("Cache miss\n");
        return -1;
    }
}

// storing the index of the temp servers corresponding to the server index

int first_temp_storage[TOTAL_SERVER] = {-1};
int second_temp_storage[TOTAL_SERVER] = {-1};
void copy_and_paste_server(int idx, int idx2, char argument[], char extra_argument[])
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
    // srand(time(NULL));
    int random = getRandomNumber(25000, 45000);
    printf("random number %d\n", random);
    char message1[1024];
    between_ss foridx1;
    between_ss foridx2;
    strcpy(foridx1.ip, ip_for_ss[idx2]);
    strcpy(foridx1.path, argument);
    foridx1.role = 1;
    foridx1.port = random;
    strcpy(message1, "copy");
    printf("SENT TO 1ST SS\n");
    send(client_socket1, &message1, sizeof(message1), 0);
    send(client_socket1, &foridx1, sizeof(foridx1), 0);
    char check1[1024];
    printf("check1 : %s", check1);

    // send(client_socket1, &foridx1, sizeof(foridx1), 0);

    strcpy(foridx2.ip, ip_for_ss[idx]);
    strcpy(foridx2.path, extra_argument);
    foridx2.role = -1;
    foridx2.port = random;
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
    char check2[20000];

    recv(client_socket1, &check1, sizeof(check1), 0);
    recv(client_socket2, &check2, sizeof(check2), 0);
    printf("check2 : %s", check2);

    // for (int i = 1; i <= currently_working_storage_servers; i++)
    // {
    //     for (int j = 1; j <= currently_working_storage_servers; j++)
    //     {

    //     }
    // }

    // trie_add(idx2, check2);

    close(client_socket1);
    close(client_socket2);
    // printf("message from SS%d = %s\n message from SS%d = %s\n ", idx, check1, idx2, check2);
}
void start_red_control(int index)
{
    if (first_temp_storage[index] != -1)
    {
        printf("done\n");
        return;
    }
    printf("reached\n");
    int size_of_each_server[TOTAL_SERVER];
    for (int idx = 1; idx <= currently_working_storage_servers; idx++)
    {
        int client_socket1;
        struct sockaddr_in server_addr;
        char message[100];
        // Create socket
        client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket1 == -1)
        {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        // Set up server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(query_port_for_ss[idx]);
        if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr.sin_addr) <= 0)
        {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }
        // Connect to server
        if (connect(client_socket1, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }
        char temp[1024];
        strcpy(temp, "retrievesize .");
        send(client_socket1, temp, strlen(temp), 0);
        long long int total_size = 0;
        if (recv(client_socket1, &total_size, sizeof(total_size), 0) > 0)
        {
        }
        printf("returned size : %lld\n", total_size);
        size_of_each_server[idx] = total_size;
    }
    long long int min1 = 1e15;
    long long int min2 = 1e15;
    int indx1 = -1;
    int indx2 = -1;
    // finding the min 2 indexs in the array size_of_each_server
    for (int i = 1; i <= currently_working_storage_servers; i++)
    {
        if (i == index || working_server[i] == 0)
            continue;
        if (size_of_each_server[i] < min1)
        {
            min2 = min1;
            indx2 = indx1;
            min1 = size_of_each_server[i];
            indx1 = i;
        }
        else if (size_of_each_server[i] < min2)
        {
            min2 = size_of_each_server[i];
            indx2 = i;
        }
    }
    first_temp_storage[index] = indx1;
    second_temp_storage[index] = indx2;
    printf("storage server %d stores in %d %d\n", index, indx1, indx2);
    // send()
    int client_socket1;
    struct sockaddr_in server_addr;
    client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket1 == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(query_port_for_ss[indx1]);
    if (inet_pton(AF_INET, ip_for_ss[indx1], &server_addr.sin_addr) <= 0)
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
    snprintf(message1, sizeof(message1), "createdir . SS%d", index);
    send(client_socket1, &message1, sizeof(message1), 0);
    bzero(message1, sizeof(message1));
    recv(client_socket1, message1, sizeof(message1), 0);

    int client_socket2;
    struct sockaddr_in server_addr2;
    client_socket2 = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket2 == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_port = htons(query_port_for_ss[indx2]);
    if (inet_pton(AF_INET, ip_for_ss[indx2], &server_addr2.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket2, (struct sockaddr *)&server_addr2, sizeof(server_addr2)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char message2[1024];
    snprintf(message2, sizeof(message2), "createdir . SS%d", index);
    send(client_socket2, &message2, sizeof(message2), 0);
    // send()
    bzero(message2, sizeof(message2));
    recv(client_socket2, message2, sizeof(message2), 0);
    printf("message from SS%d = %s\n message from SS%d = %s\n ", indx1, message1, indx2, message2);
    close(client_socket1);
    close(client_socket2);
    // strcpy(message1)
    char g1[1024];
    char g2[1024];
    // strcpy(g1,message1);
    snprintf(g1, sizeof(g1), "./SS%d", index);
    // snprintf(g2, sizeof(g2), "SS%d", indx2);
    // printf("first temp storage %d\n", first_temp_storage[index]);
    copy_and_paste_server(index, indx1, ".", g1);
    sleep(2);
    copy_and_paste_server(index, indx2, ".", g1);
    sleep(2);
    return;
}

int isPrefix(char *a, char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
        {
            return 0; // Not a prefix
        }
        a++;
        b++;
    }
    return (*a == '\0'); // True if string a is fully traversed, i.e., it is a prefix
}

void trie_delete_dir(int index, char *dir2)
{
    TrieNode *temp = Trie_for_ss1[index];
    char dir[1024];
    strcpy(dir, dir2);
    // strcat(dir, "/");
    printf("dir %s\n", dir);
    TrieNode *temp1 = temp;
    for (int i = 0; dir[i] != '\0'; i++)
    {
        int position = (int)dir[i];
        temp1 = temp;
        temp = temp->children[position];
    }
    temp->is_leaf = 1;
    for (int i = 0; i < 256; i++)
    {
        temp->children[i] = NULL;
    }
    return;
}

void trie_add(int index, char *total)
{
    char *token = strtok(total, "\n");
    while (token != NULL)
    {
        insert_trie(Trie_for_ss1[index], token);
        token = strtok(NULL, "\n");
    }
    return;
}

void *customer_handler(void *arg)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    int client_socket = *(int *)arg;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int write_flag = 0;
    int append_flag = 0;
    while (1)
    {
        while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
        {
            // Process received data (you can replace this with your own logic)
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("Received data: %s", buffer);
            char function_to_call[1024];
            char argument[1024];
            char message_store[1024];
            char *temp = (char *)malloc(sizeof(char) * 1024);
            strcpy(temp, buffer);
            strcpy(message_store, buffer);
            char *token = strtok(buffer, " \t\n");
            char *extra_argument = (char *)malloc(sizeof(char) * 1024);
            int counter1 = 0;
            while (token != NULL)
            {
                if (counter1 == 0)
                {
                    strcpy(function_to_call, token);
                    counter1++;
                }
                else if (counter1 == 1)
                {
                    strcpy(argument, token);
                    counter1++;
                }
                else
                {
                    strcpy(extra_argument, token);
                    counter1++;
                }
                token = strtok(NULL, " \t\n");
            }
            // printf("function to call %s\n", function_to_call);
            // printf("equalise %d\n", strcmp(function_to_call, "read"));
            if (counter1 == 2)
                argument[strlen(argument)] = '\0';
            else
                extra_argument[strlen(extra_argument)] = '\0';
            if (strcmp(function_to_call, "read") == 0)
            {
                printf("Client requested to read %s\n", argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                int f = 0;
                int temp1 = idx;
                printf("%d %d %d\n", idx, first_temp_storage[idx], second_temp_storage[idx]);
                printf("%d\n", working_server[idx]);
                if (first_temp_storage[idx] == -1 || second_temp_storage[idx] == -1)
                {
                }
                else
                    printf("%d %d\n", working_server[first_temp_storage[idx]], working_server[second_temp_storage[idx]]);
                if (working_server[idx] == 0)
                {
                    if (working_server[first_temp_storage[idx]] == 1)
                    {
                        f = 1;
                    }
                    else if (working_server[second_temp_storage[idx]] == 1)
                    {
                        f = 2;
                    }
                    else
                    {
                        f = 3;
                    }
                }
                if (f == 1)
                {
                    idx = first_temp_storage[idx];
                }
                else if (f == 2)
                {
                    idx = second_temp_storage[idx];
                }
                else if (f == 3)
                {
                    printf("All the servers related to this are not working\n");
                    // send(client_socket, "failed to read due to closed servers\n", sizeof("failed to read due to closed servers\n"), 0);
                    continue;
                }
                if (f != 0)
                {
                    char argument2[1024];
                    // strcpy(argument2, "./SS");
                    snprintf(argument2, sizeof(argument2), "./SS%d", temp1);
                    for (int i = 1; i < strlen(argument); i++)
                    {
                        argument2[strlen(argument2)] = argument[i];
                    }
                    strcpy(argument, argument2);

                    printf("argument %s\n", argument);
                    return_struct return_struct_to_send;
                    return_struct_to_send.flag = 1;
                    return_struct_to_send.port = port_number_for_client[idx];
                    strcpy(return_struct_to_send.ip, ip_for_ss[idx]);
                    strcpy(return_struct_to_send.fullpath, argument);
                    send(client_socket, &return_struct_to_send, sizeof(return_struct_to_send), 0);
                    continue;
                }
            }
            else if (strcmp(function_to_call, "write") == 0)
            {
                printf("Client requested to write %s\n", argument);
                write_flag = 1;
            }
            else if (strcmp(function_to_call, "retrieve") == 0)
            {
                printf("Client requested to retrieve %s\n", argument);
            }
            else if (strcmp(function_to_call, "append") == 0)
            {
                printf("Client requested to append %s\n", argument);
                append_flag = 1;
            }
            else if (strcmp(function_to_call, "createfile") == 0)
            {
                printf("Client requested to createfile at %s named %s\n", argument, extra_argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                if (idx == -1)
                {
                    printf("the location you are finding does not exist\n");
                    send(client_socket, "failed to create file\n", sizeof("failed to create file\n"), 0);
                    continue;
                }
                int client_socket1;
                struct sockaddr_in server_addr;
                char message[100];
                // Create socket
                client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
                if (client_socket1 == -1)
                {
                    perror("Error creating socket");
                    exit(EXIT_FAILURE);
                }

                // Set up server address
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(query_port_for_ss[idx]);
                if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr.sin_addr) <= 0)
                {
                    perror("Invalid address/ Address not supported");
                    exit(EXIT_FAILURE);
                }
                // Connect to server
                if (connect(client_socket1, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                {
                    perror("Connection failed");
                    exit(EXIT_FAILURE);
                }
                send(client_socket1, temp, strlen(temp), 0);
                char response[1024];
                recv(client_socket1, response, sizeof(response), 0);
                int idff = idx;
                int idff1 = first_temp_storage[idx];
                int idff2 = second_temp_storage[idx];
                /////////

                printf("response %s\n", response);
                if (strcmp(response, "fail") != 0)
                {
                    insert_trie(Trie_for_ss1[idx], response);
                    send(client_socket, "file created\n", sizeof("file created\n"), 0);
                }
                else
                {
                    send(client_socket, "failed to create file\n", sizeof("failed to create file\n"), 0);
                }
                if (idff1 != -1 && working_server[idff1] == 1)
                {

                    // GOPAL
                    int client_socket2_1;
                    struct sockaddr_in server_addr2_1;
                    char message2_1[100];
                    // Create socket
                    client_socket2_1 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_1 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_1.sin_family = AF_INET;
                    server_addr2_1.sin_port = htons(query_port_for_ss[idff1]);
                    if (inet_pton(AF_INET, ip_for_ss[idff1], &server_addr2_1.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_1, (struct sockaddr *)&server_addr2_1, sizeof(server_addr2_1)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_1[1024];
                    strcpy(temp2_1, message_store);
                    char path2_1[1024];
                    char name2_1[1024];
                    char *token2_1 = strtok(temp2_1, " ");
                    for (int i = 0; i < 3 && token2_1 != NULL; ++i)
                    {
                        // Process or print the token
                        printf("Word %d: %s\n", i + 1, token2_1);
                        if (i == 1)
                        {
                            strcpy(path2_1, token2_1);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_1, token2_1);
                        }
                        // Move to the next token
                        token2_1 = strtok(NULL, " ");
                    }
                    // strcat()
                    char path2_1_1[2000];
                    for (int i = 2; i < strlen(path2_1); i++)
                    {
                        path2_1_1[i - 2] = path2_1[i];
                    }
                    path2_1_1[strlen(path2_1) - 2] = '\0';
                    char new_path2_1[4000];
                    snprintf(new_path2_1, sizeof(new_path2_1), "./SS%d/%s", idff, path2_1_1);
                    char new_command2_1[10000];
                    snprintf(new_command2_1, sizeof(new_command2_1), "createfile %s %s", new_path2_1, name2_1);

                    send(client_socket2_1, new_command2_1, strlen(new_command2_1), 0);
                    char response2_1[1024];
                    recv(client_socket2_1, response2_1, sizeof(response2_1), 0);
                    close(client_socket2_1);
                }
                if (idff2 != -1 && working_server[idff2] == 1)
                {
                    int client_socket2_2;
                    struct sockaddr_in server_addr2_2;
                    char message2_2[100];
                    // Create socket
                    client_socket2_2 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_2 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_2.sin_family = AF_INET;
                    server_addr2_2.sin_port = htons(query_port_for_ss[idff2]);
                    if (inet_pton(AF_INET, ip_for_ss[idff2], &server_addr2_2.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_2, (struct sockaddr *)&server_addr2_2, sizeof(server_addr2_2)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_2[1024];
                    strcpy(temp2_2, message_store);
                    char path2_2[1024];
                    char name2_2[1024];
                    char *token2_2 = strtok(temp2_2, " ");

                    for (int i = 0; i < 3 && token2_2 != NULL; ++i)
                    {
                        // Process or print the token
                        printf("Word %d: %s\n", i + 1, token2_2);
                        if (i == 1)
                        {
                            strcpy(path2_2, token2_2);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_2, token2_2);
                        }
                        // Move to the next token
                        token2_2 = strtok(NULL, " ");
                    }
                    // strcat()
                    char path2_2_1[2000];
                    for (int i = 2; i < strlen(path2_2); i++)
                    {
                        path2_2_1[i - 2] = path2_2[i];
                    }
                    path2_2_1[strlen(path2_2) - 2] = '\0';
                    char new_path2_2[4000];
                    snprintf(new_path2_2, sizeof(new_path2_2), "./SS%d/%s", idff, path2_2_1);
                    char new_command2_2[10000];
                    snprintf(new_command2_2, sizeof(new_command2_2), "createfile %s %s", new_path2_2, name2_2);
                    send(client_socket2_2, new_command2_2, strlen(new_command2_2), 0);
                    char response2_2[1024];
                    recv(client_socket2_2, response2_2, sizeof(response2_2), 0);
                    close(client_socket2_2);
                }
                continue;
            }
            else if (strcmp(function_to_call, "createdir") == 0)
            {
                printf("Client requested to createdir at %s named %s\n", argument, extra_argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                if (idx == -1)
                {
                    printf("the location you are finding does not exist\n");
                    send(client_socket, "failed to create dir\n", sizeof("failed to create dir\n"), 0);
                    continue;
                }
                int f = 0;

                int client_socket1;
                struct sockaddr_in server_addr;
                char message[100];
                // Create socket
                client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
                if (client_socket1 == -1)
                {
                    perror("Error creating socket");
                    exit(EXIT_FAILURE);
                }

                // Set up server address
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(query_port_for_ss[idx]);
                if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr.sin_addr) <= 0)
                {
                    perror("Invalid address/ Address not supported");
                    exit(EXIT_FAILURE);
                }
                // Connect to server
                if (connect(client_socket1, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                {
                    perror("Connection failed");
                    exit(EXIT_FAILURE);
                }
                send(client_socket1, temp, strlen(temp), 0);
                char response[1024];
                recv(client_socket1, response, sizeof(response), 0);
                printf("response %s\n", response);
                if (strcmp(response, "fail") != 0)
                {
                    insert_trie(Trie_for_ss1[idx], response);
                    send(client_socket, "dir created\n", sizeof("dir created\n"), 0);
                }
                else
                {
                    send(client_socket, "failed to create dir\n", sizeof("failed to create dir\n"), 0);
                }
                int idff = idx;
                int idff1 = first_temp_storage[idx];
                int idff2 = second_temp_storage[idx];
                if (idff1 != -1 && working_server[idff1] == 1)
                {

                    // GOPAL
                    int client_socket2_1;
                    struct sockaddr_in server_addr2_1;
                    char message2_1[100];
                    // Create socket
                    client_socket2_1 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_1 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_1.sin_family = AF_INET;
                    server_addr2_1.sin_port = htons(query_port_for_ss[idff1]);
                    if (inet_pton(AF_INET, ip_for_ss[idff1], &server_addr2_1.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_1, (struct sockaddr *)&server_addr2_1, sizeof(server_addr2_1)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_1[1024];
                    strcpy(temp2_1, message_store);
                    char path2_1[1024];
                    char name2_1[1024];
                    char *token2_1 = strtok(temp2_1, " ");
                    for (int i = 0; i < 3 && token2_1 != NULL; ++i)
                    {
                        // Process or print the token
                        // printf("Word %d: %s\n", i + 1, token);
                        if (i == 1)
                        {
                            strcpy(path2_1, token2_1);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_1, token2_1);
                        }
                        // Move to the next token
                        token2_1 = strtok(NULL, " ");
                    }
                    // strcat()
                    char path2_1_1[1024];
                    for (int i = 2; i < strlen(path2_1); i++)
                    {
                        path2_1_1[i - 2] = path2_1[i];
                    }
                    path2_1_1[strlen(path2_1) - 2] = '\0';
                    printf("path2_1 %s\n", path2_1_1);

                    char new_path2_1[2000];
                    snprintf(new_path2_1, sizeof(new_path2_1), "./SS%d/%s", idff, path2_1_1);
                    char new_command2_1[10000];
                    snprintf(new_command2_1, sizeof(new_command2_1), "createdir %s %s", new_path2_1, name2_1);
                    printf("new_command2_1 %s\n", new_command2_1);
                    send(client_socket2_1, new_command2_1, strlen(new_command2_1), 0);
                    char response2_1[1024];
                    recv(client_socket2_1, response2_1, sizeof(response2_1), 0);
                    printf("Response from temporary 1st storage %s\n", response2_1);

                    close(client_socket2_1);

                    // Create socket}
                }
                if (idff2 != -1 && working_server[idff2] == 1)
                {
                    int client_socket2_2;
                    struct sockaddr_in server_addr2_2;
                    char message2_2[100];
                    client_socket2_2 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_2 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_2.sin_family = AF_INET;
                    server_addr2_2.sin_port = htons(query_port_for_ss[idff2]);
                    if (inet_pton(AF_INET, ip_for_ss[idff2], &server_addr2_2.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_2, (struct sockaddr *)&server_addr2_2, sizeof(server_addr2_2)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_2[1024];
                    strcpy(temp2_2, message_store);
                    char path2_2[1024];
                    char name2_2[1024];
                    char *token2_2 = strtok(temp2_2, " ");
                    for (int i = 0; i < 3 && token2_2 != NULL; ++i)
                    {
                        // Process or print the token
                        // printf("Word %d: %s\n", i + 1, token);
                        if (i == 1)
                        {
                            strcpy(path2_2, token2_2);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_2, token2_2);
                        }
                        // Move to the next token
                        token2_2 = strtok(NULL, " ");
                    }
                    char path2_2_1[1024];
                    for (int i = 2; i < strlen(path2_2); i++)
                    {
                        path2_2_1[i - 2] = path2_2[i];
                    }
                    path2_2_1[strlen(path2_2) - 2] = '\0';
                    printf("path2_1 %s\n", path2_2_1);

                    // strcat()
                    char new_path2_2[2000];
                    snprintf(new_path2_2, sizeof(new_path2_2), "./SS%d/%s", idff, path2_2_1);
                    printf("new_path2_2 %s\n", new_path2_2);
                    char new_command2_2[10000];

                    snprintf(new_command2_2, sizeof(new_command2_2), "createdir %s %s", new_path2_2, name2_2);
                    printf("new_command2_2 %s\n", new_command2_2);
                    send(client_socket2_2, new_command2_2, strlen(new_command2_2), 0);
                    char response2_2[1024];
                    recv(client_socket2_2, response2_2, sizeof(response2_2), 0);
                    printf("Response from temporary 2nd storage %s\n", response2_2);
                    close(client_socket2_2);
                }
                continue;
            }
            else if (strcmp(function_to_call, "deletefile") == 0)
            {
                printf("Client requested to deletefile at %s named %s\n", argument, extra_argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                if (idx == -1)
                {
                    printf("the location you are finding does not exist\n");
                    send(client_socket, "failed to delete file\n", sizeof("failed to delete file\n"), 0);
                    continue;
                }
                int client_socket1;
                struct sockaddr_in server_addr;
                char message[100];
                // Create socket
                client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
                if (client_socket1 == -1)
                {
                    perror("Error creating socket");
                    exit(EXIT_FAILURE);
                }

                // Set up server address
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(query_port_for_ss[idx]);
                if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr.sin_addr) <= 0)
                {
                    perror("Invalid address/ Address not supported");
                    exit(EXIT_FAILURE);
                }
                // Connect to server
                if (connect(client_socket1, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                {
                    perror("Connection failed");
                    exit(EXIT_FAILURE);
                }
                send(client_socket1, temp, strlen(temp), 0);

                char response[1024];
                recv(client_socket1, response, sizeof(response), 0);
                printf("response %s\n", response);
                if (strcmp(response, "fail") != 0)
                {
                    delete_trie(Trie_for_ss1[idx], response);
                    send(client_socket, "file deleted\n", sizeof("file deleted\n"), 0);
                }
                else
                {
                    send(client_socket, "failed to delete file\n", sizeof("failed to delete file\n"), 0);
                }
                // gopal start
                int idff = idx;
                int idff1 = first_temp_storage[idx];
                int idff2 = second_temp_storage[idx];
                // if (idff1 != -1)
                // {

                // GOPAL
                if (idff1 != -1 && working_server[idff1] == 1)
                {
                    int client_socket2_1;
                    struct sockaddr_in server_addr2_1;
                    char message2_1[100];
                    // Create socket
                    client_socket2_1 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_1 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_1.sin_family = AF_INET;
                    server_addr2_1.sin_port = htons(query_port_for_ss[idff1]);
                    if (inet_pton(AF_INET, ip_for_ss[idff1], &server_addr2_1.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_1, (struct sockaddr *)&server_addr2_1, sizeof(server_addr2_1)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_1[1024];
                    strcpy(temp2_1, message_store);
                    char path2_1[1024];
                    char name2_1[1024];
                    char *token2_1 = strtok(temp2_1, " ");
                    for (int i = 0; i < 3 && token2_1 != NULL; ++i)
                    {
                        // Process or print the token
                        // printf("Word %d: %s\n", i + 1, token);
                        if (i == 1)
                        {
                            strcpy(path2_1, token2_1);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_1, token2_1);
                        }
                        // Move to the next token
                        token2_1 = strtok(NULL, " ");
                    }
                    // strcat()
                    char path2_1_1[2000];
                    for (int i = 2; i < strlen(path2_1); i++)
                    {
                        path2_1_1[i - 2] = path2_1[i];
                    }
                    path2_1_1[strlen(path2_1) - 2] = '\0';

                    char new_path2_1[4000];
                    snprintf(new_path2_1, sizeof(new_path2_1), "./SS%d/%s", idff, path2_1_1);
                    char new_command2_1[10000];
                    snprintf(new_command2_1, sizeof(new_command2_1), "deletefile %s", new_path2_1);

                    send(client_socket2_1, new_command2_1, strlen(new_command2_1), 0);
                    char response2_1[1024];
                    recv(client_socket2_1, response2_1, sizeof(response2_1), 0);
                    close(client_socket2_1);
                }
                if (idff2 != -1 && working_server[idff2] == 1)
                {
                    int client_socket2_2;
                    struct sockaddr_in server_addr2_2;
                    char message2_2[100];
                    // Create socket
                    client_socket2_2 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_2 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_2.sin_family = AF_INET;
                    server_addr2_2.sin_port = htons(query_port_for_ss[idff2]);
                    if (inet_pton(AF_INET, ip_for_ss[idff2], &server_addr2_2.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_2, (struct sockaddr *)&server_addr2_2, sizeof(server_addr2_2)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_2[1024];
                    strcpy(temp2_2, message_store);
                    char path2_2[1024];
                    char name2_2[1024];
                    char *token2_2 = strtok(temp2_2, " ");
                    for (int i = 0; i < 3 && token2_2 != NULL; ++i)
                    {
                        // Process or print the token
                        // printf("Word %d: %s\n", i + 1, token);
                        if (i == 1)
                        {
                            strcpy(path2_2, token2_2);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_2, token2_2);
                        }
                        // Move to the next token
                        token2_2 = strtok(NULL, " ");
                    }
                    // strcat()
                    char path2_2_1[2000];
                    for (int i = 2; i < strlen(path2_2); i++)
                    {
                        path2_2_1[i - 2] = path2_2[i];
                    }
                    path2_2_1[strlen(path2_2) - 2] = '\0';

                    char new_path2_2[4000];
                    snprintf(new_path2_2, sizeof(new_path2_2), "./SS%d/%s", idff, path2_2_1);
                    char new_command2_2[10000];
                    snprintf(new_command2_2, sizeof(new_command2_2), "deletefile %s ", new_path2_2);
                    send(client_socket2_2, new_command2_2, strlen(new_command2_2), 0);
                    char response2_2[1024];
                    recv(client_socket2_2, response2_2, sizeof(response2_2), 0);
                    close(client_socket2_2);
                }
                // gopal end

                continue;
            }
            else if (strcmp(function_to_call, "deletedir") == 0)
            {
                printf("Client requested to deletedir at %s named %s\n", argument, extra_argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                if (idx == -1)
                {
                    printf("the location you are finding does not exist\n");
                    send(client_socket, "failed to delete dir\n", sizeof("failed to delete dir\n"), 0);
                    continue;
                }
                int client_socket1;
                struct sockaddr_in server_addr1234;
                char message[100];
                // Create socket
                client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
                if (client_socket1 == -1)
                {
                    perror("Error creating socket");
                    exit(EXIT_FAILURE);
                }

                // Set up server address
                server_addr1234.sin_family = AF_INET;
                server_addr1234.sin_port = htons(query_port_for_ss[idx]);
                if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr1234.sin_addr) <= 0)
                {
                    perror("Invalid address/ Address not supported");
                    exit(EXIT_FAILURE);
                }
                // Connect to server
                if (connect(client_socket1, (struct sockaddr *)&server_addr1234, sizeof(server_addr1234)) < 0)
                {
                    perror("Connection failed");
                    exit(EXIT_FAILURE);
                }
                // printf("YES %s\n", temp);
                send(client_socket1, temp, strlen(temp), 0);
                char response[1024];
                recv(client_socket1, response, sizeof(response), 0);
                printf("response %s\n", response);
                if (strcmp(response, "fail") != 0)
                {
                    // delete_trie(Trie_for_ss1[idx], response);
                    trie_delete_dir(idx, argument);
                    send(client_socket, "dir deleted\n", sizeof("dir deleted\n"), 0);
                }
                else
                {
                    send(client_socket, "failed to delete dir\n", sizeof("failed to delete dir\n"), 0);
                }
                int idff = idx;
                int idff1 = first_temp_storage[idx];
                int idff2 = second_temp_storage[idx];
                // if (idff1 != -1)
                // {

                // GOPAL
                //    if(idff1!=-1 && working_server[idff1]==1){
                if (idff1 != -1 && working_server[idff1] == 1)
                {
                    int client_socket2_1;
                    struct sockaddr_in server_addr2_1;
                    char message2_1[100];
                    // Create socket
                    client_socket2_1 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_1 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_1.sin_family = AF_INET;
                    server_addr2_1.sin_port = htons(query_port_for_ss[idff1]);
                    if (inet_pton(AF_INET, ip_for_ss[idff1], &server_addr2_1.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_1, (struct sockaddr *)&server_addr2_1, sizeof(server_addr2_1)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_1[1024];
                    strcpy(temp2_1, message_store);
                    char path2_1[1024];
                    memset(path2_1, '\0', sizeof(path2_1));
                    char name2_1[1024];
                    memset(name2_1, '\0', sizeof(name2_1));
                    printf("temp2_1 before tokenize %s\n", temp2_1);
                    char *token2_1 = strtok(temp2_1, " ");
                    for (int i = 0; i < 3 && token2_1 != NULL; ++i)
                    {
                        // Process or print the token
                        printf("Word from tokenize%d: %s\n", i, token2_1);
                        if (i == 1)
                        {
                            strcpy(path2_1, token2_1);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_1, token2_1);
                        }
                        // Move to the next token
                        token2_1 = strtok(NULL, " ");
                    }
                    printf("path2_1 %s\n", path2_1);
                    char path2_1_1[2000];
                    for (int i = 2; i < strlen(path2_1); i++)
                    {
                        path2_1_1[i - 2] = path2_1[i];
                    }
                    path2_1_1[strlen(path2_1) - 2] = '\0';
                    printf("%s\n", path2_1_1);
                    // strcat()
                    char new_path2_1[4000];
                    snprintf(new_path2_1, sizeof(new_path2_1), "./SS%d/%s", idff, path2_1_1);
                    char new_command2_1[10000];
                    snprintf(new_command2_1, sizeof(new_command2_1), "deletedir %s", new_path2_1);

                    send(client_socket2_1, new_command2_1, strlen(new_command2_1), 0);
                    char response2_1[1024];
                    recv(client_socket2_1, response2_1, sizeof(response2_1), 0);
                    // printf("response from 1st temp ss %s\n", response2_1);
                    printf("updated to 1st temp SS\n ");

                    close(client_socket2_1);
                }
                usleep(1000);
                if (idff2 != -1 && working_server[idff2] == 1)
                {
                    int client_socket2_2;
                    struct sockaddr_in server_addr2_2;
                    char message2_2[100];
                    // Create socket
                    client_socket2_2 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket2_2 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }

                    // Set up server address
                    server_addr2_2.sin_family = AF_INET;
                    server_addr2_2.sin_port = htons(query_port_for_ss[idff2]);
                    if (inet_pton(AF_INET, ip_for_ss[idff2], &server_addr2_2.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    // Connect to server
                    if (connect(client_socket2_2, (struct sockaddr *)&server_addr2_2, sizeof(server_addr2_2)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    char temp2_2[1024];
                    strcpy(temp2_2, message_store);
                    char path2_2[1024];
                    char name2_2[1024];
                    char *token2_2 = strtok(temp2_2, " ");
                    for (int i = 0; i < 3 && token2_2 != NULL; ++i)
                    {
                        // Process or print the token
                        printf("Word %d: %s\n", i + 1, token2_2);
                        if (i == 1)
                        {
                            strcpy(path2_2, token2_2);
                        }
                        else if (i == 2)
                        {
                            strcpy(name2_2, token2_2);
                        }
                        // Move to the next token
                        token2_2 = strtok(NULL, " ");
                    }
                    printf("path2_2 %s\n", path2_2);
                    char path2_2_1[2000];
                    for (int i = 2; i < strlen(path2_2); i++)
                    {
                        path2_2_1[i - 2] = path2_2[i];
                    }
                    path2_2_1[strlen(path2_2) - 2] = '\0';
                    // strcat()
                    printf("%s\n", path2_2_1);
                    char new_path2_2[4000];
                    snprintf(new_path2_2, sizeof(new_path2_2), "./SS%d/%s", idff, path2_2_1);
                    char new_command2_2[10000];
                    snprintf(new_command2_2, sizeof(new_command2_2), "deletedir %s", new_path2_2);
                    send(client_socket2_2, new_command2_2, strlen(new_command2_2), 0);
                    char response2_2[1024];
                    recv(client_socket2_2, response2_2, sizeof(response2_2), 0);
                    // printf("response from 2nd temp ss %s\n", response2_2);
                    printf("updated to 2nd temp SS\n ");
                    close(client_socket2_2);
                }
                usleep(1000);
                continue;
            }
            else if (strcmp(function_to_call, "copy") == 0)
            {
                printf("Client requested to copyfile at %s named %s\n", argument, extra_argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                int idx2 = -1;
                idx2 = get_cache(extra_argument);
                if (idx2 == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], extra_argument))
                        {
                            idx2 = i;
                            insert_cache(extra_argument, idx2);
                            break;
                        }
                    }
                }

                if (idx == -1)
                {
                    printf("the location you are finding does not exist(to copy)\n");
                    send(client_socket, "failed to copy\n", sizeof("failed to copy\n"), 0);
                    continue;
                }
                if (idx2 == -1)
                {
                    printf("the location you are finding does not exist(to paste)\n");
                    send(client_socket, "failed to copy\n", sizeof("failed to copy\n"), 0);
                    continue;
                }
                char buffer1[5000];
                strcpy(buffer1, argument);
                char *token = strtok(buffer1, "/");
                char newaddition[5000];
                memset(newaddition, 0, sizeof(newaddition));
                while (token != NULL)
                {
                    strcpy(newaddition, token);
                    token = strtok(NULL, "/");
                }
                char extra_arg1[5000];
                memset(extra_arg1, 0, sizeof(extra_arg1));
                strcpy(extra_arg1, extra_argument);
                strcat(extra_arg1, "/");
                strcat(extra_arg1, newaddition);

                if (idx != idx2)
                {
                    int random = getRandomNumber(25000, 45000);
                    int random_dup1 = getRandomNumber(25000, 45000);
                    int random_dup2 = getRandomNumber(25000, 45000);

                    between_ss foridx1;
                    between_ss foridx2;
                    printf("idx 1 = %d idex2 = %d\n", idx, idx2);

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
                    char message1[1024]; // srand(time(NULL));
                    strcpy(foridx1.ip, ip_for_ss[idx2]);
                    strcpy(foridx1.path, argument);

                    foridx1.role = 1;
                    foridx1.port = random;
                    strcpy(message1, "copy");
                    printf("SENT TO 1ST SS\n");
                    send(client_socket1, &message1, sizeof(message1), 0);
                    send(client_socket1, &foridx1, sizeof(foridx1), 0);
                    strcpy(foridx2.ip, ip_for_ss[idx]);
                    strcpy(foridx2.path, extra_argument);

                    foridx2.role = -1;
                    foridx2.port = random;

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
                    trie_add(idx2, check2);

                    if (strncmp(check1, "fail", 4) == 0 || strncmp(check2, "fail", 4) == 0)
                    {
                        printf("failed to copy\n");
                        send(client_socket, "failed to copy\n", sizeof("failed to copy\n"), 0);
                        continue;
                    }
                    else
                    {
                        // printf("YES\n");
                        // argument is the the dir to be send and extra argument
                        send(client_socket, "copy successful\n", sizeof("copy successful\n"), 0);
                        // continue;
                    }

                    close(client_socket2);
                    close(client_socket1);

                    char temp11[5000];
                    char new[2024];
                    for (int i = 2; i < strlen(extra_argument); i++)
                    {
                        new[i - 2] = extra_argument[i];
                    }
                    new[strlen(extra_argument) - 2] = '\0';
                    snprintf(temp11, sizeof(temp11), "./SS%d/%s", idx2, new);
                    sleep(5);

                    if (first_temp_storage[idx2] != -1 && working_server[first_temp_storage[idx2]] == 1)
                    {
                        int socket3 = socket(AF_INET, SOCK_STREAM, 0);
                        if (socket3 == -1)
                        {
                            perror("Error creating socket");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in server_addr3;
                        server_addr3.sin_family = AF_INET;
                        server_addr3.sin_port = htons(query_port_for_ss[idx2]);
                        if (inet_pton(AF_INET, ip_for_ss[idx2], &server_addr3.sin_addr) <= 0)
                        {
                            perror("Invalid address/ Address not supported");
                            exit(EXIT_FAILURE);
                        }
                        // Connect to server
                        if (connect(socket3, (struct sockaddr *)&server_addr3, sizeof(server_addr3)) < 0)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        char message3[1024];
                        strcpy(message3, "copy");
                        // printf("SENT TO 2nd SS\n");
                        between_ss foridx3;
                        strcpy(foridx3.ip, ip_for_ss[first_temp_storage[idx2]]);
                        strcpy(foridx3.path, extra_arg1);
                        foridx3.role = -1;
                        foridx3.port = random_dup1;
                        strcpy(message3, "copy");
                        send(socket3, &message3, sizeof(message3), 0);
                        send(socket3, &foridx3, sizeof(foridx3), 0);
                        char check3[1024];
                        // close(socket3);

                        int socket4 = socket(AF_INET, SOCK_STREAM, 0);
                        if (socket4 == -1)
                        {
                            perror("Error creating socket");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in server_addr4;
                        server_addr4.sin_family = AF_INET;
                        server_addr4.sin_port = htons(query_port_for_ss[first_temp_storage[idx2]]);
                        printf("sending to %d %s\n", query_port_for_ss[first_temp_storage[idx2]], ip_for_ss[idx2]);
                        if (inet_pton(AF_INET, ip_for_ss[first_temp_storage[idx2]], &server_addr4.sin_addr) <= 0)
                        {
                            perror("Invalid address/ Address not supported");
                            exit(EXIT_FAILURE);
                        }
                        if (connect(socket4, (struct sockaddr *)&server_addr4, sizeof(server_addr4)) < 0)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        char message4[1024];
                        strcpy(message4, "paste");
                        printf("SENT TO 2nd SS\n");
                        between_ss foridx4;
                        foridx4.role = -1;
                        foridx4.port = random_dup1;
                        strcpy(foridx4.ip, ip_for_ss[idx2]);
                        strcpy(foridx4.path, temp11);
                        send(socket4, &message4, sizeof(message4), 0);
                        send(socket4, &foridx4, sizeof(foridx4), 0);
                        char check4[1024];
                        recv(socket3, &check3, sizeof(check3), 0);
                        recv(socket4, &check4, sizeof(check4), 0);
                        printf("check3 : %s", check3);
                        printf("check4 : %s", check4);

                        close(socket3);
                        close(socket4);
                    }
                    sleep(5);
                    if (second_temp_storage[idx2] != -1 && working_server[second_temp_storage[idx2]] == 1)
                    {
                        int socket3 = socket(AF_INET, SOCK_STREAM, 0);
                        if (socket3 == -1)
                        {
                            perror("Error creating socket");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in server_addr3;
                        server_addr3.sin_family = AF_INET;
                        server_addr3.sin_port = htons(query_port_for_ss[idx2]);
                        if (inet_pton(AF_INET, ip_for_ss[idx2], &server_addr3.sin_addr) <= 0)
                        {
                            perror("Invalid address/ Address not supported");
                            exit(EXIT_FAILURE);
                        }
                        // Connect to server
                        if (connect(socket3, (struct sockaddr *)&server_addr3, sizeof(server_addr3)) < 0)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        char message3[1024];
                        strcpy(message3, "copy");
                        // printf("SENT TO 2nd SS\n");
                        between_ss foridx3;
                        strcpy(foridx3.ip, ip_for_ss[second_temp_storage[idx2]]);
                        strcpy(foridx3.path, extra_arg1);
                        foridx3.role = -1;
                        foridx3.port = random_dup2;
                        strcpy(message3, "copy");
                        send(socket3, &message3, sizeof(message3), 0);
                        send(socket3, &foridx3, sizeof(foridx3), 0);
                        char check3[1024];
                        // close(socket3);

                        int socket4 = socket(AF_INET, SOCK_STREAM, 0);
                        if (socket4 == -1)
                        {
                            perror("Error creating socket");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in server_addr4;
                        server_addr4.sin_family = AF_INET;
                        server_addr4.sin_port = htons(query_port_for_ss[second_temp_storage[idx2]]);
                        printf("sending to %d %s\n", query_port_for_ss[second_temp_storage[idx2]], ip_for_ss[idx2]);
                        if (inet_pton(AF_INET, ip_for_ss[second_temp_storage[idx2]], &server_addr4.sin_addr) <= 0)
                        {
                            perror("Invalid address/ Address not supported");
                            exit(EXIT_FAILURE);
                        }
                        if (connect(socket4, (struct sockaddr *)&server_addr4, sizeof(server_addr4)) < 0)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        char message4[1024];
                        strcpy(message4, "paste");
                        printf("SENT TO 2nd SS\n");
                        between_ss foridx4;
                        foridx4.role = -1;
                        foridx4.port = random_dup2;
                        strcpy(foridx4.ip, ip_for_ss[idx2]);
                        strcpy(foridx4.path, temp11);
                        send(socket4, &message4, sizeof(message4), 0);
                        send(socket4, &foridx4, sizeof(foridx4), 0);
                        char check4[1024];
                        recv(socket3, &check3, sizeof(check3), 0);
                        recv(socket4, &check4, sizeof(check4), 0);
                        printf("check3 : %s", check3);
                        printf("check4 : %s", check4);
                        close(socket3);
                        close(socket4);
                    }
                    // recv(socket3, &check3, sizeof(check3), 0);
                }
                // close(client_socket_dup1);
                // close(client_socket2);
                // printf("message from SS%d = %s\n message from SS%d = %s\n ", idx, check1, idx2, check2);

                else
                {
                    char messag_coe[1024];
                    strcpy(messag_coe, "copysame");
                    strcat(messag_coe, " ");
                    strcat(messag_coe, argument);
                    strcat(messag_coe, " ");
                    strcat(messag_coe, extra_argument);
                    printf("Files are in same Storage server\n");
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
                    send(client_socket1, &messag_coe, sizeof(messag_coe), 0);
                    char m1[20000];
                    recv(client_socket1, &m1, sizeof(m1), 0);
                    trie_add(idx, m1);
                    printf("received from SS %s\n", m1);
                    send(client_socket, "done", sizeof("done"), 0);
                    // close(client_socket1);
                    // int idx1 =
                    int indx1 = first_temp_storage[idx];
                    int indx2 = second_temp_storage[idx];
                    sleep(2);

                    // updating 1st server

                    strcpy(messag_coe, "copysame");
                    strcat(messag_coe, " ");
                    char new[1024];
                    strcat(messag_coe, "./SS");
                    char buffer[20];
                    snprintf(buffer, sizeof(buffer), "%d", idx);
                    strcat(messag_coe, buffer);
                    strcat(messag_coe, "/");
                    char new_argument[1024];
                    for (int i = 2; i < strlen(argument); i++)
                    {
                        new_argument[i - 2] = argument[i];
                    }
                    new_argument[strlen(argument) - 2] = '\0';

                    // strcat(new,argument)
                    strcat(messag_coe, new_argument);
                    strcat(messag_coe, " ");

                    strcat(messag_coe, "./SS");
                    char buffe[20];
                    snprintf(buffe, sizeof(buffe), "%d", idx);
                    strcat(messag_coe, buffe);
                    strcat(messag_coe, "/");
                    char extra_argument1[1024];
                    for (int i = 2; i < strlen(extra_argument); i++)
                    {
                        extra_argument1[i - 2] = extra_argument[i];
                    }
                    extra_argument1[strlen(extra_argument) - 2] = '\0';
                    strcat(messag_coe, extra_argument1);
                    printf("Sending Finally%s\n", messag_coe);
                    // printf("Files are in same Storage server\n");
                    int client_socket3_1;
                    struct sockaddr_in server_addr3_1;
                    client_socket3_1 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket3_1 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }
                    server_addr3_1.sin_family = AF_INET;
                    server_addr3_1.sin_port = htons(query_port_for_ss[indx1]);
                    printf("sending to %d %s\n", query_port_for_ss[indx1], ip_for_ss[indx1]);
                    if (inet_pton(AF_INET, ip_for_ss[indx1], &server_addr3_1.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    if (connect(client_socket3_1, (struct sockaddr *)&server_addr3_1, sizeof(server_addr3_1)) < 0)
                    {
                        perror("Connection failed here");
                        exit(EXIT_FAILURE);
                    }
                    send(client_socket3_1, &messag_coe, sizeof(messag_coe), 0);
                    // char m1[20000];
                    bzero(m1, sizeof(m1));
                    recv(client_socket3_1, &m1, sizeof(m1), 0);
                    printf("updated to SS %s\n", m1);
                    close(client_socket3_1);

                    sleep(1);
                    // updating 2nd server
                    strcpy(messag_coe, "copysame");
                    strcat(messag_coe, " ");
                    char new1[1024];
                    strcat(messag_coe, "./SS");
                    char buffer1[20];
                    snprintf(buffer1, sizeof(buffer1), "%d", idx);
                    strcat(messag_coe, buffer1);
                    strcat(messag_coe, "/");
                    char new_argument1[1024];
                    for (int i = 2; i < strlen(argument); i++)
                    {
                        new_argument1[i - 2] = argument[i];
                    }
                    new_argument1[strlen(argument) - 2] = '\0';
                    strcat(messag_coe, new_argument1);
                    strcat(messag_coe, " ");
                    strcat(messag_coe, "./SS");
                    char buffer2[20];
                    snprintf(buffer2, sizeof(buffer2), "%d", idx);
                    strcat(messag_coe, buffer1);
                    strcat(messag_coe, "/");

                    char argument2[1024];
                    for (int i = 2; i < strlen(extra_argument); i++)
                    {
                        argument2[i - 2] = extra_argument[i];
                    }
                    argument2[strlen(extra_argument) - 2] = '\0';
                    strcat(messag_coe, argument2);
                    strcat(messag_coe, " ");
                    // printf("Files are in same Storage server\n");
                    int client_socket3_2;
                    struct sockaddr_in server_addr3_2;
                    client_socket3_2 = socket(AF_INET, SOCK_STREAM, 0);
                    if (client_socket3_2 == -1)
                    {
                        perror("Error creating socket");
                        exit(EXIT_FAILURE);
                    }
                    server_addr3_2.sin_family = AF_INET;
                    server_addr3_2.sin_port = htons(query_port_for_ss[indx2]);
                    if (inet_pton(AF_INET, ip_for_ss[indx2], &server_addr3_2.sin_addr) <= 0)
                    {
                        perror("Invalid address/ Address not supported");
                        exit(EXIT_FAILURE);
                    }
                    if (connect(client_socket3_2, (struct sockaddr *)&server_addr3_2, sizeof(server_addr3_2)) < 0)
                    {
                        perror("Connection failed");
                        exit(EXIT_FAILURE);
                    }
                    printf("final sending %s\n", new1);
                    send(client_socket3_2, &messag_coe, sizeof(messag_coe), 0);
                    char m2[20000];
                    recv(client_socket3_2, &m2, sizeof(m2), 0);
                    printf("updated to SS %s\n", m2);
                    close(client_socket3_2);
                }
                continue;
            }
            else if (strcmp(function_to_call, "copydir") == 0)
            {
                printf("Client requested to copydir at %s named %s\n", argument, extra_argument);
                int idx = -1;
                idx = get_cache(argument);
                if (idx == -1)
                {
                    for (int i = 0; i < TOTAL_SERVER; i++)
                    {
                        if (search_trie(Trie_for_ss1[i], argument))
                        {
                            idx = i;
                            insert_cache(argument, idx);
                            break;
                        }
                    }
                }
                if (idx == -1)
                {
                    printf("the location you are finding does not exist\n");
                    send(client_socket, "failed to copy dir\n", sizeof("failed to copy dir\n"), 0);
                    continue;
                }
                int client_socket1;
                struct sockaddr_in server_addr;
                char message[100];
                // Create socket
                client_socket1 = socket(AF_INET, SOCK_STREAM, 0);
                if (client_socket1 == -1)
                {
                    perror("Error creating socket");
                    exit(EXIT_FAILURE);
                }

                // Set up server address
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(query_port_for_ss[idx]);
                if (inet_pton(AF_INET, ip_for_ss[idx], &server_addr.sin_addr) <= 0)
                {
                    perror("Invalid address/ Address not supported");
                    exit(EXIT_FAILURE);
                }
                // Connect to server
                if (connect(client_socket1, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                {
                    perror("Connection failed");
                    exit(EXIT_FAILURE);
                }
                send(client_socket1, temp, strlen(temp), 0);
                char response[1024];
                recv(client_socket1, response, sizeof(response), 0);
                printf("response %s\n", response);
                if (strcmp(response, "fail") != 0)
                {
                    insert_trie(Trie_for_ss1[idx], response);
                    send(client_socket, "copy executed\n", sizeof("copy executed\n"), 0);
                }
                else
                {
                    send(client_socket, "failed to copy dir\n", sizeof("failed to copy dir\n"), 0);
                }
                continue;
            }
            else
            {
                continue;
            }
            int index_of_server = -1;
            // try to search this in cache
            int rec = -1;
            index_of_server = get_cache(argument);
            if (index_of_server == -1)
            {
                for (int i = 1; i < TOTAL_SERVER; i++)
                {
                    if (index_of_server != -1)
                        break;
                    if (search_trie(Trie_for_ss1[i], argument))
                    {
                        index_of_server = i;
                        insert_cache(argument, index_of_server);
                        break;
                    }
                }
            }
            printf("got index of server %d\n", index_of_server);
            if (index_of_server == -1)
            {
                printf("The location you want to access do not exist\n");
                return_struct return_struct_to_send;
                return_struct_to_send.port = -1;
                return_struct_to_send.ip[0] = '\0';
                // failure message
                send(client_socket, &return_struct_to_send, sizeof(return_struct_to_send), 0);
                continue;
            }
            else
            {
                if (rec == -1)
                {
                    // updateAges(cache);
                    // addToCache(cache, argument, index_of_server);
                }
                // printCache(cache);
                return_struct return_struct_to_send;
                memset(&return_struct_to_send, 0, sizeof(return_struct_to_send));
                return_struct_to_send.port = port_number_for_client[index_of_server];
                strcpy(return_struct_to_send.ip, ip_for_ss[index_of_server]);
                return_struct_to_send.flag = 0;
                // Send a response back to the client (optional)
                printf("sending port %d ip %s\n", return_struct_to_send.port, return_struct_to_send.ip);
                send(client_socket, &return_struct_to_send, sizeof(return_struct_to_send), 0);
                // gopal
                if (write_flag == 1)
                {
                    char mess[20000];
                    recv(client_socket, mess, sizeof(mess), 0);
                    printf("message recievd from client  %s\n", mess);
                    char mess1[20000];
                    strcpy(mess1, mess);
                    char my_func[1024];
                    char functionpath[1024];
                    char message_file[10000];
                    sscanf(mess, "%s %s %[^\n]", my_func, functionpath, message_file);
                    char new[2000];
                    for (int i = 2; i < strlen(functionpath); i++)
                    {
                        new[i - 2] = functionpath[i];
                    }
                    new[strlen(functionpath) - 2] = '\0';

                    sprintf(mess, "./SS%d/%s", index_of_server, new);
                    char final_mess[40000];
                    snprintf(final_mess, sizeof(final_mess), "%s %s %s", my_func, mess, message_file);
                    printf("%s\n", final_mess);
                    int idx1 = first_temp_storage[index_of_server];
                    int idx2 = second_temp_storage[index_of_server];
                    if (idx1 != -1 && working_server[idx1] == 1)
                    {
                        int client_socket_for_updating = socket(AF_INET, SOCK_STREAM, 0);
                        if (client_socket_for_updating == -1)
                        {
                            perror("Socket creation failed");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in serverAddress_for_updating;
                        memset(&serverAddress_for_updating, 0, sizeof(serverAddress_for_updating));
                        serverAddress_for_updating.sin_family = AF_INET;
                        serverAddress_for_updating.sin_port = htons(port_number_for_client[idx1]);
                        if (inet_pton(AF_INET, ip_for_ss[idx1], &serverAddress_for_updating.sin_addr) <= 0)
                        {
                            perror("Address conversion failed");
                            exit(EXIT_FAILURE);
                        }

                        // Connect to the server
                        if (connect(client_socket_for_updating, (struct sockaddr *)&serverAddress_for_updating, sizeof(serverAddress_for_updating)) == -1)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        send(client_socket_for_updating, &final_mess, sizeof(final_mess), 0);
                        bzero(mess, sizeof(mess));
                        recv(client_socket_for_updating, mess, sizeof(mess), 0);
                        printf("Updated server gives message%s\n", mess);
                        close(client_socket_for_updating);
                    }
                    usleep(1000);
                    if (idx2 != -1 && working_server[idx1] == 1)
                    {
                        int client_socket_for_updating = socket(AF_INET, SOCK_STREAM, 0);
                        if (client_socket_for_updating == -1)
                        {
                            perror("Socket creation failed");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in serverAddress_for_updating;
                        memset(&serverAddress_for_updating, 0, sizeof(serverAddress_for_updating));
                        serverAddress_for_updating.sin_family = AF_INET;
                        serverAddress_for_updating.sin_port = htons(port_number_for_client[idx2]);
                        if (inet_pton(AF_INET, ip_for_ss[idx2], &serverAddress_for_updating.sin_addr) <= 0)
                        {
                            perror("Address conversion failed");
                            exit(EXIT_FAILURE);
                        }

                        // Connect to the server
                        if (connect(client_socket_for_updating, (struct sockaddr *)&serverAddress_for_updating, sizeof(serverAddress_for_updating)) == -1)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        send(client_socket_for_updating, &final_mess, sizeof(final_mess), 0);
                        bzero(mess, sizeof(mess));
                        recv(client_socket_for_updating, mess, sizeof(mess), 0);
                        printf("Updated server gives message%s\n", mess);
                        close(client_socket_for_updating);
                    }
                    write_flag = 0;
                }
                // gopal
                else if (append_flag == 1)
                {
                    char mess[20000];
                    recv(client_socket, mess, sizeof(mess), 0);
                    int idx1 = first_temp_storage[index_of_server];
                    int idx2 = second_temp_storage[index_of_server];
                    if (idx1 != -1 && working_server[idx1] == 1)
                    {
                        int client_socket_for_updating = socket(AF_INET, SOCK_STREAM, 0);
                        if (client_socket_for_updating == -1)
                        {
                            perror("Socket creation failed");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in serverAddress_for_updating;
                        memset(&serverAddress_for_updating, 0, sizeof(serverAddress_for_updating));
                        serverAddress_for_updating.sin_family = AF_INET;
                        serverAddress_for_updating.sin_port = htons(port_number_for_client[idx1]);
                        if (inet_pton(AF_INET, ip_for_ss[idx1], &serverAddress_for_updating.sin_addr) <= 0)
                        {
                            perror("Address conversion failed");
                            exit(EXIT_FAILURE);
                        }

                        // Connect to the server
                        if (connect(client_socket_for_updating, (struct sockaddr *)&serverAddress_for_updating, sizeof(serverAddress_for_updating)) == -1)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        send(client_socket_for_updating, &mess, sizeof(mess), 0);
                        bzero(mess, sizeof(mess));
                        recv(client_socket_for_updating, mess, sizeof(mess), 0);
                        printf("Updated server gives message%s\n", mess);
                        close(client_socket_for_updating);
                    }
                    usleep(1000);
                    if (idx2 != -1 && working_server[idx2] == 1)
                    {
                        int client_socket_for_updating = socket(AF_INET, SOCK_STREAM, 0);
                        if (client_socket_for_updating == -1)
                        {
                            perror("Socket creation failed");
                            exit(EXIT_FAILURE);
                        }
                        struct sockaddr_in serverAddress_for_updating;
                        memset(&serverAddress_for_updating, 0, sizeof(serverAddress_for_updating));
                        serverAddress_for_updating.sin_family = AF_INET;
                        serverAddress_for_updating.sin_port = htons(port_number_for_client[idx2]);
                        if (inet_pton(AF_INET, ip_for_ss[idx2], &serverAddress_for_updating.sin_addr) <= 0)
                        {
                            perror("Address conversion failed");
                            exit(EXIT_FAILURE);
                        }

                        // Connect to the server
                        if (connect(client_socket_for_updating, (struct sockaddr *)&serverAddress_for_updating, sizeof(serverAddress_for_updating)) == -1)
                        {
                            perror("Connection failed");
                            exit(EXIT_FAILURE);
                        }
                        send(client_socket_for_updating, &mess, sizeof(mess), 0);
                        bzero(mess, sizeof(mess));
                        recv(client_socket_for_updating, mess, sizeof(mess), 0);
                        printf("Updated server gives message%s\n", mess);
                        close(client_socket_for_updating);
                    }
                }
                // gopal
                printf("sending port %d ip %s\n", return_struct_to_send.port, return_struct_to_send.ip);

                // recv(client_socket, buffer, sizeof(buffer), 0);
                // printf("Received data: %s", buffer);
            }
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
    printf("Received IP : %s port_num : %d port_for_customer : %d server_num : %d\n", my_info.ip, my_info.port, my_info.client_port, arrived_till_now + 1);
    my_info.server_num = arrived_till_now + 1;
    port_number_for_client[my_info.server_num] = my_info.client_port;
    port_number_for_ss[my_info.server_num] = my_info.port;
    printf("%s\n", my_info.fullpath);
    query_port_for_ss[my_info.server_num] = my_info.query_port;
    ip_for_ss[my_info.server_num] = (char *)malloc(sizeof(char) * 35);
    strcpy(ip_for_ss[my_info.server_num], my_info.ip);
    char *token = strtok(my_info.fullpath, "\n");
    while (token != NULL && token[0] != '\n')
    {
        printf("%s\n", token);
        insert_trie(Trie_for_ss1[my_info.server_num], token);
        token = strtok(NULL, "\n");
    }
    working_server[my_info.server_num] = 1;
    currently_working_storage_servers++;
    arrived_till_now++;
    printf("%d storage servers are working\n", currently_working_storage_servers);

    if (currently_working_storage_servers == 3)
    {
        printf("%d storage servers are working\n", currently_working_storage_servers);
        start_red_control(1);
        start_red_control(2);
        start_red_control(3);
    }
    else if (currently_working_storage_servers > 3)
    {
        start_red_control(my_info.server_num);
    }
    // while (1)
    // {
    //     if (recv(stserver_socket, buffer, sizeof(buffer), 0) > 0)
    //         printf("Received data: %s %d\n", buffer, my_info.server_num);
    //     currently_working_storage_servers--;
    //     printf("%d storage servers are working\n", currently_working_storage_servers);
    //     working_server[my_info.server_num] = 0;

    //     if (recv(stserver_socket, buffer, sizeof(buffer), 0) > 0)
    //         printf("Received data: %s %d\n", buffer, my_info.server_num);
    //     currently_working_storage_servers++;
    //     printf("%d storage servers are working\n", currently_working_storage_servers);
    //     working_server[my_info.server_num] = 1;
    //     char newpath[5000];
    //     memset(newpath, '\0', sizeof(newpath));
    //     if (recv(stserver_socket, newpath, sizeof(newpath), 0) > 0)
    //     {
    //         // printf("Received data: %s %d", newpath, my_info.server_num);
    //     }
    //     printf("newpath %s\n", newpath);
    //     // free_trienode(Trie_for_ss1[my_info.server_num]);

    //     // trie_add(my_info.server_num, newpath);
    // }

    // currently_working_storage_servers--;
    // close(stserver_socket);

    // new additions
    while (1)
    {
        if (recv(stserver_socket, buffer, sizeof(buffer), 0) > 0)
            printf("Received data: %s %d\n", buffer, my_info.server_num);
        if (strcmp(buffer, "EXIT") != 0)
        {
            printf("Didnt Get EXIT\n");
            break;
        }
        currently_working_storage_servers--;
        printf("Now %d storage servers are working\n", currently_working_storage_servers);
        working_server[my_info.server_num] = 0;

        if (recv(stserver_socket, buffer, sizeof(buffer), 0) > 0)
            printf("Received data: %s %d\n", buffer, my_info.server_num);
        if (strcmp(buffer, "START") != 0)
        {
            printf("Didnt Get START\n");
            break;
        }

        currently_working_storage_servers++;
        printf("%d storage servers are working\n", currently_working_storage_servers);
        working_server[my_info.server_num] = 1;
        char newpath[20000];
        memset(newpath, '\0', sizeof(newpath));
        if (recv(stserver_socket, newpath, sizeof(newpath), 0) > 0)
        {
            printf("Received data: %s %d", newpath, my_info.server_num);
        }
        // printf("newpath %s\n", newpath);
        free_trienode(Trie_for_ss1[my_info.server_num]);
        Trie_for_ss1[my_info.server_num] = make_trienode('0');
        trie_add(my_info.server_num, newpath);
    }

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

        printf("Connection from storage server %s:%d\n", inet_ntoa(stserver_address.sin_addr), ntohs(stserver_address.sin_port));
        pthread_t stserver_thread;
        pthread_create(&stserver_thread, NULL, stserver_handler, (void *)&stserver_socket);
    }
    return NULL;
}

int main()
{
    for (int i = 0; i < mod; i++)
    {
        hash[i] = -1;
    }

    srand(time(NULL));
    // cache = createLRUCache(CACHE_SIZE);
    for (int i = 0; i < TOTAL_SERVER; i++)
    {
        Trie_for_ss1[i] = make_trienode('0');
        first_temp_storage[i] = -1;
        second_temp_storage[i] = -1;
    }

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
    server_address2.sin_port = htons(PORT2);

    // Bind the socket to the specified address and port
    if (bind(server_socket2, (struct sockaddr *)&server_address2, sizeof(server_address2)) == -1)
    {
        perror("Error binding socket");
        close(server_socket2);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket2, 10) == -1)
    {
        perror("Error listening for connections");
        close(server_socket2);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d for customers\n", PORT1);
    printf("Server is listening on port %d for storage servers\n", PORT2);

    pthread_create(&customer_intialize_thread, NULL, customer_intialize, NULL);
    pthread_create(&stserver_intialize_thread, NULL, stserver_intialize, NULL);
    // pthread_create(&cli_thread, NULL, cli_handler, NULL);

    // pthread_join(customer_intialize_thread, NULL);
    pthread_join(stserver_intialize_thread, NULL);
    pthread_join(cli_thread, NULL);

    // Close the server socket
    close(server_socket);

    return 0;
}
// for (int i = 1; i < 100; i++)
// {
//     pthread_mutex_lock(&lock);
//     if (search(Trie_for_ss[i], argument))
//     {
//         index_of_server = i;
//         pthread_mutex_unlock(&lock);
//         break;
//     }
//     pthread_mutex_unlock(&lock);
//     // printf("search output %d\n", search(Trie_for_ss[i], "./output.txt"));
// }
// printf("got index of server %d\n", index_of_server);

// //
//  pthread_t function_server;
//                 pthread_create(&function_server, NULL, function_server_handler, (void *)&buffer);
//                 pthread_join(function_server, NULL);
//                 send(client_socket, "done", strlen("done"), 0);