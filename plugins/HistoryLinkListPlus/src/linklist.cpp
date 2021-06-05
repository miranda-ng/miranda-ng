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

#include "stdafx.h"

MWindowList hWindowList;
HCURSOR splitCursor;
CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {DA0B09F5-9C66-488C-AE37-8A5F191C9079}
	{0xDA0B09F5, 0x9C66, 0x488C, {0xAE, 0x37, 0x8A, 0x5F, 0x19, 0x1C, 0x90, 0x79}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(LINKLIST_MODULE, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int InitOptionsDlg(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("History");
	odp.szTitle.a = LPGEN("History Linklist");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_DLG);
	odp.pfnDlgProc = OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

static INT_PTR LinkList_Main(WPARAM hContact, LPARAM)
{
	HWND hWnd = WindowList_Find(hWindowList, hContact);
	if (hWnd != nullptr) {
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		int len = GetWindowTextLength(GetDlgItem(hWnd, IDC_MAIN));
		PostMessage(GetDlgItem(hWnd, IDC_MAIN), EM_SETSEL, (WPARAM)len, (LPARAM)len);
		return 0;
	}

	DB::ECPTR pCursor(DB::Events(hContact));
	MEVENT hEvent = pCursor.FetchNext();
	if (hEvent == NULL) {
		MessageBox(nullptr, TXT_EMPTYHISTORY, TXT_PLUGINNAME, (MB_OK | MB_ICONINFORMATION));
		return 0;
	}

	int histCount = db_event_count(hContact), actCount = 0;

	RECT DesktopRect;
	GetWindowRect(GetDesktopWindow(), &DesktopRect);
	HWND hWndProgress = CreateWindow(L"Progressbar", TranslateT("Processing history..."), WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 350, 45, nullptr, nullptr, g_plugin.getInst(), nullptr);
	if (hWndProgress == nullptr) {
		MessageBox(nullptr, TranslateT("Could not create window!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}

	SetWindowPos(hWndProgress, HWND_TOP, (int)((DesktopRect.right / 2) - 175), (int)((DesktopRect.bottom / 2) - 22), 0, 0, SWP_NOSIZE);
	ShowWindow(hWndProgress, SW_SHOW);
	SetForegroundWindow(hWndProgress);

	LISTELEMENT *listStart = (LISTELEMENT *)mir_alloc(sizeof(LISTELEMENT));
	memset(listStart, 0, sizeof(LISTELEMENT));

	do {
		DB::EventInfo dbe;
		dbe.cbBlob = -1;
		db_event_get(hEvent, &dbe);

		if (dbe.eventType == EVENTTYPE_MESSAGE) {
			// Call function to find URIs
			if (ExtractURI(&dbe, hEvent, listStart) < 0) {
				mir_free(dbe.pBlob);
				RemoveList(listStart);
				MessageBox(nullptr, TranslateT("Could not allocate memory!"), TranslateT("Error"), MB_OK | MB_ICONEXCLAMATION);
				return -1;
			}
		}
		
		actCount++;
		if (((int)(((float)actCount / histCount) * 100.00)) % 10 == 0)
			SendMessage(hWndProgress, WM_COMMAND, 100, ((int)(((float)actCount / histCount) * 100.00)));
	}
		while (hEvent = pCursor.FetchNext());

	SendMessage(hWndProgress, WM_CLOSE, 0, 0);
	if (ListCount(listStart) <= 0) {
		RemoveList(listStart);
		MessageBox(nullptr, TXT_NOLINKINHISTORY, TXT_PLUGINNAME, (MB_OK | MB_ICONINFORMATION));
		return 0;
	}

	DIALOGPARAM *DlgParam = (DIALOGPARAM*)mir_alloc(sizeof(DIALOGPARAM));
	DlgParam->hContact = hContact;
	DlgParam->listStart = listStart;
	DlgParam->findMessage = 0;
	DlgParam->chrg.cpMax = -1;
	DlgParam->chrg.cpMin = -1;

	HWND hWndMain = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MAIN_DLG), nullptr, MainDlgProc, (LPARAM)DlgParam);
	if (hWndMain == nullptr) {
		mir_free(DlgParam);
		RemoveList(listStart);
		MessageBox(nullptr, TranslateT("Could not create window!"), TranslateT("Error"), (MB_OK | MB_ICONEXCLAMATION));
		return -1;
	}

	ShowWindow(hWndMain, SW_SHOW);
	return 0;
}

int CMPlugin::Load()
{
	CreateServiceFunction("Linklist/MenuCommand", LinkList_Main);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x2964dc6c, 0x9cf9, 0x4f20, 0x8f, 0x8a, 0xc6, 0xfe, 0xe2, 0x65, 0xac, 0xc9);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_LINKLISTICON));
	mi.name.w = LPGENW("&Create Linklist");
	mi.pszService = "Linklist/MenuCommand";
	Menu_AddContactMenuItem(&mi);

	hWindowList = WindowList_Create();

	WNDCLASS wndclass = {};
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ProgressBarDlg;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_LINKLISTICON));
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass.lpszClassName = L"Progressbar";
	RegisterClass(&wndclass);

	splitCursor = LoadCursor(nullptr, IDC_SIZENS);

	HookEvent(ME_OPT_INITIALISE, InitOptionsDlg);
	HookEvent(ME_DB_EVENT_ADDED, DBUpdate);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WindowList_Destroy(hWindowList);
	DestroyCursor(splitCursor);
	return 0;
}
