// menu.c - improved ASCII TUI with dynamic centering and box
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// external flag set by main
extern int g_fancy_mode;
extern char g_theme[32];

static void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static int getTerminalWidth(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return 80;
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return width > 0 ? width : 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return 80;
    return w.ws_col ? w.ws_col : 80;
#endif
}

static void repeatChar(char ch, int n) {
    for (int i = 0; i < n; ++i) putchar(ch);
}

static void repeatStr(const char *s, int n) {
    for (int i = 0; i < n; ++i) fputs(s, stdout);
}

void printMenu(void) {
    clearScreen();

    const char *title = "Mini Telephone Directory (MiniTel)";
    int termW = getTerminalWidth();

    int contentW = termW - 6; // leave margins
    if (contentW < 40) contentW = 40;
    if (contentW > 100) contentW = 100;

    int leftMargin = (termW - contentW) / 2;
    if (leftMargin < 0) leftMargin = 0;

    // Theme colors (ANSI codes) - defaults
    const char *titleColor = "\033[1;36m";    // cyan
    const char *borderColor = "\033[1;34m";   // blue
    const char *optionColor = "\033[1;33m";   // yellow

    // Select theme overrides
    if (strcmp(g_theme, "dracula") == 0) {
        titleColor = "\033[95m";   // bright magenta
        borderColor = "\033[94m";  // bright blue
        optionColor = "\033[96m";  // bright cyan
    } else if (strcmp(g_theme, "solarized") == 0) {
        titleColor = "\033[1;36m"; // cyan-ish
        borderColor = "\033[38;5;244m"; // base02 (gray)
        optionColor = "\033[38;5;136m"; // orange
    } else if (strcmp(g_theme, "gruvbox") == 0) {
        titleColor = "\033[38;5;214m"; // apricot
        borderColor = "\033[38;5;138m"; // brown
        optionColor = "\033[38;5;108m"; // green
    } else if (strcmp(g_theme, "mono") == 0) {
        titleColor = "\033[1;37m"; // bright white
        borderColor = "\033[37m"; // white
        optionColor = "\033[37m"; // white
    }

    if (g_fancy_mode) {
        // Fancy Unicode box and colors
    for (int i = 0; i < leftMargin; ++i) putchar(' ');
    printf("%s┌", borderColor); repeatStr("─", contentW); printf("┐\033[0m\n");

        int titlePad = (contentW - (int)strlen(title)) / 2;
        if (titlePad < 0) titlePad = 0;
        for (int i = 0; i < leftMargin; ++i) putchar(' ');
    printf("%s│%*s%*s│\033[0m\n", titleColor, titlePad, "", (contentW - titlePad), title);

    for (int i = 0; i < leftMargin; ++i) putchar(' ');
    printf("%s├", borderColor); repeatStr("─", contentW); printf("┤\033[0m\n");

        const char *leftCol[] = {"1. Add Contact", "2. Search Contact", "3. Delete Contact"};
        const char *rightCol[] = {"4. Restore Deleted Contacts", "5. Show All Contacts", "6. Export Snapshot"};
        int rows = 3;
        int colGap = 4;
        int colWidth = (contentW - colGap) / 2;

        for (int r = 0; r < rows; ++r) {
            for (int i = 0; i < leftMargin; ++i) putchar(' ');
            // left border
            for (int i = 0; i < leftMargin; ++i) putchar(' ');
            printf("%s│ ", borderColor);
            // left option
            printf("%s%-*s\033[0m", optionColor, colWidth - 1, leftCol[r]);
            repeatChar(' ', colGap);
            // right option
            printf("%s%-*s\033[0m", optionColor, colWidth - 1, rightCol[r]);
            // right border
            printf(" %s│\033[0m\n", borderColor);
        }

    for (int i = 0; i < leftMargin; ++i) putchar(' ');
    printf("%s└", borderColor); repeatStr("─", contentW); printf("┘\033[0m\n");

        putchar('\n'); printf("Enter choice: ");
    } else {
    // ASCII-safe box
    for (int i = 0; i < leftMargin; ++i) putchar(' ');
    // draw top border
    for (int i = 0; i < leftMargin; ++i) putchar(' ');
    putchar('+'); repeatChar('-', contentW); putchar('+'); putchar('\n');

        int titlePad = (contentW - (int)strlen(title)) / 2;
        if (titlePad < 0) titlePad = 0;
        for (int i = 0; i < leftMargin; ++i) putchar(' ');
        printf("|%*s%*s|\n", titlePad, "", (contentW - titlePad), title);

        for (int i = 0; i < leftMargin; ++i) putchar(' ');
        putchar('|'); repeatChar('-', contentW); putchar('|'); putchar('\n');

        const char *leftCol[] = {"1. Add Contact", "2. Search Contact", "3. Delete Contact"};
        const char *rightCol[] = {"4. Restore Deleted Contacts", "5. Show All Contacts", "6. Export Snapshot"};
        int rows = 3;
        int colGap = 4;
        int colWidth = (contentW - colGap) / 2;

        for (int r = 0; r < rows; ++r) {
            for (int i = 0; i < leftMargin; ++i) putchar(' ');
            putchar('|'); putchar(' ');
            printf("%s%-*s\033[0m", optionColor, colWidth - 1, leftCol[r]);
            repeatChar(' ', colGap);
            printf("%s%-*s\033[0m", optionColor, colWidth - 1, rightCol[r]);
            putchar(' '); putchar('|'); putchar('\n');
        }

        for (int i = 0; i < leftMargin; ++i) putchar(' ');
        putchar('|'); putchar(' ');
        printf("%s", "0. Exit");
        repeatChar(' ', contentW - (int)strlen("0. Exit") - 1);
        putchar(' '); putchar('|'); putchar('\n');

        for (int i = 0; i < leftMargin; ++i) putchar(' ');
        putchar('+'); repeatChar('-', contentW); putchar('+'); putchar('\n');

        putchar('\n'); printf("Enter choice: ");
    }
}



