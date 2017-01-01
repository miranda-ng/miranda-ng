/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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
#include "statusicon.h"

#define TIMERID_FLASHWND     1
#define TIMERID_TYPE         2
#define TIMEOUT_FLASHWND     900
#define TIMEOUT_TYPEOFF      10000      //send type off after 10 seconds of inactivity
#define SB_CHAR_WIDTH        45
#define SB_TIME_WIDTH        60
#define SB_GRIP_WIDTH        20         // pixels - buffer used to prevent sizegrip from overwriting statusbar icons
#define VALID_AVATAR(x)      (x == PA_FORMAT_PNG || x == PA_FORMAT_JPEG || x == PA_FORMAT_ICON || x == PA_FORMAT_BMP || x == PA_FORMAT_GIF)

static const UINT sendControls[] = { IDC_MESSAGE };

void NotifyLocalWinEvent(MCONTACT hContact, HWND hwnd, unsigned int type)
{
	if (hContact != NULL && hwnd != NULL) {
		MessageWindowEventData mwe = {};
		mwe.cbSize = sizeof(mwe);
		mwe.hContact = hContact;
		mwe.hwndWindow = hwnd;
		mwe.szModule = SRMMMOD;
		mwe.uType = type;
		mwe.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwe.hwndInput = GetDlgItem(hwnd, IDC_MESSAGE);
		mwe.hwndLog = GetDlgItem(hwnd, IDC_LOG);
		NotifyEventHooks(hHookWinEvt, 0, (LPARAM)&mwe);
	}
}

static int RTL_Detect(const wchar_t *ptszText)
{
	int iLen = (int)mir_wstrlen(ptszText);
	WORD *infoTypeC2 = (WORD*)alloca(sizeof(WORD)* (iLen + 2));
	GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE2, ptszText, iLen, infoTypeC2);

	for (int i = 0; i < iLen; i++)
		if (infoTypeC2[i] == C2_RIGHTTOLEFT)
			return 1;

	return 0;
}

int SendMessageDirect(const wchar_t *szMsg, MCONTACT hContact)
{
	if (hContact == NULL)
		return NULL;

	int flags = 0;
	if (RTL_Detect(szMsg))
		flags |= PREF_RTL;

	T2Utf sendBuffer(szMsg);
	if (!mir_strlen(sendBuffer))
		return NULL;

	if (db_mc_isMeta(hContact))
		hContact = db_mc_getSrmmSub(hContact);

	int sendId = ProtoChainSend(hContact, PSS_MESSAGE, flags, (LPARAM)sendBuffer);
	msgQueue_add(hContact, sendId, sendBuffer.detach(), flags);
	return sendId;
}

static void AddToFileList(wchar_t ***pppFiles, int *totalCount, const wchar_t* szFilename)
{
	*pppFiles = (wchar_t**)mir_realloc(*pppFiles, (++*totalCount + 1)*sizeof(wchar_t*));
	(*pppFiles)[*totalCount] = NULL;
	(*pppFiles)[*totalCount - 1] = mir_wstrdup(szFilename);

	if (GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY) {
		WIN32_FIND_DATA fd;
		wchar_t szPath[MAX_PATH];
		mir_snwprintf(szPath, L"%s\\*", szFilename);
		HANDLE hFind = FindFirstFile(szPath, &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L"..")) continue;
				mir_snwprintf(szPath, L"%s\\%s", szFilename, fd.cFileName);
				AddToFileList(pppFiles, totalCount, szPath);
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}
	}
}

struct MsgEditSubclassData
{
	DWORD lastEnterTime;
};

static void SetEditorText(HWND hwnd, const wchar_t* txt)
{
	SetWindowText(hwnd, txt);
	SendMessage(hwnd, EM_SETSEL, -1, -1);
}

