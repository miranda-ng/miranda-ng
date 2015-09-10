#ifndef _TOX_LOGGER_H_
#define _TOX_LOGGER_H_

class CLogger
{
private:
	HANDLE hNetlibUser;

public:
	CLogger(HANDLE hNetlibUser) : hNetlibUser(hNetlibUser) {}

	__inline void Log(LPCSTR szFormat, ...) const
	{
		va_list args;
		va_start(args, szFormat);
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)(CMStringA().FormatV(szFormat, args)));
		va_end(args);
	}
	__inline void Log(LPCWSTR wszFormat, ...) const
	{
		va_list args;
		va_start(args, wszFormat);
		CallService(MS_NETLIB_LOGW, (WPARAM)hNetlibUser, (LPARAM)(CMStringW().FormatV(wszFormat, args)));
		va_end(args);
	}
};

#endif //_TOX_LOGGER_H_