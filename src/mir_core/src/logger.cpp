/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

#define SECRET_SIGNATURE 0x87654321

struct Logger
{
	Logger(const char* pszName, const wchar_t *ptszDescr, const wchar_t *ptszFilename, unsigned options) :
		m_name(mir_strdup(pszName)),
		m_descr(mir_wstrdup(ptszDescr)),
		m_fileName(mir_wstrdup(ptszFilename)),
		m_options(options),
		m_signature(SECRET_SIGNATURE),
		m_out(nullptr),
		m_lastwrite(0)
	{
	}

	~Logger()
	{
		if (m_out)
			fclose(m_out);
	}

	int      m_signature;
	ptrA     m_name;
	ptrW     m_fileName, m_descr;
	FILE    *m_out;
	time_t   m_lastwrite;
	unsigned m_options;
	mir_cs   m_cs;
};

static int CompareLoggers(const Logger *p1, const Logger *p2)
{	return strcmp(p1->m_name, p2->m_name);
}

static OBJLIST<Logger> arLoggers(1, CompareLoggers);

void InitLogs()
{
}

void UninitLogs()
{
	arLoggers.destroy();
}

void CheckLogs()
{
   time_t tm = time(0);

	for (auto &p : arLoggers) {
		mir_cslock lck(p->m_cs);
		if (p->m_out && tm - p->m_lastwrite > 5) {
			fclose(p->m_out);
			p->m_out = nullptr;
		}
		else fflush(p->m_out);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(HANDLE) mir_createLog(const char* pszName, const wchar_t *ptszDescr, const wchar_t *ptszFile, unsigned options)
{
	if (ptszFile == nullptr)
		return nullptr;

	Logger *result = new Logger(pszName, ptszDescr, ptszFile, options);
	if (result == nullptr)
		return nullptr;

	int idx = arLoggers.getIndex(result);
	if (idx != -1) {
		delete result;
		return &arLoggers[idx];
	}

	CreatePathToFileW(ptszFile);
	arLoggers.insert(result);
	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////

static Logger* prepareLogger(HANDLE hLogger)
{
	if (hLogger == nullptr)
		return nullptr;

	Logger *p = (Logger*)hLogger;
	return (p->m_signature == SECRET_SIGNATURE) ? p : nullptr;
}

MIR_CORE_DLL(void) mir_closeLog(HANDLE hLogger)
{
	Logger *p = prepareLogger(hLogger);
	if (p != nullptr)
		arLoggers.remove(p);
}

////////////////////////////////////////////////////////////////////////////////////////////////

MIR_C_CORE_DLL(int) mir_writeLogA(HANDLE hLogger, const char *format, ...)
{
	Logger *p = prepareLogger(hLogger);
	if (p == nullptr)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == nullptr)
		if ((p->m_out = _wfopen(p->m_fileName, L"ab")) == nullptr)
			return 2;

	va_list args;
	va_start(args, format);
	vfprintf(p->m_out, format, args);
	va_end(args);

	p->m_lastwrite = time(0);
	return 0;
}

MIR_C_CORE_DLL(int) mir_writeLogW(HANDLE hLogger, const wchar_t *format, ...)
{
	Logger *p = prepareLogger(hLogger);
	if (p == nullptr)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == nullptr)
		if ((p->m_out = _wfopen(p->m_fileName, L"ab")) == nullptr)
			return 2;

	va_list args;
	va_start(args, format);
	vfwprintf(p->m_out, format, args);
	va_end(args);

	p->m_lastwrite = time(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) mir_writeLogVA(HANDLE hLogger, const char *format, va_list args)
{
	Logger *p = prepareLogger(hLogger);
	if (p == nullptr)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == nullptr)
		if ((p->m_out = _wfopen(p->m_fileName, L"ab")) == nullptr)
			return 2;

	vfprintf(p->m_out, format, args);

	p->m_lastwrite = time(0);
	return 0;
}

MIR_CORE_DLL(int) mir_writeLogVW(HANDLE hLogger, const wchar_t *format, va_list args)
{
	Logger *p = prepareLogger(hLogger);
	if (p == nullptr)
		return 1;

	mir_cslock lck(p->m_cs);
	if (p->m_out == nullptr)
		if ((p->m_out = _wfopen(p->m_fileName, L"ab")) == nullptr)
			return 2;

	vfwprintf(p->m_out, format, args);

	p->m_lastwrite = time(0);
	return 0;
}
