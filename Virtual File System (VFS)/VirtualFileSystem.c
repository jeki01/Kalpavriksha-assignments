#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 1024
#define NAME_LIMIT 50

// ---------- STRUCTURES ----------
typedef struct FreeBlock {
    int index;
    struct FreeBlock *prev, *next;
} FreeBlock;

typedef struct FileNode {
    char name[NAME_LIMIT];
    int isDir;
    struct FileNode *child, *next, *prev, *parent;
    int blockPointers[10], blockCount;
} FileNode;

// ---------- GLOBALS ----------
unsigned char disk[TOTAL_BLOCKS][BLOCK_SIZE];
FreeBlock *freeHead = NULL;
FileNode *root = NULL, *cwd = NULL;

// ---------- FUNCTION DECLARATIONS ----------
void initSystem();
void addFreeBlock(int i);
int allocBlock();
void freeBlock(int i);
void mkdirCmd(char name[]);
void createCmd(char name[]);
void writeCmd(char name[], char data[]);
void readCmd(char name[]);
void deleteCmd(char name[]);
void rmdirCmd(char name[]);
void lsCmd();
void cdCmd(char name[]);
void pwdCmd(FileNode *d);
void dfCmd();
void cleanup();

// ---------- MAIN ----------
int main() {
    initSystem();
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
    char cmd[50], arg1[50], arg2[512];

    while (1) {
        printf("%s > ", cwd->name);
        arg1[0] = arg2[0] = '\0';
        scanf("%s", cmd);

        if (!strcmp(cmd, "mkdir")) { scanf("%s", arg1); mkdirCmd(arg1); }
        else if (!strcmp(cmd, "create")) { scanf("%s", arg1); createCmd(arg1); }
        else if (!strcmp(cmd, "write")) { scanf("%s", arg1); fgets(arg2, sizeof(arg2), stdin); writeCmd(arg1, arg2); }
        else if (!strcmp(cmd, "read")) { scanf("%s", arg1); readCmd(arg1); }
        else if (!strcmp(cmd, "delete")) { scanf("%s", arg1); deleteCmd(arg1); }
        else if (!strcmp(cmd, "rmdir")) { scanf("%s", arg1); rmdirCmd(arg1); }
        else if (!strcmp(cmd, "ls")) lsCmd();
        else if (!strcmp(cmd, "cd")) { scanf("%s", arg1); cdCmd(arg1); }
        else if (!strcmp(cmd, "pwd")) { pwdCmd(cwd); printf("\n"); }
        else if (!strcmp(cmd, "df")) dfCmd();
        else if (!strcmp(cmd, "exit")) { cleanup(); printf("Memory released. Exiting program...\n"); break; }
        else printf("Invalid command.\n");
    }
    return 0;
}

// ---------- CORE FUNCTIONS ----------
void initSystem() {
    for (int i = 0; i < TOTAL_BLOCKS; i++) addFreeBlock(i);
    root = malloc(sizeof(FileNode));
    strcpy(root->name, "/");
    root->isDir = 1; root->child = NULL; root->parent = NULL;
    cwd = root;
}

void addFreeBlock(int i) {
    FreeBlock *b = malloc(sizeof(FreeBlock));
    b->index = i; b->prev = NULL; b->next = freeHead;
    if (freeHead) freeHead->prev = b;
    freeHead = b;
}

int allocBlock() {
    if (!freeHead) { printf("Disk full. Cannot allocate new block.\n"); return -1; }
    int i = freeHead->index;
    FreeBlock *t = freeHead;
    freeHead = freeHead->next;
    if (freeHead) freeHead->prev = NULL;
    free(t);
    return i;
}

void freeBlock(int i) { addFreeBlock(i); }

// ---------- DIRECTORY & FILE OPS ----------
void mkdirCmd(char name[]) {
    FileNode *n = malloc(sizeof(FileNode));
    strcpy(n->name, name); n->isDir = 1; n->child = NULL; n->parent = cwd;
    if (!cwd->child) n->next = n->prev = (cwd->child = n);
    else {
        FileNode *f = cwd->child, *l = f->prev;
        l->next = n; n->prev = l; n->next = f; f->prev = n;
    }
    printf("Directory '%s' created successfully.\n", name);
}

