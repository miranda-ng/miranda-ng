/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-12 Miranda IM, 2012-13 Miranda NG project,
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

HINSTANCE g_hInst = 0;
CLIST_INTERFACE* pcli = NULL;
HIMAGELIST himlCListClc = NULL;
Qt_clist *clist = NULL;
QApplication *app = NULL;

int hLangpack;

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


BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD /*dwReason*/, LPVOID /*reserved*/)
{
	g_hInst = hInstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
    // {3F9B6DD8-7F69-46f2-B4BA-360953A12BF3}
    { 0x3f9b6dd8, 0x7f69, 0x46f2, { 0xb4, 0xba, 0x36, 0x9, 0x53, 0xa1, 0x2b, 0xf3 } }

};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD /*mirandaVersion*/)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_CLIST, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////
// called when number of accounts has been changed

static int OnAccountsChanged( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
	himlCListClc = (HIMAGELIST) CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when all modules got loaded

static int OnModernOptsInit(WPARAM wParam, LPARAM lParam);
static int OnModulesLoaded( WPARAM /*wParam*/, LPARAM /*lParam*/ )
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

static INT_PTR GetStatusMode(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return pcli->currentDesiredStatusMode;
}



void qapplication_stub(void */*arg*/)
{
	while(true)
	{
		app->processEvents();
		app->thread()->wait(20);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

extern "C" __declspec(dllexport) int CListInitialise()
{
	mir_getLP( &pluginInfo );

    char text[_MAX_PATH];
    char *q;
#ifdef WIN32
    GetModuleFileNameA(g_hInst, text, sizeof(text));
#endif
    q = strstr(text, ".dll");
    q += strlen(".dll");
    *q = '\0';
    _strupr(text);

    char **argv = new char* [1];
    argv[0] = _strdup(text);
    int *argc = new int;
    *argc = 1;
    app = new QApplication(*argc, NULL);

    mir_forkthread(qapplication_stub, text);

	clist = new Qt_clist;

	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);

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



extern "C" __declspec(dllexport) int Load(void)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin unloader

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
