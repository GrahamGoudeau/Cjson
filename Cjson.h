#ifndef CJSON_H
#define CJSON_H

#include <stdlib.h>
#include <stdbool.h>

struct Cjson;

typedef struct Cjson *Cjson;

enum CJSON_TYPE {
    CJSON_INT,
    CJSON_STRING,
    CJSON_FLOAT,
    CJSON_OBJ
};

Cjson createNewCjsonObject(void);

Cjson createInitCjsonObject(char **keys, void **values, enum CJSON_TYPE *types, size_t numEntries);

bool doesKeyExist(Cjson CjsonObj, char *key);

bool doesTypeMatch(Cjson CjsonObj, char *key, enum CJSON_TYPE type);

bool addInt(Cjson CjsonObj, char *key, int value);

bool addString(Cjson CjsonObj, char *key, char *value);

bool addFloat(Cjson CjsonObj, char *key, float value);

bool addCjson(Cjson CjsonObj, char *key, Cjson value);

int getInt(Cjson CjsonObj, char *key);

char *getString(Cjson CjsonObj, char *key);

float getFloat(Cjson CjsonObj, char *key);

Cjson getCjson(Cjson CjsonObj, char *key);

bool destroyCjsonObject(Cjson CjsonObj);

#endif
