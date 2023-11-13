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

#define CHAR_SIZE 26

struct Trie
{
    int isLeaf;             // 1 when the node is a leaf node
    struct Trie* character[CHAR_SIZE];
};

struct Trie *getNewTrieNode();
void insert(struct Trie *head, char *str);
int search(struct Trie *head, char *str);
int hasChildren(struct Trie *curr);
int deletion(struct Trie **curr, char *str);