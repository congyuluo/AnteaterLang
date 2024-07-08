//
// Created by congyu on 8/20/23.
//

#ifndef CJ_2_RUNTIMEMEMORYMANAGER_H
#define CJ_2_RUNTIMEMEMORYMANAGER_H

#include "object.h"


typedef struct RuntimeBlock RuntimeBlock;

struct RuntimeBlock {
    Object block[RUNTIME_BLOCK_SIZE];
    Object* freeStack[RUNTIME_BLOCK_SIZE];
    Object** freeStackTop;
    uint32_t availableSlots;
    uint16_t blockID;
    bool revived;
};

typedef struct PriorityQueue {
    RuntimeBlock** data;
    uint32_t size;
    uint32_t capacity;
} PriorityQueue;

void initMemoryManager();
void freeMemoryManager();

Object* newObjectSlot();

#endif //CJ_2_RUNTIMEMEMORYMANAGER_H
