#include <string.h>
#include "Cjson.h"


#include <stdio.h>

static size_t INIT_CAP = 50;

enum CJSON_TYPE {
    INT,
    STRING,
    FLOAT,
    CJSON
};

struct keyValPair {
    char *key;
    void *value;
    enum CJSON_TYPE type;
};

struct Cjson {
    size_t capacity;
    size_t numKeys;

    struct keyValPair **keyValPairs;
};

Cjson createCjsonObject(void) {
    Cjson newCjson = malloc(sizeof(struct Cjson));
    newCjson->capacity = INIT_CAP;
    newCjson->numKeys = 0;
    newCjson->keyValPairs = malloc(INIT_CAP * sizeof(struct keyValPair *));

    size_t i;
    for (i = 0; i < INIT_CAP; i++) {
        newCjson->keyValPairs[i] = NULL;
    }

    return newCjson;
}

static void freeKeyValPair(struct keyValPair *pair) {
    if (pair == NULL) return;

    free(pair->key);
    free(pair->value);
    free(pair);
}

bool destroyCjsonObject(Cjson CjsonObj) {
    size_t capacity = CjsonObj->capacity;
    size_t i;
    for (i = 0; i < capacity; i++) {
        struct keyValPair *curPair = CjsonObj->keyValPairs[i];
        if (curPair != NULL) {
            freeKeyValPair(curPair);
        }
    }

    free(CjsonObj->keyValPairs);
    free(CjsonObj);

    return true;
}

static size_t jenkins_hash(char *key, size_t len) {
    size_t hash, i;
    for(hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


static inline size_t hash(size_t capacity, char *key) {
    size_t hash = jenkins_hash(key, strlen(key));

    return hash % capacity;
}

static char *copy_string(char *input) {
    size_t length = strlen(input) + 1;
    char *copied = malloc(length * sizeof(char));
    size_t c = 0;
    while (input[c]) {
        copied[c] = input[c];
        c++;
    }
    copied[c] = input[c];

    return copied;
}

static struct keyValPair *copyKeyValPair(struct keyValPair *curPair) {
    struct keyValPair *newPair = malloc(sizeof(struct keyValPair));

    newPair->key = copy_string(curPair->key);
    newPair->type = curPair->type;

    switch (newPair->type) {
        case INT:
            newPair->value = malloc(sizeof(int));
            *((int *)newPair->value) = *((int *)curPair->value);
            break;
        case STRING:
            newPair->value = malloc(sizeof(char *));
            *((char **)newPair->value) = *((char **)curPair->value);
            break;
        case FLOAT:
            newPair->value = malloc(sizeof(float));
            *((float *)newPair->value) = *((float *)curPair->value);
            break;
        case CJSON:
            fprintf(stderr, "CJSON COPYING NOT IMPLEMENTED\n");
            break;
        default:
            newPair->value = NULL;
            break;
    }

    return newPair;
}

static struct keyValPair *createKeyValPair(char *key, void *value, enum CJSON_TYPE type) {
    struct keyValPair *newPair = malloc(sizeof(struct keyValPair));
    newPair->key = key;
    newPair->value = value;
    newPair->type = type;

    return newPair;
}

/* CjsonObj - object to be rehashed into
   capacity - old capacity
*/
static void rehash(Cjson CjsonObj, size_t capacity) {
    struct keyValPair **newPairs = malloc(2 * capacity * sizeof(struct keyValPair *));

    size_t newCapacity = 2 * capacity;
    CjsonObj->capacity = newCapacity;

    size_t reset;
    for (reset = 0; reset < newCapacity; reset++) {
        newPairs[reset] = NULL;
    }

    size_t i;
    for (i = 0; i < capacity; i++) {
        struct keyValPair *curPair = CjsonObj->keyValPairs[i];
        if (curPair != NULL) {
            struct keyValPair *newPair = copyKeyValPair(curPair);
            size_t newHash = hash(newCapacity, newPair->key);
            while (newPairs[newHash] != NULL) {
                newHash = (newHash + 1) % newCapacity;
            }
            newPairs[newHash] = newPair;
            freeKeyValPair(curPair);
        }
    }

    free(CjsonObj->keyValPairs);
    CjsonObj->keyValPairs = newPairs;
}

static void ensureCapacity(Cjson CjsonObj) {
    size_t capacity = CjsonObj->capacity;
    size_t numKeys = CjsonObj->numKeys;

    // rehash and enlarge if at 0.5 capacity
    if (numKeys + 1 >= (capacity / 2)) {
        rehash(CjsonObj, capacity);
    }
}

static void insertValue(Cjson CjsonObj, size_t hash, void *addValue) {
    size_t capacity = CjsonObj->capacity;
    while (CjsonObj->keyValPairs[hash] != NULL) {
        hash = (hash + 1) % capacity;
    }

    CjsonObj->keyValPairs[hash] = addValue;

    CjsonObj->numKeys += 1;
}

bool addInt(Cjson CjsonObj, char *key, int value) {
    ensureCapacity(CjsonObj);
    int *addValue = malloc(sizeof(int));
    *addValue = value;

    char *addKey = copy_string(key);

    size_t newHash = hash(CjsonObj->capacity, key);
    struct keyValPair *newPair = createKeyValPair(addKey, addValue, INT);

    insertValue(CjsonObj, newHash, newPair);
}

static inline void *getValue(Cjson CjsonObj, char *key) {
    size_t capacity = CjsonObj->capacity;
    size_t getHash = hash(capacity, key);
    struct keyValPair **content = CjsonObj->keyValPairs;

    while (content[getHash] == NULL ||
      (strcmp(content[getHash]->key, key) != 0)) {
        getHash = (getHash + 1) % capacity;
    }

    return content[getHash]->value;
}

int getInt(Cjson CjsonObj, char *key) {
    return *((int *)getValue(CjsonObj, key));
}
