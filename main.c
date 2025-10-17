#include <stdio.h>
#include <stdlib.h>
#include "contact.h"
#include "hashmap.h"

// Functions implemented in other translation units
void loadContacts(void);
void addContact(void);
void searchContact(void);
void deleteContact(void);
void restoreContacts(void);
void showAllContacts(void);
void exportSnapshot(void);
void printMenu(void);
void clearBuffer(void);

// Helper function to clear input buffer
void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(void) {
    loadContacts();
    while (1) {
        printMenu();
        int choice;
        if (scanf("%d", &choice) != 1) {
            clearBuffer();
            printf("\033[1;31mInvalid input! Please enter a number.\033[0m\n");
            system("pause");
            continue;
        }
        clearBuffer();

        switch (choice) {
            case 1: addContact(); break;
            case 2: searchContact(); break;
            case 3: deleteContact(); break;
            case 4: restoreContacts(); break;
            case 5: showAllContacts(); break;
            case 6: exportSnapshot(); break;
            case 0: 
                printf("\033[1;32mThank you for using MiniTel! Goodbye!\033[0m\n");
                return 0;
            default: 
                printf("\033[1;31mInvalid choice! Please try again.\033[0m\n");
        }
        printf("\nPress Enter to continue...");
        getchar();
    }
    return 0;
}
