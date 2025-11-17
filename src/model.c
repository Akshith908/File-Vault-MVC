// model.c - implements data, persistence, recent queue, and undo logic

#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- internal data ---------- */

static Vault vaults[MAX_FILES];
static int   file_count = 0;

static Queue recentQ;
static Queue tempQ;

static UndoOp undoStack[UNDO_MAX];
static int    undoTop = -1;

/* ---------- helper: queue ---------- */

static void initQueue(Queue *q) {
    q->front = 0;
    q->rear  = -1;
    q->count = 0;
}

static int isEmpty(Queue *q) {
    return q->count == 0;
}

static int isFull(Queue *q) {
    return q->count == RECENT_MAX;
}

static int enqueue(Queue *q, const char *filename) {
    if (isFull(q)) return 0;
    q->rear = (q->rear + 1) % RECENT_MAX;
    strncpy(q->data[q->rear].filename, filename, MAX_LEN - 1);
    q->data[q->rear].filename[MAX_LEN - 1] = '\0';
    q->count++;
    return 1;
}

static int dequeue(Queue *q, char *outFilename) {
    if (isEmpty(q)) return 0;
    strncpy(outFilename, q->data[q->front].filename, MAX_LEN - 1);
    outFilename[MAX_LEN - 1] = '\0';
    q->front = (q->front + 1) % RECENT_MAX;
    q->count--;
    return 1;
}

/* ---------- helper: undo stack ---------- */

static int pushUndo(const char *filename, int length) {
    if (undoTop >= UNDO_MAX - 1) {
        return 0;
    }
    undoTop++;
    strncpy(undoStack[undoTop].filename, filename, MAX_LEN - 1);
    undoStack[undoTop].filename[MAX_LEN - 1] = '\0';
    undoStack[undoTop].length = length;
    return 1;
}

static int popUndo(UndoOp *op) {
    if (undoTop < 0) return 0;
    *op = undoStack[undoTop];
    undoTop--;
    return 1;
}

/* ---------- helper: vault persistence ---------- */

static void loadVault(void) {
    FILE *fp = fopen("vault.txt", "r");
    file_count = 0;
    if (fp) {
        while (file_count < MAX_FILES &&
            fscanf(fp, "%99s %99s",
                   vaults[file_count].filename,
                   vaults[file_count].password) == 2) {
            file_count++;
                   }
                   fclose(fp);
    }
}

static void saveVault(void) {
    FILE *fp = fopen("vault.txt", "w");
    if (!fp) return;
    for (int i = 0; i < file_count; i++) {
        fprintf(fp, "%s %s\n",
                vaults[i].filename,
                vaults[i].password);
    }
    fclose(fp);
}

static int findFileIndex(const char *filename) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(vaults[i].filename, filename) == 0)
            return i;
    }
    return -1;
}

/* ---------- public: init ---------- */

void model_init(void) {
    loadVault();
    initQueue(&recentQ);
    initQueue(&tempQ);
    undoTop = -1;
}

/* ---------- public: vault operations ---------- */

int model_addFile(const char *filename, const char *password) {
    if (file_count >= MAX_FILES)
        return -1; /* vault full */

        if (findFileIndex(filename) != -1)
            return -2; /* file already exists */

            FILE *fp = fopen(filename, "w");
        if (!fp) {
            return -3; /* file create error */
        }
        fclose(fp);

    strncpy(vaults[file_count].filename, filename, MAX_LEN - 1);
    vaults[file_count].filename[MAX_LEN - 1] = '\0';
    strncpy(vaults[file_count].password, password, MAX_LEN - 1);
    vaults[file_count].password[MAX_LEN - 1] = '\0';
    file_count++;

    saveVault();
    return 0;
}

int model_verifyPassword(const char *filename, const char *password) {
    int idx = findFileIndex(filename);
    if (idx < 0)
        return -1; /* file not found */
        if (strcmp(vaults[idx].password, password) == 0)
            return 1;
    return 0;
}

