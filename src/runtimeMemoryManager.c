//
// Created by congyu on 8/20/23.
//

#include "runtimeMemoryManager.h"
#include "errors.h"
#include "vm.h"

uint32_t blockIDCounter;

PriorityQueue* blockQueue;

runtimeDict* rtBlockDict;

Object* rtHead;

// Block Operations

static inline RuntimeBlock* newBlock() {
#ifdef PRINT_MEMORY_INFO
    printf("Allocating new block\n");
#endif
    RuntimeBlock* newBlock = (RuntimeBlock*) malloc(sizeof(RuntimeBlock));
    if (newBlock == NULL) raiseExceptionByName("ObjManagerError", "Memory allocation failed for new block");

    // Set block ID
    newBlock->blockID = blockIDCounter++;
    // Add block to runtime block dictionary
    dictInsertElement(rtBlockDict, NUMBER_VAL(newBlock->blockID), PTR_VAL(newBlock));

    // Set revived flag
    newBlock->revived = false;

    // Insert new free slots onto free stack
    Object** stackTop = newBlock->freeStack;
    Object* newSlot = (Object*) newBlock->block;
    for (int i = 0; i < RUNTIME_BLOCK_SIZE; i++) {
        // Set object's block id to the new block
        newSlot->blockID = newBlock->blockID;
        *stackTop++ = newSlot++;
    }

    // Update free stack top
    newBlock->freeStackTop = stackTop;
    // Set occupied slots
    newBlock->availableSlots = RUNTIME_BLOCK_SIZE;
    return newBlock;
}

static inline Object* allocateFromBlock(RuntimeBlock* block) {
    if (block->availableSlots == 0) {
        raiseExceptionByName("ObjManagerError", "Allocating from full block.");
    }
    block->availableSlots--;
    return *--block->freeStackTop;
}

static inline void deallocateFromBlock(RuntimeBlock* block, Object* slot) {
    if (block->availableSlots == RUNTIME_BLOCK_SIZE) {
        raiseExceptionByName("ObjManagerError", "Deallocating from empty block.");
    } else if (block->availableSlots == 0) {
        // Set revived flag
        block->revived = true;
    }
    *block->freeStackTop++ = slot;
    block->availableSlots++;
}

// Priority Queue Operations

void printManagementDS() {
    printf("Priority Queue: \n[");
    for (uint32_t i = 0; i < blockQueue->size; i++) {
        if (i > 0) printf(", ");
        printf("Block #%u: %u", blockQueue->data[i]->blockID, blockQueue->data[i]->availableSlots);
    }
    printf("]\n");
    printf("RT Block Dict: \n[");
    uint32_t count = 0;
    for (uint32_t i=0; i < rtBlockDict->tableSize; i++) {
        runtimeDictEntry* entry = rtBlockDict->entries[i];
        while (entry) {
            RuntimeBlock* currBlock = (RuntimeBlock*) VALUE_PTR_VAL(entry->value);
            if (count++ > 0) printf(", ");
            printf("Block #%u: %u", currBlock->blockID, currBlock->availableSlots);
            entry = entry->next;
        }
    }
    printf("]\n");
}

