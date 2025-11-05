// Simple telephone directory using CSV storage
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_FILE "contacts.csv"
#define MAX_LINE 1024

typedef struct {
    int id;
    char name[128];
    char phone[64];
    char email[128];
    char address[256];
} Contact;

typedef struct {
    Contact *list;
    int count;
    int capacity;
} Directory;

Directory* dir_create(void) {
    Directory *d = malloc(sizeof(Directory));
    d->capacity = 8;
    d->list = malloc(d->capacity * sizeof(Contact));
    d->count = 0;
    return d;
}

void dir_add(Directory *d, Contact c) {
    if (d->count >= d->capacity) {
        d->capacity *= 2;
        d->list = realloc(d->list, d->capacity * sizeof(Contact));
    }
    d->list[d->count++] = c;
}

Contact* dir_find(Directory *d, int id) {
    for (int i = 0; i < d->count; i++)
        if (d->list[i].id == id) return &d->list[i];
    return NULL;
}

void dir_delete(Directory *d, int id) {
    for (int i = 0; i < d->count; i++) {
        if (d->list[i].id == id) {
            memmove(&d->list[i], &d->list[i+1], (d->count-i-1) * sizeof(Contact));
            d->count--;
            return;
        }
    }
}

void dir_free(Directory *d) {
    free(d->list);
    free(d);
}

Directory* load_contacts(void) {
    Directory *d = dir_create();
    FILE *f = fopen(CSV_FILE, "r");
    if (!f) return d;
    
    char line[MAX_LINE];
    if (fgets(line, MAX_LINE, f)) {} // skip header
    
    while (fgets(line, MAX_LINE, f)) {
        Contact c = {0};
        sscanf(line, "%d,%127[^,],%63[^,],%127[^,],%255[^\n]", 
               &c.id, c.name, c.phone, c.email, c.address);
        dir_add(d, c);
    }
    fclose(f);
    return d;
}

int next_id(Directory *d) {
    int max = 0;
    for (int i = 0; i < d->count; i++)
        if (d->list[i].id > max) max = d->list[i].id;
    return max + 1;
}

void save_all(Directory *d) {
    FILE *f = fopen(CSV_FILE, "w");
    if (!f) return;
    fprintf(f, "id,name,phone,email,address\n");
    for (int i = 0; i < d->count; i++) {
        Contact *c = &d->list[i];
        fprintf(f, "%d,%s,%s,%s,%s\n", c->id, c->name, c->phone, c->email, c->address);
    }
    fclose(f);
}

void add_contact(void) {
    Directory *d = load_contacts();
    Contact c = {0};
    
    printf("\n  -----------------------------------------------------------\n");
    printf("                     ADD NEW CONTACT                         \n");
    printf("  -----------------------------------------------------------\n\n");
    printf("  Name    : "); scanf(" %127[^\n]", c.name);
    printf("  Phone   : "); scanf(" %63[^\n]", c.phone);
    printf("  Email   : "); scanf(" %127[^\n]", c.email);
    printf("  Address : "); scanf(" %255[^\n]", c.address);
    
    c.id = next_id(d);
    dir_add(d, c);
    save_all(d);
    printf("\n  >> Contact added successfully (ID: %d)\n\n", c.id);
    dir_free(d);
}

void list_contacts(void) {
    Directory *d = load_contacts();
    if (d->count == 0) { 
        printf("\n  No contacts found.\n\n"); 
        dir_free(d); 
        return; 
    }
    printf("\n  -----------------------------------------------------------\n");
    printf("                       CONTACT LIST                          \n");
    printf("  -----------------------------------------------------------\n\n");
    for (int i = 0; i < d->count; i++) {
        Contact *c = &d->list[i];
        printf("  --- Contact #%d --------------------------------------------\n", c->id);
        printf("  Name    : %s\n", c->name);
        printf("  Phone   : %s\n", c->phone);
        printf("  Email   : %s\n", c->email);
        printf("  Address : %s\n", c->address);
        printf("  -----------------------------------------------------------\n\n");
    }
    printf("  Total Contacts: %d\n\n", d->count);
    dir_free(d);
}

