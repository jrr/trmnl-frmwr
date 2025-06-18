#ifndef BL_H
#define BL_H

#include <cstdint>
#include <ctime>
#include "trmnl_log.h"

/**
 * @brief Function to init business logic module
 * @param none
 * @return none
 */
void bl_init(void);

/**
 * @brief Function to process business logic module
 * @param none
 * @return none
 */
void bl_process(void);

/**
 * @brief Get current time
 * @return uint32_t current time as seconds since epoch, or 0 if time sync failed
 */
uint32_t getTime(void);

/**
 * Persist log to nvram for future submission
 */
int saveLog(const char *format, time_t time, int line, const char *file, ...);

/**
 * Check if a log level should be reported (saved for submission to server)
 */
bool shouldReportLog(LogLevel level);

#endif