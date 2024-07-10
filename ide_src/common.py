SOURCE_EXTENSION = ".ant"
SOURCE_EXTENSION_REGEX = f"*{SOURCE_EXTENSION}"
C_ACC_LIBRARY_EXTENSION_REGEX = "*.c"

SOURCE_DESCRIPTION = "AnteaterLang Source"
C_ACC_LIBRARY_EXTENSION_DESCRIPTION = "C Acc Library Source"

TERMINAL_BACKGROUND_COLOR = '#2B2B2B'
EDITOR_BACKGROUND_COLOR = '#2B2B2B'

TERMINAL_FONT_SIZE = 15
EDITOR_FONT_SIZE = 15
TAB_SPACE = 4

IDE_PROJECT_SETTING_NAME = "/ideconfig.pickle"

DEFAULT_WINDOW_SIZE = "1600x800"

ANTEATERLANG_KEYWORD = "antlang"

C_ACC_LIBRARY_SO_PATH = "libuserFunctions.so"

IDE_VERSION = "0.1.0"

SAMPLE_ANT_SOURCE = """\
void function native_hello_world() {
    println("Hello World!");
}

void function main() {
    
    # Builtin function
    native_hello_world();
    
    # C function
    c_hello_world();
}
"""

SAMPLE_C_ACC_LIBRARY_SOURCE = """\
#include "chunk.h"
#include "object.h"
#include "primitiveVars.h"

// Use this macro to define a new user function in the array
#define USER_FUNCTION(inCount, outCount, funcName, cFunction) {.in = inCount, .out = outCount, .name = funcName, .cFunc = cFunction}

// This is the definition of a user function
// Value (*cMethodType)(Value, Value*, int);

// Define functions here

// Sample Function
Value helloWorld(Value self, Value* args, int numArgs){
    printf("Hello World!\n");
    
    return NONE_VAL;
}


// This is what the VM will use to integrate with the host language

// Change the number of functions here
uint32_t funcCount = 1;

// Add the functions here, add a new line & comma for each function
userFunction userFuncs[] = {
USER_FUNCTION(0, 0, "c_hello_world", helloWorld)
};
"""
