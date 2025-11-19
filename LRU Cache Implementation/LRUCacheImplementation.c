#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUE_LEN 100
#define HASH_SIZE 2003  

/** NODE STRUCT FOR QUEUE **/
typedef struct Node {
    int key;
    char value[MAX_VALUE_LEN];
    struct Node *prev, *next;
} Node;

/** HASHMAP ENTRY **/
typedef struct HashEntry {
    int key;
    Node *address;      
    struct HashEntry *next;
} HashEntry;

/** LRU CACHE STRUCT **/
typedef struct LRUCache {
    int capacity;
    int size;
    Node *head;   
    Node *tail;
    HashEntry *map[HASH_SIZE];
} LRUCache;

/** HASH FUNCTION **/
int hash(int key) {
    return key % HASH_SIZE;
}

/** CREATE NEW DLL NODE **/
Node* createNode(int key, const char *value) {
    Node *n = (Node*)malloc(sizeof(Node));
    n->key = key;
    strncpy(n->value, value, MAX_VALUE_LEN);
    n->value[MAX_VALUE_LEN - 1] = '\0';
    n->prev = n->next = NULL;
    return n;
}

/** CREATE NEW HASH ENTRY **/
HashEntry* createHashEntry(int key, Node *addr) {
    HashEntry *h = (HashEntry*)malloc(sizeof(HashEntry));
    h->key = key;
    h->address = addr;
    h->next = NULL;
    return h;
}

/** FIND IN HASHMAP **/
Node* hashGet(LRUCache *cache, int key) {
    int h = hash(key);
    HashEntry *entry = cache->map[h];
    while (entry) {
        if (entry->key == key) return entry->address;
        entry = entry->next;
    }
    return NULL;
}

/** INSERT INTO HASHMAP **/
void hashPut(LRUCache *cache, int key, Node *node) {
    int h = hash(key);
    HashEntry *newEntry = createHashEntry(key, node);
    newEntry->next = cache->map[h];
    cache->map[h] = newEntry;
}

/** DELETE FROM HASHMAP **/
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

/** MOVE NODE TO FRONT (MRU) **/
void moveToFront(LRUCache *cache, Node *node) {
    if (cache->head == node) return;

    // unlink node
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;

    // if tail removed
    if (cache->tail == node)
        cache->tail = node->prev;

    // insert at head
    node->prev = NULL;
    node->next = cache->head;
    if (cache->head) cache->head->prev = node;
    cache->head = node;

    if (cache->tail == NULL) cache->tail = node;
}

/** REMOVE LRU (TAIL) **/
void removeLRU(LRUCache *cache) {
    if (!cache->tail) return;

    Node *lru = cache->tail;
    hashDelete(cache, lru->key);

    if (lru->prev)
        lru->prev->next = NULL;

    cache->tail = lru->prev;

    if (cache->tail == NULL)
        cache->head = NULL;

    free(lru);
    cache->size--;
}

/** ADD TO FRONT AS MRU **/
void addToFront(LRUCache *cache, Node *node) {
    node->prev = NULL;
    node->next = cache->head;

    if (cache->head)
        cache->head->prev = node;

    cache->head = node;

    if (cache->tail == NULL)
        cache->tail = node;

    cache->size++;
}

/** LRU CACHE API: get(key) **/
char* get(LRUCache *cache, int key) {
    Node *node = hashGet(cache, key);

    if (!node) return NULL;

    moveToFront(cache, node);
    return node->value;
}

/** LRU CACHE API put(key, value) **/
void put(LRUCache *cache, int key, const char *value) {
    Node *node = hashGet(cache, key);

    if (node) {
        
        strncpy(node->value, value, MAX_VALUE_LEN);
        node->value[MAX_VALUE_LEN - 1] = '\0';
        moveToFront(cache, node);
        return;
    }

    
    if (cache->size == cache->capacity) {
        removeLRU(cache);
    }

    Node *newNode = createNode(key, value);
    addToFront(cache, newNode);
    hashPut(cache, key, newNode);
}

/** CREATE CACHE **/
LRUCache* createCache(int capacity) {
    LRUCache *cache = (LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = cache->tail = NULL;

    for (int i = 0; i < HASH_SIZE; i++)
        cache->map[i] = NULL;

    return cache;
}


int main() {
    LRUCache *cache = NULL;
    char command[50];

    while (1) {
        scanf("%s", command);

        if (strcmp(command, "createCache") == 0) {
            int size;
            scanf("%d", &size);
            cache = createCache(size);
        }
        else if (strcmp(command, "put") == 0) {
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
            break;
        }
    }

    return 0;
}
