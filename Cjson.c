#include <string.h>
#include "Cjson.h"


#include <stdio.h>

static size_t INIT_CAP = 50;

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

Cjson createNewCjsonObject(void) {
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

Cjson copyCjsonObject(Cjson CjsonObj) {
    Cjson newCjsonObj = createNewCjsonObject();
    struct keyValPair **values = CjsonObj->keyValPairs;

    size_t capacity = CjsonObj->capacity;
    size_t i;
    for (i = 0; i < capacity; i++) {
        struct keyValPair *curPair = values[i];
        if (curPair != NULL) {
            ensureCapacity(newCjsonObj);
            size_t newCapacity = newCjsonObj->capacity;
            size_t hash = hash(newCapacity, curPair->key);
            struct keyValPair *newPair = copyKeyValPair(curPair);
            insertKeyValPair(newCjsonObj, hash, newPair);
        }
    }

    return newCjsonObj;
}

static void freeKeyValPair(struct keyValPair *pair) {
    if (pair == NULL) return;

    free(pair->key);
    free(pair->value);
    free(pair);
}

bool destroyCjsonObject(Cjson CjsonObj) {
    if (CjsonObj == NULL) return true;

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
        case CJSON_INT:
            newPair->value = malloc(sizeof(int));
            *((int *)newPair->value) = *((int *)curPair->value);
            break;
        case CJSON_STRING:
            newPair->value = copy_string(curPair->value);
            break;
        case CJSON_FLOAT:
            newPair->value = malloc(sizeof(float));
            *((float *)newPair->value) = *((float *)curPair->value);
            break;
        case CJSON_OBJ:
            newPair->value = copyCJsonObject(curPair->value);
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

static void insertKeyValPair(Cjson CjsonObj, size_t hash, void *addValue) {
    size_t capacity = CjsonObj->capacity;
    while (CjsonObj->keyValPairs[hash] != NULL) {
        hash = (hash + 1) % capacity;
    }

    CjsonObj->keyValPairs[hash] = addValue;

    CjsonObj->numKeys += 1;
}

static inline void addValue(Cjson CjsonObj, char *key, void *value, enum CJSON_TYPE type) {
    ensureCapacity(CjsonObj);

    char *addKey = copy_string(key);

    size_t newHash = hash(CjsonObj->capacity, key);
    struct keyValPair *newPair = createKeyValPair(addKey, value, type);

    insertKeyValPair(CjsonObj, newHash, newPair);
}

bool addInt(Cjson CjsonObj, char *key, int value) {
    int *newValue = malloc(sizeof(int));
    *newValue = value;

    addValue(CjsonObj, key, newValue, CJSON_INT);
    return true;
}

bool addString(Cjson CjsonObj, char *key, char *value) {
    char *newValue = copy_string(value);

    addValue(CjsonObj, key, newValue, CJSON_STRING);
    return true;
}

bool addFloat(Cjson CjsonObj, char *key, float value) {
    float *newValue = malloc(sizeof(float));
    *newValue = value;

    addValue(CjsonObj, key, newValue, CJSON_FLOAT);
    return true;
}

static inline void *getValue(Cjson CjsonObj, char *key) {
    size_t capacity = CjsonObj->capacity;
    size_t getHash = hash(capacity, key);
    struct keyValPair **content = CjsonObj->keyValPairs;

    size_t initialHash = getHash;

    while (content[getHash] == NULL ||
      (strcmp(content[getHash]->key, key) != 0)) {
        getHash = (getHash + 1) % capacity;

        // if we have inspected all hashes and not found, NULL
        if (getHash == initialHash) return NULL;
    }

    return content[getHash]->value;
}

int getInt(Cjson CjsonObj, char *key) {
    return *((int *)getValue(CjsonObj, key));
}

char *getString(Cjson CjsonObj, char *key) {
    return (char *)getValue(CjsonObj, key);
}

float getFloat(Cjson CjsonObj, char *key) {
    return *((float *)getValue(CjsonObj, key));
}

bool doesKeyExist(Cjson CjsonObj, char *key) {
    if (getValue(CjsonObj, key) == NULL) return false;

    return true;
}

bool doesTypeMatch(Cjson CjsonObj, char *key, enum CJSON_TYPE type) {
    if (!doesKeyExist(CjsonObj, key)) return false;

    size_t checkHash = hash(CjsonObj->capacity, key);

    while (CjsonObj->keyValPairs[checkHash] == NULL ||
      strcmp(CjsonObj->keyValPairs[checkHash]->key, key) != 0) {
        checkHash = (checkHash + 1) % CjsonObj->capacity;
      }
    struct keyValPair *pair = CjsonObj->keyValPairs[checkHash];
    return pair->type == type;
}

Cjson createInitCjsonObject(char **keys, void **values, enum CJSON_TYPE *types, size_t numEntries) {
    Cjson newCjson = malloc(sizeof(struct Cjson));
    newCjson->capacity = numEntries;
    newCjson->numKeys = 0;
    newCjson->keyValPairs = malloc(numEntries * sizeof(struct keyValPair *));

    size_t kv;
    for (kv = 0; kv < numEntries; kv++) {
        newCjson->keyValPairs[kv] = NULL;
    }


    size_t i;
    for (i = 0; i < numEntries; i++) {
        addValue(newCjson, keys[i], values[i], types[i]);
    }

    return newCjson;
}
