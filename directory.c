// Directory management: contacts storage and load/append helpers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "contact.h"
#include "hashmap.h"

#define MAX_CONTACTS 10000
Contact contacts[MAX_CONTACTS];
int contactCount = 0;

// Helper function to create backup of snapshot file
void createBackup() {
    char command[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char backup_name[50];
    
    sprintf(backup_name, "contacts_backup_%04d%02d%02d.csv",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    
#ifdef _WIN32
    sprintf(command, "copy contacts_snapshot.csv %s >nul 2>&1", backup_name);
#else
    sprintf(command, "cp contacts_snapshot.csv %s 2>/dev/null", backup_name);
#endif
    
    system(command);
}

void loadContacts() {
    FILE* file = fopen("contacts_snapshot.csv", "r");
    if (!file) {
        printf("\033[1;33m⚠ No existing contacts file found. Starting fresh.\033[0m\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (contactCount >= MAX_CONTACTS) {
            printf("\033[1;31m✖ Warning: Maximum contacts limit reached!\033[0m\n");
            break;
        }

        Contact* c = &contacts[contactCount];
        if (sscanf(line, "%d,%49[^,],%14[^,],%49[^,],%d,%ld",
                   &c->id, c->name, c->phone, c->email,
                   &c->isDeleted, &c->deletedAt) == 6) {
            if (!c->isDeleted) {
                insertHash(c->name, contactCount);
            }
            contactCount++;
        }
    }
    
    fclose(file);
    printf("\033[1;32m✔ Loaded %d contacts successfully.\033[0m\n", contactCount);
}

void appendLog(Contact c) {
    FILE* log = fopen("contacts_log.csv", "a");
    if (!log) {
        printf("\033[1;31m✖ Error: Could not open log file!\033[0m\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    fprintf(log, "%d,%s,%s,%s,%d,%ld,%04d-%02d-%02d %02d:%02d:%02d\n",
            c.id, c.name, c.phone, c.email, c.isDeleted, c.deletedAt,
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
            
    fclose(log);
    
    // Create backup of snapshot file when logging changes
    static time_t last_backup = 0;
    if (difftime(now, last_backup) > 86400) { // 24 hours
        createBackup();
        last_backup = now;
    }
}
