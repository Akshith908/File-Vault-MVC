// main.c - Controller: connects View and Model
#include <stdlib.h>
#include "model.h"
#include "view.h"
#include <stdio.h>
#include <string.h>

static void controller_accessFile(const char *filename);

int main(void) {
    model_init();

    int choice;
    char filename[MAX_LEN];

    while (1) {
        view_showMainMenu();
        choice = view_getInt("Enter your choice: ");

        if (choice == 6) {
            view_showMessage("Exiting...");
            break;
        }

        switch (choice) {
            case 1: {
                char password[MAX_LEN];
                view_getString("Enter filename: ", filename,MAX_LEN);
                view_getString("Set a password for this file: ", password,MAX_LEN);

                int res = model_addFile(filename, password);
                if (res == 0) {
                    view_showMessage("File added and protected successfully.");
                } else if (res == -1) {
                    view_showError("Vault is full!");
                } else if (res == -2) {
                    view_showError("File already exists!");
                } else {
                    view_showError("Failed to create file.");
                }
                break;
            }

            case 2:
                view_getString("Enter filename: ", filename,MAX_LEN);
                controller_accessFile(filename);
                break;

            case 3: {
                char oldPwd[MAX_LEN], newPwd[MAX_LEN];

                view_getString("Enter filename: ", filename,MAX_LEN);
                view_getPassword("Enter current password: ", oldPwd, MAX_LEN);
                view_getString("Enter new password: ", newPwd,MAX_LEN);

                int res = model_changePassword(filename, oldPwd, newPwd);
                if (res == 0) {
                    view_showMessage("Password changed successfully.");
                } else if (res == -1) {
                    view_showError("File not found!");
                } else if (res == -2) {
                    view_showError("Incorrect password!");
                }
                break;
            }

            case 4: {
                char names[RECENT_MAX][MAX_LEN];
                int count = model_getRecent(names, RECENT_MAX);
                if (count == 0) {
                    view_showMessage("No recent files.");
                } else {
                    view_showRecentFiles(names, count);
                    int c = view_chooseRecentFile(count);
                    if (c > 0) {
                        controller_accessFile(names[c - 1]);
                    }
                }
                break;
            }

            case 5: {
                char undoneFile[MAX_LEN];
                int res = model_undoLastAppend(undoneFile, sizeof(undoneFile));
                if (res == 1) {
                    char msg[200];
                    snprintf(msg, sizeof(msg),
                             "Undo complete for file: %s", undoneFile);
                    view_showMessage(msg);
                } else if (res == 0) {
                    view_showMessage("Nothing to undo.");
                } else {
                    view_showError("Undo failed due to file or memory error.");
                }
                break;
            }

            default:
                view_showError("Invalid choice!");
                break;
        }
    }

    return 0;
}

/* ---------- controller helper ---------- */

static void controller_accessFile(const char *filename) {
    char pwd[MAX_LEN];

    int verifyPre = model_verifyPassword(filename, ""); // check existence only
    if (verifyPre == -1) {
        view_showError("File not found!");
        return;
    }

    view_getPassword("Enter password: ", pwd, MAX_LEN);
    int v = model_verifyPassword(filename, pwd);
    if (v != 1) {
        view_showError("Incorrect password!");
        return;
    }

    model_recordRecent(filename);

    printf("1. View file\n2. Append to file\n");
    int choice = view_getInt("Enter your choice: ");

    if (choice == 1) {
        char *content = model_getFileContents(filename);
        view_showFileContent(filename, content);
        free(content);
    } else if (choice == 2) {
        char *text = view_getMultilineText();
        if (!text) {
            view_showMessage("No content to append.");
            return;
        }

        int appendedLen = 0;
        int res = model_appendToFile(filename, text, &appendedLen);
        free(text);

        if (res == 0 && appendedLen > 0) {
            view_showMessage("Content appended successfully.");
        } else if (res == -1) {
            view_showError("Failed to open or write to file.");
        } else {
            view_showMessage("Nothing was appended.");
        }
    } else {
        view_showError("Invalid choice.");
    }
}
