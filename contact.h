#ifndef CONTACT_H
#define CONTACT_H

#include <time.h>


typedef struct {
    int id;
    char name[50];
    char phone[15];
    char email[50];
    int isDeleted;
    time_t deletedAt;
} Contact;

#endif

// Shared globals and helpers (defined in directory.c)
extern Contact contacts[];
extern int contactCount;
void appendLog(Contact c);