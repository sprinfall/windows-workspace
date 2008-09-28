#include "log.h"

#if defined HAS_LOG

#include "global.h"

static tofstream *log_stream;

void log_start()
{
	log_stream = NULL;

	TCHAR logfile[MAX_PATH];
	if (get_usr_home(logfile, MAX_PATH)) {
		_tcscat_s(logfile, MAX_PATH, _T("\\wwlog.txt"));
		log_stream = new tofstream(logfile);
	}
}

void log_close()
{
	if (log_stream) {
		log_stream->close();
		delete log_stream;
	}
}

void log(const TCHAR *msg)
{
	if (log_stream) {
		*log_stream << msg << std::endl;
	}
}

#endif // HAS_LOG