int model_changePassword(const char *filename,
                         const char *oldPwd,
                         const char *newPwd) {
    int idx = findFileIndex(filename);
    if (idx < 0)
        return -1; /* file not found */

        if (strcmp(vaults[idx].password, oldPwd) != 0)
            return -2; /* wrong current password */

            strncpy(vaults[idx].password, newPwd, MAX_LEN - 1);
        vaults[idx].password[MAX_LEN - 1] = '\0';
    saveVault();
    return 0;
                         }

                         /* ---------- public: file content ---------- */

                         char *model_getFileContents(const char *filename) {
                             FILE *fp = fopen(filename, "rb");
                             if (!fp) return NULL;

                             /* get file size */
                             if (fseek(fp, 0, SEEK_END) != 0) {
                                 fclose(fp);
                                 return NULL;
                             }
                             long size = ftell(fp);
                             if (size < 0) size = 0;
                             rewind(fp);

                             char *buffer = (char *)malloc((size_t)size + 1);
                             if (!buffer) {
                                 fclose(fp);
                                 return NULL;
                             }

                             size_t readBytes = fread(buffer, 1, (size_t)size, fp);
                             buffer[readBytes] = '\0';
                             fclose(fp);

                             return buffer;
                         }

                         int model_appendToFile(const char *filename,
                                                const char *text,
                                                int *appendedLen) {
                             *appendedLen = 0;
                             if (!text || text[0] == '\0')
                                 return -2; /* nothing appended */

                                 FILE *fp = fopen(filename, "a");
                             if (!fp) {
                                 return -1; /* open error */
                             }

                             int len = (int)strlen(text);
                             size_t written = fwrite(text, 1, (size_t)len, fp);
                             fclose(fp);

                             if ((int)written != len) {
                                 return -1; /* write error */
                             }

                             *appendedLen = len;
                             pushUndo(filename, len);
                             return 0;
                                                }

                                                /* ---------- public: recent files ---------- */

                                                void model_recordRecent(const char *filename) {
                                                    initQueue(&tempQ);
                                                    char current[MAX_LEN];

                                                    while (!isEmpty(&recentQ)) {
                                                        dequeue(&recentQ, current);
                                                        if (strcmp(current, filename) != 0) {
                                                            enqueue(&tempQ, current);
                                                        }
                                                    }

                                                    if (isFull(&tempQ)) {
                                                        dequeue(&tempQ, current);
                                                    }

                                                    enqueue(&tempQ, filename);
                                                    recentQ = tempQ;
                                                }

                                                int model_getRecent(char names[][MAX_LEN], int maxCount) {
                                                    if (maxCount > RECENT_MAX)
                                                        maxCount = RECENT_MAX;

                                                    int count = recentQ.count;
                                                    if (count > maxCount) count = maxCount;

                                                    int idx = recentQ.rear;
                                                    for (int i = 0; i < count; i++) {
                                                        strncpy(names[i], recentQ.data[idx].filename, MAX_LEN - 1);
                                                        names[i][MAX_LEN - 1] = '\0';
                                                        idx = (idx - 1 + RECENT_MAX) % RECENT_MAX;
                                                    }
                                                    return count;
                                                }

                                                /* ---------- public: undo ---------- */

                                                int model_undoLastAppend(char *outFilename, size_t bufSize) {
                                                    UndoOp op;
                                                    if (!popUndo(&op)) {
                                                        return 0; /* nothing to undo */
                                                    }

                                                    FILE *fp = fopen(op.filename, "rb");
                                                    if (!fp) {
                                                        return -1;
                                                    }

                                                    if (fseek(fp, 0, SEEK_END) != 0) {
                                                        fclose(fp);
                                                        return -1;
                                                    }
                                                    long size = ftell(fp);
                                                    if (size < 0) size = 0;
                                                    long newSize = size - op.length;
                                                    if (newSize < 0) newSize = 0;
                                                    rewind(fp);

                                                    char *buffer = (char *)malloc((size_t)newSize + 1);
                                                    if (!buffer) {
                                                        fclose(fp);
                                                        return -1;
                                                    }

                                                    size_t readBytes = fread(buffer, 1, (size_t)newSize, fp);
                                                    buffer[readBytes] = '\0';
                                                    fclose(fp);

                                                    fp = fopen(op.filename, "wb");
                                                    if (!fp) {
                                                        free(buffer);
                                                        return -1;
                                                    }

                                                    fwrite(buffer, 1, (size_t)newSize, fp);
                                                    fclose(fp);
                                                    free(buffer);

                                                    if (outFilename && bufSize > 0) {
                                                        strncpy(outFilename, op.filename, bufSize - 1);
                                                        outFilename[bufSize - 1] = '\0';
                                                    }

                                                    return 1; /* undo complete */
                                                }
