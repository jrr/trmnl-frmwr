#include <trmnl_log.h>
#include <cstdio>
#include <cstdarg>

#ifdef PIO_UNIT_TESTING

// Test implementation of trmnl_log using printf
void trmnl_log(LogLevel level, const char* file, int line, const char* format, ...) {
    const char* level_names[] = {"VERBOSE", "INFO", "ERROR", "FATAL"};
    
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    printf("[%s] %s [%d]: %s\r\n", level_names[level], file, line, buffer);
}

#endif