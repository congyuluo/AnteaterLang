//
// Created by congyu on 7/20/23.
//

#include "builtinClasses.h"
#include "errors.h"
#include "runtimeDS.h"
#include "objectManager.h"
#include "objClass.h"
#include "object.h"
#include "stringHash.h"

#include <math.h>
#include <string.h>
#include <time.h>

#define DEF_BUILTIN_CFUNC_INIT_CLASS(name, classID, in, out, cFunc) createClass(name, classID, OBJECT_VAL(createConstCallableObject(CREATE_CFUNC_METHOD(in, out, cFunc)), BUILTIN_CALLABLE), 0, C_FUNC_INIT_TYPE)
#define DEF_BUILTIN_CFUNC_METHOD_VALUE(in, out, cFunc) OBJECT_VAL(createConstCallableObject(CREATE_CFUNC_METHOD(in, out, cFunc)), BUILTIN_CALLABLE)
#define DEF_BUILTIN_CFUNC_FUNCTION_VALUE(in, out, cFunc) OBJECT_VAL(createConstCallableObject(CREATE_CFUNC_FUNCTION(in, out, cFunc)), BUILTIN_CALLABLE)

#define CHECK_NUM_TYPE(val) if (VALUE_TYPE(val) != VAL_NUMBER) raiseExceptionByName("TypeError", "Value is not of type num")

// Builtin classes
objClass* callableClass;
objClass* noneClass;
objClass* numClass;
objClass* boolClass;
objClass* stringClass;
objClass* listClass;
objClass* dictClass;
objClass* setClass;
objClass* exceptionClass;

void addGlobalReference(refTable* globalRefTable, runtimeList* globalRefList, Value val, char* name) {
    if (refTableContains(globalRefTable, name)) compilationError(0, 0, 0, "global reference already exists");
    uint16_t index = getRefIndex(globalRefTable, name);
    if (index != listAddElementReturnIndex(globalRefList, val)) compilationError(0, 0, 0, "reference table and list mismatch");
}

Value printPrim(Value self, Value* args, int numArgs) {
    printPrimitiveValue(self);
    return NONE_VAL;
}

Value equalPrim(Value self, Value* args, int numArgs) {
    Value otherObj = args[0];
    if (VALUE_TYPE(otherObj) != VALUE_TYPE(self)) return BOOL_VAL(false);
    bool resultBool;
    switch (VALUE_TYPE(self)) {
        case BUILTIN_CALLABLE:
            resultBool = self.obj == otherObj.obj;
            break;
        case VAL_NONE:
            resultBool = true;
            break;
        case VAL_NUMBER: {
            double diff = VALUE_NUMBER_VALUE(self) - VALUE_NUMBER_VALUE(otherObj);
            double epsilon = 1e-9; // Tolerance
            resultBool = fabs(diff) < epsilon ? true : false;
            break;
        }
        case VAL_BOOL:
            resultBool = VALUE_BOOL_VALUE(self) == VALUE_BOOL_VALUE(otherObj);
            break;
        case BUILTIN_STR:
            resultBool = strcmp(VALUE_STR_VALUE(self), VALUE_STR_VALUE(otherObj)) == 0;
            break;
        default: {
            raiseExceptionByName("InternalError", "Unsupported type for equalPrim"); // No return needed for fatal error types
        }

    }
    return resultBool ? BOOL_VAL(true) : BOOL_VAL(false);
}

// Runtime data structures

// List

Value initList(Value self, Value* args, int numArgs) {
    VALUE_LIST_VALUE(self) = createRuntimeList(RUNTIME_LIST_INIT_SIZE);
    for (int i=0; i<numArgs; i++) listAddElement(VALUE_LIST_VALUE(self), args[i]);
    return NONE_VAL;
}

Value listAdd(Value self, Value* args, int numArgs) {
    listAddElement(VALUE_LIST_VALUE(self), args[0]);
    return NONE_VAL;
}

Value listInsert(Value self, Value* args, int numArgs) {
    if (VALUE_TYPE(args[0]) != VAL_NUMBER) {
        raiseExceptionByName("TypeError", "Value is not of type num");
        return NONE_VAL;
    }
    listInsertElement(VALUE_LIST_VALUE(self), VALUE_NUMBER_VALUE(args[0]), args[1]);
    return NONE_VAL;
}

