/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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
#include "IEView.h"
#include "m_ieview.h"
#include "ieview_services.h"
#include "Options.h"
#include "Utils.h"
#include "Version.h"

char *ieviewModuleName;
HINSTANCE hInstance;
PLUGINLINK *pluginLink;
char *workingDirUtf8;
static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
	{0x0495171b,   0x7137,   0x4ded, {0x97, 0xf8, 0xce, 0x6f, 0xed, 0x67, 0xd6, 0x91}}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 9, 0, 0)) {
		return NULL;
	}
	return &pluginInfoEx;
}

static const MUUID interfaces[] = {MIID_LOGWINDOW, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	char text[_MAX_PATH];
	char *p, *q;

	int wdsize = GetCurrentDirectory(0, NULL);
	TCHAR *workingDir = new TCHAR[wdsize];
	GetCurrentDirectory(wdsize, workingDir);
	Utils::convertPath(workingDir);
	workingDirUtf8 = Utils::UTF8Encode(workingDir);
	delete workingDir;

	GetModuleFileNameA(hInstance, text, sizeof(text));
	p = strrchr(text, '\\');
	p++;
	q = strrchr(p, '.');
	*q = '\0';
	ieviewModuleName = _strdup(p);
	_strupr(ieviewModuleName);

	pluginLink = link;
	mir_getMMI( &mmi );
	mir_getLP(&pluginInfoEx);

	Utils::hookEvent_Ex(ME_OPT_INITIALISE, IEViewOptInit);
	Utils::hookEvent_Ex(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	Utils::hookEvent_Ex(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	Utils::createServiceFunction_Ex(MS_IEVIEW_WINDOW, HandleIEWindow);
	Utils::createServiceFunction_Ex(MS_IEVIEW_EVENT, HandleIEEvent);
	Utils::createServiceFunction_Ex(MS_IEVIEW_EVENT, HandleIENavigate);
	hHookOptionsChanged = CreateHookableEvent(ME_IEVIEW_OPTIONSCHANGED);
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	IEView::init();
	Options::init();
	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	Options::uninit();
	Utils::unhookEvents_Ex();
	Utils::destroyServices_Ex();
	DestroyHookableEvent(hHookOptionsChanged);
	IEView::release();
	delete workingDirUtf8;
	free( ieviewModuleName );
	return 0;
}
