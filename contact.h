#ifndef CONTACT_H
#define CONTACT_H

#include <time.h>

// Maximum number of contacts supported by the application
#define MAX_CONTACTS 10000

typedef struct {
    int id;
    char name[50];
    char phone[15];
    char email[50];
    int isDeleted;
    time_t deletedAt;
} Contact;

// Shared globals and helpers (defined in directory.c)
extern Contact contacts[MAX_CONTACTS];
extern int contactCount;
void appendLog(Contact c);

#endif // CONTACT_H