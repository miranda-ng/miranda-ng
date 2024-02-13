/*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "chat.h"

HCURSOR g_hCurHyperlinkHand;
HANDLE hHookIconsChanged, hHookIconPressedEvt, hHookSrmmEvent;

static HGENMENU hmiEmpty;

void LoadSrmmToolbarModule();
void UnloadSrmmToolbarModule();

/////////////////////////////////////////////////////////////////////////////////////////
// Empty history service for main menu

class CEmptyHistoryDlg : public CDlgBase
{
	MCONTACT m_hContact;
	CCtrlCheck chkDelHistory, chkForEveryone;

public:
	char *szProto;
	bool bDelHistory, bForEveryone;

	CEmptyHistoryDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_EMPTYHISTORY),
		m_hContact(hContact),
		chkDelHistory(this, IDC_DELSERVERHISTORY),
		chkForEveryone(this, IDC_BOTH)
	{
		szProto = Proto_GetBaseAccountName(hContact);
		bDelHistory = ProtoServiceExists(szProto, PS_EMPTY_SRV_HISTORY);
		bForEveryone = (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4) & PF4_DELETEFORALL) != 0;
	}

	bool OnInitDialog() override
	{
		chkDelHistory.SetState(false);
		chkDelHistory.Enable(bDelHistory);

		bool bEnabled = bDelHistory && bForEveryone;
		chkForEveryone.SetState(!bEnabled);
		chkForEveryone.Enable(bEnabled);

		LOGFONT lf;
		HFONT hFont = (HFONT)SendDlgItemMessage(m_hwnd, IDOK, WM_GETFONT, 0, 0);
		GetObject(hFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		SendDlgItemMessage(m_hwnd, IDC_TOPLINE, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), 0);

		wchar_t szFormat[256], szFinal[256];
		GetDlgItemText(m_hwnd, IDC_TOPLINE, szFormat, _countof(szFormat));
		mir_snwprintf(szFinal, szFormat, Clist_GetContactDisplayName(m_hContact));
		SetDlgItemText(m_hwnd, IDC_TOPLINE, szFinal);

		SetFocus(GetDlgItem(m_hwnd, IDNO));
		SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return true;
	}

	bool OnApply() override
	{
		bDelHistory = chkDelHistory.IsChecked();
		bForEveryone = chkForEveryone.IsChecked();
		return true;
	}

	void OnDestroy() override
	{
		DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TOPLINE, WM_GETFONT, 0, 0));
	}
};

static INT_PTR svcEmptyHistory(WPARAM hContact, LPARAM lParam)
{
	CEmptyHistoryDlg dlg(hContact);
	if (lParam == 0)
		if (dlg.DoModal() != IDOK)
			return 1;

	DB::ECPTR pCursor(DB::Events(hContact));
	while (pCursor.FetchNext())
		pCursor.DeleteEvent();

	if (Contact::IsGroupChat(hContact))
		if (auto *si = SM_FindSessionByContact(hContact))
			Chat_EmptyHistory(si);

	if (dlg.bDelHistory)
		CallContactService(hContact, PS_EMPTY_SRV_HISTORY, hContact, CDF_DEL_HISTORY | (dlg.bForEveryone ? CDF_FOR_EVERYONE : 0));
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hmiEmpty, db_event_first(hContact) != 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SrmmModulesLoaded()
{
	// menu item
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x0d4306aa, 0xe31e, 0x46ee, 0x89, 0x88, 0x3a, 0x2e, 0x05, 0xa6, 0xf3, 0xbc);
	mi.pszService = MS_HISTORY_EMPTY;
	mi.name.a = LPGEN("Empty history");
	mi.position = 1000090001;
	mi.hIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	hmiEmpty = Menu_AddContactMenuItem(&mi);

	// create menu item in main menu for empty system history
	SET_UID(mi, 0x633AD23C, 0x24B5, 0x4914, 0xB2, 0x40, 0xAD, 0x9F, 0xAC, 0xB5, 0x64, 0xED);
	mi.position = 500060002;
	mi.name.a = LPGEN("Empty system history");
	mi.pszService = MS_HISTORY_EMPTY;
	mi.hIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadSrmmModule()
{
	g_hCurHyperlinkHand = LoadCursor(nullptr, IDC_HAND);

	LoadSrmmToolbarModule();

	CreateServiceFunction(MS_HISTORY_EMPTY, svcEmptyHistory);

	hHookSrmmEvent = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookIconsChanged = CreateHookableEvent(ME_MSG_ICONSCHANGED);
	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);
	return 0;
}

void UnloadSrmmModule()
{
	DestroyHookableEvent(hHookIconsChanged);
	DestroyHookableEvent(hHookSrmmEvent);
	DestroyHookableEvent(hHookIconPressedEvt);

	DestroyCursor(g_hCurHyperlinkHand);

	UnloadSrmmToolbarModule();
}
