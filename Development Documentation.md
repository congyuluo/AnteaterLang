# Development Documentation of CJLang

## How to Build user-defined C functions

```
gcc -fPIC -shared -o userFunctions.so userFunctions.c
```

### The following content is generated with Copilot Chat, and may not be accurate.

## Startup Sequence of the Program

1. The program starts with the `main` function in `main.c`.
2. It checks the number of command-line arguments. If there are 2 arguments, it assumes the second one is the path to the source file. If there are 3 arguments, it assumes the first one is the path to a library and the second one is the path to the source file.
3. It loads the source file into memory.
4. It initializes all tables by calling `initAllTables()`, which includes initializing the string hash and object manager.
5. It attaches the source file to the error handler.
6. It creates reference tables and runtime lists for global references and classes.
7. It constructs built-in classes.
8. If a library path was provided, it loads user-defined functions from the library.
9. It initializes the tokenizer with the source file.
10. It compiles the source code, which also prepares the prelinked function array and global array.
11. If the main function of the source code expects input arguments, it prepares an array of arguments.
12. It initializes the VM with the global array, prelinked function array, and the size of the global array.
13. It runs the VM with the main function and its input arguments.
14. After the VM finishes running, it cleans up by deleting all tables and closing the library if one was loaded.

## Linking User Functions and Defining Built-in Functions and Classes

## Linking User Functions as a Library

1. The program checks if a library path was provided as a command-line argument. If it was, it proceeds with the following steps.
2. The library path is passed to the `dlopen` function, which opens the dynamic library for use in the program.
3. If the library fails to load, an error message is printed and the program exits.
4. The `dlsym` function is used to locate the `userFuncs` symbol in the dynamic library. This symbol should point to an array of user-defined functions.
5. If the `userFuncs` symbol cannot be found, an error message is printed, the library is closed, and the program exits.
6. The `dlsym` function is also used to locate the `funcCount` symbol in the dynamic library. This symbol should point to a `uint32_t` that specifies the number of user-defined functions in the `userFuncs` array.
7. If the `funcCount` symbol cannot be found, an error message is printed, the library is closed, and the program exits.
8. The `loadUserFunctions` function is called with the global reference table, global reference list, `userFuncs` array, and function count as arguments. This function adds the user-defined functions to the global reference table and list.

## Defining Built-in Functions and Classes in `builtinClasses.c`

1. The `constructBuiltinClasses` function is called with the global reference table, global reference list, and global class reference table as arguments.
2. This function defines all built-in classes and their methods. The classes are added to the global class reference table, and their methods are added to the global reference table and list.
3. The built-in classes include `Array`, `Dict`, `Set`, `Str`, `Int`, `Float`, `Bool`, `Null`, `Function`, and `Class`.
4. Each class has an `init` method that initializes the class. Some classes also have additional methods. For example, the `Array` class has `add`, `remove`, `get`, and `set` methods.

## The function template for user-defined and built-in functions

The function template `Value function(Value self, Value* args, int numArgs)` is a common pattern used in the codebase for defining methods of built-in classes. Here's a breakdown of the parameters:

- `Value self`: This is the object that the method is being called on. In the context of object-oriented programming, `self` is similar to `this` in C++ or Java. It refers to the object that the method is being called on.

- `Value* args`: This is an array of arguments passed to the function. The number of arguments can vary depending on the specific function.

- `int numArgs`: This is the number of arguments passed to the function. This parameter is used to iterate over the `args` array.

The return type of the function is `Value`, which means these functions can return a `Value` object. This `Value` object can be of any type defined in the `Value` union, including numbers, booleans, strings, lists, dictionaries, sets, and user-defined objects.

This function template is used for defining methods of built-in classes, such as `listAdd`, `listInsert`, `listSet`, `listRemove`, `listGet`, `listContains`, `listIndexOf`, and `listSize` for the `listClass`. These methods are added to the class using the `CLASS_ADD_ATTR` macro, which adds the method to the class's attribute table.'

## Value & Types

