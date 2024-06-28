#include <stdio.h>
#include <time.h>
#include <dlfcn.h>

#include "stringHash.h"
#include "object.h"
#include "common.h"
#include "vm.h"
#include "builtinClasses.h"
#include "runtimeDS.h"
#include "compiler.h"
#include "errors.h"
#include "refManager.h"
#include "objectManager.h"
#include "runtimeMemoryManager.h"

// VM definitions

void initAllTables() {
    initStringHash();
    initObjectManager();
}

void deleteAllTables() {
    freeVM();
    freeObjectManager();
    deleteStringHash();
    freeErrorTracer();
}

int main(int argc, const char* argv[]) {
    // Load sourceFile
    char* libPath = NULL;
    char* sourcePath;

    if (argc == 2) {
        sourcePath = (char*)argv[1];
    } else if (argc == 3) {
        libPath = (char*)argv[1];
        sourcePath = (char*)argv[2];
    } else {
        printf("Usage: [accLib, path] | [path]\n");
        return 64;
    }

    char* sourceFile = loadFile(sourcePath);
    if (sourceFile == NULL) return 74;

    refTable* exceptionRefTable = createRefTable(GLOBAL_REF_TABLE_INIT_SIZE);
    addBuiltinExceptions(exceptionRefTable);

    // We can raise exceptions from this point onwards
    initAllTables();

    // Attach to error handler
    attachSource(sourceFile, sourcePath);

    refTable* globalRefTable = createRefTable(GLOBAL_REF_TABLE_INIT_SIZE);
    runtimeList* globalRefList = createRuntimeList(RUNTIME_LIST_INIT_SIZE);
    refTable* globalClassRefTable = createRefTable(GLOBAL_REF_TABLE_INIT_SIZE);

    constructBuiltinClasses(globalRefTable, globalRefList, globalClassRefTable);

    // Add user functions
    void* libHandle = NULL;
    if (libPath != NULL) {
        printf("User function path: %s\n", libPath);
        libHandle = dlopen(libPath, RTLD_LAZY);
        if (!libHandle) {
            fprintf(stderr, "Error loading user function library: %s\n", dlerror());
            return 1;
        }
        userFunction* userFuncs = (userFunction*) dlsym(libHandle, "userFuncs");
        if (!userFuncs) {
            fprintf(stderr, "Error finding user functions: %s\n", dlerror());
            dlclose(libHandle);
            return 1;
        }
        uint32_t* funcCountPtr = (uint32_t*) dlsym(libHandle, "funcCount");
        if (!funcCountPtr) {
            fprintf(stderr, "Error finding user function count: %s\n", dlerror());
            dlclose(libHandle);
            return 1;
        }
        loadUserFunctions(globalRefTable, globalRefList, userFuncs, *funcCountPtr);
    }
    // End

    callable** prelinkedFunctionArray = NULL;
    Value* globalArray = NULL;
    uint32_t globalArraySize = 0;

    // Init tokenizer
    initTokenizer(sourceFile, sourcePath);

    Value mainFunc = compile(globalRefTable, globalClassRefTable, globalRefList, &prelinkedFunctionArray, &globalArray, &globalArraySize);

    Value inArgs;
    if (VALUE_CALLABLE_VALUE(mainFunc)->in != 0) {
        // Prepare in argument arrays
        inArgs = OBJECT_VAL(createConstObj(classArray[BUILTIN_LIST]), BUILTIN_LIST);
        VALUE_LIST_VALUE(inArgs) = createRuntimeList(RUNTIME_LIST_INIT_SIZE);
        for (uint32_t i=3; i < argc; i++) {
            char* currArg = (char*)argv[i];
            Value currArgVal = OBJECT_VAL(createConstStringObject(currArg), BUILTIN_STR);
            listAddElement(VALUE_LIST_VALUE(inArgs), currArgVal);
        }
    } else {
        inArgs = INTERNAL_NULL_VAL;
    }

    initVM(globalArray, prelinkedFunctionArray, globalArraySize);
    freeRuntimeList(globalRefList);

#ifdef EXECUTE_CHUNK
//  Run the VM
    // Wait until manager is ready
    initMemoryManager();
#ifdef TIME_EXECUTION
    // Timing
    clock_t t;
    t = clock();
#endif
    runVM(mainFunc, inArgs, VALUE_CALLABLE_VALUE(mainFunc)->in);
#ifdef TIME_EXECUTION
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("\nProgram took %f seconds to execute \n", time_taken);
#endif
    freeMemoryManager();
#endif

    deleteAllTables();
    // Finally, free the exception reference table
    freeRefTable(exceptionRefTable);
    if (libPath != NULL) dlclose(libHandle);
    return 0;

}
