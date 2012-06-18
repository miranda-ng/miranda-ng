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

#include <windows.h>
#include "resource.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

// Miranda SDK Includes
#pragma warning(disable:4996)
#pragma warning(disable:4100)
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_options.h>
#pragma warning(default:4100)
#pragma warning(default:4996)

#include "linklist_dlg.h"
#include "linklist_fct.h"
#include "linklist.h"
#include "language.h"

#define MIID_LINKLIST  { 0xc9c94733, 0xa054, 0x42b9, { 0x89, 0xcb, 0xb9, 0x71, 0x27, 0xa7, 0xa3, 0x43 } }

// Global variables
HINSTANCE hInst;                                    
HINSTANCE hRichEdit;                                
PLUGINLINK *pluginLink;
HANDLE hWindowList;
HCURSOR splitCursor;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"History Linklist Plus",
	 PLUGIN_MAKE_VERSION(0,0,0,2),
	"Generates a list of extracted URIs from the history",
	"Thomas Wendel, gureedo",
	"gureedo@gmail.com",
	"© 2010-2011 gureedo",
	"http://www.miranda-im.org",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
	{ 0xDA0B09F5, 0x9C66, 0x488C, { 0xAE, 0x37, 0x8A, 0x5F, 0x19, 0x1C, 0x90, 0x79 } } // {DA0B09F5-9C66-488C-AE37-8A5F191C9079}
};

static const MUUID interfaces[] = {MIID_LINKLIST, MIID_LAST};
struct MM_INTERFACE mmi;

// Functions

__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(lpvReserved);

	hInst = hinstDLL;
	//  Load Rich Edit control
	if ( fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_THREAD_ATTACH )
	{
		hRichEdit = LoadLibrary(_T("RICHED32.DLL"));
		if ( !hRichEdit )
		{   
			//  If Rich Edit DLL load fails, exit
			MessageBox(NULL, _T("Unable to load the Rich Edit control!"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}

#ifdef DEBUG
		{
			int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
			flag |= _CRTDBG_LEAK_CHECK_DF|_CRTDBG_CHECK_ALWAYS_DF;
			_CrtSetDbgFlag(flag);
		}
#endif

		DisableThreadLibraryCalls(hinstDLL);
	}
	if ( fdwReason == DLL_PROCESS_DETACH || fdwReason == DLL_THREAD_DETACH ) {
		FreeLibrary(hRichEdit);
	}

	return TRUE;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	CLISTMENUITEM linklistmenuitem;
	WNDCLASS wndclass;

	pluginLink = link;
	mir_getLP(&pluginInfo);
	CreateServiceFunction("Linklist/MenuCommand", LinkList_Main);
	ZeroMemory(&linklistmenuitem, sizeof(linklistmenuitem));
	linklistmenuitem.cbSize = sizeof(linklistmenuitem);
	linklistmenuitem.position = 0x00;
	linklistmenuitem.flags = CMIF_TCHAR;
	linklistmenuitem.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LINKLISTICON));
	linklistmenuitem.ptszName = LPGENT("&Create Linklist");
	linklistmenuitem.pszService = "Linklist/MenuCommand";
	Menu_AddContactMenuItem(&linklistmenuitem);
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ProgressBarDlg;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LINKLISTICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszClassName = _T("Progressbar");
	wndclass.lpszMenuName = NULL;
	RegisterClass(&wndclass);

	splitCursor = LoadCursor(NULL, IDC_SIZENS);

	mir_getMMI(&mmi);
	
	HookEvent(ME_OPT_INITIALISE, InitOptionsDlg);
	HookEvent(ME_DB_EVENT_ADDED, DBUpdate);
	
	return 0;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	UNREFERENCED_PARAMETER(mirandaVersion);

	return &pluginInfo;
}

int __declspec(dllexport) Unload(void)
{
	UnhookEvent(ME_DB_EVENT_ADDED);
	DestroyCursor(splitCursor);
	return 0;
}

int InitOptionsDlg(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE optionsDialog;
	UNREFERENCED_PARAMETER(lParam);

	ZeroMemory(&optionsDialog, sizeof(OPTIONSDIALOGPAGE));
	optionsDialog.cbSize = sizeof(optionsDialog);
	optionsDialog.hInstance = hInst;
	optionsDialog.ptszGroup = LPGENT("History");
	optionsDialog.ptszTitle = LPGENT("History Linklist");
	optionsDialog.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_DLG);
	optionsDialog.pfnDlgProc = OptionsDlgProc;
	optionsDialog.expertOnlyControls = NULL;
	optionsDialog.nExpertOnlyControls = 0;
	optionsDialog.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	Options_AddPage(wParam, (LPARAM)&optionsDialog);
	return 0;
}

