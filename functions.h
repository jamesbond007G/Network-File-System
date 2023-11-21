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

typedef struct TrieNode TrieNode;
#define N 257

struct TrieNode
{
    // The Trie Node Structure
    // Each node has N children, starting from the root
    // and a flag to check if it's a leaf node
    char data; // Storing for printing purposes only
    TrieNode *children[N];
    int is_leaf;
};
TrieNode *make_trienode(char data);
void free_trienode(TrieNode *node);
TrieNode *insert_trie(TrieNode *root, char *word);
int search_trie(TrieNode *root, char *word);
void print_trie(TrieNode *root);
void print_search(TrieNode *root, char *word);
TrieNode* delete_trie(TrieNode* root, char* word);
int is_leaf_node(TrieNode *root, char *word);
