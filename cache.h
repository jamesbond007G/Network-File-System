#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct cache
{
    char path[1024];
    int server_num;
    struct cache *next;

} cache;

extern cache *head;
cache *createCacheNode(const char *path, int server_num);
cache *insertCacheNode(const char *path, int server_num);
cache *deleteLastElement();
cache *insert_cache(const char *path, int server_num);
cache *deleteNodeByPath(const char *pathToDelete);
void printCacheList();
int get_cache(char *path);