In this programming language, `Value` is a struct that represents a value in the language. It can hold different types of values, including numbers, booleans, and objects. The specific type of the value is determined by the `type` field in the `Value` struct, which is a `uint16_t`.

The `ValueType` enum is used to represent the different types that a `Value` can hold. Here's a breakdown of the enum values:

- `VAL_INTERNAL_NULL`: This is an internal null value, used for internal purposes in the language.
- `VAL_NONE`: This represents a none value, similar to `None` in Python or `null` in Java.
- `VAL_BOOL`: This represents a boolean value, either true or false.
- `VAL_NUMBER`: This represents a number, which is a double in this language.
- `BUILTIN_CALLABLE`: This represents a callable object, such as a function or method.
- `BUILTIN_STR`: This represents a string.
- `BUILTIN_LIST`: This represents a list.
- `BUILTIN_DICT`: This represents a dictionary.
- `BUILTIN_SET`: This represents a set.

The enum values from `BUILTIN_CALLABLE` to `BUILTIN_SET` represent built-in object types in the language. These are objects that are built into the language and have special behavior.

If a user defines their own classes, they will be assigned an enum integer value starting from 9 and increasing for each additional user-defined class. This allows the language to distinguish between built-in types and user-defined types, and to handle them appropriately. The `IS_SYSTEM_DEFINED_TYPE(t)` macro checks if a type is a built-in type by checking if its enum value is less than 9.

## Object
In this programming language, `Value` and `Object` are two fundamental structures that are used to represent data.

## Value

`Value` is a struct that represents a value in the language. It can hold different types of values, including numbers, booleans, and objects. The specific type of the value is determined by the `type` field in the `Value` struct, which is a `uint16_t`.

Here is the original definition of `Value`:

```c
struct Value {
    union {
        Object* obj;
        double num;
        bool boolean;
    };
    uint16_t type;
};
```

The `Value` struct uses a union to hold the actual value. This means that the `Value` struct can hold either an `Object*`, a `double`, or a `bool`, but not all at the same time. The `type` field is used to determine which member of the union is currently being used.

## Object

`Object` is a struct that represents an object in the language. It has a `primValue` field which is a union that can hold different types of primary values, including strings, callables, lists, dictionaries, sets, and after-definition attributes. The specific type of the primary value is determined by the `type` field in the `Object` struct, which is a `uint16_t`.

Here is the original definition of `Object`:

```c
struct Object {
    union {
        char* str;
        callable* call;
        runtimeList* list;
        runtimeDict* dict;
        runtimeSet* set;
        strValueHash *afterDefAttributes;
    } primValue;
    Object* next;
    uint16_t type;
    uint16_t blockID;
    bool marked;
    bool isConst;
};
```

The `Object` struct uses a union to hold the primary value. This means that the `Object` struct can hold one of several types of values, but not all at the same time. The `type` field is used to determine which member of the union is currently being used.

## Relationship between Value and Object
Within the language, `Value` is the default method of handling any data, including objects.

The relationship between `Value` and `Object` is that a `Value` can hold an `Object`. When the `type` field of a `Value` is set to one of the object types (from `BUILTIN_CALLABLE` to `BUILTIN_SET`), the `obj` field of the `Value` holds a pointer to an `Object`. This `Object` then holds the actual data for the object, such as the elements of a list or the entries of a dictionary.

## String Hash & Related Memory Management
In this programming language, the `HashTable` structure is used to manage strings, specifically through the `addReference` and `removeReference` functions.

When a string is added to the hash table (via the `addReference` function), the function first checks if the string already exists in the hash table. If it does, it increments the reference count for that string. If it doesn't, it creates a new entry in the hash table with a reference count of 1. Importantly, when creating this new entry, the function makes a copy of the string. This is done in the `internalInsert` function, which is called by `addReference` when the string is not found in the hash table.

The reason for creating a copy of the string is to ensure that the hash table has its own, independent version of the string. This allows the original string to be modified or freed without affecting the string in the hash table. This is crucial for proper memory management, especially in a garbage collected language where memory can be automatically freed when it is no longer in use.

