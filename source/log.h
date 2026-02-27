#pragma once

typedef enum { NO_LOG, ERROR, WARN, INFO } log_level_e;

extern log_level_e g_log_level;

void emu_log(const log_level_e level, const char *format, ...);
