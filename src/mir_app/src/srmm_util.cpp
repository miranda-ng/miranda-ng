/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team,
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

#include "stdafx.h"
#include "chat.h"
#include "file.h"

const char *g_pszHotkeySection;

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Srmm_GetWindowData(MCONTACT hContact, MessageWindowData &mwd)
{
	if (hContact == 0)
		return 1;

	HWND hwnd = WindowList_Find(g_hWindowList, hContact);
	if (hwnd == nullptr)
		return 1;

	mwd.hwndWindow = hwnd;
	mwd.pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	mwd.uState = MSG_WINDOW_STATE_EXISTS;
	if (IsWindowVisible(hwnd))
		mwd.uState |= MSG_WINDOW_STATE_VISIBLE;
	if (GetForegroundWindow() == hwnd)
		mwd.uState |= MSG_WINDOW_STATE_FOCUS;
	if (IsIconic(hwnd))
		mwd.uState |= MSG_WINDOW_STATE_ICONIC;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_Broadcast(UINT msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(g_hWindowList, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_CreateHotkey(const char *pszSection, const char *pszDescription)
{
	g_pszHotkeySection = pszSection;

	uint16_t wHotKey = HOTKEYCODE(0, VK_RETURN);
	if (db_get_b(0, SRMSGMOD, "SendOnCtrlEnter"))
		wHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_RETURN);

	if (db_get_b(0, "Tab_SRMsg", "sendonshiftenter"))
		wHotKey = HOTKEYCODE(HOTKEYF_SHIFT, VK_RETURN);

	HOTKEYDESC hd = { "tabsrmm_send", pszDescription, pszSection, 0, wHotKey, 0, 100 };
	Hotkey_Register(&hd, g_pChatPlugin);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HWND) Srmm_FindWindow(MCONTACT hContact)
{
	return WindowList_Find(g_hWindowList, hContact);
}

MIR_APP_DLL(CMsgDialog*) Srmm_FindDialog(MCONTACT hContact)
{
	HWND hwnd = Srmm_FindWindow(hContact);
	return (hwnd) ? (CMsgDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA) : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Srmm_AddEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	wchar_t toolTip[256];
	mir_snwprintf(toolTip, TranslateT("Message from %s"), Clist_GetContactDisplayName(hContact));

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.hDbEvent = hDbEvent;
	cle.flags = CLEF_UNICODE;
	cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = MS_MSG_READMESSAGE;
	cle.szTooltip.w = toolTip;
	g_clistApi.pfnAddEvent(&cle);
}

/////////////////////////////////////////////////////////////////////////////////////////
// downloads or launches offline file

OFDTHREAD::OFDTHREAD(MEVENT _1, const CMStringW &_2, bool _3) :
	hDbEvent(_1),
	wszPath(_2),
	bOpen(_3)
{
}

OFDTHREAD::~OFDTHREAD()
{
	delete pCallback;
}

void OFDTHREAD::Finish()
{
	DBVARIANT dbv = { DBVT_WCHAR };
	dbv.pwszVal = wszPath.GetBuffer();
	db_event_setJson(hDbEvent, "lf", &dbv);

	// if the file is executable, protect it from automatic opening
	if (wszPath.isExecutable())
		if (FILE *out = _wfopen(wszPath + L":Zone.Identifier", L"wt")) {
			fputs("[ZoneTransfer]\r\nZoneId = 3\r\n", out);
			fclose(out);
		}

	NotifyEventHooks(g_hevEventEdited, db_event_getContact(hDbEvent), hDbEvent);

	pCallback->Invoke(*this);
}

void OFDTHREAD::ResetFileName(const wchar_t *pwszNewName)
{
	if (mir_wstrlen(pwszNewName))
		wszPath = FindUniqueFileName(pwszNewName);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void GenerateLocalName(const DB::EventInfo &dbei, DB::FILE_BLOB &blob, MCONTACT hContact)
{
	wchar_t wszReceiveFolder[MAX_PATH];
	if (dbei.flags & DBEF_SENT) // don't mix sent & received files
		mir_snwprintf(wszReceiveFolder, L"%s\\dlFiles\\%d", VARSW(L"%miranda_userdata%").get(), hContact);
	else
		GetContactReceivedFilesDir(hContact, wszReceiveFolder, _countof(wszReceiveFolder), true);
	CreateDirectoryTreeW(wszReceiveFolder);

	MFilePath wszFullName(wszReceiveFolder);
	wszFullName.AppendFormat(L"\\%s", blob.getName());
	blob.setLocalName(FindUniqueFileName(wszFullName));
}

void DownloadOfflineFile(MCONTACT hContact, MEVENT hDbEvent, bool bOpen, OFD_Callback *pCallback)
{
	DB::EventInfo dbei(hDbEvent);
	if (!dbei)
		return;

	DB::FILE_BLOB blob(dbei);
	if (!blob.isOffline())
		return;

	// local name was never created, make new one
	bool bDownloaded = false;
	if (!mir_wstrlen(blob.getLocalName()))
		GenerateLocalName(dbei, blob, hContact);
	else {
		struct _stat st;
		if (-1 == _wstat(blob.getLocalName(), &st)) {
			// file doesn't exist? expired? deleted? generate a new one in a local folder
			GenerateLocalName(dbei, blob, hContact);
		}
		else if (st.st_size && st.st_size == blob.getSize() && blob.isCompleted())
			bDownloaded = true;
	}

	if (bDownloaded) {
		OFDTHREAD ofd(hDbEvent, blob.getLocalName(), bOpen);
		pCallback->Invoke(ofd);
		delete pCallback;
	}
	else {
		OFDTHREAD *ofd = new OFDTHREAD(hDbEvent, blob.getLocalName(), bOpen);
		ofd->pCallback = pCallback;
		CallProtoService(dbei.szModule, PS_OFFLINEFILE, (WPARAM)ofd, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// serializes all thread-unsafe operation to the first thread

struct SSTParam
{
	HWND hwnd;
	const wchar_t *wszText;
	HICON hIcon;
};

static INT_PTR CALLBACK sttSetStatusText(void *_p)
{
	SSTParam *param = (SSTParam*)_p;

	CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(param->hwnd, GWLP_USERDATA);
	if (pDlg != nullptr)
		pDlg->SetStatusText(param->wszText, param->hIcon);
	return 0;
}

MIR_APP_DLL(void) Srmm_SetStatusText(MCONTACT hContact, const wchar_t *wszText, HICON hIcon)
{
	HWND hwnd = Srmm_FindWindow(hContact);
	if (hwnd == nullptr)
		hwnd = Srmm_FindWindow(db_mc_getMeta(hContact));
	if (hwnd == nullptr)
		return;

	SSTParam param = { hwnd, wszText, hIcon };
	CallFunctionSync(sttSetStatusText, &param);
}
