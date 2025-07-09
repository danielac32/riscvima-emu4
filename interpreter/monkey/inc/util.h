#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

extern void exit();
static void err(const int status, const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf(format, args);
    va_end(args);
    exit(1);
    //exit(EXIT_FAILURE);
}
