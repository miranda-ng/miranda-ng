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

static LIST<CMPluginBase> pluginListAddr(10, HandleKeySortT);

void RegisterModule(CMPluginBase *pPlugin)
{
	pluginListAddr.insert(pPlugin);
}

MIR_APP_DLL(HINSTANCE) GetInstByAddress(void* codePtr)
{
	if (pluginListAddr.getCount() == 0)
		return nullptr;

	int idx;
	List_GetIndex((SortedList*)&pluginListAddr, (CMPluginBase*)&codePtr, &idx);
	if (idx > 0)
		idx--;

	HINSTANCE result = pluginListAddr[idx]->getInst();
	if (result < g_plugin.getInst() && codePtr > g_plugin.getInst())
		return g_plugin.getInst();
	
	if (idx == 0 && codePtr < (void*)result)
		return nullptr;

	return result;
}

MIR_APP_DLL(CMPluginBase*) GetPluginByLangId(int _hLang)
{
	for (auto &it : pluginListAddr)
		if (it->m_hLang == _hLang)
			return it;

	return nullptr;
}

MIR_APP_DLL(CMPluginBase&) GetPluginByInstance(HINSTANCE hInst)
{
	CMPluginBase *pPlugin = pluginListAddr.find((CMPluginBase*)&hInst);
	return (pPlugin == nullptr) ? g_plugin : *pPlugin;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMPluginBase::CMPluginBase(const char *moduleName) :
	m_szModuleName(moduleName)
{
	if (m_hInst != nullptr)
		pluginListAddr.insert(this);
}

CMPluginBase::~CMPluginBase()
{
	if (m_hLogger) {
		mir_closeLog(m_hLogger);
		m_hLogger = nullptr;
	}

	pluginListAddr.remove(this);
}

void CMPluginBase::tryOpenLog()
{
	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\%S.txt", VARSW(L"%miranda_logpath%"), m_szModuleName);
	m_hLogger = mir_createLog(m_szModuleName, nullptr, path, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////

void CMPluginBase::RegisterProtocol(int type, pfnInitProto fnInit, pfnUninitProto fnUninit)
{
	if (type == PROTOTYPE_PROTOCOL && fnInit != nullptr)
		type = PROTOTYPE_PROTOWITHACCS;

	MBaseProto *pd = (MBaseProto*)Proto_RegisterModule(type, m_szModuleName);
	if (pd) {
		pd->fnInit = fnInit;
		pd->fnUninit = fnUninit;
		pd->hInst = m_hInst;
	}
}

void CMPluginBase::SetUniqueId(const char *pszUniqueId)
{
	::Proto_SetUniqueId(m_szModuleName, pszUniqueId);
}