void createCmd(char name[]) {
    FileNode *n = malloc(sizeof(FileNode));
    strcpy(n->name, name); n->isDir = 0; n->blockCount = 0; n->parent = cwd;
    if (!cwd->child) n->next = n->prev = (cwd->child = n);
    else {
        FileNode *f = cwd->child, *l = f->prev;
        l->next = n; n->prev = l; n->next = f; f->prev = n;
    }
    printf("File '%s' created successfully.\n", name);
}

void writeCmd(char name[], char data[]) {
    FileNode *t = cwd->child;
    if (!t) { printf("No file found.\n"); return; }
    do {
        if (!t->isDir && !strcmp(t->name, name)) {
            int b = allocBlock(); if (b == -1) return;
            strcpy((char *)disk[b], data);
            t->blockPointers[0] = b; t->blockCount = 1;
            printf("Data written successfully (size=%zu bytes).\n", strlen(data));
            return;
        }
        t = t->next;
    } while (t != cwd->child);
    printf("File not found.\n");
}

void readCmd(char name[]) {
    FileNode *t = cwd->child;
    if (!t) { printf("(empty)\n"); return; }
    do {
        if (!t->isDir && !strcmp(t->name, name)) {
            if (!t->blockCount) { printf("File is empty.\n"); return; }
            printf("%s\n", disk[t->blockPointers[0]]);
            return;
        }
        t = t->next;
    } while (t != cwd->child);
    printf("File not found.\n");
}

void deleteCmd(char name[]) {
    FileNode *t = cwd->child;
    if (!t) { printf("(empty)\n"); return; }
    do {
        if (!t->isDir && !strcmp(t->name, name)) {
            for (int i = 0; i < t->blockCount; i++) freeBlock(t->blockPointers[i]);
            if (t->next == t) cwd->child = NULL;
            else {
                t->prev->next = t->next; t->next->prev = t->prev;
                if (cwd->child == t) cwd->child = t->next;
            }
            free(t);
            printf("File deleted successfully.\n");
            return;
        }
        t = t->next;
    } while (t != cwd->child);
    printf("File not found.\n");
}

void rmdirCmd(char name[]) {
    FileNode *t = cwd->child;
    if (!t) { printf("(empty)\n"); return; }
    do {
        if (t->isDir && !strcmp(t->name, name)) {
            if (t->child) { printf("Directory not empty.\n"); return; }
            if (t->next == t) cwd->child = NULL;
            else {
                t->prev->next = t->next; t->next->prev = t->prev;
                if (cwd->child == t) cwd->child = t->next;
            }
            free(t);
            printf("Directory removed successfully.\n");
            return;
        }
        t = t->next;
    } while (t != cwd->child);
    printf("Directory not found.\n");
}

// ---------- OTHER COMMANDS ----------
void lsCmd() {
    if (!cwd->child) { printf("(empty)\n"); return; }
    FileNode *t = cwd->child;
    do { printf("%s%s\n", t->name, t->isDir ? "/" : ""); t = t->next; }
    while (t != cwd->child);
}

void cdCmd(char name[]) {
    if (!strcmp(name, "..")) {
        if (cwd->parent) cwd = cwd->parent;
        printf("Moved to %s\n", cwd->name);
        return;
    }
    FileNode *t = cwd->child;
    if (!t) { printf("Directory not found.\n"); return; }
    do {
        if (t->isDir && !strcmp(t->name, name)) {
            cwd = t;
            printf("Moved to %s%s\n", cwd->name[0] == '/' ? "" : "/", cwd->name);
            return;
        }
        t = t->next;
    } while (t != cwd->child);
    printf("Directory not found.\n");
}

void pwdCmd(FileNode *d) {
    if (!d) return;
    if (d->parent) { pwdCmd(d->parent); printf("/%s", d->name); }
    else printf("/");
}

void dfCmd() {
    int freeCnt = 0;
    for (FreeBlock *t = freeHead; t; t = t->next) freeCnt++;
    printf("Total Blocks: %d\n", TOTAL_BLOCKS);
    printf("Used Blocks: %d\n", TOTAL_BLOCKS - freeCnt);
    printf("Free Blocks: %d\n", freeCnt);
    printf("Disk Usage: %.2f%%\n", 100.0 * (TOTAL_BLOCKS - freeCnt) / TOTAL_BLOCKS);
}

void cleanup() {
    while (freeHead) { FreeBlock *t = freeHead; freeHead = freeHead->next; free(t); }
}
