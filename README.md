# Cjson

### Description
A (not-quite-complete) implementation of JavaScript Object Notation (JSON) for the C language.  The supported types that can be stored in a Cjson object are ints, floats, strings, and other Cjson objects.  Also supports a method to see if a key exists in an object and a method to check if the type of an existing value matches what the user expects.
### Usage
First `#include "Cjson.h"` at the top of your `.c` file, then take a look at some of the following examples:

    Cjson CjsonObj = createNewCjsonObject();
    char *key = "some random string";
    addInt(CjsonObj, key, 42);
    int retrieved = getInt(CjsonObj, key);
    printf("%d", retrieved); // 42

The same can be done with C strings.  Using the same `CjsonObj` as above:

    char *key = "new random string";
    addString(CjsonObj, key, "input string!");
    printf("%s", getString(CjsonObj, key)); // input string!
    
To take full advantage of the ability of Cjson to be nested within itself:

    char *key1 = "key1";
    char *key2 = "key2";
    Cjson CjsonObj = createNewCjsonObject();
    Cjson nested = createNewCjsonObject();
    addString(nested, key2, "nested value");
    addCjson(CjsonObj, key1, nested);
    
    Cjson retrieved_nested = getCjson(CjsonObj, key1);
    printf("%s", getString(retrieved_nested, key2)); // "nested value"
    
### Memory Management
When finished with a Cjson object, clean it up with
    
    destroyCjsonObject(CjsonObj);
    
This cleans up all of its own memory, as well as the associated memory of any Cjson object that may be nested within `CjsonObj`.
