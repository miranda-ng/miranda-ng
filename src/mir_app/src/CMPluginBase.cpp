/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
#include "plugins.h"
#include "IcoLib.h"

static int sttFakeID = -100;

static int Compare(const CMPluginBase *p1, const CMPluginBase *p2)
{
	return INT_PTR(p1->getInst()) - INT_PTR(p2->getInst());
}

static LIST<CMPluginBase> g_arPlugins(10, Compare);

void RegisterModule(CMPluginBase *pPlugin)
{
	g_arPlugins.insert(pPlugin);
}

MIR_APP_DLL(HINSTANCE) GetInstByAddress(void *codePtr)
{
	if (g_arPlugins.getCount() == 0)
		return nullptr;

	int idx;
	void *boo[2] = { 0, codePtr };
	List_GetIndex((SortedList*)&g_arPlugins, (CMPluginBase*)&boo, &idx);
	if (idx > 0)
		idx--;

	HINSTANCE result = g_arPlugins[idx]->getInst();
	if (result < g_plugin.getInst() && codePtr > g_plugin.getInst())
		return g_plugin.getInst();
	
	if (idx == 0 && codePtr < (void*)result)
		return nullptr;

	return result;
}

MIR_APP_DLL(int) GetPluginLangId(const MUUID &uuid, int _hLang)
{
	if (uuid == miid_last)
		return --sttFakeID;

	for (auto &it : g_arPlugins)
		if (it->getInfo().uuid == uuid)
			return (_hLang) ? _hLang : --sttFakeID;

	return 0;
}

MIR_APP_DLL(int) IsPluginLoaded(const MUUID &uuid)
{
	for (auto &it : g_arPlugins)
		if (it->getInfo().uuid == uuid)
			return it->getInst() != nullptr;

	return false;
}

const char* GetPluginNameByInstance(HINSTANCE hInst)
{
	HINSTANCE boo[2] = { 0, hInst };
	CMPluginBase *pPlugin = g_arPlugins.find((CMPluginBase*)&boo);
	return (pPlugin == nullptr) ? nullptr : pPlugin->getInfo().shortName;
}

MIR_APP_DLL(CMPluginBase&) GetPluginByInstance(HINSTANCE hInst)
{
	HINSTANCE boo[2] = { 0, hInst };
	CMPluginBase *pPlugin = g_arPlugins.find((CMPluginBase*)&boo);
	return (pPlugin == nullptr) ? g_plugin : *pPlugin;
}

/////////////////////////////////////////////////////////////////////////////////////////
// stubs for pascal plugins

static void wipePluginData(CMPluginBase *pPlugin)
{
	if (Miranda_IsTerminated())
		return;

	KillModuleMenus(pPlugin);
	KillModuleFonts(pPlugin);
	KillModuleColours(pPlugin);
	KillModuleEffects(pPlugin);
	KillModuleIcons(pPlugin);
	KillModuleHotkeys(pPlugin);
	KillModulePopups(pPlugin);
	KillModuleSounds(pPlugin);
	KillModuleExtraIcons(pPlugin);
	KillModuleSrmmIcons(pPlugin);
	KillModuleToolbarIcons(pPlugin);
	KillModuleOptions(pPlugin);
}

// emulates the call of CMPluginBase::CMPluginBase for Pascal plugins
EXTERN_C MIR_APP_DLL(void) RegisterPlugin(CMPluginBase *pPlugin)
{
	if (pPlugin->getInst() != nullptr)
		g_arPlugins.insert(pPlugin);
}

