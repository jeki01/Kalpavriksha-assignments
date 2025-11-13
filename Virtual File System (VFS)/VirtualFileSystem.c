#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 1024
#define NAME_LIMIT 50    /* max name length (excluding NUL) */
#define LINE_BUF 8192

/* Free block (doubly linked list) */
typedef struct FreeBlock {
    int index;
    struct FreeBlock *prev, *next;
} FreeBlock;

/* File / Directory node (circular doubly linked siblings) */
typedef struct FileNode {
    char name[NAME_LIMIT + 1];
    bool isDir;
    struct FileNode *child;   /* head of circular sibling list of children */
    struct FileNode *next;    /* sibling next */
    struct FileNode *prev;    /* sibling prev */
    struct FileNode *parent;
    int *blockPointers;       /* dynamic array for file blocks (NULL for dirs) */
    int blockCount;
    int blockCap;
} FileNode;

/* File system container */
typedef struct FileSystem {
    unsigned char disk[TOTAL_BLOCKS][BLOCK_SIZE];
    FreeBlock *freeHead;
    FreeBlock *freeTail;  /* tail for reinsertion at tail */
    int freeCount;

    FileNode *root;
    FileNode *cwd;
} FileSystem;

static FileSystem fs;

/* ---------- Helpers (short) ---------- */

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static void safe_strcpy(char *dst, const char *src) {
    strncpy(dst, src, NAME_LIMIT);
    dst[NAME_LIMIT] = '\0';
}

static bool valid_name(const char *name) {
    if (!name || name[0] == '\0' || strlen(name) > NAME_LIMIT) return false;
    for (size_t i = 0; i < strlen(name); ++i)
        if (name[i] == '/' || (unsigned char)name[i] < 32) return false;
    return true;
}

/* ---------- Free block list ---------- */

static void addFreeBlockTail(int idx) {
    FreeBlock *b = malloc(sizeof(FreeBlock));
    if (!b) die("malloc failed");
    b->index = idx;
    b->next = NULL;
    b->prev = fs.freeTail;
    if (!fs.freeHead) fs.freeHead = b;
    else fs.freeTail->next = b;
    fs.freeTail = b;
    fs.freeCount++;
}

static int allocBlock() {
    if (!fs.freeHead) return -1;
    FreeBlock *h = fs.freeHead;
    int idx = h->index;
    fs.freeHead = h->next;
    if (fs.freeHead) fs.freeHead->prev = NULL;
    else fs.freeTail = NULL;
    free(h);
    fs.freeCount--;
    return idx;
}

static void freeBlock(int idx) {
    addFreeBlockTail(idx);
}

/* ---------- FileNode helpers ---------- */

static void initFileNode(FileNode *n, const char *name, bool isDir) {
    safe_strcpy(n->name, name);
    n->isDir = isDir;
    n->child = NULL;
    n->next = n->prev = n; /* self circular default */
    n->parent = NULL;
    n->blockPointers = NULL;
    n->blockCount = 0;
    n->blockCap = 0;
}

static void ensureBlockCapacity(FileNode *f, int need) {
    if (f->blockCap >= need) return;
    int cap = (f->blockCap == 0) ? 4 : f->blockCap;
    while (cap < need) cap *= 2;
    int *tmp = realloc(f->blockPointers, sizeof(int) * cap);
    if (!tmp) die("realloc failed");
    f->blockPointers = tmp;
    f->blockCap = cap;
}

/* find child by name (in cwd or given parent) */
static FileNode *findChild(FileNode *parent, const char *name) {
    if (!parent->child) return NULL;
    FileNode *t = parent->child;
    do {
        if (strcmp(t->name, name) == 0) return t;
        t = t->next;
    } while (t != parent->child);
    return NULL;
}

/* insert child at tail (maintain circular list) */
static void insertChild(FileNode *parent, FileNode *node) {
    node->parent = parent;
    if (!parent->child) {
        parent->child = node;
        node->next = node->prev = node;
    } else {
        FileNode *head = parent->child;
        FileNode *tail = head->prev;
        tail->next = node;
        node->prev = tail;
        node->next = head;
        head->prev = node;
    }
}

/* unlink node from parent's child circular list (no freeing) */
static void unlinkNode(FileNode *node) {
    FileNode *parent = node->parent;
    if (!parent) return;
    if (node->next == node) {
        parent->child = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        if (parent->child == node) parent->child = node->next;
    }
    node->next = node->prev = node;
    node->parent = NULL;
}

/* recursively free entire subtree (node and its descendants) */
static void freeDirectoryTree(FileNode *node) {
    if (!node) return;
    FileNode *child = node->child;
    if (child) {
        FileNode *t = child;
        do {
            FileNode *next = t->next;
            freeDirectoryTree(t);
            t = next;
        } while (t != child);
    }
    if (!node->isDir) {
        for (int i = 0; i < node->blockCount; ++i)
            freeBlock(node->blockPointers[i]);
        free(node->blockPointers);
    }
    free(node);
}

/* ---------- Commands (preserve exact wording) ---------- */

