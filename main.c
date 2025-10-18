#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contact.h"
#include "hashmap.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

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

// Helper function to clear input buffer (kept for compatibility)
void clearBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Global flag to control fancy/unicode mode. Default: 0 (ASCII-safe)
int g_fancy_mode = 0;
// Theme name (default "modern")
char g_theme[32] = "modern";

// Enable ANSI escape sequences on Windows terminals (if supported)
static void enableAnsi(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    // If fancy mode is enabled, switch the console to UTF-8 so Unicode box chars and emoji print correctly
    if (g_fancy_mode) {
        // 65001 is the UTF-8 code page on Windows
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    }
#endif
}

// Cross-platform single-key input helper (file-scope)
static int get_single_key(void) {
#ifdef _WIN32
    int c = _getch();
    if (c == 0 || c == 224) {
        int c2 = _getch(); (void)c2;
        return -1;
    }
    return c;
#else
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) return -1;
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
#endif
}


int main(int argc, char **argv) {
    // check for a --fancy flag
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--fancy") == 0) {
            g_fancy_mode = 1;
        } else if (strcmp(argv[i], "--theme") == 0 && i + 1 < argc) {
            strncpy(g_theme, argv[i+1], sizeof(g_theme)-1);
            g_theme[sizeof(g_theme)-1] = '\0';
            i++; // skip value
        }
    }

    enableAnsi();
    loadContacts();

    // Cross-platform single-key input helper
    int get_single_key(void) {
#ifdef _WIN32
        int c = _getch();
        if (c == 0 || c == 224) {
            // special key prefix: consume the next byte and ignore
            int c2 = _getch(); (void)c2;
            return -1;
        }
        return c;
#else
        struct termios oldt, newt;
        if (tcgetattr(STDIN_FILENO, &oldt) != 0) return -1;
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int c = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return c;
#endif
    }

    while (1) {
        printMenu();
        int choice = -1;
        int ch = get_single_key();
        if (ch == -1 || ch == EOF) {
            // fallback to line input if single-key failed
            char buf[32];
            if (!fgets(buf, sizeof(buf), stdin)) continue;
            // trim
            char *p = buf + strlen(buf) - 1;
            if (p >= buf && *p == '\n') *p = '\0';
            if (strlen(buf) == 0) continue;
            choice = atoi(buf);
        } else {
            // Echo the pressed key so user sees it
            if (ch == '\r') ch = '\n';
            if (ch != '\n') putchar((char)ch);
            putchar('\n');
            if (ch >= '0' && ch <= '9') choice = ch - '0';
            else if (ch == 'q' || ch == 'Q') choice = 0;
            else {
                // ignore other keys
                continue;
            }
        }

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
        char buf2[32];
        fgets(buf2, sizeof(buf2), stdin);
    }
    return 0;
}