On the other hand, the `removeReference` function is used to decrement the reference count of a string in the hash table. If the reference count reaches zero, it means the string is no longer in use and can be safely removed from the hash table. This is done in the `delete` function, which is called by `removeReference` when the reference count reaches zero. The `delete` function removes the entry from the hash table and frees the memory occupied by the copied string and the entry itself.

In summary, the `HashTable` structure and its associated functions provide a robust system for managing strings and their memory in this programming language. By creating copies of strings when they are added to the hash table, the language ensures that strings in the hash table are not affected by changes to the original strings, providing a level of safety and predictability.

## Tokens

In a programming language, tokens are the smallest units of a program that have a meaning associated with them. They are the building blocks of a source code file. The process of breaking down a source code file into these tokens is called lexical analysis or tokenization.

There are several types of tokens, including:

1. **Identifiers**: These are the names used for variables, functions, classes, and other user-defined items.

2. **Keywords**: These are reserved words that have special meaning in the language. Examples include `if`, `else`, `for`, `while`, etc.

3. **Operators**: These are symbols that perform operations on one or more operands. Examples include `+`, `-`, `*`, `/`, `=`, etc.

4. **Literals**: These are the fixed values in the source code, such as numbers, strings, and booleans.

5. **Punctuation**: These are symbols that are used to organize the code but don't perform operations. Examples include `{`, `}`, `(`, `)`, `;`, etc.

Each of these tokens plays a different role in the program, and understanding these roles is key to understanding how the program works.

## OpCodes

Created by the compiler, and executed by the virtual machine, opcodes are the instructions that make up the bytecode of a program. Each opcode represents a specific operation that the virtual machine can perform. The virtual machine reads these opcodes one by one and executes the corresponding operation for each one.

The `OpCode` enum in the `chunk.h` file defines the set of operation codes (opcodes) that are used in the bytecode of this stack-based programming language. Each opcode represents a different operation that can be performed. Here's a brief description of each opcode:

- `OP_CONSTANT`: Pushes a constant value onto the stack.
- `OP_ADD`, `OP_SUB`, `OP_MUL`, `OP_DIV`, `OP_MOD`: Perform arithmetic operations (addition, subtraction, multiplication, division, modulus respectively) on the top two elements of the stack.
- `OP_LESS`, `OP_MORE`, `OP_LESS_EQUAL`, `OP_MORE_EQUAL`: Perform comparison operations (less than, greater than, less than or equal to, greater than or equal to respectively) on the top two elements of the stack.
- `OP_NEGATE`: Negates the top element of the stack.
- `OP_NOT`: Performs a logical NOT operation on the top element of the stack.
- `OP_EQUAL`, `OP_NOT_EQUAL`: Perform equality checks (equal to, not equal to respectively) on the top two elements of the stack.
- `OP_AND`, `OP_OR`: Perform logical operations (AND, OR respectively) on the top two elements of the stack.
- `OP_POW`: Raises the second element from the top of the stack to the power of the top element of the stack.
- `OP_IS`: Checks if the top two elements of the stack are the same object.
- `OP_GET_SELF`: Pushes the current instance (self) onto the stack.
- `OP_GET_INDEX_REF`, `OP_GET_GLOBAL_REF_ATTR`, `OP_GET_LOCAL_REF_ATTR`, `OP_GET_COMBINED_REF_ATTR`, `OP_GET_ATTR`, `OP_GET_ATTR_CALL`: These opcodes are used to get references to variables, attributes, or indices.
- `OP_SET_GLOBAL_REF_ATTR`, `OP_SET_LOCAL_REF_ATTR`, `OP_SET_COMBINED_REF_ATTR`, `OP_SET_INDEX_REF`, `OP_SET_ATTR`: These opcodes are used to set the values of variables, attributes, or indices.
- `OP_EXEC_FUNCTION_ENFORCE_RETURN`, `OP_EXEC_FUNCTION_IGNORE_RETURN`, `OP_EXEC_METHOD_ENFORCE_RETURN`, `OP_EXEC_METHOD_IGNORE_RETURN`: These opcodes are used to call functions or methods, with or without enforcing a return value.
- `OP_INIT`, `OP_GET_PARENT_INIT`: These opcodes are used for object initialization.
- `OP_RETURN`, `OP_RETURN_NONE`: These opcodes are used to return from a function, with or without a return value.
- `OP_JUMP`, `OP_JUMP_IF_FALSE`: These opcodes are used to control the flow of execution by jumping to different parts of the code.

