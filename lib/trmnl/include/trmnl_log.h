
#pragma once

enum LogLevel
{
  LVL_VERBOSE,
  LVL_INFO,
  LVL_ERROR,
  LVL_FATAL
};

struct LogLevelMap
{
  const char *name;
  LogLevel value;
};

static const LogLevelMap logLevelMap[] = {
    {"verbose", LVL_VERBOSE},
    {"info", LVL_INFO},
    {"error", LVL_ERROR},
    {"fatal", LVL_FATAL},
};

// Platform-specific implementation must be provided by the consuming project
void trmnl_log(LogLevel level, const char *file, int line, const char *format, ...);

const char* logLevelToString(LogLevel level);

#define Log_verbose(format, ...) trmnl_log(LVL_VERBOSE, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define Log_info(format, ...) trmnl_log(LVL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define Log_error(format, ...) trmnl_log(LVL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define Log_fatal(format, ...) trmnl_log(LVL_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)
