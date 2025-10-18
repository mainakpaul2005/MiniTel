#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

#define TABLE_SIZE 1000

typedef struct HashNode {
    char key[50];
    int index;
    struct HashNode* next;
} HashNode;

static HashNode* hashTable[TABLE_SIZE] = {NULL};

// Hash function: simple polynomial rolling hash
unsigned int hash(const char* key) {
    unsigned int h = 0;
    for (int i = 0; key[i]; i++)
        h = 31 * h + (unsigned char)key[i];
    return h % TABLE_SIZE;
}

// Insert a key and contact index into the hash table
void insertHash(const char* key, int index) {
    unsigned int h = hash(key);
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) {
        fprintf(stderr, "[WARN] insertHash: malloc failed for key '%s'\n", key);
        return;
    }

    strcpy(newNode->key, key);
    newNode->index = index;
    newNode->next = hashTable[h];
    hashTable[h] = newNode;
}

// Search for a key, return the contact index or -1 if not found
int searchHash(const char* key) {
    unsigned int h = hash(key);
    HashNode* curr = hashTable[h];

    while (curr) {
        if (strcmp(curr->key, key) == 0)
            return curr->index;
        curr = curr->next;
    }
    return -1;
}

// Free all hash nodes from memory
void clearHash() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* curr = hashTable[i];
        while (curr) {
            HashNode* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
        hashTable[i] = NULL;
    }
}
