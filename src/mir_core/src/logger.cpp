/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

#define SECRET_SIGNATURE 0x87654321

struct Logger
{
	Logger(const char* pszName, const TCHAR *ptszDescr, const TCHAR *ptszFilename, unsigned options) :
		m_name(mir_strdup(pszName)),
		m_descr(mir_tstrdup(ptszDescr)),
		m_fileName(mir_tstrdup(ptszFilename)),
		m_options(options),
		m_signature(SECRET_SIGNATURE),
		m_out(NULL),
		m_lastwrite(0)
	{
		InitializeCriticalSection(&m_cs);
	}

	~Logger()
	{
		if (m_out)
			fclose(m_out);

		DeleteCriticalSection(&m_cs);
	}

	int      m_signature;
	ptrA     m_name;
	ptrT     m_fileName, m_descr;
	FILE    *m_out;
	__int64  m_lastwrite;
	unsigned m_options;

	CRITICAL_SECTION m_cs;
};

static int CompareLoggers(const Logger *p1, const Logger *p2)
{	return strcmp(p1->m_name, p2->m_name);
}

static OBJLIST<Logger> arLoggers(1, CompareLoggers);

static __int64 llIdlePeriod;

void InitLogs()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	llIdlePeriod = li.QuadPart;
}

void UninitLogs()
{
	arLoggers.destroy();
}

void CheckLogs()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	for (int i=0; i < arLoggers.getCount(); i++) {
		Logger &p = arLoggers[i];

		mir_cslock lck(p.m_cs);
		if (p.m_out && li.QuadPart - p.m_lastwrite > llIdlePeriod) {
			fclose(p.m_out);
			p.m_out = NULL;
		}
		else fflush(p.m_out);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(HANDLE) mir_createLog(const char* pszName, const TCHAR *ptszDescr, const TCHAR *ptszFile, unsigned options)
{
	if (ptszFile == NULL)
		return NULL;

	Logger *result = new Logger(pszName, ptszDescr, ptszFile, options);
	if (result == NULL)
		return NULL;

	int idx = arLoggers.getIndex(result);
	if (idx != -1) {
		delete result;
		return &arLoggers[idx];
	}

	FILE *fp = _tfopen(ptszFile, _T("ab"));
	if (fp == NULL) {
		TCHAR tszPath[MAX_PATH];
		_tcsncpy_s(tszPath, ptszFile, _TRUNCATE);
		CreatePathToFileT(tszPath);
	}
	else fclose(fp);

	DeleteFile(ptszFile);
	arLoggers.insert(result);
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////

static Logger* prepareLogger(HANDLE hLogger)
{
	if (hLogger == NULL)
		return NULL;

	Logger *p = (Logger*)hLogger;
	return (p->m_signature == SECRET_SIGNATURE) ? p : NULL;
}

MIR_CORE_DLL(void) mir_closeLog(HANDLE hLogger)
{
	Logger *p = prepareLogger(hLogger);
	if (p != NULL)
		arLoggers.remove(p);
}

////////////////////////////////////////////////////////////////////////////////////////////////

MIR_C_CORE_DLL(int) mir_writeLogA(HANDLE hLogger, const char *format, ...)
{
	Logger *p = prepareLogger(hLogger);
	if (p == NULL)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == NULL)
		if ((p->m_out = _tfopen(p->m_fileName, _T("ab"))) == NULL)
			return 2;

	va_list args;
	va_start(args, format);
	vfprintf(p->m_out, format, args);

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	p->m_lastwrite = li.QuadPart;
	return 0;
}

MIR_C_CORE_DLL(int) mir_writeLogW(HANDLE hLogger, const WCHAR *format, ...)
{
	Logger *p = prepareLogger(hLogger);
	if (p == NULL)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == NULL)
		if ((p->m_out = _tfopen(p->m_fileName, _T("ab"))) == NULL)
			return 2;

	va_list args;
	va_start(args, format);
	vfwprintf(p->m_out, format, args);

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	p->m_lastwrite = li.QuadPart;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) mir_writeLogVA(HANDLE hLogger, const char *format, va_list args)
{
	Logger *p = prepareLogger(hLogger);
	if (p == NULL)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == NULL)
		if ((p->m_out = _tfopen(p->m_fileName, _T("ab"))) == NULL)
			return 2;

	vfprintf(p->m_out, format, args);

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	p->m_lastwrite = li.QuadPart;
	return 0;
}

MIR_CORE_DLL(int) mir_writeLogVW(HANDLE hLogger, const WCHAR *format, va_list args)
{
	Logger *p = prepareLogger(hLogger);
	if (p == NULL)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == NULL)
		if ((p->m_out = _tfopen(p->m_fileName, _T("ab"))) == NULL)
			return 2;

	vfwprintf(p->m_out, format, args);

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	p->m_lastwrite = li.QuadPart;
	return 0;
}
