// view.c - console I/O, menu, prompts, password masking

#include "view.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static void clearStdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void view_showMainMenu(void) {
    printf("\n--- FILE VAULT MENU ---\n"
    "1. Add New File\n"
    "2. Access File\n"
    "3. Change Password\n"
    "4. Show Recent Files\n"
    "5. Undo Last Append\n"
    "6. Exit\n");
}

void view_showMessage(const char *msg) {
    printf("%s\n", msg);
}

void view_showError(const char *msg) {
    printf("ERROR: %s\n", msg);
}

int view_getInt(const char *prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            clearStdin();
            return value;
        } else {
            clearStdin();
            printf("Invalid number. Try again.\n");
        }
    }
}

void view_getString(const char *prompt, char *buf, size_t len) {
    printf("%s", prompt);
    if (fgets(buf, (int)len, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

void view_getPassword(const char *prompt, char *password, size_t len) {
    struct termios oldt, newt;
    size_t i = 0;
    int ch;

    printf("%s", prompt);
    fflush(stdout);

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == '\r') {
            break;
        } else if ((ch == 127 || ch == '\b')) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < len - 1 && ch >= 32 && ch <= 126) {
            password[i++] = (char)ch;
            printf("*");
            fflush(stdout);
        }
    }

    password[i] = '\0';
    printf("\n");
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

char *view_getMultilineText(void) {
    printf("Enter text to append (single dot \".\" on its own line to finish):\n");

    size_t capacity = 256;
    size_t length   = 0;
    char  *buffer   = (char *)malloc(capacity);
    if (!buffer) return NULL;

    buffer[0] = '\0';
    char line[256];

    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, ".\n") == 0 || strcmp(line, ".\r\n") == 0 ||
            strcmp(line, ".") == 0) {
            break;
            }

            size_t lineLen = strlen(line);

        if (length + lineLen + 1 > capacity) {
            capacity *= 2;
            char *newBuf = (char *)realloc(buffer, capacity);
            if (!newBuf) {
                free(buffer);
                return NULL;
            }
            buffer = newBuf;
        }

        memcpy(buffer + length, line, lineLen);
        length += lineLen;
        buffer[length] = '\0';
    }

    if (length == 0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

void view_showFileContent(const char *filename, const char *content) {
    printf("Contents of %s:\n", filename);
    if (content) {
        printf("%s", content);
        if (content[strlen(content) - 1] != '\n')
            printf("\n");
    } else {
        printf("(no content or error reading)\n");
    }
}

void view_showRecentFiles(char names[][MAX_LEN], int count) {
    printf("Recent files (1 = most recent):\n");
    for (int i = 0; i < count; i++) {
        printf("%d. %s\n", i + 1, names[i]);
    }
}

int view_chooseRecentFile(int count) {
    int choice = view_getInt("Enter a number to open that file (0 to cancel): ");
    if (choice < 0 || choice > count) {
        printf("Invalid choice.\n");
        return 0;
    }
    return choice;
}
