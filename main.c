// Simple telephone directory using CSV storage
// Single-file program: main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CSV_FILE "contacts.csv"
#define LINE_BUF 1024
#define HASH_SIZE 256  // Must be power of 2
#define HASH_MASK (HASH_SIZE - 1)

typedef struct Contact {
    int id;
    char name[128];
    char phone[64];
    char email[128];
    char address[256];
    struct Contact *next;  // for hash chain
} Contact;

typedef struct {
    Contact **contacts;    // dynamic array of Contact pointers
    size_t size;          // number of contacts in array
    size_t capacity;      // allocated capacity
    Contact *buckets[HASH_SIZE];  // hash table buckets
} Directory;

// Hash function for contact IDs (simple multiplicative hash)
static inline uint32_t hash_int(int key) {
    return (uint32_t)key * 2654435761u;  // Knuth's multiplicative hash
}

// Get bucket index for a key
static inline size_t bucket_idx(int key) {
    return hash_int(key) & HASH_MASK;
}

// Create new directory with both array and hash table
Directory *dir_create(void) {
    Directory *dir = calloc(1, sizeof(Directory));
    if (!dir) return NULL;
    dir->capacity = 8;  // initial capacity
    dir->contacts = malloc(dir->capacity * sizeof(Contact*));
    if (!dir->contacts) { free(dir); return NULL; }
    return dir;
}

// Ensure array has space for one more contact
static int ensure_capacity(Directory *dir) {
    if (dir->size >= dir->capacity) {
        size_t new_cap = dir->capacity * 2;
        Contact **new_arr = realloc(dir->contacts, new_cap * sizeof(Contact*));
        if (!new_arr) return 0;
        dir->contacts = new_arr;
        dir->capacity = new_cap;
    }
    return 1;
}

// Insert contact into directory (takes ownership of contact)
int dir_insert(Directory *dir, Contact *c) {
    // First add to hash table
    size_t idx = bucket_idx(c->id);
    c->next = dir->buckets[idx];
    dir->buckets[idx] = c;
    
    // Then add to array
    if (!ensure_capacity(dir)) {
        // Roll back hash table insert
        dir->buckets[idx] = c->next;
        return 0;
    }
    dir->contacts[dir->size++] = c;
    return 1;
}

// Find contact by ID (O(1) using hash table)
Contact *dir_find(Directory *dir, int id) {
    size_t idx = bucket_idx(id);
    for (Contact *c = dir->buckets[idx]; c; c = c->next) {
        if (c->id == id) return c;
    }
    return NULL;
}

// Helper: remove contact from array at given index
static void remove_from_array(Directory *dir, size_t arr_idx) {
    memmove(&dir->contacts[arr_idx], 
            &dir->contacts[arr_idx + 1], 
            (dir->size - arr_idx - 1) * sizeof(Contact*));
    dir->size--;
}

// Remove contact by ID (returns removed contact if found)
Contact *dir_remove(Directory *dir, int id) {
    // First remove from hash table
    size_t idx = bucket_idx(id);
    Contact *prev = NULL;
    for (Contact *c = dir->buckets[idx]; c; prev = c, c = c->next) {
        if (c->id == id) {
            // Remove from hash chain
            if (prev) prev->next = c->next;
            else dir->buckets[idx] = c->next;
            c->next = NULL;
            
            // Remove from array
            for (size_t i = 0; i < dir->size; i++) {
                if (dir->contacts[i] == c) {
                    remove_from_array(dir, i);
                    break;
                }
            }
            return c;
        }
    }
    return NULL;
}

// Free directory and all contacts
void dir_free(Directory *dir) {
    if (!dir) return;
    // Free all contacts (only once, from array)
    for (size_t i = 0; i < dir->size; i++) {
        free(dir->contacts[i]);
    }
    free(dir->contacts);
    free(dir);
}

// Helper: create new contact struct
static Contact *contact_new(void) {
    return calloc(1, sizeof(Contact));
}

