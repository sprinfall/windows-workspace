#ifndef _LOG_H_
#define _LOG_H_

#if defined _DEBUG

#include "type.h"

void log(const tstring &msg);
#define LOG(msg) do { log(msg); } while (0)

#else

#define LOG(msg) do {} while (0)

#endif // _DEBUG

#endif // _LOG_H_
