#include "system.h"

#include <stdio.h>
#include <stdlib.h>


#define BUFFER_MAX_SIZE 512


// Why is this called "buffer", not "errorBuffer?"
// Well, I'm not going to rename it now!
static char* buffer = NULL;


i16 init_system() {

    if (buffer != NULL) {

        fprintf(stderr, "WARNING! System already initialized.\n");
        return 1;
    }

    buffer = (char*) calloc(BUFFER_MAX_SIZE, sizeof(char));
    if (buffer == NULL) {

        // No need to return anything, just panic
        fprintf(stderr, "Fatal memory error: failed to initialize error buffer!\n");
        exit(1);
    }

    return 0;
}


void system_set_error(str baseMessage,
    str param1, str param2, str filename, i32 line) {

    if (buffer == NULL) return;

    if (param1 == NULL && param2 == NULL) {

        snprintf(buffer, BUFFER_MAX_SIZE, 
            "Error in file %s on line %d: %s\n", 
            filename, line, baseMessage);
    }
    else if (param1 != NULL && param2 == NULL) {

        snprintf(buffer, BUFFER_MAX_SIZE, 
            "Error in file %s on line %d: %s%s\n", 
            filename, line, baseMessage, param1);
    }
    else if (param1 != NULL && param2 != NULL) {

        snprintf(buffer, BUFFER_MAX_SIZE, 
            "Error in file %s on line %d: %s%s%s\n", 
            filename, line, baseMessage, param1, param2);
    }
    else {

        system_set_error(baseMessage, param2, NULL, filename, line);
    }
}


str system_get_error() {

    return buffer;
}


void system_flush_error() {

    buffer[0] = '\0';
}
