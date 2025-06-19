#include "trmnl_log.h"
#include "bl.h"
#include <stdarg.h>
#include <ArduinoLog.h>

void trmnl_log(LogLevel level, const char* file, int line, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    switch (level) {
    case LVL_VERBOSE:
        Log.verbose("%s [%d]: %s\r\n", file, line, buffer);
        break;
    case LVL_INFO:
        Log.info("%s [%d]: %s\r\n", file, line, buffer);
        break;
    case LVL_ERROR:
        Log.error("%s [%d]: %s\r\n", file, line, buffer);
        break;
    case LVL_FATAL:
        Log.fatal("%s [%d]: %s\r\n", file, line, buffer);
        break;
    }

    if (shouldReportLog(level))
    {
        saveLog(level, "%s", getTime(), line, file, buffer);
    }
}

const char* logLevelToString(LogLevel level) {
    for (const LogLevelMap &entry : logLevelMap) {
        if (level == entry.value) {
            return entry.name;
        }
    }
    return "unknown";
}