#include "cache.h"
#define CACHE_SIZE 2
int mod = 100007;
int hash[100007]={-1};
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
