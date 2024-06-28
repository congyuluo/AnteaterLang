//
// Created by congyu on 7/18/23.
//

#ifndef CJ_2_ERRORS_H
#define CJ_2_ERRORS_H

#include "chunk.h"
#include "refManager.h"
#include <stdbool.h>

extern uint64_t** ipStack[VM_LOCAL_REF_TABLE_STACK_INIT_SIZE];
extern uint64_t*** ipStackTop;
extern bool isRuntime;

void attachSource(char* s, char* sourceName);
void attachChunkArray(Chunk** ca, uint32_t size);

typedef struct Exception {
    uint16_t ID;
    bool fatal;
} Exception;

extern Exception exceptionArray[EXCEPTION_ARRAY_SIZE];

void addException(char* name, bool fatal);

uint16_t getExceptionID(char* name);

void addBuiltinExceptions(refTable* exceptionRefTable);

void raiseException(char* name, char* message);

void parsingError(uint16_t line, uint8_t index, uint8_t sourceIndex, char *message);

void compilationError(uint16_t line, uint8_t index, uint8_t sourceIndex, char *message);

void freeErrorTracer();

#endif //CJ_2_ERRORS_H