static INT_PTR LinkList_Main(WPARAM wParam,LPARAM lParam)
{
	HANDLE hEvent;
	HANDLE hContact = (HANDLE)wParam;
	DBEVENTINFO dbe;
    HWND hWnd;
	HWND hWndProgress;
	HWND hWndMain;

	int histCount = 0;
	int actCount = 0;
	
	RECT DesktopRect;
	DIALOGPARAM *DlgParam;
	LISTELEMENT *listStart;

	UNREFERENCED_PARAMETER(lParam);

	listStart = (LISTELEMENT*)malloc(sizeof(LISTELEMENT));
	ZeroMemory(listStart, sizeof(LISTELEMENT));

	hWnd = WindowList_Find(hWindowList,hContact);
	if ( hWnd != NULL )
	{
		int len;
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		len = GetWindowTextLength(GetDlgItem(hWnd, IDC_MAIN));
		PostMessage(GetDlgItem(hWnd, IDC_MAIN), EM_SETSEL, (WPARAM)len, (LPARAM)len);
		return 0;
	}	
	
	hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0);
	if ( hEvent == NULL )
	{
		MessageBox(NULL, TranslateT(TXT_EMPTYHISTORY), TranslateT(TXT_PLUGINNAME), MB_OK | MB_ICONINFORMATION );
		return 0;
	}

	histCount = CallService(MS_DB_EVENT_GETCOUNT, (WPARAM)hContact, 0);
	ZeroMemory(&dbe, sizeof(dbe));
	dbe.cbSize = sizeof(dbe);
	dbe.cbBlob = (int)CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, 0);
	dbe.pBlob  = (PBYTE)malloc(dbe.cbBlob+1);
	CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbe);
	dbe.pBlob[dbe.cbBlob] = 0;

	GetWindowRect(GetDesktopWindow(), &DesktopRect);
	hWndProgress = CreateWindow(_T("Progressbar"), TranslateT("Processing history..."), WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 350, 45, NULL, NULL, hInst, NULL);
	if ( hWndProgress == 0 )
	{
		free(dbe.pBlob);
		MessageBox(NULL, TranslateT("Could not create window!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION );
		return -1;
	}
	SetWindowPos(hWndProgress, HWND_TOP, (int)(DesktopRect.right*0.5)-175, (int)(DesktopRect.bottom*0.5)-22, 0, 0, SWP_NOSIZE);
	ShowWindow(hWndProgress, SW_SHOW);
	SetForegroundWindow(hWndProgress);

	while( 1 )
	{
		if ( dbe.eventType == EVENTTYPE_URL || dbe.eventType == EVENTTYPE_MESSAGE )
		{
			// Call function to find URIs
			if ( ExtractURI(&dbe, hEvent, listStart) < 0 )
			{
				free(dbe.pBlob);
				RemoveList(listStart);
				MessageBox(NULL, TranslateT("Could not allocate memory!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION);
				return -1;
			}
		}
		actCount++;
		if ( ((int)(((float)actCount/histCount)*100.00)) % 10 == 0 )
			SendMessage(hWndProgress, WM_COMMAND, 100, ((int)(((float)actCount/histCount)*100.00)));
		
		hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hEvent, 0);
		if ( hEvent == NULL )
			break;

		free(dbe.pBlob);
		dbe.cbBlob = (int)CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, 0);
		dbe.pBlob = (PBYTE)malloc(dbe.cbBlob+1);
		CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbe);
		dbe.pBlob[dbe.cbBlob] = 0;
	}
	free(dbe.pBlob);
	SendMessage(hWndProgress, WM_CLOSE, 0, 0);
	if ( ListCount(listStart) <= 0 ) {	
		RemoveList(listStart);
		MessageBox(NULL, TranslateT("There are no links in history!"), TranslateT(TXT_PLUGINNAME), MB_OK | MB_ICONINFORMATION);
		return 0;
	}



	DlgParam = (DIALOGPARAM*)malloc(sizeof(DIALOGPARAM));
	DlgParam->hContact    = hContact;
	DlgParam->listStart   = listStart;
	DlgParam->findMessage = 0;
	DlgParam->chrg.cpMax  = -1;
	DlgParam->chrg.cpMin  = -1;

	hWndMain = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, MainDlgProc, (LPARAM)DlgParam);
	if ( hWndMain == 0 )
	{
		RemoveList(listStart);
		MessageBox(NULL, TranslateT("Could not create window!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION );
		return -1;
	}

	ShowWindow(hWndMain, SW_SHOW);	
	return 0;
}
