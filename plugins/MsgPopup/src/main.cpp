/*

MessagePopup - replacer of MessageBox'es

Copyright 2004 Denis Stanishevskiy

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
#include "common.h"

HINSTANCE hInst;
int hLangpack;

MSGBOXOPTIONS optionsDefault =
{
	{0, 0xFFFFFF, 0, 0},
	{0xBFFFFF, 0x0000FF, 0xB0B0FF, 0xECFF93},
	{-1,-1,-1,-1},
	TRUE
};
MSGBOXOPTIONS options;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	SERVICENAME,
	PLUGIN_MAKE_VERSION( 0,0,0,1 ),
	"This stuff will replace MessageBox'es [whose have only OK button] into Popups",
	"Denis Stanishevskiy // StDenis",
	"stdenformiranda(at)fromru(dot)com",
	"Copyright (c) 2004, Denis Stanishevskiy",
	"",
	0, 0
};

typedef int (WINAPI *MSGBOXPROC)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

MSGBOXPROC prevMessageBoxA;

#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516

void popupMessage(LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	POPUPDATAT ppd = {0};	
	int iIcon;
	int indx;

	switch(uType & 0xF0)
	{
		case MB_ICONHAND:
			indx = 1;
			iIcon = OIC_HAND;
			break;
		case MB_ICONEXCLAMATION:
			indx = 2;
			iIcon = OIC_BANG;
			break;
		case MB_ICONQUESTION:
			indx = 3;
			iIcon = OIC_QUES;
			break;
		default:
			indx = 0;
			iIcon = OIC_NOTE;
			break;
			
	}
	ppd.colorBack = options.BG[indx];
	ppd.colorText = options.FG[indx];
	ppd.iSeconds  = options.Timeout[indx];

	ppd.lchIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(iIcon), IMAGE_ICON, SM_CXSMICON, SM_CYSMICON, LR_SHARED);
	lstrcpy(ppd.lpzContactName, lpCaption);
	lstrcpy(ppd.lpzText, lpText);
	PUAddPopUpT(&ppd);
	if(options.Sound)
		MessageBeep(uType);
}

int WINAPI newMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	if(CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == CALLSERVICE_NOTFOUND || (uType & 0x0F))
		return prevMessageBoxA(hWnd, lpText, lpCaption, uType);

	popupMessage(lpText, lpCaption,uType);
	return IDOK;
}

BOOL g_HookError = FALSE;
BOOL g_HookError2 = FALSE;
int g_mod = 0;

void HookOnImport(HMODULE hModule, char *lpszImpModName, DWORD lpOrigFunc, DWORD lpNewFunc)
{
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) ImageDirectoryEntryToData(
		hModule, 
		TRUE,
		IMAGE_DIRECTORY_ENTRY_IMPORT,
		&ulSize);
	if(pImportDesc == NULL) return;

	for(; pImportDesc->Name; pImportDesc++) 
	{
		char *pszModName = (char *)((PBYTE)hModule + pImportDesc->Name);
		
		if (lstrcmpiA(lpszImpModName, pszModName) == 0)
		{
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModule + pImportDesc->FirstThunk);

			for (; pThunk->u1.Function; pThunk++) 
			{
				DWORD* ppfn = (DWORD*) &pThunk->u1.Function;

				if(*ppfn == lpOrigFunc)
				{
					DWORD oldProtect;

					g_mod++;

					if(!VirtualProtect((LPVOID)ppfn, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
					{
						if(!g_HookError)
						{
							char buf[200];

							g_HookError = TRUE;
							wsprintf(buf, "VirtualProtect failed. Code %d\n"
								"Try to call the author", GetLastError());
							prevMessageBoxA(0, buf, "MsgBox", MB_OK);
						}
					}
					*(DWORD*)ppfn = lpNewFunc;
					if(*(DWORD*)ppfn != lpNewFunc)
					{
						if(!g_HookError2)
						{
							g_HookError2 = TRUE;
							prevMessageBoxA(0, "Hmm. Something goes wrong. I can't write into the memory.\n"
								          "And as u can c, there are no any exception raised..\n"
										  "Try to call the author", "MsgBox", MB_OK);
						}
					}
				}
			}
		}
	}
}

void HookAPI()
{
	DWORD lpMessageBox = (DWORD)GetProcAddress(GetModuleHandle("USER32.DLL"), "MessageBoxA");
	DWORD lpPopupMsgBox = (DWORD)newMessageBoxA;

	prevMessageBoxA = (MSGBOXPROC)lpMessageBox;

	BOOL          bFound      = FALSE; 
	HANDLE        hModuleSnap = NULL; 
	MODULEENTRY32 me32        = {0}; 

	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if(hModuleSnap == INVALID_HANDLE_VALUE) 
		return; 
 
	me32.dwSize = sizeof(MODULEENTRY32); 
	if(Module32First(hModuleSnap, &me32)) 
	{ 
		do 
		{ 
			HookOnImport(me32.hModule, "USER32.DLL", lpMessageBox, lpPopupMsgBox);
		} 
		while (!bFound && Module32Next(hModuleSnap, &me32)); 
	} 
	CloseHandle (hModuleSnap); 

	return; 
}

int HookedInit(WPARAM wParam, LPARAM lParam)
{
	HookAPI();

	return 0;
}

int HookedOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.ptszTitle = LPGENT("MessagePopup");
	odp.ptszGroup = LPGENT("Popups");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = OptionsDlgProc;
	Options_AddPage(wParam, &odp);

	return 0;
}

void LoadConfig()
{
	char *szNameFG = "FGx";
	char *szNameBG = "BGx";
	char *szNameTO = "TOx";
	int indx;

	for(indx = 0; indx < 4; indx++)
	{
		szNameFG[2] = szNameBG[2] = szNameTO[2] = (char)(indx + '0');
		options.FG[indx] = DBGetContactSettingDword(NULL, SERVICENAME, szNameFG, optionsDefault.FG[indx]);
		options.BG[indx] = DBGetContactSettingDword(NULL, SERVICENAME, szNameBG, optionsDefault.BG[indx]);
		options.Timeout[indx] = DBGetContactSettingDword(NULL, SERVICENAME, szNameTO, (DWORD)optionsDefault.Timeout[indx]);
	}
	options.Sound = DBGetContactSettingByte(NULL, SERVICENAME, "Sound", (DWORD)optionsDefault.Sound);

}
extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, HookedInit);
    HookEvent(ME_OPT_INITIALISE, HookedOptions);

	LoadConfig();

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}