#define ENTERCLICKTIME   1000   //max time in ms during which a double-tap on enter will cause a send

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmWindow *pdat = (CSrmmWindow *)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	struct MsgEditSubclassData *dat = (struct MsgEditSubclassData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_DROPFILES:
		SendMessage(GetParent(hwnd), WM_DROPFILES, wParam, lParam);
		break;

	case EM_SUBCLASSED:
		dat = (struct MsgEditSubclassData *) mir_alloc(sizeof(struct MsgEditSubclassData));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		dat->lastEnterTime = 0;
		return 0;

	case WM_CHAR:
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY)
			break;

		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) {      //ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}

		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) {     // ctrl-w
			SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			if (!(GetKeyState(VK_SHIFT) & 0x8000) && ((GetKeyState(VK_CONTROL) & 0x8000) != 0) != g_dat.bSendOnEnter) {
				PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (g_dat.bSendOnDblEnter) {
				if (dat->lastEnterTime + ENTERCLICKTIME < GetTickCount())
					dat->lastEnterTime = GetTickCount();
				else {
					SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
					SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
					PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
					return 0;
				}
			}
		}
		else dat->lastEnterTime = 0;

		if (((wParam == VK_INSERT && (GetKeyState(VK_SHIFT) & 0x8000)) || (wParam == 'V' && (GetKeyState(VK_CONTROL) & 0x8000))) &&
			!(GetKeyState(VK_MENU) & 0x8000)) { // ctrl-v (paste clean text)
			SendMessage(hwnd, WM_PASTE, 0, 0);
			return 0;
		}

		if (wParam == VK_UP && (GetKeyState(VK_CONTROL) & 0x8000) && g_dat.bCtrlSupport && !g_dat.bAutoClose) {
			if (pdat->m_cmdList.getCount()) {
				if (pdat->m_cmdListInd < 0) {
					pdat->m_cmdListInd = pdat->m_cmdList.getCount() - 1;
					SetEditorText(hwnd, pdat->m_cmdList[pdat->m_cmdListInd]);
				}
				else if (pdat->m_cmdListInd > 0) {
					SetEditorText(hwnd, pdat->m_cmdList[--pdat->m_cmdListInd]);
				}
			}
			EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), GetWindowTextLength(hwnd) != 0);
			pdat->UpdateReadChars();
			return 0;
		}

		if (wParam == VK_DOWN && (GetKeyState(VK_CONTROL) & 0x8000) && g_dat.bCtrlSupport && !g_dat.bAutoClose) {
			if (pdat->m_cmdList.getCount() && pdat->m_cmdListInd >= 0) {
				if (pdat->m_cmdListInd < (pdat->m_cmdList.getCount() - 1))
					SetEditorText(hwnd, pdat->m_cmdList[++pdat->m_cmdListInd]);
				else {
					pdat->m_cmdListInd = -1;
					SetEditorText(hwnd, pdat->m_cmdList[pdat->m_cmdList.getCount() - 1]);
				}
			}

			EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), GetWindowTextLength(hwnd) != 0);
			pdat->UpdateReadChars();
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MOUSEWHEEL:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_SYSCHAR:
		dat->lastEnterTime = 0;
		if ((wParam == 's' || wParam == 'S') && GetKeyState(VK_MENU) & 0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
		{
			static const CHARRANGE all = { 0, -1 };

			MessageWindowPopupData mwpd = {};
			mwpd.cbSize = sizeof(mwpd);
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = pdat->m_hContact;
			mwpd.hwnd = hwnd;

			HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));

			mwpd.hMenu = GetSubMenu(hMenu, 2);
			TranslateMenu(mwpd.hMenu);

			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(mwpd.hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
			}
			if (!SendMessage(hwnd, EM_CANUNDO, 0, 0))
				EnableMenuItem(mwpd.hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);

			if (!SendMessage(hwnd, EM_CANREDO, 0, 0))
				EnableMenuItem(mwpd.hMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);

			if (!SendMessage(hwnd, EM_CANPASTE, 0, 0)) {
				if (!IsClipboardFormatAvailable(CF_HDROP))
					EnableMenuItem(mwpd.hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_PASTESEND, MF_BYCOMMAND | MF_GRAYED);
			}

			if (lParam == 0xFFFFFFFF) {
				SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&mwpd.pt, sel.cpMax);
				ClientToScreen(hwnd, &mwpd.pt);
			}
			else {
				mwpd.pt.x = GET_X_LPARAM(lParam);
				mwpd.pt.y = GET_Y_LPARAM(lParam);
			}


			// First notification
			NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

			// Someone added items?
			if (GetMenuItemCount(mwpd.hMenu) > 0) {
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				mwpd.selection = TrackPopupMenu(mwpd.hMenu, TPM_RETURNCMD, mwpd.pt.x, mwpd.pt.y, 0, hwnd, NULL);
			}

			// Second notification
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

			switch (mwpd.selection) {
			case IDM_UNDO:
				SendMessage(hwnd, WM_UNDO, 0, 0);
				break;

			case IDM_REDO:
				SendMessage(hwnd, EM_REDO, 0, 0);
				break;

			case IDM_CUT:
				SendMessage(hwnd, WM_CUT, 0, 0);
				break;

			case IDM_COPY:
				SendMessage(hwnd, WM_COPY, 0, 0);
				break;

			case IDM_PASTE:
				SendMessage(hwnd, WM_PASTE, 0, 0);
				break;

			case IDM_PASTESEND:
				SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
				PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				break;

			case IDM_DELETE:
				SendMessage(hwnd, EM_REPLACESEL, TRUE, 0);
				break;

			case IDM_SELECTALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
				break;

			case IDM_CLEAR:
				SetWindowText(hwnd, L"");
				break;
			}
			DestroyMenu(hMenu);
			return 0;
		}

	case WM_PASTE:
		if (IsClipboardFormatAvailable(CF_HDROP)) {
			if (OpenClipboard(hwnd)) {
				HANDLE hDrop = GetClipboardData(CF_HDROP);
				if (hDrop)
					SendMessage(hwnd, WM_DROPFILES, (WPARAM)hDrop, 0);
				CloseClipboard();
			}
		}
		else SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
		return 0;

	case WM_DESTROY:
		mir_free(dat);
		break;
	}
	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
		}
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			RECT rc;
			GetClientRect(hwnd, &rc);
			SendMessage(GetParent(hwnd), DM_SPLITTERMOVED, rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

static int MessageDialogResize(HWND, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	CSrmmWindow *dat = (CSrmmWindow*)lParam;

	switch (urc->wId) {
	case IDC_LOG:
		if (!g_dat.bShowButtons)
			urc->rcItem.top -= dat->m_lineHeight;
		urc->rcItem.bottom -= dat->m_splitterPos - dat->m_originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_SPLITTERY:
		urc->rcItem.top -= dat->m_splitterPos - dat->m_originalSplitterPos;
		urc->rcItem.bottom -= dat->m_splitterPos - dat->m_originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_MESSAGE:
		if (!g_dat.bSendButton)
			urc->rcItem.right = urc->dlgNewSize.cx - urc->rcItem.left;
		if (g_dat.bShowAvatar && dat->m_avatarPic)
			urc->rcItem.left = dat->m_avatarWidth + 4;

		urc->rcItem.top -= dat->m_splitterPos - dat->m_originalSplitterPos;
		if (!g_dat.bSendButton)
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDCANCEL:
	case IDOK:
		urc->rcItem.top -= dat->m_splitterPos - dat->m_originalSplitterPos;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_AVATAR:
		urc->rcItem.top = urc->rcItem.bottom - (dat->m_avatarHeight + 2);
		urc->rcItem.right = urc->rcItem.left + (dat->m_avatarWidth + 2);
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

/////////////////////////////////////////////////////////////////////////////////////////

CSrmmWindow::CSrmmWindow(MCONTACT hContact, bool noActivate, const char *szInitialText, bool bIsUnicode)
	: CDlgBase(g_hInst, IDD_MSG),
	m_cmdList(20),
	m_bNoActivate(noActivate),
	m_hContact(hContact)
{
	m_wszInitialText = (bIsUnicode) ? mir_wstrdup((wchar_t*)szInitialText) : mir_a2u(szInitialText);
}

void CSrmmWindow::OnInitDialog()
{
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LPARAM)this);

	m_bIsMeta = db_mc_isMeta(m_hContact) != 0;
	m_hTimeZone = TimeZone_CreateByContact(m_hContact, 0, TZF_KNOWNONLY);
	m_wMinute = 61;

	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPENING);
	if (m_wszInitialText) {
		SetDlgItemText(m_hwnd, IDC_MESSAGE, m_wszInitialText);

		int len = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE));
		PostMessage(GetDlgItem(m_hwnd, IDC_MESSAGE), EM_SETSEL, len, len);
		mir_free(m_wszInitialText);
	}

	m_szProto = GetContactProto(m_hContact);
	RichUtil_SubClass(GetDlgItem(m_hwnd, IDC_LOG));
	RichUtil_SubClass(GetDlgItem(m_hwnd, IDC_MESSAGE));

	// avatar stuff
	m_limitAvatarH = db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ? db_get_dw(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT) : 0;

	if (m_hContact && m_szProto != NULL)
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
	else
		m_wStatus = ID_STATUS_OFFLINE;
	m_wOldStatus = m_wStatus;
	m_splitterPos = (int)db_get_dw(g_dat.bSavePerContact ? m_hContact : NULL, SRMMMOD, "splitterPos", (DWORD)-1);
	m_cmdListInd = -1;
	m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
	SetTimer(m_hwnd, TIMERID_TYPE, 1000, NULL);

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rc);
	POINT pt = { 0, (rc.top + rc.bottom) / 2 };
	ScreenToClient(m_hwnd, &pt);
	m_originalSplitterPos = pt.y;
	if (m_splitterPos == -1)
		m_splitterPos = m_originalSplitterPos;

	Srmm_CreateToolbarIcons(m_hwnd, BBBF_ISIMBUTTON);

	WindowList_Add(pci->hWindowList, m_hwnd, m_hContact);
	GetWindowRect(GetDlgItem(m_hwnd, IDC_MESSAGE), &m_minEditInit);
	SendMessage(m_hwnd, DM_UPDATESIZEBAR, 0, 0);
	m_hwndStatus = NULL;

	EnableWindow(GetDlgItem(m_hwnd, IDC_AVATAR), FALSE);

	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_SCROLL);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_AUTOURLDETECT, TRUE, 0);

	if (m_hContact && m_szProto) {
		int nMax = CallProtoService(m_szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, m_hContact);
		if (nMax)
			SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_LIMITTEXT, nMax, 0);

		// get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF
		SendDlgItemMessage(m_hwnd, IDC_LOG, EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	}

	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_MESSAGE), MessageEditSubclassProc);
	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SUBCLASSED, 0, 0);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERY), SplitterSubclassProc);

	if (m_hContact) {
		int historyMode = db_get_b(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);
		// This finds the first message to display, it works like shit
		m_hDbEventFirst = db_event_firstUnread(m_hContact);
		switch (historyMode) {
		case LOADHISTORY_COUNT:
			for (int i = db_get_w(NULL, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT); i--;) {
				MEVENT hPrevEvent;
				if (m_hDbEventFirst == NULL)
					hPrevEvent = db_event_last(m_hContact);
				else
					hPrevEvent = db_event_prev(m_hContact, m_hDbEventFirst);
				if (hPrevEvent == NULL)
					break;

				DBEVENTINFO dbei = { sizeof(dbei) };
				m_hDbEventFirst = hPrevEvent;
				db_event_get(hPrevEvent, &dbei);
				if (!DbEventIsShown(&dbei))
					i++;
			}
			break;

		case LOADHISTORY_TIME:
			DBEVENTINFO dbei = { sizeof(dbei) };
			if (m_hDbEventFirst == NULL)
				dbei.timestamp = (DWORD)time(NULL);
			else
				db_event_get(m_hDbEventFirst, &dbei);

			DWORD firstTime = dbei.timestamp - 60 * db_get_w(NULL, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
			for (;;) {
				MEVENT hPrevEvent;
				if (m_hDbEventFirst == NULL)
					hPrevEvent = db_event_last(m_hContact);
				else
					hPrevEvent = db_event_prev(m_hContact, m_hDbEventFirst);
				if (hPrevEvent == NULL)
					break;

				dbei.cbBlob = 0;
				db_event_get(hPrevEvent, &dbei);
				if (dbei.timestamp < firstTime)
					break;
				m_hDbEventFirst = hPrevEvent;
			}
			break;
		}
	}

	MEVENT hdbEvent = db_event_last(m_hContact);
	if (hdbEvent) {
		do {
			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hdbEvent, &dbei);
			if ((dbei.eventType == EVENTTYPE_MESSAGE) && !(dbei.flags & DBEF_SENT)) {
				m_lastMessage = dbei.timestamp;
				PostMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
				break;
			}
		} while (hdbEvent = db_event_prev(m_hContact, hdbEvent));
	}

	OnOptionsApplied(false);

	// restore saved msg if any...
	if (m_hContact) {
		DBVARIANT dbv;
		if (!db_get_ws(m_hContact, SRMSGMOD, DBSAVEDMSG, &dbv)) {
			if (dbv.ptszVal[0]) {
				SetDlgItemText(m_hwnd, IDC_MESSAGE, dbv.ptszVal);
				EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
				UpdateReadChars();
				PostMessage(GetDlgItem(m_hwnd, IDC_MESSAGE), EM_SETSEL, -1, -1);
			}
			db_free(&dbv);
		}
	}
	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETEVENTMASK, 0, ENM_CHANGE);

	int flag = m_bNoActivate ? RWPF_HIDDEN : 0;
	if (Utils_RestoreWindowPosition(m_hwnd, g_dat.bSavePerContact ? m_hContact : NULL, SRMMMOD, "", flag)) {
		if (g_dat.bSavePerContact) {
			if (Utils_RestoreWindowPosition(m_hwnd, NULL, SRMMMOD, "", flag | RWPF_NOMOVE))
				SetWindowPos(m_hwnd, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
		}
		else SetWindowPos(m_hwnd, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	if (!g_dat.bSavePerContact && g_dat.bCascade)
		WindowList_Broadcast(pci->hWindowList, DM_CASCADENEWWINDOW, (WPARAM)m_hwnd, (LPARAM)&m_windowWasCascaded);

	if (m_bNoActivate) {
		SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
	}
	else {
		SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetForegroundWindow(m_hwnd);
		SetFocus(GetDlgItem(m_hwnd, IDC_MESSAGE));
	}

	SendMessage(m_hwnd, DM_GETAVATAR, 0, 0);
	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPEN);
}

void CSrmmWindow::OnDestroy()
{
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSING);

	// save string from the editor
	if (m_hContact) {
		int len = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE)) + 1;
		wchar_t *msg = (wchar_t*)alloca(sizeof(wchar_t)*len);
		GetDlgItemText(m_hwnd, IDC_MESSAGE, msg, len);
		if (msg[0])
			db_set_ws(m_hContact, SRMSGMOD, DBSAVEDMSG, msg);
		else
			db_unset(m_hContact, SRMSGMOD, DBSAVEDMSG);
	}
	KillTimer(m_hwnd, TIMERID_TYPE);
	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
		NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (m_hBkgBrush)
		DeleteObject(m_hBkgBrush);
	if (m_hwndStatus)
		DestroyWindow(m_hwndStatus);

	for (int i = 0; i < m_cmdList.getCount(); i++)
		mir_free(m_cmdList[i]);
	m_cmdList.destroy();

	WindowList_Remove(pci->hWindowList, m_hwnd);

	MCONTACT hContact = (g_dat.bSavePerContact) ? m_hContact : NULL;
	db_set_dw(hContact ? m_hContact : NULL, SRMMMOD, "splitterPos", m_splitterPos);

	if (m_hFont) {
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	WINDOWPLACEMENT wp = { sizeof(wp) };
	GetWindowPlacement(m_hwnd, &wp);
	if (!m_windowWasCascaded) {
		db_set_dw(hContact, SRMMMOD, "x", wp.rcNormalPosition.left);
		db_set_dw(hContact, SRMMMOD, "y", wp.rcNormalPosition.top);
	}
	db_set_dw(hContact, SRMMMOD, "width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	db_set_dw(hContact, SRMMMOD, "height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

	NotifyLocalWinEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSE);
	if (m_hContact && g_dat.bDeleteTempCont)
		if (db_get_b(m_hContact, "CList", "NotOnList", 0))
			db_delete_contact(m_hContact);

	Window_FreeIcon_IcoLib(m_hwnd);
}

