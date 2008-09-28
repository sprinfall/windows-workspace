#ifndef _LOG_H_
#define _LOG_H_

// This simple logging utility is for debug use mostly

#define HAS_LOG 1 // comment this to disable

#if defined HAS_LOG

#include <tchar.h>

void log_start();
void log_close();
void log(const TCHAR *msg);

#define LOG_START() do { log_start(); } while (0)
#define LOG_CLOSE() do { log_close(); } while (0)
#define LOG(msg) do { log(msg); } while (0)

#else

#define LOG_START() do {} while (0)
#define LOG_CLOSE() do {} while (0)
#define LOG(msg) do {} while (0)

#endif // HAS_LOG

#endif // _LOG_H_