Value listSet(Value self, Value* args, int numArgs) {
    if (VALUE_TYPE(args[0]) != VAL_NUMBER) {
        raiseExceptionByName("TypeError", "Value is not of type num");
        return NONE_VAL;
    }
    listSetElement(VALUE_LIST_VALUE(self),VALUE_NUMBER_VALUE(args[0]), args[1]);
    return NONE_VAL;
}

Value listRemove(Value self, Value* args, int numArgs) {
    if (VALUE_TYPE(args[0]) != VAL_NUMBER) {
        raiseExceptionByName("TypeError", "Value is not of type num");
        return NONE_VAL;
    }
    listRemoveElement(VALUE_LIST_VALUE(self),VALUE_NUMBER_VALUE(args[0]));
    return NONE_VAL;
}

Value listGet(Value self, Value* args, int numArgs) {
    if (VALUE_TYPE(args[0]) != VAL_NUMBER) {
        raiseExceptionByName("TypeError", "Value is not of type num");
        return NONE_VAL;
    }
    return listGetElement(VALUE_LIST_VALUE(self), VALUE_NUMBER_VALUE(args[0]));
}

Value listContains(Value self, Value* args, int numArgs) {
    bool result = listContainsElement(VALUE_LIST_VALUE(self), args[0]);
    return result ? BOOL_VAL(true) : BOOL_VAL(false);
}

Value listIndexOf(Value self, Value* args, int numArgs) {
    int result = listIndexOfElement(VALUE_LIST_VALUE(self), args[0]);
    Value resultObj = NUMBER_VAL(result);
    return resultObj;
}

Value listSize(Value self, Value* args, int numArgs) {
    int result = VALUE_LIST_VALUE(self)->size;
    Value resultObj = NUMBER_VAL(result);
    return resultObj;
}

// Dict
Value initDict(Value self, Value* args, int numArgs) {
    if (numArgs % 2 != 0) {
        raiseExceptionByName("ParameterError", "Dict init must have even number of arguments");
        return NONE_VAL;
    }
    VALUE_DICT_VALUE(self) = createRuntimeDict(RUNTIME_DICT_INIT_SIZE);
    for (int i=0; i<numArgs; i+=2) dictInsertElement(VALUE_DICT_VALUE(self), args[i], args[i+1]);
    return NONE_VAL;
}

Value dictInsert(Value self, Value* args, int numArgs) {
    dictInsertElement(VALUE_DICT_VALUE(self), args[0], args[1]);
    return NONE_VAL;
}

Value dictGet(Value self, Value* args, int numArgs) {
    dictGetElement(VALUE_DICT_VALUE(self), args[0]);
    return NONE_VAL;
}

Value dictContains(Value self, Value* args, int numArgs) {
    bool result = dictContainsElement(VALUE_DICT_VALUE(self), args[0]);
    return result ? BOOL_VAL(true) : BOOL_VAL(false);
}

Value dictRemove(Value self, Value* args, int numArgs) {
    dictRemoveElement(VALUE_DICT_VALUE(self), args[0]);
    return NONE_VAL;
}

Value dictSize(Value self, Value* args, int numArgs) {
    int result = VALUE_DICT_VALUE(self)->numEntries;
    Value resultObj = NUMBER_VAL(result);
    return resultObj;
}

// Set
Value initSet(Value self, Value* args, int numArgs) {
    VALUE_SET_VALUE(self) = createRuntimeSet(RUNTIME_SET_INIT_SIZE);
    for (int i=0; i<numArgs; i++) setInsertElement(VALUE_SET_VALUE(self), args[i]);
    return NONE_VAL;
}

Value setInsert(Value self, Value* args, int numArgs) {
    setInsertElement(VALUE_SET_VALUE(self), args[0]);
    return NONE_VAL;
}

Value setContains(Value self, Value* args, int numArgs) {
    bool result = setContainsElement(VALUE_SET_VALUE(self), args[0]);
    return result ? BOOL_VAL(true) : BOOL_VAL(false);
}

Value setRemove(Value self, Value* args, int numArgs) {
    setRemoveElement(VALUE_SET_VALUE(self), args[0]);
    return NONE_VAL;
}

Value setSize(Value self, Value* args, int numArgs) {
    int result = VALUE_SET_VALUE(self)->dict->numEntries;
    Value resultObj = NUMBER_VAL(result);
    return resultObj;
}