static inline void swap(RuntimeBlock** a, RuntimeBlock** b) {
    RuntimeBlock* temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(uint32_t index) {
    while (index > 0) {
        uint32_t parent = (index - 1) / 2;
        if (blockQueue->data[parent]->availableSlots <= blockQueue->data[index]->availableSlots) break;
        swap(&blockQueue->data[parent], &blockQueue->data[index]);
        index = parent;
    }
}

void heapifyDown(uint32_t index) {
    uint32_t left, right, smallest;
    while (1) {
        left = 2 * index + 1;
        right = 2 * index + 2;
        smallest = index;

        if (left < blockQueue->size && blockQueue->data[left]->availableSlots < blockQueue->data[smallest]->availableSlots) {
            smallest = left;
        }
        if (right < blockQueue->size && blockQueue->data[right]->availableSlots < blockQueue->data[smallest]->availableSlots) {
            smallest = right;
        }
        if (smallest == index) break;

        swap(&blockQueue->data[smallest], &blockQueue->data[index]);
        index = smallest;
    }
}

void initPriorityQueue() {
    blockQueue = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    if (blockQueue == NULL) raiseExceptionByName("ObjManagerError", "Memory allocation failed for priority queue.");
    blockQueue->data = (RuntimeBlock**) malloc(sizeof(RuntimeBlock*) * INITIAL_PRIORITY_QUEUE_CAPACITY);
    if (blockQueue->data == NULL) raiseExceptionByName("ObjManagerError", "Memory allocation failed for priority queue data.");
    blockQueue->size = 0;
    blockQueue->capacity = INITIAL_PRIORITY_QUEUE_CAPACITY;
}

static inline void freePriorityQueue() {
    free(blockQueue->data);
    free(blockQueue);
}

void resizePriorityQueue() {
    blockQueue->capacity *= PRIORITY_QUEUE_GROWTH_FACTOR;
    blockQueue->data = (RuntimeBlock**) realloc(blockQueue->data, sizeof(RuntimeBlock*) * blockQueue->capacity);
    if (blockQueue->data == NULL) raiseExceptionByName("ObjManagerError", "Memory reallocation failed for priority queue data.");
}

void pqAddBlock(RuntimeBlock* block) {
    if (blockQueue->size == blockQueue->capacity) {
        resizePriorityQueue();
    }
    blockQueue->data[blockQueue->size] = block;
    heapifyUp(blockQueue->size);
    blockQueue->size++;
#ifdef PRINT_BLOCK_ORDER
    printf("Block %d added to priority queue\n", block->blockID);
    printManagementDS();
#endif
}

static inline void reHeapify() {
    for (uint32_t i = (blockQueue->size / 2) - 1; i >= 0; i--) heapifyDown(i);
}

void updateBlock(uint32_t blockID, uint32_t newAvailableSlots) {
    for (uint32_t i = 0; i < blockQueue->size; i++) {
        if (blockQueue->data[i]->blockID == blockID) {
            blockQueue->data[i]->availableSlots = newAvailableSlots;
            heapifyUp(i);
            heapifyDown(i);
            return;
        }
    }
}

void pqRemoveBlock(uint32_t blockID) {
    int index = -1;
    for (uint32_t i = 0; i < blockQueue->size; i++) {
        if (blockQueue->data[i]->blockID == blockID) {
            index = (int) i;
            break;
        }
    }
    if (index == -1) raiseExceptionByName("ObjManagerError", "Block not found in priority queue.");

    blockQueue->data[index] = blockQueue->data[blockQueue->size - 1];
    blockQueue->size--;
    heapifyDown(index);
#ifdef PRINT_BLOCK_ORDER
    printf("Block %d removed from priority queue\n", blockID);
    printManagementDS();
#endif
}

static inline RuntimeBlock* getTopBlock(PriorityQueue* pq) {
    if (pq->size == 0) raiseExceptionByName("ObjManagerError", "Getting top block from empty priority queue.");
    return pq->data[0];
}

// Memory Manager Operations

void initMemoryManager() {
    // Init priority queue
    initPriorityQueue();
    // Set block ID counter
    blockIDCounter = 0;
    // Initialize runtime block dictionary
    rtBlockDict = createRuntimeDict(RUNTIME_DICT_INIT_SIZE);
    // Create initial block & Insert initial block into priority queue
    pqAddBlock(newBlock());
    // Init runtime head
    rtHead = NULL;
}

void freeMemoryManager() {
    // Free priority queue
    freePriorityQueue();
    for (uint32_t i=0; i < rtBlockDict->tableSize; i++) {
        runtimeDictEntry* entry = rtBlockDict->entries[i];
        while (entry) {
            RuntimeBlock* currBlock = (RuntimeBlock*) VALUE_PTR_VAL(entry->value);
            free(currBlock);
            entry = entry->next;
        }
    }
    // Free runtime block dictionary
    freeRuntimeDict(rtBlockDict);
}

// Forward declaration
static inline void collectGarbage();

Object* newObjectSlot() {
    // First, check is priority queue is empty
    if (blockQueue->size == 0) collectGarbage();
    // If priority queue is still empty, allocate new block
    if (blockQueue->size == 0) pqAddBlock(newBlock());
    // Get top block
    RuntimeBlock* topBlock = getTopBlock(blockQueue);
    // Allocate from top block
    Object* newSlot = allocateFromBlock(topBlock);
#ifdef PRINT_MEMORY_INFO
    printf("Allocated new object slot from block %d\n", topBlock->blockID);
#endif
    // Check if the block is full
    if (topBlock->availableSlots == 0) {
        // Remove the block from priority queue
        pqRemoveBlock(topBlock->blockID);
    }
    newSlot->next = rtHead;
    rtHead = newSlot;
    return newSlot;
}

// Print function
void printRTLL() {
    Object* currObj = rtHead;
    while (currObj != NULL) {
        printObject(currObj);
        printf("\n");
        currObj = currObj->next;
    }
}

// Forward declaration
static inline void iterateValue(Value val);

void iterateList(Value val) {
    runtimeList* list = VALUE_LIST_VALUE(val);
    Value* currValPtr = list->list;
    for (int i=0; i<list->size; i++) {
        Value currVal = *currValPtr;
        if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
            Object* currObj = VALUE_OBJ_VAL(currVal);
            if (!(currObj->isConst || currObj->marked)) {
                currObj->marked = true;
                if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
            }
        }
        currValPtr++;
    }
}

