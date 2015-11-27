#ifndef CJSON_H
#define CJSON_H

#include <stdlib.h>
#include <stdbool.h>

struct Cjson;

typedef struct Cjson *Cjson;

//Cjson createCjsonObject(size_t num_key_val_pairs, char **keys, void **values);
Cjson createCjsonObject(void);

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
