#include "stdafx.h"

//file debug.cpp
#ifdef _DEBUG

#ifdef USECONSTREAM
	ConStream g_ConStream;

	void InitDebug()
	{
		g_ConStream.Open();
	}

	void UnInitDebug()
	{
		g_ConStream.Close();
	}
#else
	void InitDebug()
	{
	}

	void UnInitDebug()
	{
	}
#endif

void _trace(TCHAR *fmt, ...)
{
	TCHAR out[1024];
	va_list body;
	va_start(body, fmt);
#ifdef _UNICODE
	vswprintf(out, fmt, body);
#else
	vsprintf(out,fmt,body);
#endif

	va_end(body);
#ifdef USECONSTREAM
	g_ConStream << out;
#else
	OutputDebugString(out);
#endif

}
#else

void InitDebug()
{
}

void UnInitDebug()
{
}

#endif