// Load contacts into directory (both array and hash table)
Directory *load_contacts(void) {
    FILE *f = fopen(CSV_FILE, "r");
    Directory *dir = dir_create();
    if (!f) {
        return dir;
    }
    
    char line[LINE_BUF];
    // skip header if present
    if (!fgets(line, sizeof(line), f)) { fclose(f); return dir; }
    // check if header starts with id or digit
    if (strncmp(line, "id,", 3) != 0 && !(line[0] >= '0' && line[0] <= '9')) {
        // assume header present, read next
    } else {
        // the first line might be data; if it begins with digit treat as data
        if (line[0] >= '0' && line[0] <= '9') {
            // process first line as data by rewinding
            fseek(f, 0, SEEK_SET);
        }
    }
    while (fgets(line, sizeof(line), f)) {
        // trim newline
        char *nl = strchr(line, '\n'); if (nl) *nl = '\0';
        Contact *c = contact_new();
        // parse CSV line supporting quoted fields
        char *p = line;
        char *fields[5] = {0};
        int fi = 0;
        while (*p && fi < 5) {
            if (*p == '"') {
                // quoted field
                p++; // skip opening quote
                char *bufout = malloc(strlen(p) + 1);
                char *o = bufout;
                while (*p) {
                    if (*p == '"') {
                        if (p[1] == '"') { // escaped quote
                            *o++ = '"'; p += 2; continue;
                        } else { p++; break; }
                    }
                    *o++ = *p++;
                }
                *o = '\0';
                fields[fi++] = bufout;
                if (*p == ',') p++;
            } else {
                // unquoted field
                char *start = p;
                while (*p && *p != ',') p++;
                int len = p - start;
                char *bufout = malloc(len + 1);
                strncpy(bufout, start, len);
                bufout[len] = '\0';
                fields[fi++] = bufout;
                if (*p == ',') p++;
            }
        }
        if (fi >= 1 && fields[0]) c->id = atoi(fields[0]);
        if (fi >= 2 && fields[1]) strncpy(c->name, fields[1], sizeof(c->name)-1);
        if (fi >= 3 && fields[2]) strncpy(c->phone, fields[2], sizeof(c->phone)-1);
        if (fi >= 4 && fields[3]) strncpy(c->email, fields[3], sizeof(c->email)-1);
        if (fi >= 5 && fields[4]) strncpy(c->address, fields[4], sizeof(c->address)-1);
        for (int k = 0; k < fi; ++k) free(fields[k]);

    dir_insert(dir, c);
    }
    fclose(f);
    return dir;
}

int next_id(Directory *dir) {
    int mx = 0;
    for (size_t i = 0; i < dir->size; i++) {
        if (dir->contacts[i]->id > mx) mx = dir->contacts[i]->id;
    }
    return mx + 1;
}