void iterateDict(Value val) {
    runtimeDict* dict = VALUE_DICT_VALUE(val);
    for (uint32_t i=0; i < dict->tableSize; i++) {
        runtimeDictEntry* entry = dict->entries[i];
        while (entry) {
            Value currVal = entry->key;
            if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
                Object* currObj = VALUE_OBJ_VAL(currVal);
                if (!(currObj->isConst || currObj->marked)) {
                    currObj->marked = true;
                    if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
                }
            }
            currVal = entry->value;
            if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
                Object* currObj = VALUE_OBJ_VAL(currVal);
                if (!(currObj->isConst || currObj->marked)) {
                    currObj->marked = true;
                    if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
                }
            }
            entry = entry->next;
        }
    }
}

void iterateSet(Value val) {
    runtimeSet* set = VALUE_SET_VALUE(val);
    runtimeDict* dict = set->dict;
    for (uint32_t i=0; i < dict->tableSize; i++) {
        runtimeDictEntry* entry = dict->entries[i];
        while (entry) {
            Value currVal = entry->key;
            if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
                Object* currObj = VALUE_OBJ_VAL(currVal);
                if (!(currObj->isConst || currObj->marked)) {
                    currObj->marked = true;
                    if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
                }
            }
            entry = entry->next;
        }
    }
}

void iterateStrObjHashTable(strValueHash* table) {
    for (uint32_t i=0; i < table->table_size; i++) {
        strValueEntry* entry = table->entries[i];
        while (entry != NULL) {
            Value currVal = entry->value;
            if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
                Object* currObj = VALUE_OBJ_VAL(currVal);
                if (!(currObj->isConst || currObj->marked)) {
                    currObj->marked = true;
                    if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
                }
            }
            entry = entry->next;
        }
    }
}

static inline void iterateValue(Value val) {
    if (!IS_SYSTEM_DEFINED_TYPE(val.type)) {
        iterateStrObjHashTable(VALUE_ATTRS(val));
        return;
    }
    // Runtime data structure attributes
    switch (val.type) {
        case BUILTIN_LIST:
            iterateList(val);
            break;
        case BUILTIN_DICT:
            iterateDict(val);
            break;
        case BUILTIN_SET:
            iterateSet(val);
            break;
        default:
            raiseExceptionByName("GCError", "Invalid value type for iteration");
    }
}

static inline void markObject() {
    VM* currVM = vm;
    // Iterate stack
    Value* currStackPtr = currVM->stack;
    Value currVal = *currStackPtr;
    while (currStackPtr != currVM->stackTop) {
        if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
            Object* currObj = VALUE_OBJ_VAL(currVal);
            if (!(currObj->isConst || currObj->marked)) {
                currObj->marked = true;
                if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
            }
        }
        currVal = *currStackPtr++;
    }
    // Iterate global ref array
    currStackPtr = currVM->globalRefArray;
    currVal = *currStackPtr;
    for (int i=0; i<vm->globalRefCount; i++) {
        if (!IS_INTERNAL_NULL(currVal) && IS_MARKABLE_VAL(currVal)) {
            Object* currObj = VALUE_OBJ_VAL(currVal);
            if (!(currObj->isConst || currObj->marked)) {
                currObj->marked = true;
                if (IS_ITERABLE_VAL(currVal)) iterateValue(currVal);
            }
        }
        currVal = *currStackPtr++;
    }
}

