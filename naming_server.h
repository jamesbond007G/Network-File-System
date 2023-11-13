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

#define PORT11 12345
#define PORT2 54321
#define PORT2 5432
#define BUFFER_SIZE 1024
typedef struct
{
    char ip[32];
    int port;
    int client_port;
    int server_num;
    char fullpath[1024];

} My_info;