// String
Value strAdd(Value self, Value* args, int numArgs) {
    // Check other object type
    if (VALUE_TYPE(args[0]) != BUILTIN_STR) {
        raiseExceptionByName("TypeError", "Cannot add string to non-string object");
        return NONE_VAL;
    }
    char* otherStr = VALUE_STR_VALUE(args[0]);
    char* newStr = malloc(strlen(VALUE_STR_VALUE(self)) + strlen(otherStr) + 1);
    strcpy(newStr, VALUE_STR_VALUE(self));
    strcat(newStr, otherStr);
    // Use addReference to create a copy in the string dict
    Value result = OBJECT_VAL(createRuntimeStringObject(addReference(newStr)), BUILTIN_STR);
    // Free the temporary string to avoid memory leak
    free(newStr);
    return result;
}

// General-purpose functions

Value print(Value self, Value* args, int numArgs) {
    for (uint32_t i=0; i<numArgs; i++) DSPrintValue(args[i]);
    return NONE_VAL;
}

Value println(Value self, Value* args, int numArgs) {
    for (uint32_t i=0; i<numArgs; i++) DSPrintValue(args[i]);
    printf("\n");
    return NONE_VAL;
}

Value type(Value self, Value* args, int numArgs) {
    return OBJECT_VAL(createRuntimeStringObject(VALUE_CLASS(args[0])->className), BUILTIN_STR);
}

Value input(Value self, Value* args, int numArgs) {
    if (numArgs == 1) printf("%s", VALUE_STR_VALUE(args[0]));
    if (numArgs != 0 && numArgs != 1) {
        raiseExceptionByName("ParameterError", "Invalid number of arguments for input");
        return NONE_VAL;
    }

    char *line = NULL;
    size_t len = 0;
    getline(&line, &len, stdin);
    // Remove the trailing newline character
    size_t line_length = strlen(line);
    if (line_length > 0 && line[line_length - 1] == '\n') {
        line[line_length - 1] = '\0';
    }
    Value result = OBJECT_VAL(createRuntimeStringObject(line), BUILTIN_STR);
    free(line);
    return result;
}

Value clockFunc(Value self, Value* args, int numArgs) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value hasAttr(Value self, Value* args, int numArgs) {
    Value target = args[0];
    Value attrName = args[1];
    // Check if attrName is string
    if (VALUE_TYPE(attrName) != BUILTIN_STR) {
        raiseExceptionByName("ParameterError", "Attribute name must be a string");
        return NONE_VAL;
    }
    if (!IS_MARKABLE_VAL(target)) return BOOL_VAL(false);
    // Try to find the attribute in the object
    Value result = ignoreNullGetAttr(target, VALUE_STR_VALUE(attrName));
    return BOOL_VAL(VALUE_TYPE(result) != VAL_INTERNAL_NULL);
}