void search_contacts(void) {
    char q[256];
    printf("\n  -----------------------------------------------------------\n");
    printf("                     SEARCH CONTACTS                         \n");
    printf("  -----------------------------------------------------------\n\n");
    printf("  Enter name or phone: ");
    scanf(" %255[^\n]", q);
    
    Directory *d = load_contacts();
    printf("\n  Search results for: '%s'\n\n", q);
    
    int found = 0;
    for (int i = 0; i < d->count; i++) {
        Contact *c = &d->list[i];
        if (strstr(c->name, q) || strstr(c->phone, q)) {
            printf("  [%d] %-30s  %-15s\n", c->id, c->name, c->phone);
            found = 1;
        }
    }
    if (!found) printf("  >> No matches found.\n");
    printf("\n");
    dir_free(d);
}

void delete_contact(void) {
    int id;
    printf("\n  -----------------------------------------------------------\n");
    printf("                     DELETE CONTACT                          \n");
    printf("  -----------------------------------------------------------\n\n");
    printf("  Enter Contact ID: ");
    scanf("%d", &id);
    
    Directory *d = load_contacts();
    Contact *c = dir_find(d, id);
    if (!c) { 
        printf("\n  >> Contact #%d not found.\n\n", id); 
        dir_free(d); 
        return; 
    }
    dir_delete(d, id);
    save_all(d);
    dir_free(d);
    printf("\n  >> Contact #%d deleted successfully.\n\n", id);
}

void update_contact(void) {
    char buf[512];
    int id;
    printf("\n  -----------------------------------------------------------\n");
    printf("                     UPDATE CONTACT                          \n");
    printf("  -----------------------------------------------------------\n\n");
    printf("  Enter Contact ID: ");
    scanf("%d", &id);
    
    Directory *d = load_contacts();
    Contact *c = dir_find(d, id);
    if (!c) { 
        printf("\n  >> Contact #%d not found.\n\n", id); 
        dir_free(d); 
        return; 
    }
    
    printf("\n  Editing Contact #%d\n", id);
    printf("  -----------------------------------------------------------\n");
    printf("  Name    [%s]: ", c->name);
    scanf(" %511[^\n]", buf);
    if (strlen(buf) > 0) strcpy(c->name, buf);
    
    printf("  Phone   [%s]: ", c->phone);
    scanf(" %511[^\n]", buf);
    if (strlen(buf) > 0) strcpy(c->phone, buf);
    
    printf("  Email   [%s]: ", c->email);
    scanf(" %511[^\n]", buf);1
    if (strlen(buf) > 0) strcpy(c->email, buf);
    
    printf("  Address [%s]: ", c->address);
    scanf(" %511[^\n]", buf);
    if (strlen(buf) > 0) strcpy(c->address, buf);
    
    save_all(d);
    dir_free(d);
    printf("\n  >> Contact #%d updated successfully.\n\n", id);
}

int main(void) {
    int c;
    while (1) {
        printf("\n  -----------------------------------------------------------\n");
        printf("                 MINITEL DIRECTORY MANAGER                   \n");
        printf("  -----------------------------------------------------------\n");
        printf("\n  [1] Add Contact      [2] List All       [3] Search\n");
        printf("  [4] Delete Contact   [5] Update         [0] Exit\n");
        printf("\n  Enter your choice: ");
        scanf("%d", &c);
        
        if (c == 1) add_contact();
        else if (c == 2) list_contacts();
        else if (c == 3) search_contacts();
        else if (c == 4) delete_contact();
        else if (c == 5) update_contact();
        else if (c == 0) { 
            printf("\n  Thank you for using MiniTel. Goodbye!\n\n"); 
            return 0; 
        }
        else printf("\n  >> Invalid choice. Please try again.\n");
    }
}
