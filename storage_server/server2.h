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
#include "netinet/in.h"
#include "tar.h"
#include <dirent.h>
#include <sys/wait.h>
// #include"libtar.h"
#include "errno.h"
#define PORT 54326
#define SERVER_IP "10.42.0.56"
// #define CLIENT_PORT 4771
#define BUFFER_SIZE 5000
// #define PERSONAL_PORT 2171
#define server_no 2
typedef struct
{
    int role;
    char path[1024];
    char ip[50];
    int port;
    // char next_path[1024];
} between_ss;
void sendFileDetailsToSocket(const char *filename, int socket);
long long calculateDirectorySize(const char *dirname);

#define OK_PORT 9900