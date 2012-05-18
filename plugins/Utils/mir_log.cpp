/*
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "mir_log.h"

#include <stdio.h>

#include <newpluginapi.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_clist.h>

#define ENABLE_LOG 


int MLog::deep = 0;

MLog::MLog(const char *aModule, const char *aFunction) 
	: module(aModule)
{
	memset(&total, 0, sizeof(total));

	function = "";
	for(int i = 0; i < deep; i++)
		function += "   ";
	function += aFunction;

	deep ++;

	mlog(module.c_str(), function.c_str(), "BEGIN");

	StartTimer();
}

MLog::~MLog()
{
	StopTimer();

	mlog(module.c_str(), function.c_str(), "END in %2.1lf ms", GetTotalTimeMS());
	deep --;
}

int MLog::log(const char *fmt, ...)
{
	StopTimer();

	double elapsed = GetElapsedTimeMS();

	va_list va;
	va_start(va, fmt);

    char text[1024];
	mir_snprintf(text, sizeof(text) - 10, "%s [in %2.1lf ms | total %2.1lf ms]", 
											fmt, GetElapsedTimeMS(), GetTotalTimeMS());

	int ret = mlog(module.c_str(), function.c_str(), text, va);

	va_end(va);

	StartTimer();

	return ret;
}

void MLog::StartTimer()
{
	QueryPerformanceCounter(&start);
}

void MLog::StopTimer()
{
	QueryPerformanceCounter(&end);

	total.QuadPart += end.QuadPart - start.QuadPart;
}

double MLog::GetElapsedTimeMS()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	return (end.QuadPart - start.QuadPart) * 1000. / frequency.QuadPart;
}

double MLog::GetTotalTimeMS()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	return total.QuadPart * 1000. / frequency.QuadPart;
}


int mlog(const char *module, const char *function, const char *fmt, va_list va)
{
#ifdef ENABLE_LOG

    char text[1024];
	size_t len;

	mir_snprintf(text, sizeof(text) - 10, "[%08u - %08u] [%s] [%s] ", 
				 GetCurrentThreadId(), GetTickCount(), module, function);
	len = strlen(text);

    mir_vsnprintf(&text[len], sizeof(text) - len, fmt, va);

#ifdef LOG_TO_NETLIB

    return CallService(MS_NETLIB_LOG, NULL, (LPARAM) text);

#else
	char file[256];
	_snprintf(file, sizeof(file), "c:\\miranda_%s.log.txt", module);

	FILE *fp = fopen(file,"at");

	if (fp != NULL)
	{
		fprintf(fp, "%s\n", text);
		fclose(fp);
		return 0;
	}
	else
	{
		return -1;
	}
	
#endif

#else

	return 0;

#endif
}


int mlog(const char *module, const char *function, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

	int ret = mlog(module, function, fmt, va);

    va_end(va);

	return ret;
}

int mlogC(const char *module, const char *function, HANDLE hContact, const char *fmt, ...)
{
#ifdef ENABLE_LOG

    va_list va;
    char text[1024];
	size_t len;

	char *name = NULL;
	char *proto = NULL;
	if (hContact != NULL)
	{
		name = (char*) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
		proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	}

	mir_snprintf(text, sizeof(text) - 10, "[%08u - %08u] [%s] [%s] [%08d - %s - %s] ", 
				 GetCurrentThreadId(), GetTickCount(), module, function,
				 hContact, proto == NULL ? "" : proto, name == NULL ? "" : name);
	len = strlen(text);

    va_start(va, fmt);
    mir_vsnprintf(&text[len], sizeof(text) - len, fmt, va);
    va_end(va);

#ifdef LOG_TO_NETLIB

    return CallService(MS_NETLIB_LOG, NULL, (LPARAM) text);

#else
	char file[256];
	_snprintf(file, sizeof(file), "c:\\miranda_%s.log.txt", module);

	FILE *fp = fopen(file,"at");

	if (fp != NULL)
	{
		fprintf(fp, "%s\n", text);
		fclose(fp);
		return 0;
	}
	else
	{
		return -1;
	}
	
#endif

#else

	return 0;

#endif
}
