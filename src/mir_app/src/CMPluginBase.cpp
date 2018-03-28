/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
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

CMPluginBase::CMPluginBase(const char *moduleName) :
	m_szModuleName(moduleName)
{
}

CMPluginBase::~CMPluginBase()
{
	if (m_hLogger) {
		mir_closeLog(m_hLogger);
		m_hLogger = nullptr;
	}
}

void CMPluginBase::tryOpenLog()
{
	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\%s.txt", VARSW(L"%miranda_logpath%"), m_szModuleName);
	m_hLogger = mir_createLog(m_szModuleName, nullptr, path, 0);
}

void CMPluginBase::debugLogA(LPCSTR szFormat, ...)
{
	if (m_hLogger == nullptr)
		tryOpenLog();

	va_list args;
	va_start(args, szFormat);
	mir_writeLogVA(m_hLogger, szFormat, args);
	va_end(args);
}

void CMPluginBase::debugLogW(LPCWSTR wszFormat, ...)
{
	if (m_hLogger == nullptr)
		tryOpenLog();

	va_list args;
	va_start(args, wszFormat);
	mir_writeLogVW(m_hLogger, wszFormat, args);
	va_end(args);
}

void CMPluginBase::RegisterProtocol(int type, pfnInitProto fnInit, pfnUninitProto fnUninit)
{
	PROTOCOLDESCRIPTOR pd = {};
	pd.cbSize = (fnInit == nullptr) ? PROTOCOLDESCRIPTOR_V3_SIZE : sizeof(pd);
	pd.szName = (char*)m_szModuleName;
	pd.type = type;
	pd.fnInit = fnInit;
	pd.fnUninit = fnUninit;
	Proto_RegisterModule(&pd);
}