These opcodes form the basis of the bytecode for this stack-based programming language. The exact behavior of each opcode will depend on the implementation in the virtual machine that executes the bytecode.

## objArray & Chunk

The `objArray` and `Chunk` structures are key components of the bytecode representation in this stack-based programming language.

The design of the bytecode is 64-bit, for performance reasons.

```c
typedef struct objArray {
    int count;
    int capacity;
    Value* data;
} valueArray;
```

```c
typedef struct Chunk {
uint32_t count;
uint32_t capacity;
uint8_t currIndexAtLine; // Current index of the line within the 64 bits
uint16_t localRefArraySize;
uint64_t* code;
uint16_t* lines;
uint8_t* indices;
uint8_t* sourceIndices;
valueArray* constants;
} Chunk;
```

The `objArray` structure is a dynamic array that holds `Value` objects. It has three fields:

- `count`: The number of elements currently in the array.
- `capacity`: The total number of elements the array can hold before it needs to be resized.
- `data`: A pointer to the array of `Value` objects.

The `Value` type represents a value that can be manipulated by the bytecode. It could be a number, a string, a boolean, or any other type of data that the language supports.

The `Chunk` structure represents a chunk of bytecode, which corresponds to a function in the source code. It contains several fields:

- `count`, `capacity`: The number of bytes in the bytecode and the total number of bytes the bytecode can hold before it needs to be resized, respectively.
- `currIndexAtLine`: The current index of the line within the 64 bits.
- `localRefArraySize`: The size of the local reference array.
- `code`: A pointer to the array of bytes that make up the bytecode.
- `lines`: An array that maps each byte of bytecode to the line number in the source code where it was generated.
- `indices`: An array of indices.
- `sourceIndices`: An array of source indices.
- `constants`: A pointer to an `objArray` that holds the constant values that are used in the bytecode.

When a function is compiled, its code is translated into bytecode and stored in a `Chunk`. Any constant values used in the function are added to the `constants` `objArray` in the `Chunk`. The `lines` array is filled in with the line numbers from the source code, so that if an error occurs while the bytecode is being executed, the error message can indicate the line in the source code where the problem occurred.

The `Chunk` structure also includes several functions for manipulating chunks of bytecode, such as `writeChunk4`, `writeChunk8`, `writeChunk16`, and `writeChunk32`, which write a 4-bit, 8-bit, 16-bit, or 32-bit value to the bytecode, respectively. There are also functions for writing operation codes (`writeOp`), writing constant values (`writeValConstant`), and handling jumps (`writeJump`, `patchJump`, `patchJumpAtCurrent`, `writeJumpBack`).

In summary, the `objArray` and `Chunk` structures work together to represent a function in bytecode form, with the `Chunk` holding the bytecode and the `objArray` holding the constant values used in the bytecode.

## Compilation

The `compiler.c` file contains the implementation of a Pratt parser for a programming language. A Pratt parser is a top-down operator-precedence parser that extends the idea of precedence parsing to include both infix and prefix operators. It provides a way to specify the grammar of a language in a way that is easy to understand and modify.

Here's a high-level overview of the compilation process in `compiler.c`:

1. **Tokenization**: The source code is first tokenized into a sequence of tokens. This is done by the `tokenize` function, which reads the source code character by character and groups them into tokens.

2. **Parsing**: The tokens are then parsed into an abstract syntax tree (AST) using a Pratt parser. The Pratt parser uses a table-driven approach, where each token type has associated parsing functions that know how to parse that type of token in both prefix and infix positions. This is done by the `parsePrecedence` function, which takes a precedence level as an argument and parses all operators with a higher precedence.

