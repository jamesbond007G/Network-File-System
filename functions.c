// Define the character size
#include "functions.h"


TrieNode *make_trienode(char data)
{
    // Allocate memory for a TrieNode
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    for (int i = 0; i < N; i++)
        node->children[i] = NULL;
    node->is_leaf = 0;
    node->data = data;
    return node;
}

void free_trienode(TrieNode *node)
{
    // Free the trienode sequence
    for (int i = 0; i < N; i++)
    {
        if (node->children[i] != NULL)
        {
            free_trienode(node->children[i]);
        }
        else
        {
            continue;
        }
    }
    free(node);
}

TrieNode *insert_trie(TrieNode *root, char *word)
{
    // Inserts the word onto the Trie
    // ASSUMPTION: The word only has lower case characters
    TrieNode *temp = root;

    for (int i = 0; word[i] != '\0'; i++)
    {
        // Get the relative position in the alphabet list
        int idx = (int)word[i];
        if (temp->children[idx] == NULL)
        {
            // If the corresponding child doesn't exist,
            // simply create that child!
            temp->children[idx] = make_trienode(word[i]);
        }
        else
        {
            // Do nothing. The node already exists
        }
        // Go down a level, to the child referenced by idx
        // since we have a prefix match
        temp = temp->children[idx];
    }
    // At the end of the word, mark this node as the leaf node
    temp->is_leaf = 1;
    return root;
}

int search_trie(TrieNode *root, char *word)
{


    // Searches for word in the Trie
    TrieNode *temp = root;

    for (int i = 0; word[i] != '\0'; i++)
    {
        int position = (int)word[i];
        if (temp->children[position] == NULL)
            return 0;
        temp = temp->children[position];
    }
    if (temp != NULL && temp->is_leaf == 1)
        return 1;
    return 0;
}

void print_trie(TrieNode *root)
{
    // printf(" ");
    // Prints the nodes of the trie
    if (!root)
        return;
    TrieNode *temp = root;
    printf("%c -> ", temp->data);
    for (int i = 0; i < N; i++)
    {
        print_trie(temp->children[i]);
    }
}

void print_search(TrieNode *root, char *word)
{
    printf("Searching for %s: ", word);
    if (search_trie(root, word) == 0)
        printf("Not Found\n");
    else
        printf("Found!\n");
}
int is_leaf_node(TrieNode* root, char* word) {
    // Checks if the prefix match of word and root
    // is a leaf node
    TrieNode* temp = root;
    for (int i=0; word[i]; i++) {
        int position = (int) word[i] - 'a';
        if (temp->children[position]) {
            temp = temp->children[position];
        }
    }
    return temp->is_leaf;
}
TrieNode* delete_trie(TrieNode* root, char* word) {
    // Will try to delete the word sequence from the Trie only it 
    // ends up in a leaf node
    if (!root)
        return NULL;
    if (!word || word[0] == '\0')
        return root;
    // If the node corresponding to the match is not a leaf node,
    // we stop
    if (!is_leaf_node(root, word)) {
        return root;
    }
    // TODO
}