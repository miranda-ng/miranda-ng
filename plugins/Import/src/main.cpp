/*

Import plugin for Miranda NG

Copyright (C) 2012 George Hazan

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

#include "import.h"
#include "version.h"
#include "resource.h"

#include "..\..\Utils\mir_icons.h"

int nImportOption;
int nCustomOptions;

static HANDLE hImportService = NULL;

HINSTANCE hInst;
INT_PTR CALLBACK WizardDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);

static HWND hwndWizard = NULL;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Import contacts and messages",
	__VERSION_DWORD,
	"Imports contacts and messages from another Miranda profile or from an external program.",
	"Miranda team",
	"info@miranda-im.org",
	"© 2000-2010 Martin Öberg, Richard Hughes, Dmitry Kuzkin, George Hazan",
	"http://nightly.miranda.im/",
	UNICODE_AWARE,	//{2D77A746-00A6-4343-BFC5-F808CDD772EA}
      {0x2d77a746, 0xa6, 0x4343, { 0xbf, 0xc5, 0xf8, 0x8, 0xcd, 0xd7, 0x72, 0xea }}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static INT_PTR ImportCommand(WPARAM wParam,LPARAM lParam)
{
	if (IsWindow(hwndWizard)) {
		SetForegroundWindow(hwndWizard);
		SetFocus(hwndWizard);
	}
	else hwndWizard = CreateDialog(hInst, MAKEINTRESOURCE(IDD_WIZARD), NULL, WizardDlgProc);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns an information about a plugin

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaInterfaces - returns the protocol interface to the core

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_IMPORT, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////
// Performs a primary set of actions upon plugin loading

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	if (DBGetContactSettingByte(NULL, IMPORT_MODULE, IMP_KEY_FR, 0))
		return 0;

	// Only autorun import wizard if at least one protocol is installed
	int nProtocols = 0;
	PROTOCOLDESCRIPTOR **ppProtos = NULL;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&nProtocols, (LPARAM)&ppProtos);
	for (int n=0; n < nProtocols; n++) {
		if (ppProtos[n]->type == PROTOTYPE_PROTOCOL) {
			CallService(IMPORT_SERVICE, 0, 0);
			DBWriteContactSettingByte(NULL, IMPORT_MODULE, IMP_KEY_FR, 1);
			break;
	}	}
	return 0;
}

static int OnExit(WPARAM wParam, LPARAM lParam)
{
	if ( hwndWizard )
		SendMessage(hwndWizard, WM_CLOSE, 0, 0);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP( &pluginInfo );

	hImportService = CreateServiceFunction(IMPORT_SERVICE, ImportCommand);
	
	CLISTMENUITEM mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.icolibItem = IcoLib_Register("import_main", "Import", LPGEN("Import..."), IDI_IMPORT);
	mi.pszName = LPGEN("&Import...");
	mi.position = 500050000;
	mi.pszService = IMPORT_SERVICE;
	mi.flags = CMIF_ICONFROMICOLIB;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_OKTOEXIT, OnExit);
	
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icex);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload a plugin

extern "C" __declspec(dllexport) int Unload(void)
{
	if (hImportService)
		DestroyServiceFunction(hImportService);

	return 0;
}
