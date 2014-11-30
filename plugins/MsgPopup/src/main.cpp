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
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {CF25D645-4DAB-4B0A-B9F1-DE1E86231F9B}
	{0xcf25d645, 0x4dab, 0x4b0a, {0xb9, 0xf1, 0xde, 0x1e, 0x86, 0x23, 0x1f, 0x9b}}
};

typedef int (WINAPI *MSGBOXPROC)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

MSGBOXPROC prevMessageBox;

#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516

void popupMessage(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
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
	mir_tstrcpy(ppd.lptzContactName, lpCaption);
	mir_tstrcpy(ppd.lptzText, lpText);
	PUAddPopupT(&ppd);
	if(options.Sound)
		MessageBeep(uType);
}

int WINAPI newMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	if (CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0) == CALLSERVICE_NOTFOUND || (uType & 0x0F))
		return prevMessageBox(hWnd, lpText, lpCaption, uType);

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
							TCHAR buf[200];

							g_HookError = TRUE;
							mir_sntprintf(buf, SIZEOF(buf), TranslateT("VirtualProtect failed. Code %d\nTry to call the author"), GetLastError());
							prevMessageBox(0, buf, TranslateT("Error"), MB_OK);
						}
					}
					*(DWORD*)ppfn = lpNewFunc;
					if(*(DWORD*)ppfn != lpNewFunc)
					{
						if(!g_HookError2)
						{
							g_HookError2 = TRUE;
							prevMessageBox(0, TranslateT("Hmm. Something goes wrong. I can't write into the memory.\nAnd as you can see, there are no any exception raised...\nTry to call the author"), TranslateT("Error"), MB_OK);
						}
					}
				}
			}
		}
	}
}

void HookAPI()
{
	DWORD lpMessageBox = (DWORD)GetProcAddress(GetModuleHandle(_T("USER32.DLL")), "MessageBoxW");
	DWORD lpPopupMsgBox = (DWORD)newMessageBox;

	prevMessageBox = (MSGBOXPROC)lpMessageBox;

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
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.hInstance = hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
		odp.ptszTitle = LPGENT("MessagePopup");
		odp.ptszGroup = LPGENT("Popups");
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		odp.pfnDlgProc = OptionsDlgProc;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

void LoadConfig()
{
	char szNameFG[4];
	char szNameBG[4];
	char szNameTO[4];
	int indx;

	for(indx = 0; indx < 4; indx++)
	{
		mir_snprintf(szNameFG, SIZEOF(szNameFG), "FG%d", indx);
		mir_snprintf(szNameBG, SIZEOF(szNameBG), "BG%d", indx);
		mir_snprintf(szNameTO, SIZEOF(szNameTO), "TO%d", indx);
		options.FG[indx] = db_get_dw(NULL, SERVICENAME, szNameFG, optionsDefault.FG[indx]);
		options.BG[indx] = db_get_dw(NULL, SERVICENAME, szNameBG, optionsDefault.BG[indx]);
		options.Timeout[indx] = db_get_dw(NULL, SERVICENAME, szNameTO, (DWORD)optionsDefault.Timeout[indx]);
	}
	options.Sound = db_get_b(NULL, SERVICENAME, "Sound", (DWORD)optionsDefault.Sound);

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