3. **Code Generation**: Once the tokens are built, it is walked to generate bytecode. The bytecode is a lower-level representation of the source code that can be executed by a virtual machine. Each node in the AST knows how to generate its corresponding bytecode. This is done by the `expression`, `statement`, and other similar functions.

4. **Global Reference Table**: The compiler maintains a global reference table (`globalRefTable`), which is a symbol table that maps identifiers to their values. This table is used to look up the values of variables during code generation.

5. **Function and Method Definitions**: The compiler handles function and method definitions by creating a new chunk of bytecode for each function or method, and storing it in the global reference table. This is done by the `defFunction` and `defMethod` functions.

6. **Class Definitions**: The compiler handles class definitions by creating a new class object for each class, and storing it in the global class reference table (`globalClassRefTable`). This is done by the `defClass` function.

7. **Error Handling**: If an error is encountered at any point during the compilation process, an error message is printed and the compilation process is halted. This is done by the `compilationError` function.

8. **Optimization**: After the bytecode is generated, the compiler performs some optimizations to improve the performance of the resulting code. This includes removing unused variables and optimizing arithmetic operations.

9. **Linking**: Finally, the compiler links all the chunks of bytecode together into a single executable program. This includes resolving function calls to their correct addresses.

The result of the compilation process is a chunk of bytecode that can be executed by the virtual machine.

## Virtual Machine & Execution

The `vm.c` file typically contains the implementation of the virtual machine (VM) that executes the bytecode generated by the compiler. The VM is responsible for interpreting the bytecode and performing the operations specified by the opcodes.

Here's a high-level overview of how the VM might work:

1. **Initialization**: The VM is initialized with the bytecode to be executed. This could involve setting up the stack, initializing registers, and setting the program counter to the start of the bytecode.

2. **Bytecode Execution Loop**: The VM enters a loop where it repeatedly fetches the next instruction from the bytecode, decodes it, and executes it. This is often called the "fetch-decode-execute" cycle.

3. **Instruction Fetch**: The VM fetches the next instruction to be executed. This is typically the opcode pointed to by the program counter.

4. **Instruction Decode**: The VM decodes the fetched instruction to determine what operation to perform. This could involve looking up the opcode in a table of opcode handlers.

5. **Instruction Execute**: The VM executes the decoded instruction. This could involve performing an arithmetic operation, pushing or popping the stack, jumping to a different part of the bytecode, or performing a function call.

6. **Stack Operations**: Many of the operations performed by the VM involve the stack. For example, the VM might push operands onto the stack before an operation and pop the result off the stack after the operation.

7. **Function Calls**: The VM handles function calls by pushing the return address and arguments onto the stack, then jumping to the bytecode for the called function. When the function returns, the VM pops the return value off the stack and jumps back to the return address.

8. **Error Handling**: If the VM encounters an error during execution (such as an undefined opcode or a stack underflow), it halts execution and reports the error.

9. **Cleanup**: After the VM has finished executing the bytecode (or if an error occurs), it performs cleanup, such as freeing any memory it allocated.

Please note that the exact details of how the VM works can vary depending on the specific implementation. For a more detailed understanding, you would need to examine the source code of the `vm.c` file.

## Runtime Memory Management
The `runtimeMemoryManager.c` file is responsible for managing the memory used by the program at runtime. It includes functions for initializing and freeing the memory manager, creating new memory blocks, and allocating new object slots. It also includes functions for garbage collection, which involves marking and sweeping objects.

Here's a high-level overview of the key components:

1. **RuntimeMemoryManager**: This is a structure that manages the memory used by the program. It includes a stack of free memory slots and a linked list of memory blocks.

2. **newBlock**: This function creates a new memory block and adds it to the linked list of blocks in the memory manager. It also adds the new memory slots in the block to the free stack.

3. **initMemoryManager**: This function initializes the memory manager by allocating memory for it and setting up the free stack and the linked list of blocks. It also creates the first memory block.

4. **freeMemoryManager**: This function frees the memory used by the memory manager. It does this by freeing each memory block in the linked list, and then freeing the memory manager itself.

5. **newObjectSlot**: This function allocates a new object slot. If the free stack is empty, it first attempts to collect garbage. If the free stack is still empty after garbage collection, it creates a new memory block. It then pops a slot from the free stack and returns it.

