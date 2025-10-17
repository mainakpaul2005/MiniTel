// menu.c
#include <stdio.h>
#include <stdlib.h>

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printBanner() {
    printf("\033[1;36m");
    printf("╭────────────────────────────────────────────╮\n");
    printf("│       📞  Mini Telephone Directory  📇      │\n");
    printf("├────────────────────────────────────────────┤\n");
    printf("\033[0m");
}

void printFooter() {
    printf("\033[1;36m");
    printf("╰────────────────────────────────────────────╯\n");
    printf("\033[0m");
}

void printMenu() {
    clearScreen();
    printBanner();
    printf("\033[1;33m");
    printf("│  1️⃣  Add Contact                           │\n");
    printf("│  2️⃣  Search Contact                        │\n");
    printf("│  3️⃣  Delete Contact                        │\n");
    printf("│  4️⃣  Restore Deleted Contacts              │\n");
    printf("│  5️⃣  Show All Contacts                     │\n");
    printf("│  6️⃣  Export Snapshot                       │\n");
    printf("│  0️⃣  Exit                                  │\n");
    printf("\033[0m");
    printFooter();
    printf("\nEnter choice: ");
}
