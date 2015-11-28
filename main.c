#include <stdlib.h>
#include <stdio.h>
#include "Cjson.h"

#define BREAK "\n\n-----------------------------\n\n"

int keyLength = 5;

void gen_random(char *s, const int len) {
    int i;
    for (i = 0; i < len; ++i) {
        int randomChar = rand()%(26+26+10);
        if (randomChar < 26)
            s[i] = 'a' + randomChar;
        else if (randomChar < 26+26)
            s[i] = 'A' + randomChar - 26;
        else
            s[i] = '0' + randomChar - 26 - 26;
    }
    s[len] = 0;
}

char *newRandomString(void) {
    char *newChar = malloc(keyLength * sizeof(char));
    gen_random(newChar, keyLength - 1);
    return newChar;
}

int main(int argc, char **argv) {
    fprintf(stderr, "Cjson unit tests" BREAK);
    int seed = time(NULL);
    srand(seed);
    fprintf(stderr, "Creating object\n");
    Cjson CjsonObj = createNewCjsonObject();
    destroyCjsonObject(CjsonObj);
    fprintf(stderr, "Objected destroyed" BREAK);

    fprintf(stderr, "Creating new object\n");
    CjsonObj = createNewCjsonObject();
    char *key = newRandomString();
    fprintf(stderr, "Adding key: '%s' with value: 99\n", key);
    addInt(CjsonObj, key, 99);
    char *key2 = newRandomString();
    fprintf(stderr, "Adding key: '%s' with value: 21\n", key2);
    addInt(CjsonObj, key2, 21);
    fprintf(stderr, "Getting value from key '%s': %d\n", key, getInt(CjsonObj, key));
    fprintf(stderr, "Getting value from key '%s': %d\n", key2, getInt(CjsonObj, key2));
    free(key);
    free(key2);
    fprintf(stderr, BREAK);

    key = newRandomString();
    key2 = newRandomString();
    fprintf(stderr, "Adding key: '%s' with subobject '{\"%s\": \"%s\"}'\n", key, key2, "from subobject");
    Cjson subobject = createNewCjsonObject();
    addString(subobject, key2, "test string");
    addCjson(CjsonObj, key, subobject);
    Cjson retrieved = getCjson(CjsonObj, key);
    char *retrievedString = getString(retrieved, key2);
    fprintf(stderr, "Retrieved '%s' from subobject\n", retrievedString);

    free(key);
    free(key2);

    destroyCjsonObject(CjsonObj);
    return 0;
}
