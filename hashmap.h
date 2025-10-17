#ifndef HASHMAP_H
#define HASHMAP_H

unsigned int hash(const char* key);
void insertHash(const char* key, int index);
int searchHash(const char* key);
void clearHash();

#endif
