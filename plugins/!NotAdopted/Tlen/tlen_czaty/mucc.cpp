/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

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

#include "../commons.h"
#include "mucc.h"
#include "mucc_services.h"
#include "HelperDialog.h"
#include "Options.h"
#include "Utils.h"

char *muccModuleName;
HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hHookEvent = NULL;
HIMAGELIST hImageList = NULL;
HICON muccIcon[MUCC_ICON_TOTAL];
static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
#ifdef _UNICODE
#ifdef _X64
	"Tlen Czaty (x64, Unicode)",
#else
	"Tlen Czaty (Unicode)",
#endif
#else
	"Tlen Czaty",
#endif
	PLUGIN_MAKE_VERSION(MUCC_MAJOR_VERSION,MUCC_MINOR_VERSION,MUCC_RELEASE_NUM,MUCC_BUILD_NUM),
	"Group chats GUI plugin for Miranda IM (formerly known as mucc.dll) (version: "MUCC_VERSION_STRING" ; compiled: "__DATE__" "__TIME__")",
	"Piotr Piastucki",
	"the_leech@users.berlios.de",
	"(c) 2004-2012 Piotr Piastucki",
	"http://mtlen.berlios.de",
	0,
	0,
#if defined( _UNICODE )
#ifdef _X64
	{0x9061ae31, 0x3d33, 0x49ce, { 0xaf, 0x00, 0x78, 0x9c, 0xbc, 0x25, 0xd9, 0xba }}
#else
	{0xadd9390c, 0x1dd4, 0x4c0d, { 0x9b, 0xa9, 0xcc, 0x76, 0x5d, 0x3d, 0xe5, 0x97 }}
#endif
#else
    {0x5cf4792c, 0xa050, 0x46b6, { 0xaf, 0xd0, 0x03, 0x2d, 0x6e, 0xfc, 0xd3, 0x9c }}
#endif
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,8,0,15)) {
		MessageBox(NULL, _TEXT("The Tlen Czaty plugin cannot be loaded. It requires Miranda IM 0.8.15 or later."), _TEXT("Tlen Czaty plugin (MUCC)"), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST);
		return NULL;
	}
	return &pluginInfoEx;
}

#ifndef MIID_TLEN_MUCC
#define MIID_TLEN_MUCC	{ 0xba658997, 0x0bce, 0x4f96, { 0xba, 0x48, 0x54, 0x55, 0x34, 0x16, 0x73, 0xea } }
#endif

extern "C" __declspec(dllexport) const MUUID interfaces[] = {
		MIID_TLEN_MUCC,
		MIID_LAST
	};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


static void LoadIcons() {
	int i;
	static int iconList[] = {
		IDI_CHAT,
		IDI_GLOBALOWNER,
		IDI_OWNER,
		IDI_ADMIN,
		IDI_REGISTERED,
		IDI_R_MODERATED,
		IDI_R_MEMBERS,
		IDI_R_ANONYMOUS,
		IDI_PREV,
		IDI_NEXT,
		IDI_SEARCH,
		IDI_BOLD,
		IDI_ITALIC,
		IDI_UNDERLINE,
		IDI_OPTIONS,
		IDI_INVITE,
		IDI_ADMINISTRATION,
		IDI_SMILEY,
		IDI_MESSAGE

	};
	for (i=0; i<MUCC_ICON_TOTAL; i++) {
		muccIcon[i] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(iconList[i]), IMAGE_ICON, 0, 0, 0);
	}
	if (hImageList != NULL) {
		ImageList_Destroy(hImageList);
	}
	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,0,3);
//	ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_BLANK)));
//	ImageList_AddIcon(hImageList, LoadIcon(hInst, MAKEINTRESOURCE(IDI_BLANK)));
	ImageList_AddIcon(hImageList, muccIcon[MUCC_IDI_MESSAGE]);

}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
//	char text[_MAX_PATH];
//	char *p, *q;
//	GetModuleFileNameA(hInst, text, sizeof(text));
//	p = strrchr(text, '\\');
//	p++;
//	q = strrchr(p, '.');
//	*q = '\0';
//	muccModuleName = _strdup(p);
//	_strupr(muccModuleName);

	muccModuleName = "MUCC";

	pluginLink = link;
	HookEvent(ME_OPT_INITIALISE, MUCCOptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	CreateServiceFunction(MS_MUCC_QUERY_RESULT, MUCCQueryResult);
	CreateServiceFunction(MS_MUCC_NEW_WINDOW, MUCCNewWindow);
	CreateServiceFunction(MS_MUCC_EVENT, MUCCEvent);
	hHookEvent = CreateHookableEvent(ME_MUCC_EVENT);

	mir_getMMI(&(Utils::mmi));

	LoadIcons();
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	Options::init();
	HelperDialog::init();
	ManagerWindow::init();
	ChatWindow::init();
	ChatContainer::init();
	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	ChatContainer::release();
	ChatWindow::release();
	ManagerWindow::release();
	HelperDialog::release();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

