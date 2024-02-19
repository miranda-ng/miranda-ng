/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team,
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

MIR_APP_DLL(void) Srmm_AddEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (Clist_FindEvent(hContact, hDbEvent))
		return;

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

MIR_APP_DLL(void) Srmm_ApplyOptions()
{
	Srmm_Broadcast(DM_OPTIONSAPPLIED, 0, 0);
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

MIR_APP_DLL(int) Srmm_GetWindowData(MCONTACT hContact, MessageWindowData &mwd)
{
	if (hContact == 0)
		return 1;

	HWND hwnd = WindowList_Find(g_hWindowList, hContact);
	if (hwnd == nullptr)
		return 1;

	mwd.hwndWindow = hwnd;
	mwd.pDlg = (CSrmmBaseDialog *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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
// downloads or launches cloud file

static mir_cs csLocked;
static std::map<MEVENT, bool> arLocked;

OFDTHREAD::OFDTHREAD(MEVENT _1, const CMStringW &_2, int iCommand) :
	hDbEvent(_1),
	wszPath(_2)
{
	bOpen = (iCommand & OFD_RUN) != 0;
	bCopy = (iCommand & OFD_COPYURL) != 0;
}

OFDTHREAD::~OFDTHREAD()
{
	if (bLocked) {
		mir_cslock lck(csLocked);
		arLocked.erase(hDbEvent);
	}

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

void GetContactSentFilesDir(MCONTACT hContact, wchar_t *szDir, int cchDir)
{
	mir_snwprintf(szDir, cchDir, L"%s\\dlFiles\\%d\\", VARSW(L"%miranda_userdata%").get(), hContact);
}

static void GenerateLocalName(const DB::EventInfo &dbei, DB::FILE_BLOB &blob, MCONTACT hContact)
{
	wchar_t wszReceiveFolder[MAX_PATH];
	if (dbei.flags & DBEF_SENT) // don't mix sent & received files
		GetContactSentFilesDir(hContact, wszReceiveFolder, _countof(wszReceiveFolder));
	else
		File::GetReceivedFolder(hContact, wszReceiveFolder, _countof(wszReceiveFolder), true);
	CreateDirectoryTreeW(wszReceiveFolder);

	MFilePath wszFullName(wszReceiveFolder);
	wszFullName.Append(blob.getName());
	blob.setLocalName(FindUniqueFileName(wszFullName));
}

void DownloadOfflineFile(MCONTACT hContact, MEVENT hDbEvent, DB::EventInfo &dbei, int iCommand, OFD_Callback *pCallback)
{
	std::unique_ptr<OFD_Callback> callback(pCallback);

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
		OFDTHREAD ofd(hDbEvent, blob.getLocalName(), iCommand);
		pCallback->Invoke(ofd);
	}
	else {
		{
			mir_cslock lck(csLocked);
			if (arLocked.find(hDbEvent) != arLocked.end())
				return;

			arLocked[hDbEvent] = true;
		}

		OFDTHREAD *ofd = new OFDTHREAD(hDbEvent, blob.getLocalName(), iCommand);
		ofd->bLocked = true;
		ofd->pCallback = callback.release();
		CallProtoService(dbei.szModule, PS_OFFLINEFILE, (WPARAM)ofd);
	}
}

MIR_APP_DLL(void) Srmm_DownloadOfflineFile(MCONTACT hContact, MEVENT hDbEvent, int iCommand)
{
	DB::EventInfo dbei(hDbEvent);
	if (!dbei)
		return;

	DB::FILE_BLOB blob(dbei);
	OFD_Callback *pCallback = 0;

	switch (iCommand & 0xFFF) {
	case OFD_COPYURL:
		{
			OFDTHREAD *ofd = new OFDTHREAD(hDbEvent, L"", iCommand);
			ofd->pCallback = new OFD_CopyUrl(blob.getUrl());
			CallProtoService(dbei.szModule, PS_OFFLINEFILE, (WPARAM)ofd);
		}
		return;

	case OFD_DOWNLOAD:
		pCallback = new OFD_Download();
		break;

	case OFD_SAVEAS:
		wchar_t str[MAX_PATH];
		mir_wstrncpy(str, blob.getName(), _countof(str));

		wchar_t filter[512];
		mir_snwprintf(filter, L"%s (*)%c*%c", TranslateT("All files"), 0, 0);

		OPENFILENAME ofn = {};
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
		ofn.lpstrFilter = filter;
		ofn.lpstrFile = str;
		ofn.nMaxFile = _countof(str);
		ofn.nMaxFileTitle = MAX_PATH;
		if (!GetSaveFileNameW(&ofn))
			return;

		pCallback = new OFD_SaveAs(str);
		break;
	}

	DownloadOfflineFile(hContact, hDbEvent, dbei, iCommand, pCallback);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Quote

MIR_APP_DLL(CMStringW) Srmm_Quote(const wchar_t *pwzsText, int iWrapWidth)
{
	CMStringW ret;

	if (auto *p = pwzsText) {
		bool justDoneLineBreak = true;
		for (int lineChar = 0; *p;) {
			if (justDoneLineBreak && *p != '\r' && *p != '\n') {
				ret.AppendChar('>');
				ret.AppendChar(' ');
				lineChar = 2;
			}

			if (lineChar == iWrapWidth && *p != '\r' && *p != '\n') {
				int decreasedBy = 0;
				for (int outChar = ret.GetLength() - 1; lineChar > 10; lineChar--, p--, outChar--, decreasedBy++)
					if (ret[outChar] == ' ' || ret[outChar] == '\t' || ret[outChar] == '-')
						break;

				if (lineChar <= 10) {
					lineChar += decreasedBy;
					p += decreasedBy;
				}
				else ret.Truncate(ret.GetLength() - decreasedBy);

				ret.Append(L"\r\n");
				justDoneLineBreak = true;
				continue;
			}
			ret.AppendChar(*p);
			lineChar++;
			if (*p == '\n' || *p == '\r') {
				if (*p == '\r' && p[1] != '\n')
					ret.AppendChar('\n');
				justDoneLineBreak = 1;
				lineChar = 0;
			}
			else justDoneLineBreak = false;
			p++;
		}
	}

	ret.Append(L"\r\n");
	return ret;
}
