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
#include <dirent.h>
#define PORT 5432
// #define SERVER_IP "10.42.0.1"
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE 1024
#define server_no 1