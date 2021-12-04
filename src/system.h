#ifndef PROJECTNAME_SYSTEM_H
#define PROJECTNAME_SYSTEM_H


#include "types.h"


#define m_throw_error(base, param1, param2) system_set_error(base, param1, param2, __FILE__, __LINE__)
#define m_memory_error() m_throw_error("Memory allocation error!", NULL, NULL)

#define m_free(a) if (a != NULL) { free(a); a = NULL; }


i16 init_system();

void system_set_error(str baseMessage,
    str param1, str param2, str filename, i32 line);
str system_get_error();
void system_flush_error();


#endif // PROJECTNAME_SYSTEM_H