void CSrmmWindow::OnOptionsApplied(bool bUpdateAvatar)
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == NULL)
			continue;

		bool bShow = false;
		if (m_hContact) {
			if (cbd->m_dwButtonCID == IDC_ADD) {
				bShow = 0 != db_get_b(m_hContact, "CList", "NotOnList", 0);
				cbd->m_bHidden = !bShow;
			}
			else bShow = g_dat.bShowButtons;
		}
		ShowWindow(hwndButton, (bShow) ? SW_SHOW : SW_HIDE);
	}

	if (!m_hwndStatus) {
		int grip = (GetWindowLongPtr(m_hwnd, GWL_STYLE) & WS_THICKFRAME) ? SBARS_SIZEGRIP : 0;
		m_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | grip, 0, 0, 0, 0, m_hwnd, NULL, g_hInst, NULL);
		SendMessage(m_hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
	}

	ShowWindow(GetDlgItem(m_hwnd, IDCANCEL), SW_HIDE);
	ShowWindow(GetDlgItem(m_hwnd, IDC_SPLITTERY), SW_SHOW);
	ShowWindow(GetDlgItem(m_hwnd, IDOK), g_dat.bSendButton ? SW_SHOW : SW_HIDE);
	EnableWindow(GetDlgItem(m_hwnd, IDOK), GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE)) != 0);
	if (m_avatarPic == NULL || !g_dat.bShowAvatar)
		ShowWindow(GetDlgItem(m_hwnd, IDC_AVATAR), SW_HIDE);
	SendMessage(m_hwnd, DM_UPDATETITLE, 0, 0);
	SendMessage(m_hwnd, WM_SIZE, 0, 0);

	if (m_hBkgBrush)
		DeleteObject(m_hBkgBrush);

	COLORREF colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
	m_hBkgBrush = CreateSolidBrush(colour);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETBKGNDCOLOR, 0, colour);
	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETBKGNDCOLOR, 0, colour);

	// avatar stuff
	m_avatarPic = NULL;
	m_limitAvatarH = 0;
	if (CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_AVATARS)
		m_limitAvatarH = db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ?
		db_get_dw(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT) : 0;

	if (bUpdateAvatar)
		SendMessage(m_hwnd, DM_GETAVATAR, 0, 0);

	InvalidateRect(GetDlgItem(m_hwnd, IDC_MESSAGE), NULL, FALSE);

	LOGFONT lf;
	CHARFORMAT cf = {};
	if (m_hFont)
		DeleteObject(m_hFont);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &cf.crTextColor);
	m_hFont = CreateFontIndirect(&lf);
	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));

	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	SendDlgItemMessage(m_hwnd, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (WPARAM)&cf);

	// configure message history for proper RTL formatting
	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(pf2));
	pf2.cbSize = sizeof(pf2);

	pf2.wEffects = PFE_RTLPARA;
	pf2.dwMask = PFM_RTLPARA;
	SetDlgItemText(m_hwnd, IDC_LOG, L"");
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	pf2.wEffects = 0;
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	SendDlgItemMessage(m_hwnd, IDC_LOG, EM_SETLANGOPTIONS, 0, SendDlgItemMessage(m_hwnd, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);

	SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
	SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
}

