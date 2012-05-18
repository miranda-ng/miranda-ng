/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2005 Miranda ICQ/IM project,
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
#include <m_icolib.h>

HINSTANCE g_hInst = 0;
PLUGINLINK *pluginLink;
CLIST_INTERFACE* pcli = NULL;
HIMAGELIST himlCListClc = NULL;
int hLangpack;

struct MM_INTERFACE mmi;
BOOL(WINAPI * MySetLayeredWindowAttributes) (HWND, COLORREF, BYTE, DWORD) = NULL;

/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void RegisterCListFonts( void );
void InitCustomMenus( void );
void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint);

int ClcOptInit(WPARAM wParam, LPARAM lParam);
int ClcModernOptInit(WPARAM wParam, LPARAM lParam);
int CluiOptInit(WPARAM wParam, LPARAM lParam);
int CluiModernOptInit(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);
int CListModernOptInit(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// dll stub

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Classic contact list",
	__VERSION_DWORD,
	"Display contacts, event notifications, protocol status",
	"Miranda IM project",
	"ghazan@miranda-im.org",
	"Copyright 2000-2008 Miranda IM project",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	DEFMOD_CLISTALL,
	#if defined( _UNICODE )
    {0x240a91dc, 0x9464, 0x457a, { 0x97, 0x87, 0xff, 0x1e, 0xa8, 0x8e, 0x77, 0xe3 }} //{240A91DC-9464-457a-9787-FF1EA88E77E3}
	#else
    {0x552cf71a, 0x249f, 0x4650, { 0xbb, 0x2b, 0x7c, 0xdb, 0x1f, 0xe7, 0xd1, 0x78 }} //{552CF71A-249F-4650-BB2B-7CDB1FE7D178}
	#endif
};

__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < MIRANDA_VERSION_CORE)
		return NULL;
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

static const MUUID interfaces[] = {MIID_CLIST, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when number of accounts has been changed

static int OnAccountsChanged( WPARAM wParam, LPARAM lParam )
{
	himlCListClc = (HIMAGELIST) CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when all modules got loaded

static int OnModernOptsInit(WPARAM wParam, LPARAM lParam);
static int OnModulesLoaded( WPARAM wParam, LPARAM lParam )
{
	HookEvent(ME_MODERNOPT_INITIALIZE, OnModernOptsInit);
	RegisterCListFonts();
	himlCListClc = (HIMAGELIST) CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options iniatialization

static int OnOptsInit(WPARAM wParam, LPARAM lParam)
{
	ClcOptInit(wParam, lParam);
	CluiOptInit(wParam, lParam);
	CListOptInit(wParam, lParam);
	return 0;
}

static int OnModernOptsInit(WPARAM wParam, LPARAM lParam)
{
	ClcModernOptInit(wParam, lParam);
	CListModernOptInit(wParam, lParam);
	CluiModernOptInit(wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// menu status services

static INT_PTR GetStatusMode(WPARAM wParam, LPARAM lParam)
{
	return pcli->currentDesiredStatusMode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

int __declspec(dllexport) CListInitialise(PLUGINLINK * link)
{
	pluginLink = link;
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// get the internal malloc/free()
	mir_getMMI( &mmi );
	mir_getLP( &pluginInfo );

	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);
	if ( (INT_PTR)pcli == CALLSERVICE_NOTFOUND ) {
LBL_Error:
		MessageBoxA( NULL, "This version of plugin requires Miranda IM 0.8.0.9 or later", "Fatal error", MB_OK );
		return 1;
	}
	if ( pcli->version < 6 )
		goto LBL_Error;

	pcli->pfnPaintClc = PaintClc;

	MySetLayeredWindowAttributes = (BOOL(WINAPI *) (HWND, COLORREF, BYTE, DWORD)) GetProcAddress(
		LoadLibraryA("user32.dll"), "SetLayeredWindowAttributes");

	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptsInit);

	InitCustomMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin loader aware of CList exports will never call this.

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin unloader

int __declspec(dllexport) Unload(void)
{
	return 0;
}