void save_all(Directory *dir) {
    FILE *f = fopen(CSV_FILE, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "id,name,phone,email,address\n");
    // Write from array to maintain order
    for (size_t i = 0; i < dir->size; i++) {
        Contact *c = dir->contacts[i];
        // write id then 4 fields with CSV escaping
        fprintf(f, "%d,", c->id);
            char *fields[4] = { (char*)c->name, (char*)c->phone, (char*)c->email, (char*)c->address };
        for (int j = 0; j < 4; ++j) {
            char *s = fields[j];
            int need_quote = 0;
            for (char *t = s; *t; ++t) if (*t == ',' || *t == '"' || *t == '\n') { need_quote = 1; break; }
            if (!need_quote) {
                fprintf(f, "%s", s);
            } else {
                fputc('"', f);
                for (char *t = s; *t; ++t) {
                    if (*t == '"') fputc('"', f);
                    fputc(*t, f);
                }
                fputc('"', f);
            }
            if (j < 3) fputc(',', f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

void add_contact(void) {
    Directory *dir = load_contacts();
    Contact *c = contact_new();
    char buf[512];
    
    printf("\n  Add New Contact");
    printf("\n  --------------\n");
    printf("  Name:    "); if (!fgets(buf, sizeof(buf), stdin)) { free(c); dir_free(dir); return; } 
    buf[strcspn(buf, "\n")] = '\0'; strncpy(c->name, buf, sizeof(c->name)-1);
    printf("  Phone:   "); if (!fgets(buf, sizeof(buf), stdin)) { free(c); dir_free(dir); return; }
    buf[strcspn(buf, "\n")] = '\0'; strncpy(c->phone, buf, sizeof(c->phone)-1);
    printf("  Email:   "); if (!fgets(buf, sizeof(buf), stdin)) { free(c); dir_free(dir); return; }
    buf[strcspn(buf, "\n")] = '\0'; strncpy(c->email, buf, sizeof(c->email)-1);
    printf("  Address: "); if (!fgets(buf, sizeof(buf), stdin)) { free(c); dir_free(dir); return; }
    buf[strcspn(buf, "\n")] = '\0'; strncpy(c->address, buf, sizeof(c->address)-1);

    c->id = next_id(dir);
    if (!dir_insert(dir, c)) {
        printf("Error: Failed to add contact\n");
        free(c);
        dir_free(dir);
        return;
    }
    save_all(dir);
    int saved_id = c->id;
    dir_free(dir);
    printf("Contact added with id %d.\n", saved_id);
}

void list_contacts(void) {
    Directory *dir = load_contacts();
    if (dir->size == 0) { 
        printf("\n  No contacts found in directory.\n\n"); 
        dir_free(dir); 
        return; 
    }
    printf("\n  Contact List");
    printf("\n  ------------\n\n");
    // Print from array to maintain order
    for (size_t i = 0; i < dir->size; i++) {
        Contact *c = dir->contacts[i];
        printf("  Contact #%d\n", c->id);
        printf("  - Name:    %s\n", c->name);
        printf("  - Phone:   %s\n", c->phone);
        printf("  - Email:   %s\n", c->email);
        printf("  - Address: %s\n", c->address);
        printf("  -----------------------------------\n");
    }
    printf("\n  Total Contacts: %zu\n\n", dir->size);
    dir_free(dir);
}

void search_contacts(void) {
    printf("\n  Search Contacts");
    printf("\n  --------------\n");
    printf("  Enter name or phone: ");
    char q[256]; if (!fgets(q, sizeof(q), stdin)) return; q[strcspn(q, "\n")] = '\0';
    if (strlen(q) == 0) { printf("\n  Error: Search query cannot be empty.\n\n"); return; }
    
    Directory *dir = load_contacts();
    if (dir->size == 0) { printf("\n  No contacts in directory.\n\n"); dir_free(dir); return; }
    
    printf("\n  Search Results for '%s'\n", q);
    printf("  -----------------------------------\n\n");
    
    int found = 0;
    for (size_t i = 0; i < dir->size; i++) {
        Contact *c = dir->contacts[i];
        if (strstr(c->name, q) || strstr(c->phone, q)) {
            printf("  Contact #%d\n", c->id);
            printf("  - Name:    %s\n", c->name);
            printf("  - Phone:   %s\n", c->phone);
            printf("  - Email:   %s\n", c->email);
            printf("  - Address: %s\n", c->address);
            printf("  -----------------------------------\n");
            found = 1;
        }
    }
    if (!found) printf("  No matches found.\n");
    printf("\n");
    dir_free(dir);
}

void delete_contact(void) {
    printf("\n  Delete Contact");
    printf("\n  --------------\n");
    printf("  Enter contact ID to delete: ");
    char buf[64]; if (!fgets(buf, sizeof(buf), stdin)) return; int id = atoi(buf);
    if (id <= 0) { printf("\n  Error: Invalid contact ID.\n\n"); return; }
    
    Directory *dir = load_contacts();
    if (dir->size == 0) { printf("\n  No contacts in directory.\n\n"); dir_free(dir); return; }
    
    Contact *found = dir_remove(dir, id);
    if (!found) { 
        printf("\n  Error: Contact #%d not found.\n\n", id); 
        dir_free(dir); 
        return; 
    }
    save_all(dir);
    free(found);
    dir_free(dir);
    printf("\n  Success: Contact #%d has been deleted.\n\n", id);
}

// Helper to prompt with default: if user enters empty line, keep default
static void prompt_with_default(const char *label, char *out, size_t outsz, const char *def) {
    char buf[512];
    if (def && strlen(def) > 0) printf("%s [%s]: ", label, def); else printf("%s: ", label);
    if (!fgets(buf, sizeof(buf), stdin)) return;
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) == 0) {
        if (def) strncpy(out, def, outsz-1);
    } else {
        strncpy(out, buf, outsz-1);
    }
}

void update_contact(void) {
    printf("\n  Update Contact");
    printf("\n  --------------\n");
    printf("  Enter contact ID to update: ");
    char buf[64]; if (!fgets(buf, sizeof(buf), stdin)) return; int id = atoi(buf);
    if (id <= 0) { printf("\n  Error: Invalid contact ID.\n\n"); return; }
    
    Directory *dir = load_contacts();
    if (dir->size == 0) { printf("\n  No contacts in directory.\n\n"); dir_free(dir); return; }
    
    Contact *found = dir_find(dir, id);
    if (!found) { printf("\n  Error: Contact #%d not found.\n\n", id); dir_free(dir); return; }
    
    printf("\n  Editing Contact #%d", id);
    printf("\n  Press Enter to keep current values\n\n");
    
    printf("  Current name:    %s\n", found->name);
    printf("  New name:      "); 
    prompt_with_default("", found->name, sizeof(found->name), found->name);
    
    printf("\n  Current phone:   %s\n", found->phone);
    printf("  New phone:     ");
    prompt_with_default("", found->phone, sizeof(found->phone), found->phone);
    
    printf("\n  Current email:   %s\n", found->email);
    printf("  New email:     ");
    prompt_with_default("", found->email, sizeof(found->email), found->email);
    
    printf("\n  Current address: %s\n", found->address);
    printf("  New address:   ");
    prompt_with_default("", found->address, sizeof(found->address), found->address);
    
    save_all(dir);
    dir_free(dir);
    printf("\n  Success: Contact #%d has been updated.\n\n", id);
}

void print_menu(void) {
    printf("\n  MiniTel Directory Manager");
    printf("\n  ----------------------\n");
    printf("  1. Add New Contact\n");
    printf("  2. View All Contacts\n");
    printf("  3. Search Contacts\n");
    printf("  4. Delete Contact\n");
    printf("  5. Update Contact\n");
    printf("  0. Exit Application\n");
    printf("  ----------------------\n");
    printf("  Select option: ");
}

int main(void) {
    char choice[16];
    for (;;) {
        print_menu();
        if (!fgets(choice, sizeof(choice), stdin)) break;
        int c = atoi(choice);
        switch (c) {
            case 1: add_contact(); break;
            case 2: list_contacts(); break;
            case 3: search_contacts(); break;
            case 4: delete_contact(); break;
            case 5: update_contact(); break;
            case 0: printf("Goodbye.\n"); return 0;
            default: printf("Invalid option\n");
        }
    }
    return 0;
}
