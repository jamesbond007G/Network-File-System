#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT1 12345
#define PORT2 54321
#define OK_PORT 9935
#define BUFFER_SIZE 5000
#define SERVER_IP "10.42.0.1"
typedef struct
{
    char ip[32];
    int port;
    int client_port;
    int server_num;
    int query_port;
    char fullpath[BUFFER_SIZE];

} My_info;

typedef struct
{

    char ip[32];
    int port;
    int flag;
    char fullpath[BUFFER_SIZE];
} return_struct;

typedef struct
{
    int server_num;
    char argument[1024];
} sender_struct;
typedef struct
{
    int role;
    char path[1024];
    char ip[50];
    int port;
} between_ss;