// emulates the call of CMPluginBase::~CMPluginBase for Pascal plugins
EXTERN_C MIR_APP_DLL(void) UnregisterPlugin(CMPluginBase *pPlugin)
{
	wipePluginData(pPlugin);
	g_arPlugins.remove(pPlugin);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int CompareIcons(const IcolibItem *p1, const IcolibItem *p2)
{
	return p1->default_indx - p2->default_indx;
}

CMPluginBase::CMPluginBase(const char *moduleName, const PLUGININFOEX &pInfo) :
	m_szModuleName(moduleName),
	m_pInfo(pInfo),
	m_arIcons(10, CompareIcons)
{
	if (m_hInst != nullptr)
		g_arPlugins.insert(this);
}

CMPluginBase::~CMPluginBase()
{
	wipePluginData(this);

	if (m_hLogger) {
		mir_closeLog(m_hLogger);
		m_hLogger = nullptr;
	}

	g_arPlugins.remove(this);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::Load()
{
	return 0;
}

int CMPluginBase::Unload()
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMPluginBase::tryOpenLog()
{
	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\%S.txt", VARSW(L"%miranda_logpath%").get(), m_szModuleName);
	m_hLogger = mir_createLog(m_szModuleName, nullptr, path, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::addOptions(WPARAM wParam, OPTIONSDIALOGPAGE *odp)
{
	return ::Options_AddPage(wParam, odp, this);
}

void CMPluginBase::openOptions(const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab)
{
	::Options_Open(pszGroup, pszPage, pszTab, this);
}

void CMPluginBase::openOptionsPage(const wchar_t *pszGroup, const wchar_t *pszPage, const wchar_t *pszTab)
{
	::Options_OpenPage(pszGroup, pszPage, pszTab, this);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::addFont(FontID *pFont)
{
	return Font_Register(pFont, this);
}

int CMPluginBase::addFont(FontIDW *pFont)
{
	return Font_RegisterW(pFont, this);
}

int CMPluginBase::addColor(ColourID *pColor)
{
	return Colour_Register(pColor, this);
}

int CMPluginBase::addColor(ColourIDW *pColor)
{
	return Colour_RegisterW(pColor, this);
}

int CMPluginBase::addEffect(EffectID *pEffect)
{
	return Effect_Register(pEffect, this);
}

int CMPluginBase::addEffect(EffectIDW *pEffect)
{
	return Effect_RegisterW(pEffect, this);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::addFrame(const CLISTFrame *F)
{
	return (int)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)F, (LPARAM)this);
}

int CMPluginBase::addHotkey(const HOTKEYDESC *hk)
{
	return Hotkey_Register(hk, this);
}

HANDLE CMPluginBase::addIcon(const SKINICONDESC *sid)
{
	return IcoLib_AddIcon(sid, this);
}

HGENMENU CMPluginBase::addRootMenu(int hMenuObject, LPCWSTR ptszName, int position, HANDLE hIcoLib)
{
	return Menu_CreateRoot(hMenuObject, ptszName, position, hIcoLib, this);
}

HANDLE CMPluginBase::addTTB(const struct TTBButton *pButton)
{
	return (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)pButton, (LPARAM)this);
}

int CMPluginBase::addUserInfo(WPARAM wParam, USERINFOPAGE *uip)
{
	uip->pPlugin = this;
	return CallService("UserInfo/AddPage", wParam, (LPARAM)uip);
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

#ifdef _WINDOWS
int CMPluginBase::addImgListIcon(HIMAGELIST himl, int iconId)
{
	HICON hIcon = getIcon(iconId);
	int ret = ::ImageList_AddIcon(himl, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return ret;
}
#endif

HICON CMPluginBase::getIcon(int iconId, bool big)
{
	return IcoLib_GetIconByHandle(getIconHandle(iconId), big);
}

HANDLE CMPluginBase::getIconHandle(int iconId)
{
	IcolibItem *p = (IcolibItem*)alloca(sizeof(IcolibItem));
	p->default_indx = -iconId;
	return m_arIcons.find(p);
}

void CMPluginBase::releaseIcon(int iconId, bool big)
{
	IcoLib_ReleaseIcon(getIcon(iconId), big);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMPluginBase::RegisterProtocol(int type, pfnInitProto fnInit, pfnUninitProto fnUninit)
{
	if (isPluginBanned(m_pInfo.uuid))
		return;

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
	if (pszUniqueId == nullptr)
		return;

	MBaseProto *pd = g_arProtos.find((MBaseProto*)&m_szModuleName);
	if (pd != nullptr)
		pd->szUniqueId = mir_strdup(pszUniqueId);
}
