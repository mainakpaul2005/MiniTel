#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "contact.h"
#include "hashmap.h"

#define MAX_CONTACTS 10000

// Input validation functions
int isValidName(const char* name) {
    if (strlen(name) < 2 || strlen(name) > 49) return 0;
    for (int i = 0; name[i]; i++) {
        if (!isalpha(name[i]) && !isspace(name[i])) return 0;
    }
    return 1;
}

int isValidPhone(const char* phone) {
    if (strlen(phone) < 10 || strlen(phone) > 14) return 0;
    for (int i = 0; phone[i]; i++) {
        if (!isdigit(phone[i]) && phone[i] != '+' && phone[i] != '-') return 0;
    }
    return 1;
}

int isValidEmail(const char* email) {
    if (strlen(email) < 5 || strlen(email) > 49) return 0;
    const char* at = strchr(email, '@');
    const char* dot = strrchr(email, '.');
    return at && dot && dot > at && dot[1];
}

void addContact() {
    if (contactCount >= MAX_CONTACTS) {
        printf("\033[1;31m✖ Error: Contact list is full!\033[0m\n");
        return;
    }

    Contact c;
    c.id = contactCount + 1;
    
    do {
        printf("Enter name (letters only): ");
        scanf(" %[^\n]", c.name);
        if (!isValidName(c.name)) {
            printf("\033[1;31m✖ Invalid name format! Use only letters and spaces.\033[0m\n");
        }
    } while (!isValidName(c.name));

    if (searchHash(c.name) != -1) {
        printf("\033[1;31m✖ Error: Contact with this name already exists!\033[0m\n");
        return;
    }

    do {
        printf("Enter phone (e.g., +1-1234567890): ");
        scanf(" %[^\n]", c.phone);
        if (!isValidPhone(c.phone)) {
            printf("\033[1;31m✖ Invalid phone format! Use digits, +, and - only.\033[0m\n");
        }
    } while (!isValidPhone(c.phone));

    do {
        printf("Enter email: ");
        scanf(" %[^\n]", c.email);
        if (!isValidEmail(c.email)) {
            printf("\033[1;31m✖ Invalid email format!\033[0m\n");
        }
    } while (!isValidEmail(c.email));

    c.isDeleted = 0;
    c.deletedAt = 0;

    contacts[contactCount] = c;
    insertHash(c.name, contactCount);
    appendLog(c);
    contactCount++;

    printf("\033[1;32m✔ Contact added successfully!\033[0m\n");
}

void searchContact() {
    char name[50];
    printf("Enter name to search: ");
    scanf(" %[^\n]", name);
    
    int idx = searchHash(name);
    if (idx == -1 || contacts[idx].isDeleted) {
        printf("\033[1;31m✖ Contact not found.\033[0m\n");
    } else {
        printf("\n\033[1;36m┌─────────────────────────────────────┐\033[0m\n");
        printf("\033[1;36m│ ID:    %-30d │\033[0m\n", contacts[idx].id);
        printf("\033[1;36m│ Name:  %-30s │\033[0m\n", contacts[idx].name);
        printf("\033[1;36m│ Phone: %-30s │\033[0m\n", contacts[idx].phone);
        printf("\033[1;36m│ Email: %-30s │\033[0m\n", contacts[idx].email);
        printf("\033[1;36m└─────────────────────────────────────┘\033[0m\n");
    }
}

void deleteContact() {
    char name[50];
    printf("Enter name to delete: ");
    scanf(" %[^\n]", name);
    
    int idx = searchHash(name);
    if (idx == -1 || contacts[idx].isDeleted) {
        printf("\033[1;31m✖ Contact not found.\033[0m\n");
        return;
    }

    printf("\033[1;33m⚠ Are you sure you want to delete %s? (y/n): \033[0m", name);
    char confirm;
    scanf(" %c", &confirm);
    
    if (tolower(confirm) == 'y') {
        contacts[idx].isDeleted = 1;
        contacts[idx].deletedAt = time(NULL);
        appendLog(contacts[idx]);
        printf("\033[1;33m⚠ Contact moved to recycle bin.\033[0m\n");
    } else {
        printf("\033[1;32m✔ Deletion cancelled.\033[0m\n");
    }
}

void restoreContacts() {
    time_t now = time(NULL);
    int restored = 0;
    
    printf("\033[1;36m📋 Deleted contacts (last 15 days):\033[0m\n\n");
    
    for (int i = 0; i < contactCount; i++) {
        if (contacts[i].isDeleted && difftime(now, contacts[i].deletedAt) <= 1296000) {
            printf("  %d. %s (deleted %s)", 
                contacts[i].id, 
                contacts[i].name, 
                ctime(&contacts[i].deletedAt));
            restored++;
        }
    }

    if (restored == 0) {
        printf("\033[1;33mNo contacts to restore.\033[0m\n");
        return;
    }

    printf("\n\033[1;33mDo you want to restore all contacts? (y/n): \033[0m");
    char confirm;
    scanf(" %c", &confirm);

    if (tolower(confirm) == 'y') {
        for (int i = 0; i < contactCount; i++) {
            if (contacts[i].isDeleted && difftime(now, contacts[i].deletedAt) <= 1296000) {
                contacts[i].isDeleted = 0;
                contacts[i].deletedAt = 0;
                insertHash(contacts[i].name, i);
                appendLog(contacts[i]);
                printf("\033[1;32m♻ Restored: %s\033[0m\n", contacts[i].name);
            }
        }
    } else {
        printf("\033[1;32m✔ Restoration cancelled.\033[0m\n");
    }
}

void showAllContacts() {
    int active = 0;
    printf("\n\033[1;36m📋 Active Contacts:\033[0m\n\n");
    printf("\033[1;36m┌────┬──────────────────┬──────────────────┬──────────────────┐\033[0m\n");
    printf("\033[1;36m│ ID │ Name             │ Phone            │ Email            │\033[0m\n");
    printf("\033[1;36m├────┼──────────────────┼──────────────────┼──────────────────┤\033[0m\n");
    
    for (int i = 0; i < contactCount; i++) {
        if (!contacts[i].isDeleted) {
            printf("\033[1;37m│ %-2d │ %-16s │ %-16s │ %-16s │\033[0m\n",
                contacts[i].id, contacts[i].name, contacts[i].phone, contacts[i].email);
            active++;
        }
    }
    
    printf("\033[1;36m└────┴──────────────────┴──────────────────┴──────────────────┘\033[0m\n");
    printf("\nTotal active contacts: %d\n", active);
}

void exportSnapshot() {
    FILE* file = fopen("contacts_snapshot.csv", "w");
    if (!file) {
        printf("\033[1;31m✖ Error: Could not create snapshot file!\033[0m\n");
        return;
    }

    for (int i = 0; i < contactCount; i++) {
        fprintf(file, "%d,%s,%s,%s,%d,%ld\n",
            contacts[i].id, contacts[i].name, contacts[i].phone,
            contacts[i].email, contacts[i].isDeleted, contacts[i].deletedAt);
    }
    
    fclose(file);
    printf("\033[1;32m✔ Contacts exported successfully to contacts_snapshot.csv!\033[0m\n");
}