static void mkdirCmd(const char *name) {
    if (!valid_name(name)) { printf("Invalid name.\n"); return; }
    if (findChild(fs.cwd, name)) { printf("Entry with name '%s' already exists.\n", name); return; }
    FileNode *n = malloc(sizeof(FileNode));
    if (!n) die("malloc failed");
    initFileNode(n, name, true);
    insertChild(fs.cwd, n);
    printf("Directory '%s' created successfully.\n", name);
}

static void createCmd(const char *name) {
    if (!valid_name(name)) { printf("Invalid name.\n"); return; }
    if (findChild(fs.cwd, name)) { printf("Entry with name '%s' already exists.\n", name); return; }
    FileNode *n = malloc(sizeof(FileNode));
    if (!n) die("malloc failed");
    initFileNode(n, name, false);
    insertChild(fs.cwd, n);
    printf("File '%s' created successfully.\n", name);
}

static void writeCmd(const char *name, const char *data) {
    FileNode *f = findChild(fs.cwd, name);
    if (!f) { printf("File not found.\n"); return; }
    if (f->isDir) { printf("'%s' is a directory.\n", name); return; }

    size_t len = strlen(data);
    size_t totalBytes = len + 1; /* include NUL to make read simpler */
    int required = (int)((totalBytes + BLOCK_SIZE - 1) / BLOCK_SIZE);
    if (required > TOTAL_BLOCKS) { printf("Data too large.\n"); return; }

    /* free old blocks */
    for (int i = 0; i < f->blockCount; ++i) freeBlock(f->blockPointers[i]);
    f->blockCount = 0;

    ensureBlockCapacity(f, required);

    size_t offset = 0;
    for (int i = 0; i < required; ++i) {
        int blk = allocBlock();
        if (blk == -1) {
            /* rollback */
            for (int j = 0; j < f->blockCount; ++j) freeBlock(f->blockPointers[j]);
            f->blockCount = 0;
            printf("Write failed: disk full.\n");
            return;
        }
        f->blockPointers[f->blockCount++] = blk;
        size_t toCopy = (totalBytes - offset > BLOCK_SIZE) ? BLOCK_SIZE : (totalBytes - offset);
        memcpy(fs.disk[blk], data + offset, toCopy);
        if (toCopy < BLOCK_SIZE) memset(fs.disk[blk] + toCopy, 0, BLOCK_SIZE - toCopy);
        offset += toCopy;
    }

    printf("Data written successfully (size=%zu bytes).\n", len);
}

static void readCmd(const char *name) {
    FileNode *f = findChild(fs.cwd, name);
    if (!f) { printf("File not found.\n"); return; }
    if (f->isDir) { printf("'%s' is a directory.\n", name); return; }
    if (f->blockCount == 0) { printf("File is empty.\n"); return; }

    for (int i = 0; i < f->blockCount; ++i) {
        if (i < f->blockCount - 1) {
            fwrite(fs.disk[f->blockPointers[i]], 1, BLOCK_SIZE, stdout);
        } else {
            char *blk = (char *)fs.disk[f->blockPointers[i]];
            size_t n = strnlen(blk, BLOCK_SIZE);
            fwrite(blk, 1, n, stdout);
        }
    }
    printf("\n");
}

static void deleteCmd(const char *name) {
    FileNode *f = findChild(fs.cwd, name);
    if (!f) { printf("File not found.\n"); return; }
    if (f->isDir) { printf("'%s' is a directory. Use rmdir to remove directories.\n", name); return; }

    unlinkNode(f);
    for (int i = 0; i < f->blockCount; ++i) freeBlock(f->blockPointers[i]);
    free(f->blockPointers);
    free(f);
    printf("File deleted successfully.\n");
}

static void rmdirCmd(const char *name) {
    FileNode *d = findChild(fs.cwd, name);
    if (!d) { printf("Directory not found.\n"); return; }
    if (!d->isDir) { printf("'%s' is not a directory.\n", name); return; }
    if (d->child) { printf("Directory not empty.\n"); return; }

    unlinkNode(d);
    free(d);
    printf("Directory removed successfully.\n");
}

static void lsCmd(void) {
    if (!fs.cwd->child) { printf("(empty)\n"); return; }
    FileNode *t = fs.cwd->child;
    do {
        printf("%s%s\n", t->name, t->isDir ? "/" : "");
        t = t->next;
    } while (t != fs.cwd->child);
}

static void cdCmd(const char *name) {
    if (strcmp(name, "..") == 0) {
        if (fs.cwd->parent) fs.cwd = fs.cwd->parent;
        printf("Moved to %s\n", fs.cwd->name);
        return;
    }
    FileNode *d = findChild(fs.cwd, name);
    if (!d || !d->isDir) { printf("Directory not found.\n"); return; }
    fs.cwd = d;
    /* print path as in sample: Moved to /docs */
    if (fs.cwd == fs.root) printf("Moved to /\n");
    else {
        /* build path */
        char path[4096] = {0};
        const FileNode *stack[1024];
        int top = 0;
        const FileNode *cur = fs.cwd;
        while (cur && cur->parent) {
            stack[top++] = cur;
            cur = cur->parent;
            if (top >= 1023) break;
        }
        strcat(path, "");
        for (int i = top - 1; i >= 0; --i) {
            strcat(path, "/");
            strcat(path, stack[i]->name);
        }
        printf("Moved to %s\n", path);
    }
}

