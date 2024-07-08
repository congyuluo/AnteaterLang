# Makefile for building and installing the static library and executables

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -fPIC -std=c11

# Source files for the main executable
SRCS = chunk.c constList.c objClass.c object.c objectManager.c refManager.c runtimeDS.c runtimeMemoryManager.c stringHash.c tokenizer.c vm.c builtinClasses.c errors.c debug.c compiler.c main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Static library
LIB = liblang.a

# Main executable
MAIN_EXEC = antlang

# User functions compiler executable
USER_FUNC_EXEC = antcompilec

# Installation directories
PREFIX_LIB = /usr/local/lib
PREFIX_BIN = /usr/local/bin
INCLUDE_DIR = /usr/local/include/anteaterlang

# Targets
all: $(LIB) $(MAIN_EXEC) $(USER_FUNC_EXEC) clean_objs

$(LIB): $(OBJS)
	ar rcs $@ $(filter-out main.o,$(OBJS))

$(MAIN_EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS) -L. -llang -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(USER_FUNC_EXEC): $(LIB)
	@echo "#!/bin/bash" > $(USER_FUNC_EXEC)
	@echo "$(CC) -fPIC -I$(INCLUDE_DIR) -c userFunctions.c -o userFunctions.o" >> $(USER_FUNC_EXEC)
	@echo "$(CC) -shared -o libuserFunctions.so userFunctions.o -L$(PREFIX_LIB) -llang" >> $(USER_FUNC_EXEC)
	@echo "rm -f userFunctions.o" >> $(USER_FUNC_EXEC)
	@chmod +x $(USER_FUNC_EXEC)

install: all $(USER_FUNC_EXEC)
	install -d $(PREFIX_LIB)
	install -m 644 $(LIB) $(PREFIX_LIB)
	install -d $(INCLUDE_DIR)
	install -m 644 chunk.h constList.h objClass.h object.h objectManager.h refManager.h runtimeDS.h runtimeMemoryManager.h stringHash.h tokenizer.h vm.h common.h primitiveVars.h $(INCLUDE_DIR)
	install -d $(PREFIX_BIN)
	install -m 755 $(MAIN_EXEC) $(PREFIX_BIN)
	install -m 755 $(USER_FUNC_EXEC) $(PREFIX_BIN)
	rm -f $(MAIN_EXEC)
	rm -f $(USER_FUNC_EXEC)

clean_objs:
	rm -f $(OBJS)

clean: clean_objs
	rm -f $(LIB) $(MAIN_EXEC) $(USER_FUNC_EXEC) libuserFunctions.so

.PHONY: all install clean clean_objs