void CSrmmWindow::NotifyTyping(int mode)
{
	if (!m_hContact)
		return;

	// Don't send to protocols who don't support typing
	// Don't send to users who are unchecked in the typing notification options
	// Don't send to protocols that are offline
	// Don't send to users who are not visible and
	// Don't send to users who are not on the visible list when you are in invisible mode.
	if (!db_get_b(m_hContact, SRMMMOD, SRMSGSET_TYPING, db_get_b(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)))
		return;

	if (!m_szProto)
		return;

	DWORD protoStatus = CallProtoService(m_szProto, PS_GETSTATUS, 0, 0);
	DWORD protoCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	DWORD typeCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0);

	if (!(typeCaps & PF4_SUPPORTTYPING))
		return;

	if (protoStatus < ID_STATUS_ONLINE)
		return;

	if (protoCaps & PF1_VISLIST && db_get_w(m_hContact, m_szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		return;

	if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && db_get_w(m_hContact, m_szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		return;

	if (!g_dat.bTypingUnknown && db_get_b(m_hContact, "CList", "NotOnList", 0))
		return;

	// End user check
	m_nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, m_hContact, m_nTypeMode);
}

void CSrmmWindow::ShowAvatar()
{
	if (g_dat.bShowAvatar) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, getActiveContact(), 0);
		if (ace && (INT_PTR)ace != CALLSERVICE_NOTFOUND && (ace->dwFlags & AVS_BITMAP_VALID) && !(ace->dwFlags & AVS_HIDEONCLIST))
			m_avatarPic = ace->hbmPic;
		else
			m_avatarPic = NULL;
	}
	else m_avatarPic = NULL;

	SendMessage(m_hwnd, DM_UPDATESIZEBAR, 0, 0);
	SendMessage(m_hwnd, DM_AVATARSIZECHANGE, 0, 0);
}

void CSrmmWindow::ShowTime()
{
	if (m_hwndStatus && m_hTimeZone) {
		SYSTEMTIME st;
		GetSystemTime(&st);
		if (m_wMinute != st.wMinute) {
			wchar_t buf[32];
			unsigned i = g_dat.bShowReadChar ? 2 : 1;

			TimeZone_PrintDateTime(m_hTimeZone, L"t", buf, _countof(buf), 0);
			SendMessage(m_hwndStatus, SB_SETTEXT, i, (LPARAM)buf);
			m_wMinute = st.wMinute;
		}
	}
}