static void pwdCmd(FileNode *d) {
    if (!d) return;
    if (d == fs.root) { printf("/"); return; }

    const FileNode *stack[1024];
    int top = 0;
    const FileNode *cur = d;
    while (cur && cur->parent) {
        stack[top++] = cur;
        cur = cur->parent;
        if (top >= 1023) break;
    }
    for (int i = top - 1; i >= 0; --i) {
        printf("/%s", stack[i]->name);
    }
}

static void dfCmd(void) {
    int used = TOTAL_BLOCKS - fs.freeCount;
    printf("Total Blocks: %d\n", TOTAL_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", fs.freeCount);
    printf("Disk Usage: %.2f%%\n", 100.0 * used / TOTAL_BLOCKS);
}

/* ---------- Init & Cleanup ---------- */

static void initSystem(void) {
    fs.freeHead = fs.freeTail = NULL;
    fs.freeCount = 0;
    for (int i = 0; i < TOTAL_BLOCKS; ++i) addFreeBlockTail(i);

    fs.root = malloc(sizeof(FileNode));
    if (!fs.root) die("malloc failed");
    initFileNode(fs.root, "/", true);
    fs.root->parent = NULL;
    fs.root->child = NULL;
    fs.root->next = fs.root->prev = fs.root;
    fs.cwd = fs.root;
}

static void cleanup(void) {
    /* free free-list */
    FreeBlock *fb = fs.freeHead;
    while (fb) {
        FreeBlock *next = fb->next;
        free(fb);
        fb = next;
    }
    fs.freeHead = fs.freeTail = NULL;
    fs.freeCount = 0;

    /* free filesystem tree */
    if (fs.root) {
        FileNode *child = fs.root->child;
        if (child) {
            FileNode *t = child;
            do {
                FileNode *next = t->next;
                freeDirectoryTree(t);
                t = next;
            } while (t != child);
        }
        free(fs.root);
        fs.root = fs.cwd = NULL;
    }
}

/* ---------- Command parsing & main ---------- */

int main(void) {
    initSystem();
    printf("Compact VFS - ready. Type 'exit' to quit.\n");

    char line[LINE_BUF];
    char cmd[64], arg1[NAME_LIMIT + 2], argRest[LINE_BUF];

    while (1) {
        /* prompt: root shows '/' and others show their name */
        if (fs.cwd == fs.root) printf("/ > ");
        else printf("%s > ", fs.cwd->name);

        if (!fgets(line, sizeof(line), stdin)) break;
        size_t ln = strlen(line);
        if (ln && line[ln - 1] == '\n') line[ln - 1] = '\0';

        cmd[0] = arg1[0] = argRest[0] = '\0';
        /* parse with width limits; argRest captures remainder (data maybe with spaces) */
        int scanned = sscanf(line, "%63s %50s %8191[^\n]", cmd, arg1, argRest);

        if (scanned <= 0) continue;

        if (strcmp(cmd, "mkdir") == 0) {
            if (scanned >= 2) mkdirCmd(arg1);
            else printf("Usage: mkdir <name>\n");
        }
        else if (strcmp(cmd, "create") == 0) {
            if (scanned >= 2) createCmd(arg1);
            else printf("Usage: create <name>\n");
        }
        else if (strcmp(cmd, "write") == 0) {
            if (scanned >= 2) {
                const char *data = (scanned == 3) ? argRest : "";
                writeCmd(arg1, data);
            } else printf("Usage: write <filename> <data...>\n");
        }
        else if (strcmp(cmd, "read") == 0) {
            if (scanned >= 2) readCmd(arg1);
            else printf("Usage: read <filename>\n");
        }
        else if (strcmp(cmd, "delete") == 0) {
            if (scanned >= 2) deleteCmd(arg1);
            else printf("Usage: delete <filename>\n");
        }
        else if (strcmp(cmd, "rmdir") == 0) {
            if (scanned >= 2) rmdirCmd(arg1);
            else printf("Usage: rmdir <dirname>\n");
        }
        else if (strcmp(cmd, "ls") == 0) lsCmd();
        else if (strcmp(cmd, "cd") == 0) {
            if (scanned >= 2) cdCmd(arg1);
            else printf("Usage: cd <dirname|..>\n");
        }
        else if (strcmp(cmd, "pwd") == 0) { pwdCmd(fs.cwd); printf("\n"); }
        else if (strcmp(cmd, "df") == 0) dfCmd();
        else if (strcmp(cmd, "exit") == 0) { cleanup(); printf("Memory released. Exiting program...\n"); break; }
        else printf("Invalid command.\n");
    }

    return 0;
}
