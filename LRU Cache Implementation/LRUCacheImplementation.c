#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUE_LEN 100
#define HASH_SIZE 2003   

/********************* DOUBLY LINKED LIST NODE *********************/
typedef struct QueueNode {
    int key;
    char value[MAX_VALUE_LEN];
    struct QueueNode *prev, *next;
} QueueNode;

/********************* HASHMAP ENTRY *********************/
typedef struct HashEntry {
    int key;
    QueueNode *address;         
    struct HashEntry *next;
} HashEntry;

/********************* LRU CACHE STRUCT *********************/
typedef struct LRUCache {
    int capacity;
    int size;
    QueueNode *head; 
    QueueNode *tail; 
    HashEntry *map[HASH_SIZE];
} LRUCache;

/********************* HASH FUNCTION FOR NEGATIVE KEYS *********************/
int hash(int key) {
    int h = key % HASH_SIZE;
    if (h < 0) h += HASH_SIZE;
    return h;
}

/********************* CREATE NEW QUEUE NODE *********************/
QueueNode* createQueueNode(int key, const char *value) {
    QueueNode *node = (QueueNode*)malloc(sizeof(QueueNode));
    node->key = key;
    strncpy(node->value, value, MAX_VALUE_LEN);
    node->value[MAX_VALUE_LEN - 1] = '\0';
    node->prev = node->next = NULL;
    return node;
}

/********************* CREATE NEW HASH ENTRY *********************/
HashEntry* createHashEntry(int key, QueueNode *addr) {
    HashEntry *entry = (HashEntry*)malloc(sizeof(HashEntry));
    entry->key = key;
    entry->address = addr;
    entry->next = NULL;
    return entry;
}

/********************* HASHMAP GET *********************/
QueueNode* hashGet(LRUCache *cache, int key) {
    int h = hash(key);
    HashEntry *entry = cache->map[h];
    while (entry) {
        if (entry->key == key) return entry->address;
        entry = entry->next;
    }
    return NULL;
}

/********************* HASHMAP PUT *********************/
void hashPut(LRUCache *cache, int key, QueueNode *node) {
    int h = hash(key);
    HashEntry *newEntry = createHashEntry(key, node);
    newEntry->next = cache->map[h];
    cache->map[h] = newEntry;
}

/********************* HASHMAP DELETE *********************/
void hashDelete(LRUCache *cache, int key) {
    int h = hash(key);
    HashEntry *entry = cache->map[h], *prev = NULL;

    while (entry) {
        if (entry->key == key) {
            if (prev) prev->next = entry->next;
            else cache->map[h] = entry->next;
            free(entry);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

/********************* MOVE NODE TO FRONT (MRU) *********************/
void moveToFront(LRUCache *cache, QueueNode *node) {
    if (cache->head == node) return; // already MRU

    // unlink
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    // update tail if needed
    if (cache->tail == node)
        cache->tail = node->prev;

    // place at head
    node->prev = NULL;
    node->next = cache->head;

    if (cache->head)
        cache->head->prev = node;

    cache->head = node;

    if (cache->tail == NULL)
        cache->tail = node;
}

/********************* REMOVE LRU NODE *********************/
void removeLRU(LRUCache *cache) {
    if (!cache->tail) return;

    QueueNode *lru = cache->tail;
    hashDelete(cache, lru->key);

    if (lru->prev)
        lru->prev->next = NULL;

    cache->tail = lru->prev;

    if (!cache->tail)
        cache->head = NULL;

    free(lru);
    cache->size--;
}

/********************* ADD NODE TO FRONT *********************/
void addToFront(LRUCache *cache, QueueNode *node) {
    node->prev = NULL;
    node->next = cache->head;

    if (cache->head)
        cache->head->prev = node;

    cache->head = node;

    if (cache->tail == NULL)
        cache->tail = node;

    cache->size++;
}

/********************* LRU GET *********************/
char* get(LRUCache *cache, int key) {
    QueueNode *node = hashGet(cache, key);
    if (!node) return NULL;

    moveToFront(cache, node);
    return node->value;
}

/********************* LRU PUT *********************/
void put(LRUCache *cache, int key, const char *value) {
    QueueNode *node = hashGet(cache, key);

    if (node) {
        // update value
        strncpy(node->value, value, MAX_VALUE_LEN);
        node->value[MAX_VALUE_LEN - 1] = '\0';
        moveToFront(cache, node);
        return;
    }

    // remove LRU if cache full
    if (cache->size == cache->capacity) {
        removeLRU(cache);
    }

    QueueNode *newNode = createQueueNode(key, value);
    addToFront(cache, newNode);
    hashPut(cache, key, newNode);
}

/********************* CREATE CACHE *********************/
LRUCache* createCache(int capacity) {
    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;

    for (int i = 0; i < HASH_SIZE; i++)
        cache->map[i] = NULL;

    return cache;
}

/********************* CLEANUP *********************/
void freeCache(LRUCache *cache) {
    // free DLL
    QueueNode *cur = cache->head;
    while (cur) {
        QueueNode *next = cur->next;
        free(cur);
        cur = next;
    }

    // free hashmap
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry *entry = cache->map[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(cache);
}

/********************* MAIN DRIVER *********************/
int main() {
    int size;
    printf("Enter cache capacity: ");
    scanf("%d", &size);

    LRUCache *cache = createCache(size);

    char command[50];

    while (1) {
        scanf("%s", command);

        if (strcmp(command, "put") == 0) {
            int key;
            char data[MAX_VALUE_LEN];
            scanf("%d %s", &key, data);
            put(cache, key, data);
        }
        else if (strcmp(command, "get") == 0) {
            int key;
            scanf("%d", &key);
            char *result = get(cache, key);
            if (result) printf("%s\n", result);
            else printf("NULL\n");
        }
        else if (strcmp(command, "exit") == 0) {
            freeCache(cache);
            break;
        }
    }

    return 0;
}
