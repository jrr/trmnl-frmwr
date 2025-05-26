#include <stddef.h>
#include <persistence_interface.h>

void store_log(const char *log_buffer, size_t size, Persistence &persistence);

void gather_stored_logs(String &log, Persistence &persistence);

void clear_stored_logs(Persistence &persistence);