void CSrmmWindow::SetupStatusBar()
{
	int i = 0, statwidths[4];
	int icons_width = GetStatusIconsCount(m_hContact) * (GetSystemMetrics(SM_CXSMICON) + 2) + SB_GRIP_WIDTH;

	RECT rc;
	GetWindowRect(m_hwndStatus, &rc);
	int cx = rc.right - rc.left;

	if (m_hTimeZone) {
		if (g_dat.bShowReadChar)
			statwidths[i++] = cx - SB_TIME_WIDTH - SB_CHAR_WIDTH - icons_width;
		statwidths[i++] = cx - SB_TIME_WIDTH - icons_width;
	}
	else if (g_dat.bShowReadChar)
		statwidths[i++] = cx - SB_CHAR_WIDTH - icons_width;

	statwidths[i++] = cx - icons_width;
	statwidths[i++] = -1;
	SendMessage(m_hwndStatus, SB_SETPARTS, i, (LPARAM)statwidths);

	UpdateReadChars();
	ShowTime();
	SendMessage(m_hwnd, DM_STATUSICONCHANGE, 0, 0);
}

void CSrmmWindow::SetStatusData(StatusTextData *st)
{
	SendMessage(m_hwndStatus, SB_SETICON, 0, (LPARAM)(st == NULL ? 0 : st->hIcon));
	SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)(st == NULL ? L"" : st->tszText));
}

void CSrmmWindow::UpdateReadChars()
{
	if (m_hwndStatus && g_dat.bShowReadChar) {
		wchar_t buf[32];
		int len = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE));

		mir_snwprintf(buf, L"%d", len);
		SendMessage(m_hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
	}
}

