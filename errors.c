//
// Created by congyu on 7/18/23.
//

#include "errors.h"
#include "debug.h"
#include "stringHash.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>

char* sourceArray[MAX_SOURCE_SIZE];
char* fileNameArray[MAX_SOURCE_SIZE];

Exception exceptionArray[EXCEPTION_ARRAY_SIZE];
// Local reference of table, first initiated by add builtin exceptions
refTable* erTable = NULL;

uint32_t sourceCount = 0;
Chunk** cArray = NULL;
uint32_t chunkArraySize = 0;

void attachSource(char* s, char* sourceName) {
    sourceArray[sourceCount] = s;
    fileNameArray[sourceCount] = addReference(sourceName);
    sourceCount++;
}

void attachChunkArray(Chunk** ca, uint32_t size) {
    cArray = ca;
    chunkArraySize = size;
}

// Internal error
void nullSourceError() {
    fprintf(stderr, "\nnullSourceError: Source never attached\n");
    exit(EXIT_FAILURE);
}

void printSourceLocation(uint32_t line, uint32_t index, uint32_t sourceIndex) {
    if (sourceIndex >= sourceCount) nullSourceError();
    int lineCount = 0;
    char* ptr = sourceArray[sourceIndex];
    // Find the line
    while (lineCount < line) {
        if (lineCount >= line - 2) fprintf(stderr, "%c", *ptr);
        if (*ptr == '\n') lineCount++;
        ptr++;
    }
    // Print the line
    while (*ptr != '\n' && *ptr != '\0') {
        fprintf(stderr, "%c", *ptr);
        ptr++;
    }
    fprintf(stderr, "\n");
    for (int i = 0; i < index; i++) fprintf(stderr, " ");
    fprintf(stderr, "^\n");
    for (int i = 0; i < index; i++) fprintf(stderr, " ");
    fprintf(stderr, "In \"%s\": [line: %d, index %d]\n", fileNameArray[sourceIndex], line+1, index+1);
}

void printFrame(uint64_t* ip) {
    ip--;
    for (uint32_t i=0; i<chunkArraySize; i++) {
        if (ip >= cArray[i]->code && ip < cArray[i]->code + cArray[i]->count) {
            uint32_t offset = ip - cArray[i]->code;
            uint16_t line = cArray[i]->lines[offset];

            uint8_t index = cArray[i]->indices[offset];
            uint8_t sourceIndex = cArray[i]->sourceIndices[offset];
#ifdef PRINT_ERROR_OP
            printf("Current instruction: \n");
            printInstr(*ip, cArray[i]);
            printf("\n");
#endif
            printSourceLocation(line, index, sourceIndex);
            return;
        }
    }
    fprintf(stderr, "Instruction pointer not found in any chunk\n");
}

void printRuntimeTraceback() {
    fprintf(stderr, "Runtime traceback:\n");
    uint64_t*** currIpStackPtr = ipStackTop-1;
    uint32_t offset = currIpStackPtr - ipStack;
    while (1) {
        fprintf(stderr, "\nCall Frame [%u]:\n", offset--);
        printFrame(**currIpStackPtr);
        if (currIpStackPtr == ipStack) break;
        currIpStackPtr--;
    }
}

// Special error function for internal errors of exception manager
void exceptionManagerError(char *message) {
    fprintf(stderr, "\nexceptionManagerError: %s\n", message);
    if (isRuntime) printRuntimeTraceback();
    exit(EXIT_FAILURE);
}


void addException(char* name, bool fatal) {
    if (erTable == NULL) exceptionManagerError("Uninitiated global reference to exception table");
    if (refTableContains(erTable, name)) exceptionManagerError("Adding duplicate exception");
    if (erTable->numEntries >= EXCEPTION_ARRAY_SIZE) exceptionManagerError("Exception table overflow");
    uint16_t index = getRefIndex(erTable, name);
    // Set at array index
    exceptionArray[index].ID = index;
    exceptionArray[index].fatal = fatal;
}

uint16_t getExceptionID(char* name) {
    if (erTable == NULL) exceptionManagerError("Uninitiated global reference to exception table");
    return getRefIndex(erTable, name);
}

void addBuiltinExceptions(refTable* exceptionRefTable) {
    // Initiate the array
    for (uint32_t i=0; i<EXCEPTION_ARRAY_SIZE; i++) exceptionArray[i].ID = EXCEPTION_ARRAY_SIZE;
    // Add reference to builtin exception
    erTable = exceptionRefTable;
    addException("varError", true);
    addException("objHashError", true);
    addException("callableError", true);
    addException("objManagerError", true);
    addException("strHashError", true);
    addException("listError", true);
    addException("dictError", true);
    addException("setError", true);
    addException("runtimeError", false);
    addException("GCError", true);
}

void raiseException(char* name, char* message) {
    if (erTable == NULL) exceptionManagerError("Uninitiated global reference to exception table");
    if (!refTableContains(erTable, name)) exceptionManagerError("Exception could not be found");
    // Find the current exception
    uint32_t currErrorIndex = getRefIndex(erTable, name);
    Exception* currException = &exceptionArray[currErrorIndex];

    // Determine if exception is unrecoverable
    if (!currException->fatal) {
        // Todo: Try to handle it
        exceptionHandler* currHandler = vm->handlerStackTop -1;
        bool handled = false;
        while (currHandler >= vm->handlerStack) {
            if (currHandler->handlesAll) { // If handles all
                handled = true;
                break;
            } else { // Else, check type
                if (currHandler->type == currErrorIndex) {
                    handled = true;
                    break;
                }
            }
            currHandler--;
        }
        if (handled) {
            // Set vm to panic
            vm->panic = true;
            vm->targetIP = currHandler->ipLoc;
            vm->targetLine = currHandler->toLine;
            // Pop the handler batch
            vm->handlerStackTop -= currHandler->batchCount;
            return;
        }
    }
    // If not, just quit and print message
    // Print error message
    fprintf(stderr, "\n%s: %s\n", name, message == NULL ? "" : message);
    // Print traceback if possible
    if (isRuntime) printRuntimeTraceback();
    exit(EXIT_FAILURE);
}

// Pre-runtime errors
void parsingError(uint16_t line, uint8_t index, uint8_t sourceIndex, char *message) {
    fprintf(stderr, "\nparsingError: %s\n", message);
    printSourceLocation(line, index, sourceIndex);
    exit(EXIT_FAILURE);
}

void compilationError(uint16_t line, uint8_t index, uint8_t sourceIndex, char *message) {
    fprintf(stderr, "\ncompilationError: %s\n", message);
    printSourceLocation(line, index, sourceIndex);
    exit(EXIT_FAILURE);
}

// Final free function
void freeErrorTracer() {
    for (uint32_t i=0; i<sourceCount; i++) free(sourceArray[i]);
    if (cArray != NULL) free(cArray);
}