static inline uint32_t sweepObject() {
    uint32_t removedCount = 0;
#ifdef PRINT_GC_REMOVAL
    uint32_t totalCount = 0;
#endif

    Object* currObj = rtHead;
    Object* prevObj = NULL;
    while (currObj != NULL) {
        if (currObj->marked) { // Marked object
            // Unmark object
            currObj->marked = false;
            // Move to next object
            prevObj = currObj;
            currObj = currObj->next;
        } else { // Unmarked object
#ifdef PRINT_GC_REMOVAL
            printf("Removed Object [%u]: ", totalCount);
            printObject(currObj);
            printf("\n");
#endif
            removedCount++;
            // LL delete
            if (prevObj == NULL) {
                rtHead = currObj->next;
            } else {
                prevObj->next = currObj->next;
            }
            Object* nextObj = currObj->next;

            // Find block in which object is allocated
            RuntimeBlock* block = (RuntimeBlock*) VALUE_PTR_VAL(dictGetElement(rtBlockDict, NUMBER_VAL(currObj->blockID)));
            // Deallocate object
            deallocateFromBlock(block, currObj);
            currObj = nextObj;
        }
#ifdef PRINT_GC_REMOVAL
        totalCount++;
#endif
    }
#ifdef PRINT_GC_INFO
    printf("GC removed %u objects\n", removedCount);
#endif
    return removedCount;
}

static inline uint32_t determineFreeCount(uint32_t totalBlockCount, uint32_t freeBlockCount) {
    // Heuristic to determine number of blocks to free
    return freeBlockCount - MAX_ALLOWED_EMPTY_BLOCK;
}

static inline void freeUnusedBlocks() {
    // Create runtime list to store empty blocks
    runtimeList* emptyList = createRuntimeList(RUNTIME_LIST_INIT_SIZE);

    // Iterate to find empty blocks
    for (uint32_t i=0; i < rtBlockDict->tableSize; i++) {
        runtimeDictEntry* entry = rtBlockDict->entries[i];
        while (entry) {
            RuntimeBlock* currBlock = (RuntimeBlock*) VALUE_PTR_VAL(entry->value);
            if (currBlock->availableSlots == RUNTIME_BLOCK_SIZE) {
                // Add to empty list
                listAddElement(emptyList, PTR_VAL(currBlock));
            }
            entry = entry->next;
        }
    }

    // Determine number of blocks to free
    uint32_t targetFreeCount = determineFreeCount(rtBlockDict->numEntries, emptyList->size);

    if (targetFreeCount > 0) {
        // Free blocks
        // Iterate list to find empty blocks
        uint32_t freedCount = 0;

        Value* listValueArray = emptyList->list;
        for (uint32_t i=0; i < emptyList->size; i++) {
            RuntimeBlock* currBlock = (RuntimeBlock*) VALUE_PTR_VAL(listValueArray[i]);
            // Free block
            free(currBlock);
            freedCount++;
            if (freedCount >= targetFreeCount) break;
        }
    }

    // Free runtime list
    freeRuntimeList(emptyList);
}

static inline void reviveBlocks() {
    // Reorder block queue
    reHeapify();
    // Iterate to find possibly revived blocks
    for (uint32_t i=0; i < rtBlockDict->tableSize; i++) {
        runtimeDictEntry* entry = rtBlockDict->entries[i];
        while (entry) {
            RuntimeBlock* currBlock = (RuntimeBlock*) VALUE_PTR_VAL(entry->value);
            if (currBlock->revived) {
                // Reset flag
                currBlock->revived = false;
                // Insert into heap
                pqAddBlock(currBlock);
            }
            entry = entry->next;
        }
    }
}

static inline void collectGarbage() {
    // Mark and sweep
    markObject();
    // Sweep
    if (sweepObject() > 0) reviveBlocks();
    // Free unused blocks
    freeUnusedBlocks();
#ifdef PRINT_BLOCK_ORDER
    printf("GC Cleaned\n");
    printManagementDS();
#endif
}
