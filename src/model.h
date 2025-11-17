// model.h - data and business logic (no printf/scanf!)

#ifndef MODEL_H
#define MODEL_H

#include <stddef.h>

#define MAX_FILES   100
#define MAX_LEN     100
#define RECENT_MAX  5
#define UNDO_MAX    50

typedef struct {
    char filename[MAX_LEN];
    char password[MAX_LEN];
} Vault;

typedef struct {
    char filename[MAX_LEN];
} RecentFile;

typedef struct {
    RecentFile data[RECENT_MAX];
    int front;
    int rear;
    int count;
} Queue;

typedef struct {
    char filename[MAX_LEN];
    int length;
} UndoOp;

/* Initialization */
void model_init(void);

/* Vault operations */
int  model_addFile(const char *filename, const char *password);
/* returns:
 *   0 = success
 *  -1 = vault full
 *  -2 = file already exists
 *  -3 = file create error
 */

int  model_changePassword(const char *filename,
                          const char *oldPwd,
                          const char *newPwd);
/* returns:
 *   0 = success
 *  -1 = file not found
 *  -2 = wrong old password
 */

int  model_verifyPassword(const char *filename, const char *password);
/* returns:
 *   1 = ok
 *   0 = wrong password
 *  -1 = file not found
 */

/* File content operations */
char *model_getFileContents(const char *filename);
/* returns malloc'd string or NULL (caller must free) */

int  model_appendToFile(const char *filename,
                        const char *text,
                        int *appendedLen);
/* returns:
 *   0 = success
 *  -1 = file open error
 *  -2 = nothing appended
 */

/* Recent files */
void model_recordRecent(const char *filename);

/* fill from most recent to oldest (up to maxCount).
 *   returns actual count. */
int  model_getRecent(char names[][MAX_LEN], int maxCount);

/* Undo */
int  model_undoLastAppend(char *outFilename, size_t bufSize);
/* returns:
 *   1 = undo done
 *   0 = nothing to undo
 *  -1 = error (file open/memory/etc)
 */

#endif // MODEL_H
