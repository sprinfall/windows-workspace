#include "log.h"

#if defined _DEBUG

static const TCHAR *logFile = _T("log.txt");
void log(const tstring &msg)
{
	tofstream logstream(logFile, std::ios_base::app);
	logstream << msg << std::endl;
	logstream.close();
}

#endif // _DEBUG