6. **collectGarbage**: This function performs garbage collection. It first marks all reachable objects, and then sweeps all unmarked objects.

7. **markObject**: This function marks all reachable objects. It does this by iterating over the stack and the global reference array, and marking each object that it finds.

8. **sweepObject**: This function sweeps all unmarked objects. It does this by iterating over the linked list of objects, and removing each unmarked object that it finds.

9. **iterateValue, iterateList, iterateDict, iterateSet, iterateStrObjHashTable**: These functions are used to iterate over different types of values and mark all reachable objects. They are used by the `markObject` function during garbage collection.

This file implements a form of manual memory management, where memory is explicitly allocated and deallocated, and a form of garbage collection, where unreachable objects are automatically reclaimed. This can help to prevent memory leaks and make the program more efficient.

## Constant Memory Management

The `objectManager.c` file contains functions for managing objects in the program. It includes functions for creating constant objects, which are objects that are created prior to program execution and do not change during the program's runtime.

Here's a high-level overview of how constants are stored prior to program execution:

1. **initObjectManager**: This function initializes the object manager by initializing the class array and creating the constant list. The constant list is a data structure that stores all the constant objects created prior to program execution.

2. **createConstObj**: This function creates a new constant object of a given class. It first checks if the program is in runtime, and if it is, it throws an error because constant objects cannot be created during runtime. Then, it adds the new object to the constant list and initializes its fields. The object's `isConst` field is set to `true` to indicate that it is a constant object.

3. **createConstCallableObject, createConstStringObject, createConstListObject, createConstDictObject, createConstSetObject**: These functions create new constant objects of specific types. They call the `createConstObj` function to create a new constant object, and then set the object's value to the given value.

The constant objects are stored in the constant list, which is created during the initialization of the object manager. The constant list is a data structure that allows the program to quickly access all the constant objects created prior to program execution. This can be useful for tasks such as garbage collection and serialization.

The constant objects are immutable, meaning their values cannot be changed once they are created. This is enforced by the `createConstObj` function, which sets the `isConst` field of the object to `true`. Any attempt to change the value of a constant object during runtime should result in an error.

## Runtime Objects & Constant Objects

In the context of the provided `objectManager.c` code, the difference between runtime objects and constants lies in their lifecycle and mutability.

**Runtime Objects**: These are objects that are created during the execution of the program, i.e., at runtime. They are mutable, meaning their values can be changed during the program's execution. The `createRuntimeObj` function is used to create a new runtime object. It first checks if the program is in runtime, and if it is, it creates a new object and sets its `isConst` field to `false` to indicate that it is a runtime object.

In the provided `runtimeMemoryManager.c` code, all runtime objects have a `next` field. This field is used to create a singly linked list of all runtime objects. This linked list is used during the garbage collection process, specifically during the mark and sweep phases.

In the **mark phase**, the garbage collector traverses the linked list of runtime objects and marks all objects that are still reachable from the root set. The root set typically includes global variables, variables on the stack, and other references that are directly accessible.

In the **sweep phase**, the garbage collector again traverses the linked list of runtime objects. This time, it reclaims the memory of all unmarked objects, i.e., objects that are no longer reachable from the root set. These objects are then added back to the free list for future allocations.

The use of a linked list for storing runtime objects is a common technique in garbage collected environments. It allows for efficient iteration over all objects during the mark and sweep phases. The `next` field in each object allows the garbage collector to move from one object to the next during these phases.

**Constants**: These are objects that are created prior to the program's execution and do not change during the program's runtime. They are immutable, meaning their values cannot be changed once they are created. The `createConstObj` function is used to create a new constant object. It first checks if the program is in runtime, and if it is, it throws an error because constant objects cannot be created during runtime. Then, it creates a new object and sets its `isConst` field to `true` to indicate that it is a constant object.

In summary, the main difference between runtime objects and constants in this context is when they are created and whether or not their values can be changed. Runtime objects are created and can be modified during the program's execution, while constants are created before the program's execution and cannot be modified.