INT_PTR CSrmmWindow::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ENLINK *pLink;
	CHARRANGE sel;

	switch (uMsg) {
	case WM_CONTEXTMENU:
		if (m_hwndStatus && m_hwndStatus == (HWND)wParam) {
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2.x = pt.x; pt2.y = pt.y;
			ScreenToClient(m_hwndStatus, &pt);

			// no popup menu for status icons - this is handled via NM_RCLICK notification and the plugins that added the icons
			RECT rc;
			SendMessage(m_hwndStatus, SB_GETRECT, SendMessage(m_hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
			if (pt.x >= rc.left)
				break;

			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			TrackPopupMenu(hMenu, 0, pt2.x, pt2.y, 0, m_hwnd, NULL);
			DestroyMenu(hMenu);
		}
		break;

	case WM_DROPFILES: // Mod from tabsrmm
		if (m_szProto == NULL) break;
		if (!(CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_1, 0)&PF1_FILESEND)) break;
		if (m_wStatus == ID_STATUS_OFFLINE) break;
		if (m_hContact != NULL) {
			wchar_t szFilename[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			int fileCount = DragQueryFile(hDrop, -1, NULL, 0), totalCount = 0, i;
			wchar_t **ppFiles = NULL;
			for (i = 0; i < fileCount; i++) {
				DragQueryFile(hDrop, i, szFilename, _countof(szFilename));
				AddToFileList(&ppFiles, &totalCount, szFilename);
			}
			CallServiceSync(MS_FILE_SENDSPECIFICFILEST, m_hContact, (LPARAM)ppFiles);
			for (i = 0; ppFiles[i]; i++)
				mir_free(ppFiles[i]);
			mir_free(ppFiles);
		}
		break;

	case HM_AVATARACK:
		ShowAvatar();
		break;

	case DM_AVATARCALCSIZE:
		if (m_avatarPic == NULL || !g_dat.bShowAvatar) {
			m_avatarWidth = 50;
			m_avatarHeight = 50;
			ShowWindow(GetDlgItem(m_hwnd, IDC_AVATAR), SW_HIDE);
			return 0;
		}
		else {
			BITMAP bminfo;
			GetObject(m_avatarPic, sizeof(bminfo), &bminfo);
			m_avatarWidth = bminfo.bmWidth + 2;
			m_avatarHeight = bminfo.bmHeight + 2;
			if (m_limitAvatarH && m_avatarHeight > m_limitAvatarH) {
				m_avatarWidth = bminfo.bmWidth * m_limitAvatarH / bminfo.bmHeight + 2;
				m_avatarHeight = m_limitAvatarH + 2;
			}
			ShowWindow(GetDlgItem(m_hwnd, IDC_AVATAR), SW_SHOW);
		}
		break;

	case DM_UPDATESIZEBAR:
		m_minEditBoxSize.cx = m_minEditInit.right - m_minEditInit.left;
		m_minEditBoxSize.cy = m_minEditInit.bottom - m_minEditInit.top;
		if (g_dat.bShowAvatar) {
			SendMessage(m_hwnd, DM_AVATARCALCSIZE, 0, 0);
			if (m_avatarPic && m_minEditBoxSize.cy <= m_avatarHeight)
				m_minEditBoxSize.cy = m_avatarHeight;
		}
		break;

	case DM_AVATARSIZECHANGE:
		{
			RECT rc;
			GetWindowRect(GetDlgItem(m_hwnd, IDC_MESSAGE), &rc);
			if (rc.bottom - rc.top < m_minEditBoxSize.cy)
				SendMessage(m_hwnd, DM_SPLITTERMOVED, rc.top - (rc.bottom - rc.top - m_minEditBoxSize.cy - 4), (LPARAM)GetDlgItem(m_hwnd, IDC_SPLITTERY));

			SendMessage(m_hwnd, WM_SIZE, 0, 0);
		}
		break;

	case DM_GETAVATAR:
		{
			PROTO_AVATAR_INFORMATION ai = {};
			ai.hContact = m_hContact;
			CallProtoService(m_szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&ai);

			ShowAvatar();
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, 1);
		}
		break;

	case DM_TYPING:
		m_nTypeSecs = (INT_PTR)lParam > 0 ? (int)lParam : 0;
		break;

	case DM_UPDATEWINICON:
		if (g_dat.bUseStatusWinIcon) {
			Window_FreeIcon_IcoLib(m_hwnd);

			if (m_szProto) {
				WORD wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
				Window_SetProtoIcon_IcoLib(m_hwnd, m_szProto, wStatus);
				break;
			}
		}
		Window_SetSkinIcon_IcoLib(m_hwnd, SKINICON_EVENT_MESSAGE);
		break;

	case DM_USERNAMETOCLIP:
		if (m_hContact) {
			ptrW id(Contact_GetInfo(CNF_UNIQUEID, m_hContact, m_szProto));
			if (id != NULL && OpenClipboard(m_hwnd)) {
				EmptyClipboard();
				HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, mir_wstrlen(id) * sizeof(wchar_t) + 1);
				mir_wstrcpy((wchar_t*)GlobalLock(hData), id);
				GlobalUnlock(hData);
				SetClipboardData(CF_UNICODETEXT, hData);
				CloseClipboard();
			}
		}
		break;

	case DM_UPDATELASTMESSAGE:
		if (!m_hwndStatus || m_nTypeSecs)
			break;

		if (m_lastMessage) {
			wchar_t date[64], time[64], fmt[128];
			TimeZone_PrintTimeStamp(NULL, m_lastMessage, L"d", date, _countof(date), 0);
			TimeZone_PrintTimeStamp(NULL, m_lastMessage, L"t", time, _countof(time), 0);
			mir_snwprintf(fmt, TranslateT("Last message received on %s at %s."), date, time);
			SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)fmt);
		}
		else SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)L"");

		SendMessage(m_hwndStatus, SB_SETICON, 0, 0);
		break;

	case DM_OPTIONSAPPLIED:
		OnOptionsApplied(wParam != 0);
		break;

	case DM_UPDATETITLE:
		wchar_t newtitle[256];
		if (m_hContact && m_szProto) {
			m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
			wchar_t *contactName = pcli->pfnGetContactDisplayName(m_hContact, 0);

			wchar_t *szStatus = pcli->pfnGetStatusModeDescription(m_szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE), 0);
			if (g_dat.bUseStatusWinIcon)
				mir_snwprintf(newtitle, L"%s - %s", contactName, TranslateT("Message session"));
			else
				mir_snwprintf(newtitle, L"%s (%s): %s", contactName, szStatus, TranslateT("Message session"));

			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)wParam;
			if (!cws || (!mir_strcmp(cws->szModule, m_szProto) && !mir_strcmp(cws->szSetting, "Status"))) {
				if (m_szProto) {
					int dwStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
					HICON hIcon = Skin_LoadProtoIcon(m_szProto, dwStatus);
					if (hIcon) {
						SendDlgItemMessage(m_hwnd, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
						IcoLib_ReleaseIcon(hIcon);
					}
				}
				if (g_dat.bUseStatusWinIcon)
					SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
			}

			m_wOldStatus = m_wStatus;
		}
		else mir_wstrncpy(newtitle, TranslateT("Message session"), _countof(newtitle));

		wchar_t oldtitle[256];
		GetWindowText(m_hwnd, oldtitle, _countof(oldtitle));
		if (mir_wstrcmp(newtitle, oldtitle)) { //swt() flickers even if the title hasn't actually changed
			SetWindowText(m_hwnd, newtitle);
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
		}
		break;

	case DM_NEWTIMEZONE:
		m_hTimeZone = TimeZone_CreateByContact(m_hContact, 0, TZF_KNOWNONLY);
		m_wMinute = 61;
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		break;

	case DM_CASCADENEWWINDOW:
		if ((HWND)wParam != m_hwnd) {
			RECT rcThis, rcNew;
			GetWindowRect(m_hwnd, &rcThis);
			GetWindowRect((HWND)wParam, &rcNew);
			if (abs(rcThis.left - rcNew.left) < 3 && abs(rcThis.top - rcNew.top) < 3) {
				int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
				SetWindowPos((HWND)wParam, 0, rcNew.left + offset, rcNew.top + offset, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
				*(int *)lParam = 1;
			}
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		SetFocus(GetDlgItem(m_hwnd, IDC_MESSAGE));
		// fall through
	case WM_MOUSEACTIVATE:
		if (KillTimer(m_hwnd, TIMERID_FLASHWND))
			FlashWindow(m_hwnd, FALSE);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			RECT rcWindow, rcLog;
			GetWindowRect(m_hwnd, &rcWindow);
			GetWindowRect(GetDlgItem(m_hwnd, IDC_LOG), &rcLog);
			mmi->ptMinTrackSize.x = rcWindow.right - rcWindow.left - ((rcLog.right - rcLog.left) - m_minEditBoxSize.cx);
			mmi->ptMinTrackSize.y = rcWindow.bottom - rcWindow.top - ((rcLog.bottom - rcLog.top) - m_minEditBoxSize.cy);
		}
		return 0;

	case WM_CBD_LOADICONS:
		Srmm_UpdateToolbarIcons(m_hwnd);
		break;

	case WM_CBD_UPDATED:
		SetButtonsPos(m_hwnd, false);
		break;

	case WM_SIZE:
		if (!IsIconic(m_hwnd)) {
			BOOL bottomScroll = TRUE;

			if (m_hwndStatus) {
				SendMessage(m_hwndStatus, WM_SIZE, 0, 0);
				SetupStatusBar();
			}

			if (GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_LOG), GWL_STYLE) & WS_VSCROLL) {
				SCROLLINFO si = {};
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
				GetScrollInfo(GetDlgItem(m_hwnd, IDC_LOG), SB_VERT, &si);
				bottomScroll = (si.nPos + (int)si.nPage + 5) >= si.nMax;
			}

			Utils_ResizeDialog(m_hwnd, g_hInst, MAKEINTRESOURCEA(IDD_MSG), MessageDialogResize, (LPARAM)this);
			SetButtonsPos(m_hwnd, false);

			// The statusbar sometimes draws over these 2 controls so
			// redraw them
			if (m_hwndStatus) {
				RedrawWindow(GetDlgItem(m_hwnd, IDOK), NULL, NULL, RDW_INVALIDATE);
				RedrawWindow(GetDlgItem(m_hwnd, IDC_MESSAGE), NULL, NULL, RDW_INVALIDATE);
			}
			
			if (g_dat.bShowAvatar && m_avatarPic)
				RedrawWindow(GetDlgItem(m_hwnd, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);

			if (bottomScroll)
				PostMessage(m_hwnd, DM_SCROLLLOGTOBOTTOM, 0, 0);
		}
		return TRUE;

	case DM_SPLITTERMOVED:
		if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERY)) {
			HWND hwndLog = GetDlgItem(m_hwnd, IDC_LOG);

			RECT rc, rcLog;
			GetClientRect(m_hwnd, &rc);
			GetWindowRect(hwndLog, &rcLog);

			POINT pt;
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(m_hwnd, &pt);

			int oldSplitterY = m_splitterPos;
			m_splitterPos = rc.bottom - pt.y + 23;
			GetWindowRect(GetDlgItem(m_hwnd, IDC_MESSAGE), &rc);
			if (rc.bottom - rc.top + (m_splitterPos - oldSplitterY) < m_minEditBoxSize.cy)
				m_splitterPos = oldSplitterY + m_minEditBoxSize.cy - (rc.bottom - rc.top);
			if (rcLog.bottom - rcLog.top - (m_splitterPos - oldSplitterY) < m_minEditBoxSize.cy)
				m_splitterPos = oldSplitterY - m_minEditBoxSize.cy + (rcLog.bottom - rcLog.top);

			SendMessage(m_hwnd, WM_SIZE, 0, 0);
		}
		break;

	case DM_REMAKELOG:
		StreamInEvents(m_hDbEventFirst, -1, 0);
		break;

	case DM_APPENDTOLOG:   //takes wParam=hDbEvent
		StreamInEvents(wParam, 1, 1);
		break;

	case DM_SCROLLLOGTOBOTTOM:
		{
			HWND hwndLog = GetDlgItem(m_hwnd, IDC_LOG);
			if (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL) {
				SCROLLINFO si = {};
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE;
				GetScrollInfo(hwndLog, SB_VERT, &si);
				si.fMask = SIF_POS;
				si.nPos = si.nMax - si.nPage;
				SetScrollInfo(hwndLog, SB_VERT, &si, TRUE);
				SendMessage(hwndLog, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
		}
		break;

	case HM_DBEVENTADDED:
		if (wParam == m_hContact) {
			MEVENT hDbEvent = lParam;
			if (m_hDbEventFirst == NULL)
				m_hDbEventFirst = hDbEvent;

			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			bool isMessage = (dbei.eventType == EVENTTYPE_MESSAGE), isSent = ((dbei.flags & DBEF_SENT) != 0);
			if (DbEventIsShown(&dbei)) {
				// Sounds *only* for sent messages, not for custom events
				if (isMessage && !isSent) {
					if (GetForegroundWindow() == m_hwnd)
						SkinPlaySound("RecvMsgActive");
					else
						SkinPlaySound("RecvMsgInactive");
				}
				if (isMessage && !isSent) {
					m_lastMessage = dbei.timestamp;
					SendMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
				}
				if (hDbEvent != m_hDbEventFirst && db_event_next(m_hContact, hDbEvent) == NULL)
					SendMessage(m_hwnd, DM_APPENDTOLOG, lParam, 0);
				else
					SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);

				// Flash window *only* for messages, not for custom events
				if (isMessage && !isSent) {
					if (GetActiveWindow() == m_hwnd && GetForegroundWindow() == m_hwnd) {
						HWND hwndLog = GetDlgItem(m_hwnd, IDC_LOG);
						if (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL) {
							SCROLLINFO si = {};
							si.cbSize = sizeof(si);
							si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
							GetScrollInfo(hwndLog, SB_VERT, &si);
							if ((si.nPos + (int)si.nPage + 5) < si.nMax)
								SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
						}
					}
					else SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
				}
			}
		}
		break;

	case WM_TIMECHANGE:
		PostMessage(m_hwnd, DM_NEWTIMEZONE, 0, 0);
		PostMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND) {
			FlashWindow(m_hwnd, TRUE);
			if (m_nFlash > 2 * g_dat.nFlashMax) {
				KillTimer(m_hwnd, TIMERID_FLASHWND);
				FlashWindow(m_hwnd, FALSE);
				m_nFlash = 0;
			}
			m_nFlash++;
		}
		else if (wParam == TIMERID_TYPE) {
			ShowTime();
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - m_nLastTyping > TIMEOUT_TYPEOFF)
				NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

			if (m_bShowTyping) {
				if (m_nTypeSecs) {
					m_nTypeSecs--;
					if (GetForegroundWindow() == m_hwnd)
						SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
				}
				else {
					SendMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
					if (g_dat.bShowTypingWin)
						SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
					m_bShowTyping = false;
				}
			}
			else {
				if (m_nTypeSecs) {
					wchar_t szBuf[256];
					wchar_t* szContactName = pcli->pfnGetContactDisplayName(m_hContact, 0);
					HICON hTyping = Skin_LoadIcon(SKINICON_OTHER_TYPING);

					mir_snwprintf(szBuf, TranslateT("%s is typing a message..."), szContactName);
					m_nTypeSecs--;

					SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
					SendMessage(m_hwndStatus, SB_SETICON, 0, (LPARAM)hTyping);
					if (g_dat.bShowTypingWin && GetForegroundWindow() != m_hwnd) {
						HICON hIcon = (HICON)SendMessage(m_hwnd, WM_GETICON, ICON_SMALL, 0);
						SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hTyping);
						IcoLib_ReleaseIcon(hIcon);
					}
					m_bShowTyping = true;
				}
			}
		}
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);
			
			if (dis->hwndItem == m_hwndStatus) {
				DrawStatusIcons(m_hContact, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}

			if (dis->CtlID == IDC_AVATAR && m_avatarPic && g_dat.bShowAvatar) {
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				HPEN hOldPen = (HPEN)SelectObject(dis->hDC, hPen);
				Rectangle(dis->hDC, 0, 0, m_avatarWidth, m_avatarHeight);
				SelectObject(dis->hDC, hOldPen);
				DeleteObject(hPen);

				BITMAP bminfo;
				GetObject(m_avatarPic, sizeof(bminfo), &bminfo);

				HDC hdcMem = CreateCompatibleDC(dis->hDC);
				HBITMAP hbmMem = (HBITMAP)SelectObject(hdcMem, m_avatarPic);

				SetStretchBltMode(dis->hDC, HALFTONE);
				StretchBlt(dis->hDC, 1, 1, m_avatarWidth - 2, m_avatarHeight - 2, hdcMem, 0, 0,
					bminfo.bmWidth, bminfo.bmHeight, SRCCOPY);

				SelectObject(hdcMem, hbmMem);
				DeleteDC(hdcMem);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		if (!lParam && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_hContact))
			break;

		// custom button handling
		if (HIWORD(wParam) == BN_CLICKED)
			if (LOWORD(wParam) >= MIN_CBUTTONID && LOWORD(wParam) <= MAX_CBUTTONID) {
				Srmm_ClickToolbarIcon(m_hContact, LOWORD(wParam), GetDlgItem(m_hwnd, LOWORD(wParam)), 0);
				break;
			}

		switch (LOWORD(wParam)) {
		case IDOK:
			if (IsWindowEnabled(GetDlgItem(m_hwnd, IDOK))) {
				int bufSize = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE)) + 1;
				wchar_t *temp = (wchar_t*)alloca(bufSize * sizeof(wchar_t));
				GetDlgItemText(m_hwnd, IDC_MESSAGE, temp, bufSize);
				if (!temp[0])
					break;

				int sendId = SendMessageDirect(rtrimw(temp), m_hContact);
				if (sendId) {
					m_cmdList.insert(mir_wstrdup(temp));

					m_cmdListInd = -1;
					if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
						NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

					EnableWindow(GetDlgItem(m_hwnd, IDOK), FALSE);
					SetFocus(GetDlgItem(m_hwnd, IDC_MESSAGE));

					SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");

					if (g_dat.bAutoClose)
						DestroyWindow(m_hwnd);
					else if (g_dat.bAutoMin)
						ShowWindow(m_hwnd, SW_MINIMIZE);
				}
				return TRUE;
			}
			break;

		case IDCANCEL:
			DestroyWindow(m_hwnd);
			return TRUE;

		case IDC_USERMENU:
			if (GetKeyState(VK_SHIFT) & 0x8000)    // copy user name
				SendMessage(m_hwnd, DM_USERNAMETOCLIP, 0, 0);
			else {
				RECT rc;
				HMENU hMenu = Menu_BuildContactMenu(m_hContact);
				GetWindowRect(GetDlgItem(m_hwnd, LOWORD(wParam)), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, m_hwnd, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, m_hContact, 0);
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, m_hContact, 0);
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = {};
				acs.hContact = m_hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = 0;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)m_hwnd, (LPARAM)&acs);
			}
			if (!db_get_b(m_hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(m_hwnd, IDC_ADD), FALSE);
			break;

		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int len = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_MESSAGE));
				UpdateReadChars();
				EnableWindow(GetDlgItem(m_hwnd, IDOK), len != 0);
				if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
					m_nLastTyping = GetTickCount();
					if (len) {
						if (m_nTypeMode == PROTOTYPE_SELFTYPING_OFF)
							NotifyTyping(PROTOTYPE_SELFTYPING_ON);
					}
					else if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
						NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
				}
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == m_hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK || ((LPNMHDR)lParam)->code == NM_RCLICK) {
				NMMOUSE *nm = (NMMOUSE *)lParam;
				RECT rc;

				SendMessage(m_hwndStatus, SB_GETRECT, SendMessage(m_hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
				if (nm->pt.x >= rc.left)
					CheckStatusIconClick(m_hContact, m_hwndStatus, nm->pt, rc, 2, ((LPNMHDR)lParam)->code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
				return TRUE;
			}
		}

		HCURSOR hCur;
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_LOG:
			switch (((LPNMHDR)lParam)->code) {
			case EN_MSGFILTER:
				switch (((MSGFILTER *)lParam)->msg) {
				case WM_LBUTTONDOWN:
					hCur = GetCursor();
					if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE) || hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE)) {
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;

				case WM_MOUSEMOVE:
					hCur = GetCursor();
					if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE) || hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE))
						SetCursor(LoadCursor(NULL, IDC_ARROW));
					break;

				case WM_RBUTTONUP:
					CHARRANGE all = { 0, -1 };
					HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
					HMENU hSubMenu = GetSubMenu(hMenu, 0);
					TranslateMenu(hSubMenu);
					SendMessage(((NMHDR *)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
					if (sel.cpMin == sel.cpMax)
						EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);

					pLink = (ENLINK*)lParam;
					POINT pt = { GET_X_LPARAM(pLink->lParam), GET_Y_LPARAM(pLink->lParam) };
					ClientToScreen(pLink->nmhdr.hwndFrom, &pt);

					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL)) {
					case IDM_COPY:
						SendMessage(pLink->nmhdr.hwndFrom, WM_COPY, 0, 0);
						break;
					case IDM_COPYALL:
						SendMessage(pLink->nmhdr.hwndFrom, EM_EXSETSEL, 0, (LPARAM)&all);
						SendMessage(pLink->nmhdr.hwndFrom, WM_COPY, 0, 0);
						SendMessage(pLink->nmhdr.hwndFrom, EM_EXSETSEL, 0, (LPARAM)&sel);
						break;
					case IDM_SELECTALL:
						SendMessage(pLink->nmhdr.hwndFrom, EM_EXSETSEL, 0, (LPARAM)&all);
						break;
					case IDM_CLEAR:
						SetDlgItemText(m_hwnd, IDC_LOG, L"");
						m_hDbEventFirst = NULL;
						break;
					}
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;

			case EN_VSCROLL:
				if (LOWORD(wParam) == IDC_LOG && GetWindowLongPtr((HWND)lParam, GWL_STYLE) & WS_VSCROLL) {
					SCROLLINFO si = {};
					si.cbSize = sizeof(si);
					si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
					GetScrollInfo((HWND)lParam, SB_VERT, &si);
					if ((si.nPos + (int)si.nPage + 5) >= si.nMax)
						if (KillTimer(m_hwnd, TIMERID_FLASHWND))
							FlashWindow(m_hwnd, FALSE);
				}
				break;

			case EN_LINK:
				pLink = (ENLINK*)lParam;
				switch (pLink->msg) {
				case WM_SETCURSOR:
					SetCursor(hCurHyperlinkHand);
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;

				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
					SendDlgItemMessage(m_hwnd, IDC_LOG, EM_EXGETSEL, 0, (LPARAM)&sel);
					if (sel.cpMin != sel.cpMax)
						break;

					TEXTRANGE tr;
					tr.chrg = pLink->chrg;
					tr.lpstrText = (wchar_t*)_alloca((tr.chrg.cpMax - tr.chrg.cpMin + 8) * sizeof(wchar_t));
					SendDlgItemMessage(m_hwnd, IDC_LOG, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
					if (wcschr(tr.lpstrText, '@') != NULL && wcschr(tr.lpstrText, ':') == NULL && wcschr(tr.lpstrText, '/') == NULL) {
						memmove(tr.lpstrText + 7, tr.lpstrText, (tr.chrg.cpMax - tr.chrg.cpMin + 1) * sizeof(wchar_t));
						memcpy(tr.lpstrText, L"mailto:", 7 * sizeof(wchar_t));
					}

					if (pLink->msg == WM_RBUTTONDOWN) {
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
						HMENU hSubMenu = GetSubMenu(hMenu, 1);
						TranslateMenu(hSubMenu);

						POINT pt = { GET_X_LPARAM(pLink->lParam), GET_Y_LPARAM(pLink->lParam) };
						ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

						switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, NULL)) {
						case IDM_OPENLINK:
							ShellExecute(NULL, L"open", tr.lpstrText, NULL, NULL, SW_SHOW);
							break;

						case IDM_COPYLINK:
							if (OpenClipboard(m_hwnd)) {
								EmptyClipboard();
								HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (mir_wstrlen(tr.lpstrText) + 1) * sizeof(wchar_t));
								mir_wstrcpy((wchar_t*)GlobalLock(hData), tr.lpstrText);
								GlobalUnlock(hData);
								SetClipboardData(CF_UNICODETEXT, hData);
								CloseClipboard();
							}
							break;
						}

						DestroyMenu(hMenu);
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}

					ShellExecute(NULL, L"open", tr.lpstrText, NULL, NULL, SW_SHOW);
					SetFocus(GetDlgItem(m_hwnd, IDC_MESSAGE));
					break;
				}
			}
		}
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(m_hwndStatus, SB_SETTEXT, (SBT_OWNERDRAW | (SendMessage(m_hwndStatus, SB_GETPARTS, 0, 0) - 1)), 0);
		break;
	}
	return CDlgBase::DlgProc(uMsg, wParam, lParam);
}