void constructBuiltinClasses(refTable* globalRefTable, runtimeList* globalRefList, refTable* globalClassTable) {

    callableClass = createClass("callable", BUILTIN_CALLABLE, INTERNAL_NULL_VAL, 0, NONE_INIT_TYPE);

    // Internal NULL class
    objClass* internalNull = createClass("Internal NULL", getRefIndex(globalClassTable, "Internal NULL"), INTERNAL_NULL_VAL, 0, NONE_INIT_TYPE);

    // None class
    noneClass = createClass("none", getRefIndex(globalClassTable, "none"), INTERNAL_NULL_VAL, 0, NONE_INIT_TYPE);
    CLASS_ADD_ATTR(noneClass, "_eq", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &equalPrim));

    // Bool class
    boolClass = createClass("bool", getRefIndex(globalClassTable, "bool"), INTERNAL_NULL_VAL, 0,  NONE_INIT_TYPE);
    CLASS_ADD_ATTR(boolClass, "print", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 0, &printPrim));
    CLASS_ADD_ATTR(boolClass, "_eq", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &equalPrim));

    // Num class
    numClass = createClass("num", getRefIndex(globalClassTable, "num"), INTERNAL_NULL_VAL, 0,  NONE_INIT_TYPE);
    CLASS_ADD_ATTR(numClass, "print", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 0, &printPrim));
    CLASS_ADD_ATTR(numClass, "_eq", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &equalPrim));

    // Callable class
    getRefIndex(globalClassTable, "callable");
    CLASS_ADD_ATTR(callableClass, "_eq", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &equalPrim));

    // String class
    stringClass = createClass("str", getRefIndex(globalClassTable, "str"), INTERNAL_NULL_VAL, 0,  NONE_INIT_TYPE);
    CLASS_ADD_ATTR(stringClass, "print", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 0, &printPrim));
    CLASS_ADD_ATTR(stringClass, "_eq", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &equalPrim));
    CLASS_ADD_ATTR(stringClass, "_add", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 0, &strAdd));

    // List class
    listClass = DEF_BUILTIN_CFUNC_INIT_CLASS("list", getRefIndex(globalClassTable, "list"), -1, 0, &initList);
    CLASS_ADD_ATTR(listClass, "print", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 0, &printPrim));
    CLASS_ADD_ATTR(listClass, "add", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 0, &listAdd));
    CLASS_ADD_ATTR(listClass, "insert", DEF_BUILTIN_CFUNC_METHOD_VALUE(2, 0, &listInsert));
    CLASS_ADD_ATTR(listClass, "set", DEF_BUILTIN_CFUNC_METHOD_VALUE(2, 0, &listSet));
    CLASS_ADD_ATTR(listClass, "remove", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 0, &listRemove));
    CLASS_ADD_ATTR(listClass, "get", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &listGet));
    CLASS_ADD_ATTR(listClass, "contains", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &listContains));
    CLASS_ADD_ATTR(listClass, "index", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &listIndexOf));
    CLASS_ADD_ATTR(listClass, "size", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 1, &listSize));

    // Dict class
    dictClass = DEF_BUILTIN_CFUNC_INIT_CLASS("dict", getRefIndex(globalClassTable, "dict"), -1, 0, &initDict);
    CLASS_ADD_ATTR(dictClass, "print", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 0, &printPrim));
    CLASS_ADD_ATTR(dictClass, "add", DEF_BUILTIN_CFUNC_METHOD_VALUE(2, 0, &dictInsert));
    CLASS_ADD_ATTR(dictClass, "get", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &dictGet));
    CLASS_ADD_ATTR(dictClass, "contains", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &dictContains));
    CLASS_ADD_ATTR(dictClass, "remove", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 0, &dictRemove));
    CLASS_ADD_ATTR(dictClass, "size", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 1, &dictSize));


    // Set class
    setClass = DEF_BUILTIN_CFUNC_INIT_CLASS("set", getRefIndex(globalClassTable, "set"), -1, 0, &initSet);
    CLASS_ADD_ATTR(setClass, "print", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 0, &printPrim));
    CLASS_ADD_ATTR(setClass, "add", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 0, &setInsert));
    CLASS_ADD_ATTR(setClass, "contains", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 1, &setContains));
    CLASS_ADD_ATTR(setClass, "remove", DEF_BUILTIN_CFUNC_METHOD_VALUE(1, 0, &setRemove));
    CLASS_ADD_ATTR(setClass, "size", DEF_BUILTIN_CFUNC_METHOD_VALUE(0, 1, &setSize));

    // Create exception class
    exceptionClass = createClass("exception", getRefIndex(globalClassTable, "exception"), INTERNAL_NULL_VAL, 0, NONE_INIT_TYPE);

    // Builtin functions
    Value printFunc = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(-1, 0, &print);
    addGlobalReference(globalRefTable, globalRefList, printFunc, "print");

    Value printlnFunc = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(-1, 0, &println);
    addGlobalReference(globalRefTable, globalRefList, printlnFunc, "println");

    Value typeFunc = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(1, 1, &type);
    addGlobalReference(globalRefTable, globalRefList, typeFunc, "type");

    Value inputFunc = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(-1, 1, &input);
    addGlobalReference(globalRefTable, globalRefList, inputFunc, "input");

    Value clockFuncVal = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(0, 1, &clockFunc);
    addGlobalReference(globalRefTable, globalRefList, clockFuncVal, "clock");

    Value hasAttrFunc = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(2, 1, &hasAttr);
    addGlobalReference(globalRefTable, globalRefList, hasAttrFunc, "hasAttr");
}

void loadUserFunctions(refTable* globalRefTable, runtimeList* globalRefList, userFunction* userFunctions, uint32_t funcCount) {
    for (uint32_t i=0; i<funcCount; i++) {
        Value currFunc = DEF_BUILTIN_CFUNC_FUNCTION_VALUE(userFunctions[i].in, userFunctions[i].out, userFunctions[i].cFunc);
        addGlobalReference(globalRefTable, globalRefList, currFunc, userFunctions[i].name);
    }
    printf("User functions loaded\n");
}