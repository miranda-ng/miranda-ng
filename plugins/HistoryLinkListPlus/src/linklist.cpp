// History Linklist Plus
// Copyright (C) 2010 Thomas Wendel, gureedo
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "linklist.h"

// Global variables
HINSTANCE hInst;                                    
HINSTANCE hRichEdit;                                

HANDLE hWindowList;
HCURSOR splitCursor;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {DA0B09F5-9C66-488C-AE37-8A5F191C9079}
	{0xDA0B09F5, 0x9C66, 0x488C, {0xAE, 0x37, 0x8A, 0x5F, 0x19, 0x1C, 0x90, 0x79}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	//  Load Rich Edit control
	hRichEdit = LoadLibrary(_T("Msftedit.DLL"));
	if (!hRichEdit)
	{   
		//  If Rich Edit DLL load fails, exit
		MessageBox(NULL, _T("Unable to load the Rich Edit control!"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		FreeLibrary(hRichEdit);
		return 1;
	}

#ifdef DEBUG
	{
		int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		flag |= _CRTDBG_LEAK_CHECK_DF|_CRTDBG_CHECK_ALWAYS_DF;
		_CrtSetDbgFlag(flag);
	}
#endif

	CreateServiceFunction("Linklist/MenuCommand", LinkList_Main);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LINKLISTICON));
	mi.ptszName = LPGENT("&Create Linklist");
	mi.pszService = "Linklist/MenuCommand";
	Menu_AddContactMenuItem(&mi);
	
	hWindowList = WindowList_Create();

	WNDCLASS wndclass = { 0 };
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ProgressBarDlg;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LINKLISTICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszClassName = _T("Progressbar");
	RegisterClass(&wndclass);

	splitCursor = LoadCursor(NULL, IDC_SIZENS);
	
	HookEvent(ME_OPT_INITIALISE, InitOptionsDlg);
	HookEvent(ME_DB_EVENT_ADDED, DBUpdate);
	
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hWindowList);
	DestroyCursor(splitCursor);
	return 0;
}

int InitOptionsDlg(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszGroup = LPGEN("History");
	odp.pszTitle = LPGEN("History Linklist");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_DLG);
	odp.pfnDlgProc = OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

static INT_PTR LinkList_Main(WPARAM hContact, LPARAM)
{
	HWND hWnd = WindowList_Find(hWindowList, hContact);
	if ( hWnd != NULL ) {
		int len;
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		len = GetWindowTextLength(GetDlgItem(hWnd, IDC_MAIN));
		PostMessage(GetDlgItem(hWnd, IDC_MAIN), EM_SETSEL, (WPARAM)len, (LPARAM)len);
		return 0;
	}	
	
	HANDLE hEvent = db_event_first(hContact);
	if (hEvent == NULL) {
		MessageBox(NULL, TXT_EMPTYHISTORY, TXT_PLUGINNAME, MB_OK | MB_ICONINFORMATION );
		return 0;
	}

	int histCount = db_event_count(hContact), actCount = 0;

	DBEVENTINFO dbe = { sizeof(dbe) };
	dbe.cbBlob = db_event_getBlobSize(hEvent);
	dbe.pBlob  = (PBYTE)malloc(dbe.cbBlob+1);
	db_event_get(hEvent, &dbe);
	dbe.pBlob[dbe.cbBlob] = 0;

	RECT DesktopRect;
	GetWindowRect(GetDesktopWindow(), &DesktopRect);
	HWND hWndProgress = CreateWindow(_T("Progressbar"), TranslateT("Processing history..."), WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 350, 45, NULL, NULL, hInst, NULL);
	if ( hWndProgress == 0 ) {
		free(dbe.pBlob);
		MessageBox(NULL, TranslateT("Could not create window!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION );
		return -1;
	}
	SetWindowPos(hWndProgress, HWND_TOP, (int)(DesktopRect.right*0.5)-175, (int)(DesktopRect.bottom*0.5)-22, 0, 0, SWP_NOSIZE);
	ShowWindow(hWndProgress, SW_SHOW);
	SetForegroundWindow(hWndProgress);

	LISTELEMENT *listStart = (LISTELEMENT*)malloc(sizeof(LISTELEMENT));
	ZeroMemory(listStart, sizeof(LISTELEMENT));

	while( 1 ) {
		if ( dbe.eventType == EVENTTYPE_URL || dbe.eventType == EVENTTYPE_MESSAGE ) {
			// Call function to find URIs
			if ( ExtractURI(&dbe, hEvent, listStart) < 0 ) {
				free(dbe.pBlob);
				RemoveList(listStart);
				MessageBox(NULL, TranslateT("Could not allocate memory!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION);
				return -1;
			}
		}
		actCount++;
		if ( ((int)(((float)actCount/histCount)*100.00)) % 10 == 0 )
			SendMessage(hWndProgress, WM_COMMAND, 100, ((int)(((float)actCount/histCount)*100.00)));
		
		hEvent = db_event_next(hContact, hEvent);
		if ( hEvent == NULL )
			break;

		free(dbe.pBlob);
		dbe.cbBlob = db_event_getBlobSize(hEvent);
		dbe.pBlob = (PBYTE)malloc(dbe.cbBlob+1);
		db_event_get(hEvent, &dbe);
		dbe.pBlob[dbe.cbBlob] = 0;
	}
	free(dbe.pBlob);
	SendMessage(hWndProgress, WM_CLOSE, 0, 0);
	if ( ListCount(listStart) <= 0 ) {	
		RemoveList(listStart);
		MessageBox(NULL, TXT_NOLINKINHISTORY, TXT_PLUGINNAME, MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	DIALOGPARAM *DlgParam = (DIALOGPARAM*)malloc(sizeof(DIALOGPARAM));
	DlgParam->hContact    = hContact;
	DlgParam->listStart   = listStart;
	DlgParam->findMessage = 0;
	DlgParam->chrg.cpMax  = -1;
	DlgParam->chrg.cpMin  = -1;

	HWND hWndMain = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, MainDlgProc, (LPARAM)DlgParam);
	if (hWndMain == 0) {
		RemoveList(listStart);
		MessageBox(NULL, TranslateT("Could not create window!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION );
		return -1;
	}

	ShowWindow(hWndMain, SW_SHOW);	
	return 0;
}
