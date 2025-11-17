// view.h - all user interaction (I/O only, no business logic)

#ifndef VIEW_H
#define VIEW_H

#include "model.h"  // for MAX_LEN

void view_showMainMenu(void);
void view_showMessage(const char *msg);
void view_showError(const char *msg);

/* Reads an integer from user, with a prompt. */
int  view_getInt(const char *prompt);

/* Reads a line of text (no newline at end). */
void view_getString(const char *prompt, char *buf, size_t len);

/* Reads a password without echoing characters. */
void view_getPassword(const char *prompt, char *buf, size_t len);

/* Multiline input until single '.' line; returns malloc'd text or NULL. */
char *view_getMultilineText(void);

/* Shows contents of a file. */
void view_showFileContent(const char *filename, const char *content);

/* Recent files display & choice */
void view_showRecentFiles(char names[][MAX_LEN], int count);
int  view_chooseRecentFile(int count);

#endif // VIEW_H
