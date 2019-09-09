/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

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

static wchar_t* GetQuotedTextW(wchar_t *text)
{
	size_t i, j, l = mir_wstrlen(text);
	int newLine = 1;
	int wasCR = 0;
	for (i = j = 0; i < l; i++) {
		if (text[i] == '\r') {
			wasCR = 1;
			newLine = 1;
			j += text[i + 1] != '\n' ? 2 : 1;
		}
		else if (text[i] == '\n') {
			newLine = 1;
			j += wasCR ? 1 : 2;
			wasCR = 0;
		}
		else {
			j++;
			if (newLine) {
				//for (;i<l && text[i]=='>';i++) j--;
				j += 2;
			}
			newLine = 0;
			wasCR = 0;
		}
	}
	j += 3;

	wchar_t *out = (wchar_t*)mir_alloc(sizeof(wchar_t)* j);
	newLine = 1;
	wasCR = 0;
	for (i = j = 0; i < l; i++) {
		if (text[i] == '\r') {
			wasCR = 1;
			newLine = 1;
			out[j++] = '\r';
			if (text[i + 1] != '\n')
				out[j++] = '\n';
		}
		else if (text[i] == '\n') {
			newLine = 1;
			if (!wasCR)
				out[j++] = '\r';

			out[j++] = '\n';
			wasCR = 0;
		}
		else {
			if (newLine) {
				out[j++] = '>';
				out[j++] = ' ';
				//for (;i<l && text[i]=='>';i++) j--;
			}
			newLine = 0;
			wasCR = 0;
			out[j++] = text[i];
		}
	}
	out[j++] = '\r';
	out[j++] = '\n';
	out[j++] = '\0';
	return out;
}

static void AddToFileList(wchar_t ***pppFiles, int *totalCount, const wchar_t* szFilename)
{
	*pppFiles = (wchar_t**)mir_realloc(*pppFiles, (++*totalCount + 1)*sizeof(wchar_t*));
	(*pppFiles)[*totalCount] = nullptr;
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

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK ConfirmSendAllDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		RECT rcParent, rcChild;
		TranslateDialogDefault(hwndDlg);
		GetWindowRect(GetParent(hwndDlg), &rcParent);
		GetWindowRect(hwndDlg, &rcChild);
		rcChild.bottom -= rcChild.top;
		rcChild.right -= rcChild.left;
		rcParent.bottom -= rcParent.top;
		rcParent.right -= rcParent.left;
		rcChild.left = rcParent.left + (rcParent.right - rcChild.right) / 2;
		rcChild.top = rcParent.top + (rcParent.bottom - rcChild.bottom) / 2;
		MoveWindow(hwndDlg, rcChild.left, rcChild.top, rcChild.right, rcChild.bottom, FALSE);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDYES:
		case IDNO:
		case IDCANCEL:
			int result = LOWORD(wParam);
			if (IsDlgButtonChecked(hwndDlg, IDC_REMEMBER))
				result |= 0x10000;
			EndDialog(hwndDlg, result);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMsgDialog::CMsgDialog(MCONTACT hContact, bool bIncoming) :
	CSuper(g_plugin, IDD_MSG),
	m_bIncoming(bIncoming),
	m_splitterX(this, IDC_SPLITTERX),
	m_splitterY(this, IDC_SPLITTERY),

	m_btnOk(this, IDOK),
	m_btnAdd(this, IDC_ADD),
	m_btnQuote(this, IDC_QUOTE),
	m_btnDetails(this, IDC_DETAILS),
	m_btnUserMenu(this, IDC_USERMENU)
{
	m_hContact = hContact;

	m_btnAdd.OnClick = Callback(this, &CMsgDialog::onClick_Add);
	m_btnQuote.OnClick = Callback(this, &CMsgDialog::onClick_Quote);
	m_btnDetails.OnClick = Callback(this, &CMsgDialog::onClick_Details);
	m_btnUserMenu.OnClick = Callback(this, &CMsgDialog::onClick_UserMenu);
	
	Init();
}

CMsgDialog::CMsgDialog(SESSION_INFO *si) :
	CSuper(g_plugin, IDD_MSG, si),
	m_splitterX(this, IDC_SPLITTERX),
	m_splitterY(this, IDC_SPLITTERY),

	m_btnOk(this, IDOK),
	m_btnAdd(this, IDC_ADD),
	m_btnQuote(this, IDC_QUOTE),
	m_btnDetails(this, IDC_DETAILS),
	m_btnUserMenu(this, IDC_USERMENU)
{
	m_btnFilter.OnClick = Callback(this, &CMsgDialog::onClick_Filter);
	m_btnNickList.OnClick = Callback(this, &CMsgDialog::onClick_ShowList);

	m_splitterX.OnChange = Callback(this, &CMsgDialog::onChange_SplitterX);

	Init();
}

void CMsgDialog::Init()
{
	m_autoClose = CLOSE_ON_CANCEL;
	m_szProto = GetContactProto(m_hContact);

	SetParent(GetParentWindow(m_hContact, isChat()));
	m_pParent = (ParentWindowData *)GetWindowLongPtr(m_hwndParent, GWLP_USERDATA);

	m_btnOk.OnClick = Callback(this, &CMsgDialog::onClick_Ok);

	m_message.OnChange = Callback(this, &CMsgDialog::onChange_Message);
	m_splitterY.OnChange = Callback(this, &CMsgDialog::onChange_SplitterY);
}

bool CMsgDialog::OnInitDialog()
{
	CSuper::OnInitDialog();

	if (m_si)
		m_si->pDlg = this;

	NotifyEvent(MSG_WINDOW_EVT_OPENING);

	if (m_hContact && m_szProto != nullptr)
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
	else
		m_wStatus = ID_STATUS_OFFLINE;

	m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
	SetTimer(m_hwnd, TIMERID_TYPE, 1000, nullptr);

	m_lastEventType = -1;
	m_lastEventTime = time(0);
	m_startTime = time(0);

	m_bUseRtl = g_plugin.getByte(m_hContact, "UseRTL", 0) != 0;
	m_bUseIEView = g_dat.ieviewInstalled ? g_dat.flags.bUseIeview : false;

	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(pf2));
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	if (!m_bUseRtl) {
		pf2.wEffects = 0;
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) & ~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
	}
	else {
		pf2.wEffects = PFE_RTLPARA;
		SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
	}
	m_message.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	
	// Workaround to make Richedit display RTL messages correctly
	memset(&pf2, 0, sizeof(pf2));
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA | PFM_OFFSETINDENT | PFM_RIGHTINDENT;
	pf2.wEffects = PFE_RTLPARA;
	pf2.dxStartIndent = 30;
	pf2.dxRightIndent = 30;
	m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);

	pf2.dwMask = PFM_RTLPARA;
	pf2.wEffects = 0;
	m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	
	DWORD dwExStyle = GetWindowLongPtr(m_log.GetHwnd(), GWL_EXSTYLE);
	SetWindowLongPtr(m_log.GetHwnd(), GWL_EXSTYLE, (m_bUseRtl) ? dwExStyle | WS_EX_LEFTSCROLLBAR :dwExStyle & ~WS_EX_LEFTSCROLLBAR);

	RECT rc;
	GetWindowRect(m_message.GetHwnd(), &rc);
	m_minLogBoxHeight = m_minEditBoxHeight = rc.bottom - rc.top;
	if (m_minEditBoxHeight < g_dat.minInputAreaHeight)
		m_minEditBoxHeight = g_dat.minInputAreaHeight;
	if (m_pParent->iSplitterY == -1)
		m_pParent->iSplitterY = m_minEditBoxHeight;
	if (m_pParent->iSplitterX == -1) {
		GetWindowRect(m_nickList.GetHwnd(), &rc);
		m_pParent->iSplitterX = rc.right - rc.left;
	}

	SetMinSize(BOTTOM_RIGHT_AVATAR_HEIGHT, m_minLogBoxHeight + TOOLBAR_HEIGHT + m_minEditBoxHeight + max(INFO_BAR_HEIGHT, BOTTOM_RIGHT_AVATAR_HEIGHT - TOOLBAR_HEIGHT) + 5);

	if (m_wszInitialText) {
		m_message.SetText(m_wszInitialText);
		mir_free(m_wszInitialText);
	}
	else if (g_dat.flags.bSaveDrafts) {
		int len = 0;
		ptrW ptszSavedMsg(db_get_wsa(m_hContact, "SRMM", "SavedMsg"));
		if (ptszSavedMsg)
			len = m_message.SetRichText(ptszSavedMsg);
		PostMessage(m_message.GetHwnd(), EM_SETSEL, len, len);
	}

	SendMessage(m_hwnd, DM_CHANGEICONS, 0, 0);

	m_log.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
	m_log.SendMsg(EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
	m_log.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_log.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));
	m_log.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0, 0));
	m_log.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	m_log.SendMsg(EM_HIDESELECTION, TRUE, 0);
	m_log.SendMsg(EM_AUTOURLDETECT, TRUE, 0);

	m_message.SetReadOnly(false);
	m_message.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_message.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE | ENM_REQUESTRESIZE);
	if (m_hContact && m_szProto) {
		int nMax = CallProtoService(m_szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, m_hContact);
		if (nMax)
			m_message.SendMsg(EM_EXLIMITTEXT, 0, nMax);
	}

	// get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF
	m_log.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	::DragAcceptFiles(m_message.GetHwnd(), TRUE);
	CreateInfobar();
	
	if (m_bUseIEView) {
		IEVIEWWINDOW ieWindow = { sizeof(IEVIEWWINDOW) };
		ieWindow.iType = IEW_CREATE;
		ieWindow.dwMode = IEWM_SCRIVER;
		ieWindow.parent = m_hwnd;
		ieWindow.cx = 200;
		ieWindow.cy = 300;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		m_hwndIeview = ieWindow.hwnd;
		if (m_hwndIeview == nullptr)
			m_bUseIEView = false;
	}

	if (isChat()) {
		UpdateOptions();
		UpdateStatusBar();
		UpdateTitle();
		UpdateNickList();
	}
	else {
		m_nickList.Hide();
		m_splitterX.Hide();

		bool notifyUnread = false;
		if (m_hContact) {
			int historyMode = g_plugin.iHistoryMode;
			// This finds the first message to display, it works like shit
			m_hDbEventFirst = db_event_firstUnread(m_hContact);
			if (m_hDbEventFirst != 0) {
				DBEVENTINFO dbei = {};
				db_event_get(m_hDbEventFirst, &dbei);
				if (DbEventIsMessageOrCustom(&dbei) && !(dbei.flags & DBEF_READ) && !(dbei.flags & DBEF_SENT))
					notifyUnread = true;
			}

			DBEVENTINFO dbei = {};
			MEVENT hPrevEvent;
			switch (historyMode) {
			case LOADHISTORY_COUNT:
				for (int i = g_plugin.iLoadCount; i > 0; i--) {
					if (m_hDbEventFirst == 0)
						hPrevEvent = db_event_last(m_hContact);
					else
						hPrevEvent = db_event_prev(m_hContact, m_hDbEventFirst);
					if (hPrevEvent == 0)
						break;
					dbei.cbBlob = 0;
					m_hDbEventFirst = hPrevEvent;
					db_event_get(m_hDbEventFirst, &dbei);
					if (!DbEventIsShown(dbei))
						i++;
				}
				break;

			case LOADHISTORY_TIME:
				if (m_hDbEventFirst == 0) {
					dbei.timestamp = time(0);
					hPrevEvent = db_event_last(m_hContact);
				}
				else {
					db_event_get(m_hDbEventFirst, &dbei);
					hPrevEvent = db_event_prev(m_hContact, m_hDbEventFirst);
				}

				DWORD firstTime = dbei.timestamp - 60 * g_plugin.iLoadTime;
				for (;;) {
					if (hPrevEvent == 0)
						break;
					dbei.cbBlob = 0;
					db_event_get(hPrevEvent, &dbei);
					if (dbei.timestamp < firstTime)
						break;
					if (DbEventIsShown(dbei))
						m_hDbEventFirst = hPrevEvent;
					hPrevEvent = db_event_prev(m_hContact, hPrevEvent);
				}
				break;
			}
		}

		MEVENT hdbEvent = db_event_last(m_hContact);
		if (hdbEvent) {
			DBEVENTINFO dbei = {};
			do {
				db_event_get(hdbEvent, &dbei);
				if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
					m_lastMessage = dbei.timestamp;
					break;
				}
			} while ((hdbEvent = db_event_prev(m_hContact, hdbEvent)));
		}

		SendMessage(m_hwndParent, CM_POPUPWINDOW, m_bIncoming, (LPARAM)m_hwnd);
		SendMessage(m_hwnd, DM_OPTIONSAPPLIED, 0, 0);

		if (notifyUnread) {
			if (GetForegroundWindow() != m_hwndParent || m_pParent->hwndActive != m_hwnd) {
				m_iShowUnread = 1;
				SendMessage(m_hwnd, DM_UPDATEICON, 0, 0);
				SetTimer(m_hwnd, TIMERID_UNREAD, TIMEOUT_UNREAD, nullptr);
			}
			SendMessage(m_hwndParent, CM_STARTFLASHING, 0, 0);
		}

		m_iMessagesInProgress = ReattachSendQueueItems(m_hwnd, m_hContact);
		if (m_iMessagesInProgress > 0)
			SendMessage(m_hwnd, DM_SHOWMESSAGESENDING, 0, 0);
	}

	SendMessage(m_hwndParent, CM_ADDCHILD, (WPARAM)this, 0);

	NotifyEvent(MSG_WINDOW_EVT_OPEN);
	return true;
}

void CMsgDialog::OnDestroy()
{
	NotifyEvent(MSG_WINDOW_EVT_CLOSING);

	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
		NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (m_hXStatusTip != nullptr) {
		DestroyWindow(m_hXStatusTip);
		m_hXStatusTip = nullptr;
	}

	IcoLib_ReleaseIcon(m_hStatusIcon); m_hStatusIcon = nullptr;
	IcoLib_ReleaseIcon(m_hStatusIconBig); m_hStatusIconBig = nullptr;
	if (m_hStatusIconOverlay != nullptr) {
		DestroyIcon(m_hStatusIconOverlay);
		m_hStatusIconOverlay = nullptr;
	}	

	ReleaseSendQueueItems(m_hwnd);
	if (g_dat.flags.bSaveDrafts) {
		ptrA szText(m_message.GetRichTextRtf(true));
		if (szText)
			db_set_utf(m_hContact, "SRMM", "SavedMsg", szText);
		else
			db_unset(m_hContact, "SRMM", "SavedMsg");
	}

	tcmdlist_free(cmdList);

	HFONT hFont = (HFONT)m_message.SendMsg(WM_GETFONT, 0, 0);
	if (hFont != nullptr && hFont != (HFONT)m_btnOk.SendMsg(WM_GETFONT, 0, 0))
		DeleteObject(hFont);

	g_plugin.setByte(m_hContact, "UseRTL", m_bUseRtl);
	if (m_hContact && g_dat.flags.bDelTemp) {
		m_hContact = INVALID_CONTACT_ID; // to prevent recursion

		if (db_get_b(m_hContact, "CList", "NotOnList", 0))
			db_delete_contact(m_hContact);
	}

	if (m_si)
		m_si->pDlg = nullptr;

	SendMessage(m_hwndParent, CM_REMOVECHILD, 0, (LPARAM)m_hwnd);
	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
		ieWindow.iType = IEW_DESTROY;
		ieWindow.hwnd = m_hwndIeview;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	}
	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	mir_free(m_wszSearchQuery);
	mir_free(m_wszSearchResult);

	CSuper::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::onClick_Ok(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_hContact == 0)
		return;

	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(pf2));
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_RTLPARA;
	m_message.SendMsg(EM_GETPARAFORMAT, 0, (LPARAM)&pf2);

	MessageSendQueueItem msi = {};
	if (pf2.wEffects & PFE_RTLPARA)
		msi.flags |= PREF_RTL;

	msi.sendBuffer = m_message.GetRichTextRtf(true);
	msi.sendBufferSize = (int)mir_strlen(msi.sendBuffer);
	if (msi.sendBufferSize == 0)
		return;

	if (Utils_IsRtl(ptrW(mir_utf8decodeW(msi.sendBuffer))))
		msi.flags |= PREF_RTL;

	// Store messaging history
	TCmdList *cmdListNew = tcmdlist_last(cmdList);
	while (cmdListNew != nullptr && cmdListNew->temporary) {
		cmdList = tcmdlist_remove(cmdList, cmdListNew);
		cmdListNew = tcmdlist_last(cmdList);
	}
	if (msi.sendBuffer != nullptr)
		cmdList = tcmdlist_append(cmdList, mir_strdup(rtrim(msi.sendBuffer)), 20, FALSE);

	cmdListCurrent = nullptr;

	if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
		NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	m_message.SetText(L"");
	m_btnOk.Disable();
	if (g_plugin.bAutoMin)
		ShowWindow(m_hwndParent, SW_MINIMIZE);

	if (isChat()) {
		CMStringW ptszText(ptrW(mir_utf8decodeW(msi.sendBuffer)));
		g_chatApi.DoRtfToTags(ptszText, 0, nullptr);
		ptszText.Trim();
		ptszText.Replace(L"%", L"%%");

		if (m_si->pMI->bAckMsg) {
			EnableWindow(m_message.GetHwnd(), FALSE);
			m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
		}
		else m_message.SetText(L"");

		Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, ptszText, 0);
	}
	else {
		if (pButton == nullptr)
			SendMessage(m_hwndParent, DM_SENDMESSAGE, 0, (LPARAM)& msi);
		else
			SendMessage(m_hwnd, DM_SENDMESSAGE, 0, (LPARAM)& msi);
	}
}

void CMsgDialog::onClick_UserMenu(CCtrlButton *pButton)
{
	if (GetKeyState(VK_SHIFT) & 0x8000) // copy user name
		SendMessage(m_hwnd, DM_USERNAMETOCLIP, 0, 0);
	else {
		RECT rc;
		HMENU hMenu = Menu_BuildContactMenu(m_hContact);
		GetWindowRect(pButton->GetHwnd(), &rc);
		TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, m_hwnd, nullptr);
		DestroyMenu(hMenu);
	}
}

void CMsgDialog::onClick_Quote(CCtrlButton*)
{
	if (m_hDbEventLast == 0)
		return;

	SETTEXTEX  st;
	st.flags = ST_SELECTION;
	st.codepage = 1200;

	wchar_t *buffer = nullptr;
	if (m_hwndIeview != nullptr) {
		IEVIEWEVENT evt = { sizeof(evt) };
		evt.hwnd = m_hwndIeview;
		evt.hContact = m_hContact;
		evt.iType = IEE_GET_SELECTION;
		buffer = mir_wstrdup((wchar_t*)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&evt));
	}
	else buffer = GetRichEditSelection(m_log.GetHwnd());

	if (buffer != nullptr) {
		wchar_t *quotedBuffer = GetQuotedTextW(buffer);
		m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)quotedBuffer);
		mir_free(quotedBuffer);
		mir_free(buffer);
	}
	else {
		DBEVENTINFO dbei = {};
		dbei.cbBlob = db_event_getBlobSize(m_hDbEventLast);
		if (dbei.cbBlob == 0xFFFFFFFF)
			return;
		dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
		db_event_get(m_hDbEventLast, &dbei);
		if (DbEventIsMessageOrCustom(&dbei)) {
			buffer = DbEvent_GetTextW(&dbei, CP_ACP);
			if (buffer != nullptr) {
				wchar_t *quotedBuffer = GetQuotedTextW(buffer);
				m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)quotedBuffer);
				mir_free(quotedBuffer);
				mir_free(buffer);
			}
		}
		mir_free(dbei.pBlob);
	}
	SetFocus(m_message.GetHwnd());
}

void CMsgDialog::onClick_Add(CCtrlButton*)
{
	Contact_Add(m_hContact, m_hwnd);

	if (!db_get_b(m_hContact, "CList", "NotOnList", 0))
		ShowWindow(GetDlgItem(m_hwnd, IDC_ADD), SW_HIDE);
}

void CMsgDialog::onClick_Details(CCtrlButton*)
{
	CallService(MS_USERINFO_SHOWDIALOG, m_hContact, 0);
}

void CMsgDialog::onClick_History(CCtrlButton*)
{
	CallService(MS_HISTORY_SHOWCONTACTHISTORY, m_hContact, 0);
}

void CMsgDialog::onChange_Message(CCtrlEdit*)
{
	cmdListCurrent = nullptr;

	int len = m_message.GetRichTextLength();
	UpdateReadChars();
	EnableWindow(GetDlgItem(m_hwnd, IDOK), len != 0);

	if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
		m_nLastTyping = GetTickCount();
		if (len != 0) {
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_OFF)
				NotifyTyping(PROTOTYPE_SELFTYPING_ON);
		}
		else if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::onClick_ShowList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_bNicklistEnabled = !m_bNicklistEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bNicklistEnabled ? IDI_NICKLIST : IDI_NICKLIST2));
	ScrollToBottom();
	Resize();
}

void CMsgDialog::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_bFilterEnabled = !m_bFilterEnabled;
	pButton->SendMsg(BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(m_bFilterEnabled ? IDI_FILTER : IDI_FILTER2));
	if (m_bFilterEnabled && db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0)
		ShowFilterMenu();
	else
		RedrawLog();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::onChange_SplitterX(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_pParent->iSplitterX = rc.right - pSplitter->GetPos() + 1;
	if (m_pParent->iSplitterX < 35)
		m_pParent->iSplitterX = 35;
	if (m_pParent->iSplitterX > rc.right - rc.left - 35)
		m_pParent->iSplitterX = rc.right - rc.left - 35;
}

void CMsgDialog::onChange_SplitterY(CSplitter *pSplitter)
{
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	m_pParent->iSplitterY = rc.bottom - pSplitter->GetPos();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::CloseTab()
{
	Close();
}

HICON CMsgDialog::GetTabIcon()
{
	if (m_bShowTyping)
		return g_plugin.getIcon(IDI_TYPING);

	if (m_iShowUnread != 0)
		return m_hStatusIconOverlay;

	return m_hStatusIcon;
}

void CMsgDialog::GetTitlebarIcon(TitleBarData *tbd)
{
	if (m_bShowTyping && g_dat.flags2.bShowTypingWin)
		tbd->hIconNot = tbd->hIcon = g_plugin.getIcon(IDI_TYPING);
	else if (m_iShowUnread && (GetActiveWindow() != m_hwndParent || GetForegroundWindow() != m_hwndParent)) {
		tbd->hIcon = m_hStatusIcon;
		tbd->hIconNot = g_dat.hMsgIcon;
	}
	else {
		tbd->hIcon = m_hStatusIcon;
		tbd->hIconNot = nullptr;
	}
	tbd->hIconBig = m_hStatusIconBig;
}

bool CMsgDialog::IsTypingNotificationSupported()
{
	if (!m_hContact || !m_szProto)
		return false;

	DWORD typeCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (!(typeCaps & PF4_SUPPORTTYPING))
		return false;
	return true;
}

bool CMsgDialog::IsTypingNotificationEnabled()
{
	if (!g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew))
		return FALSE;

	DWORD protoStatus = Proto_GetStatus(m_szProto);
	if (protoStatus < ID_STATUS_ONLINE)
		return FALSE;

	DWORD protoCaps = CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (protoCaps & PF1_VISLIST && db_get_w(m_hContact, m_szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		return FALSE;

	if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && db_get_w(m_hContact, m_szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		return FALSE;

	if (db_get_b(m_hContact, "CList", "NotOnList", 0) && !g_plugin.bTypingUnknown)
		return FALSE;
	return TRUE;
}

void CMsgDialog::LoadSettings()
{
	m_clrInputBG = g_plugin.getDword(SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG);
}

// Don't send to protocols who don't support typing
// Don't send to users who are unchecked in the typing notification options
// Don't send to protocols that are offline
// Don't send to users who are not visible and
// Don't send to users who are not on the visible list when you are in invisible mode.

void CMsgDialog::NotifyTyping(int mode)
{
	if (!IsTypingNotificationSupported())
		return;

	if (!IsTypingNotificationEnabled())
		return;

	// End user check
	m_nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, m_hContact, m_nTypeMode);
}

void CMsgDialog::Reattach(HWND hwndContainer)
{
	MCONTACT hContact = m_hContact;

	POINT pt;
	GetCursorPos(&pt);
	HWND hParent = WindowFromPoint(pt);
	while (GetParent(hParent) != nullptr)
		hParent = GetParent(hParent);

	hParent = WindowList_Find(g_dat.hParentWindowList, (UINT_PTR)hParent);
	if ((hParent != nullptr && hParent != hwndContainer) || (hParent == nullptr && m_pParent->childrenCount > 1 && (GetKeyState(VK_CONTROL) & 0x8000))) {
		if (hParent == nullptr) {
			hParent = GetParentWindow(hContact, false);

			RECT rc;
			GetWindowRect(hParent, &rc);

			rc.right = (rc.right - rc.left);
			rc.bottom = (rc.bottom - rc.top);
			rc.left = pt.x - rc.right / 2;
			rc.top = pt.y - rc.bottom / 2;
			HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);

			RECT rcDesktop = mi.rcWork;
			if (rc.left < rcDesktop.left)
				rc.left = rcDesktop.left;
			if (rc.top < rcDesktop.top)
				rc.top = rcDesktop.top;
			MoveWindow(hParent, rc.left, rc.top, rc.right, rc.bottom, FALSE);
		}
		NotifyEvent(MSG_WINDOW_EVT_CLOSING);
		NotifyEvent(MSG_WINDOW_EVT_CLOSE);
		SetParent(hParent);
		SendMessage(hwndContainer, CM_REMOVECHILD, 0, (LPARAM)m_hwnd);
		SendMessage(m_hwnd, DM_SETPARENT, 0, (LPARAM)hParent);
		SendMessage(hParent, CM_ADDCHILD, (WPARAM)this, 0);
		SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
		SendMessage(hParent, CM_ACTIVATECHILD, 0, (LPARAM)m_hwnd);
		NotifyEvent(MSG_WINDOW_EVT_OPENING);
		NotifyEvent(MSG_WINDOW_EVT_OPEN);
		ShowWindow(hParent, SW_SHOWNA);
		EnableWindow(hParent, TRUE);
	}
}

void CMsgDialog::ScrollToBottom()
{
	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_SCROLLBOTTOM;
		ieWindow.hwnd = m_hwndIeview;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		return;
	}

	if (GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) {
		SCROLLINFO si = { sizeof(si) };
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		if (GetScrollInfo(m_log.GetHwnd(), SB_VERT, &si)) {
			if (m_log.GetHwnd() != GetFocus()) {
				si.fMask = SIF_POS;
				si.nPos = si.nMax - si.nPage + 1;
				SetScrollInfo(m_log.GetHwnd(), SB_VERT, &si, TRUE);

				PostMessage(m_log.GetHwnd(), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
		}
	}
}

void CMsgDialog::SetDialogToType()
{
	if (!isChat()) {
		ParentWindowData *pdat = m_pParent;
		if (pdat->flags2.bShowInfoBar)
			ShowWindow(m_hwndInfo, SW_SHOW);
		else
			ShowWindow(m_hwndInfo, SW_HIDE);

		SendMessage(m_hwnd, DM_CLISTSETTINGSCHANGED, 0, 0);
		UpdateReadChars();
	}

	m_message.Show();
	m_log.Show(m_hwndIeview == nullptr);

	m_splitterY.Show();
	m_btnOk.Enable(m_message.GetRichTextLength() != 0);
	Resize();
}

void CMsgDialog::SetStatusIcon()
{
	if (m_szProto == nullptr)
		return;

	MCONTACT hContact = db_mc_getSrmmSub(m_hContact);
	if (hContact == 0)
		hContact = m_hContact;

	char *szProto = GetContactProto(hContact);
	m_hStatusIcon = Skin_LoadProtoIcon(szProto, m_wStatus, false);
	m_hStatusIconBig = Skin_LoadProtoIcon(szProto, m_wStatus, true);

	if (m_hStatusIconOverlay != nullptr)
		DestroyIcon(m_hStatusIconOverlay);

	int index = ImageList_ReplaceIcon(g_dat.hHelperIconList, 0, m_hStatusIcon);
	m_hStatusIconOverlay = ImageList_GetIcon(g_dat.hHelperIconList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
}

void CMsgDialog::ShowAvatar()
{
	INT_PTR res = CallService(MS_AV_GETAVATARBITMAP, m_hContact, 0);
	m_ace = res != CALLSERVICE_NOTFOUND ? (AVATARCACHEENTRY*)res : nullptr;
	m_hbmpAvatarPic = (m_ace != nullptr && (m_ace->dwFlags & AVS_HIDEONCLIST) == 0) ? m_ace->hbmPic : nullptr;
	Resize();

	RefreshInfobar();

	RedrawWindow(GetDlgItem(m_hwnd, IDC_AVATAR), nullptr, nullptr, RDW_INVALIDATE);
}

void CMsgDialog::UpdateStatusBar()
{
	if (m_pParent->hwndActive != m_hwnd)
		return;

	if (isChat()) {
		wchar_t szTemp[512];
		mir_snwprintf(szTemp, L"%s : %s", m_si->pMI->ptszModDispName, m_si->ptszStatusbarText ? m_si->ptszStatusbarText : L"");

		StatusBarData sbd;
		sbd.iItem = 0;
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		sbd.hIcon = m_si->wStatus == ID_STATUS_ONLINE ? m_si->pMI->hOnlineIcon : m_si->pMI->hOfflineIcon;
		sbd.pszText = szTemp;
		SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)& sbd, (LPARAM)m_hwnd);

		sbd.iItem = 1;
		sbd.hIcon = nullptr;
		sbd.pszText = L"";
		SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)& sbd, (LPARAM)m_hwnd);

		Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 0, 0);
	}
	else {
		wchar_t szText[256];
		StatusBarData sbd = { 0 };
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		if (m_iMessagesInProgress && g_dat.flags.bShowProgress) {
			sbd.hIcon = g_plugin.getIcon(IDI_TIMESTAMP);
			sbd.pszText = szText;
			mir_snwprintf(szText, TranslateT("Sending in progress: %d message(s) left..."), m_iMessagesInProgress);
		}
		else if (m_nTypeSecs) {
			sbd.hIcon = g_plugin.getIcon(IDI_TYPING);
			sbd.pszText = szText;
			mir_snwprintf(szText, TranslateT("%s is typing a message..."), Clist_GetContactDisplayName(m_hContact));
			m_nTypeSecs--;
		}
		else if (m_lastMessage) {
			wchar_t date[64], time[64];
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"d", date, _countof(date), 0);
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"t", time, _countof(time), 0);
			mir_snwprintf(szText, TranslateT("Last message received on %s at %s."), date, time);
			sbd.pszText = szText;
		}
		else sbd.pszText = L"";

		SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);
		UpdateReadChars();

		Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 0, MBF_DISABLED);

		if (IsTypingNotificationSupported() && g_dat.flags2.bShowTypingSwitch) {
			int mode = g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew);
			Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 1, mode ? 0 : MBF_DISABLED);
		}
		else Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 1, MBF_HIDDEN);
	}
}

void CMsgDialog::UpdateReadChars()
{
	if (m_pParent->hwndActive == m_hwnd) {
		wchar_t szText[256];
		int len = m_message.GetRichTextLength(1200);

		StatusBarData sbd;
		sbd.iItem = 1;
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		sbd.hIcon = nullptr;
		sbd.pszText = szText;
		mir_snwprintf(szText, L"%d", len);
		SendMessage(m_hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)m_hwnd);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static const wchar_t *titleTokenNames[] = { L"%name%", L"%status%", L"%statusmsg%", L"%account%" };

void CMsgDialog::UpdateTitle()
{
	TitleBarData tbd = {};
	tbd.iFlags = TBDF_TEXT | TBDF_ICON;

	CMStringW wszTitle;
	if (isChat()) {
		tbd.hIcon = (m_si->wStatus == ID_STATUS_ONLINE) ? m_si->pMI->hOnlineIcon : m_si->pMI->hOfflineIcon;
		tbd.hIconBig = (m_si->wStatus == ID_STATUS_ONLINE) ? m_si->pMI->hOnlineIconBig : m_si->pMI->hOfflineIconBig;
		tbd.hIconNot = (m_si->wState & (GC_EVENT_HIGHLIGHT | STATE_TALK)) ? g_plugin.getIcon(IDI_OVERLAY) : nullptr;

		int nUsers = m_si->getUserList().getCount();
		switch (m_si->iType) {
		case GCW_CHATROOM:
			wszTitle.Format((nUsers == 1) ? TranslateT("%s: chat room (%u user)") : TranslateT("%s: chat room (%u users)"), m_si->ptszName, nUsers);
			break;
		case GCW_PRIVMESS:
			wszTitle.Format((nUsers == 1) ? TranslateT("%s: message session") : TranslateT("%s: message session (%u users)"), m_si->ptszName, nUsers);
			break;
		case GCW_SERVER:
			wszTitle.Format(L"%s: Server", m_si->ptszName);
			break;
		}
	}
	else {
		if (g_dat.wszTitleFormat[0])
			wszTitle = g_dat.wszTitleFormat;
		else
			wszTitle = L"%name% - ";

		if (m_hContact && m_szProto) {
			wszTitle.Replace(L"%name%", Clist_GetContactDisplayName(m_hContact));
			wszTitle.Replace(L"%status%", Clist_GetStatusModeDescription(db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE), 0));

			CMStringW tszStatus = ptrW(db_get_wsa(m_hContact, "CList", "StatusMsg"));
			tszStatus.Replace(L"\r\n", L" ");
			wszTitle.Replace(L"%statusmsg%", tszStatus);

			char *accModule = Proto_GetBaseAccountName(m_hContact);
			if (accModule != nullptr) {
				PROTOACCOUNT *proto = Proto_GetAccount(accModule);
				if (proto != nullptr)
					wszTitle.Replace(L"%account%", proto->tszAccountName);
			}
		}

		if (g_dat.wszTitleFormat[0] == 0)
			wszTitle.Append(TranslateT("Message session"));

		GetTitlebarIcon(&tbd);
	}

	tbd.pszText = wszTitle.GetBuffer();
	SendMessage(m_hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)m_hwnd);

	if (isChat())
		SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::MessageDialogResize(int w, int h)
{
	ParentWindowData *pdat = m_pParent;
	bool bToolbar = pdat->flags2.bShowToolBar;
	int logY, logH;

	if (isChat()) {
		bool bNick = m_si->iType != GCW_SERVER && m_bNicklistEnabled;
		int  hSplitterMinTop = TOOLBAR_HEIGHT + m_minLogBoxHeight, hSplitterMinBottom = m_minEditBoxHeight;
		int  toolbarHeight = bToolbar ? TOOLBAR_HEIGHT : 0;

		if (h - pdat->iSplitterY < hSplitterMinTop)
			pdat->iSplitterY = h - hSplitterMinTop;
		if (pdat->iSplitterY < hSplitterMinBottom)
			pdat->iSplitterY = hSplitterMinBottom;

		m_splitterX.Show(bNick);
		if (m_si->iType != GCW_SERVER)
			m_nickList.Show(m_bNicklistEnabled);
		else
			m_nickList.Hide();

		if (m_si->iType == GCW_SERVER) {
			m_btnNickList.Enable(false);
			m_btnFilter.Enable(false);
			m_btnChannelMgr.Enable(false);
		}
		else {
			m_btnNickList.Enable(true);
			m_btnFilter.Enable(true);
			if (m_si->iType == GCW_CHATROOM)
				m_btnChannelMgr.Enable(m_si->pMI->bChanMgr);
		}

		logY = h - pdat->iSplitterY;
		if (bToolbar)
			logY -= toolbarHeight;
		logH = logY;

		HDWP hdwp = BeginDeferWindowPos(5);
		hdwp = DeferWindowPos(hdwp, m_log.GetHwnd(), nullptr, 1, 0, bNick ? w - pdat->iSplitterX - 1 : w - 2, logH, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_nickList.GetHwnd(), nullptr, w - pdat->iSplitterX + 2, 0, pdat->iSplitterX - 3, logY, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_splitterX.GetHwnd(), nullptr, w - pdat->iSplitterX, 1, 2, logY - 1, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_splitterY.GetHwnd(), nullptr, 0, h - pdat->iSplitterY, w, SPLITTER_HEIGHT, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_message.GetHwnd(), nullptr, 1, h - pdat->iSplitterY + SPLITTER_HEIGHT, w - 2, pdat->iSplitterY - SPLITTER_HEIGHT - 1, SWP_NOZORDER);
		EndDeferWindowPos(hdwp);

		RedrawWindow(m_nickList.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
	}
	else {
		int hSplitterPos = pdat->iSplitterY, toolbarHeight = (bToolbar) ? TOOLBAR_HEIGHT : 0;
		int hSplitterMinTop = toolbarHeight + m_minLogBoxHeight, hSplitterMinBottom = m_minEditBoxHeight;
		int infobarInnerHeight = INFO_BAR_INNER_HEIGHT;
		int infobarHeight = INFO_BAR_HEIGHT;
		int avatarWidth = 0, avatarHeight = 0;
		int toolbarWidth = w;
		int messageEditWidth = w - 2;

		if (hSplitterMinBottom < g_dat.minInputAreaHeight)
			hSplitterMinBottom = g_dat.minInputAreaHeight;

		if (!pdat->flags2.bShowInfoBar) {
			infobarHeight = 0;
			infobarInnerHeight = 0;
		}

		if (hSplitterPos > (h - toolbarHeight - infobarHeight + SPLITTER_HEIGHT + 1) / 2)
			hSplitterPos = (h - toolbarHeight - infobarHeight + SPLITTER_HEIGHT + 1) / 2;

		if (h - hSplitterPos - infobarHeight < hSplitterMinTop)
			hSplitterPos = h - hSplitterMinTop - infobarHeight;

		if (hSplitterPos < avatarHeight)
			hSplitterPos = avatarHeight;

		if (hSplitterPos < hSplitterMinBottom)
			hSplitterPos = hSplitterMinBottom;

		if (!pdat->flags2.bShowInfoBar) {
			if (m_hbmpAvatarPic && g_dat.flags.bShowAvatar) {
				avatarWidth = BOTTOM_RIGHT_AVATAR_HEIGHT;
				avatarHeight = toolbarHeight + hSplitterPos - 2;
				if (avatarHeight < BOTTOM_RIGHT_AVATAR_HEIGHT) {
					avatarHeight = BOTTOM_RIGHT_AVATAR_HEIGHT;
					hSplitterPos = avatarHeight - toolbarHeight + 2;
				}
				else avatarHeight = BOTTOM_RIGHT_AVATAR_HEIGHT;

				avatarWidth = avatarHeight;
				if (avatarWidth > BOTTOM_RIGHT_AVATAR_HEIGHT && avatarWidth > w / 4)
					avatarWidth = w / 4;

				if ((toolbarWidth - avatarWidth - 2) < 0)
					avatarWidth = toolbarWidth - 2;

				toolbarWidth -= avatarWidth + 2;
				messageEditWidth -= avatarWidth + 1;
			}
		}

		pdat->iSplitterY = hSplitterPos;

		logY = infobarInnerHeight;
		logH = h - hSplitterPos - toolbarHeight - infobarInnerHeight - SPLITTER_HEIGHT;

		HDWP hdwp = BeginDeferWindowPos(5);
		hdwp = DeferWindowPos(hdwp, m_hwndInfo, nullptr, 1, 0, w - 2, infobarInnerHeight - 2, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_log.GetHwnd(), nullptr, 1, logY, w - 2, logH, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_message.GetHwnd(), nullptr, 1, h - hSplitterPos - 1, messageEditWidth, hSplitterPos, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, GetDlgItem(m_hwnd, IDC_AVATAR), nullptr, w - avatarWidth - 1, h - (avatarHeight + avatarWidth) / 2 - 1, avatarWidth, avatarWidth, SWP_NOZORDER);
		hdwp = DeferWindowPos(hdwp, m_splitterY.GetHwnd(), nullptr, 0, h - hSplitterPos - SPLITTER_HEIGHT - 1, toolbarWidth, SPLITTER_HEIGHT, SWP_NOZORDER);
		EndDeferWindowPos(hdwp);

		RefreshInfobar();
		RedrawWindow(GetDlgItem(m_hwnd, IDC_AVATAR), nullptr, nullptr, RDW_INVALIDATE);
	}

	SetButtonsPos(m_hwnd, m_hContact, bToolbar);

	if (m_hwndIeview != nullptr) {
		IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = m_hwnd;
		ieWindow.hwnd = m_hwndIeview;
		ieWindow.x = 0;
		ieWindow.y = logY;
		ieWindow.cx = w;
		ieWindow.cy = logH;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)& ieWindow);
	}
	else RedrawWindow(m_log.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);

	RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMsgDialog::WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int result = InputAreaShortcuts(m_log.GetHwnd(), msg, wParam, lParam);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_MEASUREITEM:
		MeasureMenuItem(wParam, lParam);
		return TRUE;

	case WM_DRAWITEM:
		return DrawMenuItem(wParam, lParam);

	case WM_LBUTTONUP:
		if (isChat()) {
			CHARRANGE sel;
			SendMessage(m_log.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				SendMessage(m_log.GetHwnd(), WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax;
				SendMessage(m_log.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
			}
			SetFocus(m_message.GetHwnd());
		}
		break;

	case WM_CONTEXTMENU:
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
		HMENU hSubMenu = GetSubMenu(hMenu, 0);
		TranslateMenu(hSubMenu);

		CHARRANGE sel, all = { 0, -1 };
		m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin == sel.cpMax)
			EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);

		POINT pt;
		if (lParam == 0xFFFFFFFF) {
			m_log.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(m_log.GetHwnd(), &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}

		POINT ptl = pt;
		ScreenToClient(m_log.GetHwnd(), &ptl);
		ptrW pszWord(GetRichTextWord(m_log.GetHwnd(), &ptl));
		if (pszWord && pszWord[0]) {
			wchar_t szMenuText[4096];
			mir_snwprintf(szMenuText, TranslateT("Look up '%s':"), pszWord.get());
			ModifyMenu(hSubMenu, 5, MF_STRING | MF_BYPOSITION, 5, szMenuText);
			SetSearchEngineIcons(hMenu, g_dat.hSearchEngineIconList);
		}
		else ModifyMenu(hSubMenu, 5, MF_STRING | MF_GRAYED | MF_BYPOSITION, 5, TranslateT("No word to look up"));

		m_bInMenu = true;
		int uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_log.GetHwnd(), nullptr);
		m_bInMenu = false;

		switch (uID) {
		case IDM_COPY:
			m_log.SendMsg(WM_COPY, 0, 0);
			break;
		
		case IDM_COPYALL:
			m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
			m_log.SendMsg(WM_COPY, 0, 0);
			m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
			break;
		
		case IDM_SELECTALL:
			m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
			break;
		
		case IDM_CLEAR:
			SendMessage(m_hwnd, DM_CLEARLOG, 0, 0);
			break;
		
		case IDM_SEARCH_GOOGLE:
		case IDM_SEARCH_BING:
		case IDM_SEARCH_YANDEX:
		case IDM_SEARCH_YAHOO:
		case IDM_SEARCH_WIKIPEDIA:
		case IDM_SEARCH_FOODNETWORK:
		case IDM_SEARCH_GOOGLE_MAPS:
		case IDM_SEARCH_GOOGLE_TRANSLATE:
			SearchWord(pszWord, uID - IDM_SEARCH_GOOGLE + SEARCHENGINE_GOOGLE);
			PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
			break;
		}
		DestroyMenu(hMenu);
		return TRUE;
	}

	return CSuper::WndProc_Log(msg, wParam, lParam);
}

LRESULT CMsgDialog::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int result = InputAreaShortcuts(m_message.GetHwnd(), msg, wParam, lParam);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			if (CheckSend())
				return 0;
		}
		else m_iLastEnterTime = 0;

		if (isChat()) {
			bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			bool isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			bool isAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

			if (wParam == VK_TAB && isShift && !isCtrl) { // SHIFT-TAB (go to nick list)
				SetFocus(m_nickList.GetHwnd());
				return TRUE;
			}

			if (wParam == VK_TAB && !isCtrl && !isShift) { // tab-autocomplete
				m_message.SendMsg(WM_SETREDRAW, FALSE, 0);
				TabAutoComplete();
				m_message.SendMsg(WM_SETREDRAW, TRUE, 0);
				RedrawWindow(m_nickList.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
				return 0;
			}
			
			if (wParam != VK_RIGHT && wParam != VK_LEFT) {
				replaceStrW(m_wszSearchQuery, nullptr);
				replaceStrW(m_wszSearchResult, nullptr);
			}

			if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
				return TRUE;

			if (wParam == 0x4e && isCtrl && !isAlt) { // ctrl-n (nicklist)
				if (m_btnNickList.Enabled())
					m_btnNickList.Click();
				return TRUE;
			}

			if (wParam == 0x48 && isCtrl && !isAlt) { // ctrl-h (history)
				m_btnHistory.Click();
				return TRUE;
			}

			if (wParam == 0x4f && isCtrl && !isAlt) { // ctrl-o (options)
				if (m_btnChannelMgr.Enabled())
					m_btnChannelMgr.Click();
				return TRUE;
			}

			if (((wParam == VK_INSERT && isShift) || (wParam == 'V' && isCtrl)) && !isAlt) { // ctrl-v (paste clean text)
				m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
				return TRUE;
			}

			if (wParam == VK_NEXT || wParam == VK_PRIOR) {
				m_log.SendMsg(msg, wParam, lParam);
				return TRUE;
			}
		}
		break;

	case EM_ACTIVATE:
		SetActiveWindow(m_hwnd);
		break;

	case WM_MOUSEWHEEL:
		if ((GetWindowLongPtr(m_message.GetHwnd(), GWL_STYLE) & WS_VSCROLL) == 0)
			m_log.SendMsg(WM_MOUSEWHEEL, wParam, lParam);
		m_iLastEnterTime = 0;
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		m_iLastEnterTime = 0;
		break;

	case WM_SYSCHAR:
		m_iLastEnterTime = 0;
		if ((wParam == 's' || wParam == 'S') && (GetKeyState(VK_MENU) & 0x8000)) {
			PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;

	case EM_PASTESPECIAL:
	case WM_PASTE:
		if (IsClipboardFormatAvailable(CF_HDROP)) {
			if (OpenClipboard(m_message.GetHwnd())) {
				HANDLE hDrop = GetClipboardData(CF_HDROP);
				if (hDrop)
					SendMessage(m_hwnd, WM_DROPFILES, (WPARAM)hDrop, 0);
				CloseClipboard();
			}
			return 0;
		}
		break;

	case WM_DROPFILES:
		SendMessage(m_hwnd, WM_DROPFILES, wParam, lParam);
		return 0;

	case WM_CONTEXTMENU:
		InputAreaContextMenu(m_message.GetHwnd(), wParam, lParam, m_hContact);
		return TRUE;
	}
	return CSuper::WndProc_Message(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMsgDialog::WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int result = InputAreaShortcuts(m_nickList.GetHwnd(), msg, wParam, lParam);
	if (result != -1)
		return result;

	switch (msg) {
	case WM_GETDLGCODE:
		{
			BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
			BOOL isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) && !isAlt;

			LPMSG lpmsg;
			if ((lpmsg = (LPMSG)lParam) != nullptr) {
				if (lpmsg->message == WM_KEYDOWN
					&& (lpmsg->wParam == VK_RETURN || lpmsg->wParam == VK_ESCAPE || (lpmsg->wParam == VK_TAB && (isAlt || isCtrl))))
					return DLGC_WANTALLKEYS;
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			int index = m_nickList.SendMsg(LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				USERINFO *ui = g_chatApi.SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				Chat_DoEventHook(m_si, GC_USER_PRIVMESS, ui, nullptr, 0);
			}
			break;
		}

		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT || wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END)
			m_wszSearch[0] = 0;
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		/*
		* simple incremental search for the user (nick) - list control
		* typing esc or movement keys will clear the current search string
		*/
		if (wParam == 27 && m_wszSearch[0]) {						// escape - reset everything
			m_wszSearch[0] = 0;
			break;
		}
		else if (wParam == '\b' && m_wszSearch[0])					// backspace
			m_wszSearch[mir_wstrlen(m_wszSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else {
			wchar_t szNew[2];
			szNew[0] = (wchar_t)wParam;
			szNew[1] = '\0';
			if (mir_wstrlen(m_wszSearch) >= _countof(m_wszSearch) - 2) {
				MessageBeep(MB_OK);
				break;
			}
			mir_wstrcat(m_wszSearch, szNew);
		}
		if (m_wszSearch[0]) {
			// iterate over the (sorted) list of nicknames and search for the
			// string we have
			int iItems = m_nickList.SendMsg(LB_GETCOUNT, 0, 0);
			for (int i = 0; i < iItems; i++) {
				USERINFO *ui = g_chatApi.UM_FindUserFromIndex(m_si, i);
				if (ui) {
					if (!wcsnicmp(ui->pszNick, m_wszSearch, mir_wstrlen(m_wszSearch))) {
						m_nickList.SendMsg(LB_SETCURSEL, i, 0);
						InvalidateRect(m_nickList.GetHwnd(), nullptr, FALSE);
						return 0;
					}
				}
			}

			MessageBeep(MB_OK);
			m_wszSearch[mir_wstrlen(m_wszSearch) - 1] = '\0';
			return 0;
		}
		break;
	}

	return CSuper::WndProc_Nicklist(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CMsgDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PARAFORMAT2 pf2;
	LPNMHDR pNmhdr;

	switch (msg) {
	case DM_GETCONTEXTMENU:
		{
			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, (LONG_PTR)hMenu);
		}
		return TRUE;

	case WM_CONTEXTMENU:
		if (m_hwndParent == (HWND)wParam) {
			POINT pt;
			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, m_hwnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			ScrollToBottom();
		else
			SendMessage(m_hwndParent, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;

	case WM_DROPFILES:
		if (m_szProto == nullptr) break;
		if (!(CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_1, 0)&PF1_FILESEND)) break;
		if (m_wStatus == ID_STATUS_OFFLINE) break;
		if (m_hContact != 0) {
			wchar_t szFilename[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			int fileCount = DragQueryFile(hDrop, -1, nullptr, 0), totalCount = 0, i;
			wchar_t** ppFiles = nullptr;
			for (i = 0; i < fileCount; i++) {
				DragQueryFile(hDrop, i, szFilename, _countof(szFilename));
				AddToFileList(&ppFiles, &totalCount, szFilename);
			}
			CallServiceSync(MS_FILE_SENDSPECIFICFILEST, m_hContact, (LPARAM)ppFiles);
			for (i = 0; ppFiles[i]; i++) mir_free(ppFiles[i]);
			mir_free(ppFiles);
		}
		break;

	case DM_AVATARCHANGED:
		ShowAvatar();
		break;

	case DM_GETAVATAR:
		{
			PROTO_AVATAR_INFORMATION ai = { 0 };
			ai.hContact = m_hContact;
			CallProtoService(m_szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&ai);
			ShowAvatar();
		}
		break;

	case DM_TYPING:
		m_nTypeSecs = (int)lParam > 0 ? (int)lParam : 0;
		break;

	case DM_CHANGEICONS:
		UpdateStatusBar();
		SetStatusIcon();

	case DM_UPDATEICON:
		{
			TitleBarData tbd = { 0 };
			tbd.iFlags = TBDF_ICON;
			GetTitlebarIcon(&tbd);
			SendMessage(m_hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)m_hwnd);

			TabControlData tcd;
			tcd.iFlags = TCDF_ICON;
			tcd.hIcon = GetTabIcon();
			SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
			SendDlgItemMessage(m_hwnd, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hStatusIcon);
		}
		break;

	case DM_UPDATETABCONTROL:
		TabControlData tcd;
		tcd.iFlags = TCDF_TEXT | TCDF_ICON;
		tcd.hIcon = GetTabIcon();
		tcd.pszText = Clist_GetContactDisplayName(m_hContact);
		SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
		break;

	case DM_CLISTSETTINGSCHANGED:
		if (wParam == m_hContact && m_hContact && m_szProto) {
			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
			char idbuf[128], buf[128];
			GetContactUniqueId(idbuf, sizeof(idbuf));
			mir_snprintf(buf, Translate("User menu - %s"), idbuf);
			SendDlgItemMessage(m_hwnd, IDC_USERMENU, BUTTONADDTOOLTIP, (WPARAM)buf, 0);

			if (cws && !mir_strcmp(cws->szModule, m_szProto) && !mir_strcmp(cws->szSetting, "Status"))
				m_wStatus = cws->value.wVal;

			SetStatusIcon();
			SendMessage(m_hwnd, DM_UPDATEICON, 0, 0);
			UpdateTitle();
			SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
			ShowAvatar();
		}
		break;

	case DM_OPTIONSAPPLIED:
		m_bUseIEView = g_dat.ieviewInstalled ? g_dat.flags.bUseIeview : false;
		if (m_bUseIEView && m_hwndIeview == nullptr) {
			IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
			ieWindow.iType = IEW_CREATE;
			ieWindow.dwMode = IEWM_SCRIVER;
			ieWindow.parent = m_hwnd;
			ieWindow.cx = 200;
			ieWindow.cy = 300;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
			m_hwndIeview = ieWindow.hwnd;
			if (m_hwndIeview == nullptr)
				m_bUseIEView = false;
		}
		else if (!m_bUseIEView && m_hwndIeview != nullptr) {
			if (m_hwndIeview != nullptr) {
				IEVIEWWINDOW ieWindow = { sizeof(ieWindow) };
				ieWindow.iType = IEW_DESTROY;
				ieWindow.hwnd = m_hwndIeview;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
			}
			m_hwndIeview = nullptr;
		}

		SendMessage(m_hwnd, DM_GETAVATAR, 0, 0);
		SetDialogToType();
		{
			COLORREF colour = g_plugin.getDword(SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
			m_log.SendMsg(EM_SETBKGNDCOLOR, 0, colour);
			colour = g_plugin.getDword(SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
			m_message.SendMsg(EM_SETBKGNDCOLOR, 0, colour);
			InvalidateRect(m_message.GetHwnd(), nullptr, FALSE);

			LOGFONT lf;
			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &colour);

			CHARFORMAT2 cf2;
			memset(&cf2, 0, sizeof(cf2));
			cf2.cbSize = sizeof(cf2);
			cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
			cf2.crTextColor = colour;
			cf2.bCharSet = lf.lfCharSet;
			wcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
			cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
			cf2.wWeight = (WORD)lf.lfWeight;
			cf2.bPitchAndFamily = lf.lfPitchAndFamily;
			cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat.logPixelSY;
			m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2);
			m_message.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_message.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
		}

		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_OFFSET;
		pf2.dxOffset = (g_dat.flags.bIndentText) ? g_dat.indentSize * 1440 / g_dat.logPixelSX : 0;

		ClearLog();
		m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		m_log.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_log.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));

		SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		UpdateTitle();
		SendMessage(m_hwnd, DM_UPDATETABCONTROL, 0, 0);
		UpdateStatusBar();
		m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
		SetupInfobar();
		break;

	case DM_USERNAMETOCLIP:
		if (m_hContact) {
			char buf[128];
			GetContactUniqueId(buf, sizeof(buf));
			if (!OpenClipboard(m_hwnd) || !mir_strlen(buf))
				break;

			EmptyClipboard();
			HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, mir_strlen(buf) + 1);
			mir_strcpy((LPSTR)GlobalLock(hData), buf);
			GlobalUnlock(hData);
			SetClipboardData(CF_TEXT, hData);
			CloseClipboard();
		}
		break;

	case DM_SWITCHINFOBAR:
	case DM_SWITCHTOOLBAR:
		if (isChat())
			Resize();
		else
			SetDialogToType();
		break;

	case DM_SWITCHTYPING:
		if (IsTypingNotificationSupported()) {
			BYTE typingNotify = (g_plugin.getByte(m_hContact, SRMSGSET_TYPING, g_plugin.bTypingNew));
			g_plugin.setByte(m_hContact, SRMSGSET_TYPING, (BYTE)!typingNotify);
			Srmm_SetIconFlags(m_hContact, SRMM_MODULE, 1, typingNotify ? MBF_DISABLED : 0);
		}
		break;

	case DM_SWITCHRTL:
		memset(&pf2, 0, sizeof(pf2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_RTLPARA;
		m_bUseRtl = !m_bUseRtl;
		if (m_bUseRtl) {
			pf2.wEffects = PFE_RTLPARA;
			SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
			SetWindowLongPtr(m_log.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_log.GetHwnd(), GWL_EXSTYLE) | WS_EX_LEFTSCROLLBAR);
		}
		else {
			pf2.wEffects = 0;
			SetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_message.GetHwnd(), GWL_EXSTYLE) &~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
			SetWindowLongPtr(m_log.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_log.GetHwnd(), GWL_EXSTYLE) &~(WS_EX_LEFTSCROLLBAR));
		}
		m_message.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
		break;

	case DM_ACTIVATE:
		if (isChat()) {
			if (m_si->wState & STATE_TALK) {
				m_si->wState &= ~STATE_TALK;
				db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
			}

			if (m_si->wState & GC_EVENT_HIGHLIGHT) {
				m_si->wState &= ~GC_EVENT_HIGHLIGHT;

				if (g_clistApi.pfnGetEvent(m_hContact, 0))
					g_clistApi.pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
			}

			FixTabIcons();
			if (!m_si->pDlg) {
				ShowRoom(m_si);
				SendMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
			}
			break;
		}
		__fallthrough;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		__fallthrough;
	case WM_MOUSEACTIVATE:
		if (isChat()) {
			SetFocus(m_message.GetHwnd());

			g_chatApi.SetActiveSession(m_si);

			if (db_get_w(m_hContact, m_si->pszModule, "ApparentMode", 0) != 0)
				db_set_w(m_hContact, m_si->pszModule, "ApparentMode", 0);
			if (g_clistApi.pfnGetEvent(m_hContact, 0))
				g_clistApi.pfnRemoveEvent(m_hContact, GC_FAKE_EVENT);
		}
		else {
			if (m_hDbUnreadEventFirst != 0) {
				MEVENT hDbEvent = m_hDbUnreadEventFirst;
				m_hDbUnreadEventFirst = 0;
				while (hDbEvent != 0) {
					DBEVENTINFO dbei = {};
					db_event_get(hDbEvent, &dbei);
					if (!(dbei.flags & DBEF_SENT) && DbEventIsMessageOrCustom(&dbei))
						g_clistApi.pfnRemoveEvent(m_hContact, hDbEvent);
					hDbEvent = db_event_next(m_hContact, hDbEvent);
				}
			}
			if (m_iShowUnread) {
				m_iShowUnread = 0;
				KillTimer(m_hwnd, TIMERID_UNREAD);
				SendMessage(m_hwnd, DM_UPDATEICON, 0, 0);
			}
		}
		break;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)g_chatApi.hListBkgBrush;

	case WM_LBUTTONDOWN:
		SendMessage(m_hwndParent, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;

	case DM_SETFOCUS:
		if (lParam == WM_MOUSEACTIVATE) {
			HWND hLog;
			POINT pt;
			GetCursorPos(&pt);
			if (m_hwndIeview != nullptr)
				hLog = m_hwndIeview;
			else
				hLog = m_log.GetHwnd();

			RECT rc;
			GetWindowRect(hLog, &rc);
			if (pt.x >= rc.left && pt.x <= rc.right && pt.y >= rc.top && pt.y <= rc.bottom)
				return TRUE;
		}
		if (g_dat.hFocusWnd == m_hwnd)
			SetFocus(m_message.GetHwnd());
		return TRUE;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_SETFOCUS:
		SendMessage(m_hwndParent, CM_ACTIVATECHILD, 0, (LPARAM)m_hwnd);
		g_dat.hFocusWnd = m_hwnd;
		PostMessage(m_hwnd, DM_SETFOCUS, 0, 0);
		return TRUE;

	case DM_SETPARENT:
		SetParent((HWND)lParam);
		m_pParent = (ParentWindowData *)GetWindowLongPtr(m_hwndParent, GWLP_USERDATA);
		return TRUE;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			ScrollToBottom();

		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			if (rc.right && rc.bottom)
				MessageDialogResize(rc.right, rc.bottom);
		}
		return TRUE;

	case DM_REMAKELOG:
		m_lastEventType = -1;
		if (wParam == 0 || wParam == m_hContact)
			StreamInEvents(m_hDbEventFirst, -1, 0);

		InvalidateRect(m_log.GetHwnd(), nullptr, FALSE);
		break;

	case HM_DBEVENTADDED:
		if (wParam == m_hContact) {
			MEVENT hDbEvent = lParam;
			DBEVENTINFO dbei = {};
			db_event_get(hDbEvent, &dbei);
			if (m_hDbEventFirst == 0)
				m_hDbEventFirst = hDbEvent;
			if (DbEventIsShown(dbei)) {
				bool bIsActive = IsActive();
				if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & (DBEF_SENT))) {
					/* store the event when the container is hidden so that clist notifications can be removed */
					if (!IsWindowVisible(m_hwndParent) && m_hDbUnreadEventFirst == 0)
						m_hDbUnreadEventFirst = hDbEvent;
					m_lastMessage = dbei.timestamp;
					UpdateStatusBar();
					if (bIsActive)
						Skin_PlaySound("RecvMsgActive");
					else
						Skin_PlaySound("RecvMsgInactive");
					if (g_dat.flags2.bSwitchToActive && (IsIconic(m_hwndParent) || GetActiveWindow() != m_hwndParent) && IsWindowVisible(m_hwndParent))
						SendMessage(m_hwndParent, CM_ACTIVATECHILD, 0, (LPARAM)m_hwnd);
					if (IsAutoPopup(m_hContact))
						SendMessage(m_hwndParent, CM_POPUPWINDOW, 1, (LPARAM)m_hwnd);
				}

				if (hDbEvent != m_hDbEventFirst && db_event_next(m_hContact, hDbEvent) == 0)
					StreamInEvents(hDbEvent, 1, 1);
				else
					SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);

				if (!(dbei.flags & DBEF_SENT) && !DbEventIsCustomForMsgWindow(&dbei)) {
					if (!bIsActive) {
						m_iShowUnread = 1;
						SendMessage(m_hwnd, DM_UPDATEICON, 0, 0);
						SetTimer(m_hwnd, TIMERID_UNREAD, TIMEOUT_UNREAD, nullptr);
					}
					SendMessage(m_hwndParent, CM_STARTFLASHING, 0, 0);
				}
			}
		}
		break;

	case DM_CLEARLOG:
		if (m_hwndIeview != nullptr) {
			IEVIEWEVENT evt = { sizeof(evt) };
			evt.iType = IEE_CLEAR_LOG;
			evt.dwFlags = (m_bUseRtl) ? IEEF_RTL : 0;
			evt.hwnd = m_hwndIeview;
			evt.hContact = m_hContact;
			evt.pszProto = m_szProto;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&evt);
		}

		ClearLog();
		m_hDbEventFirst = 0;
		m_lastEventType = -1;
		break;

	case WM_TIMER:
		if (wParam == TIMERID_MSGSEND)
			ReportSendQueueTimeouts(m_hwnd);
		else if (wParam == TIMERID_TYPE) {
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - m_nLastTyping > TIMEOUT_TYPEOFF)
				NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

			if (m_bShowTyping) {
				if (m_nTypeSecs)
					m_nTypeSecs--;
				else {
					m_bShowTyping = false;
					UpdateStatusBar();
					SendMessage(m_hwnd, DM_UPDATEICON, 0, 0);
				}
			}
			else {
				if (m_nTypeSecs) {
					m_bShowTyping = true;
					UpdateStatusBar();
					SendMessage(m_hwnd, DM_UPDATEICON, 0, 0);
				}
			}
		}
		else if (wParam == TIMERID_UNREAD) {
			tcd.iFlags = TCDF_ICON;
			if (!m_bShowTyping) {
				m_iShowUnread++;
				tcd.hIcon = (m_iShowUnread & 1) ? m_hStatusIconOverlay : m_hStatusIcon;
				SendMessage(m_hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)m_hwnd);
			}
		}
		break;

	case DM_SENDMESSAGE:
		if (lParam) {
			MessageSendQueueItem *msi = (MessageSendQueueItem *)lParam;
			SendMessage(m_hwnd, DM_STARTMESSAGESENDING, 0, 0);

			MessageSendQueueItem *item = CreateSendQueueItem(m_hwnd);
			item->hContact = m_hContact;
			item->proto = mir_strdup(m_szProto);
			item->flags = msi->flags;
			item->sendBufferSize = msi->sendBufferSize;
			item->sendBuffer = mir_strndup(msi->sendBuffer, msi->sendBufferSize);
			SendSendQueueItem(item);
		}
		break;

	case DM_STARTMESSAGESENDING:
		m_iMessagesInProgress++;
	case DM_SHOWMESSAGESENDING:
		SetTimer(m_hwnd, TIMERID_MSGSEND, 1000, nullptr);
		if (g_dat.flags.bShowProgress)
			UpdateStatusBar();
		break;

	case DM_STOPMESSAGESENDING:
		if (m_iMessagesInProgress > 0) {
			m_iMessagesInProgress--;
			if (g_dat.flags.bShowProgress)
				UpdateStatusBar();
		}
		if (m_iMessagesInProgress == 0)
			KillTimer(m_hwnd, TIMERID_MSGSEND);
		break;

	case DM_SHOWERRORMESSAGE:
		if (lParam) {
			SendMessage(m_hwnd, DM_STOPMESSAGESENDING, 0, 0);
			CErrorDlg *pDlg = (CErrorDlg*)lParam;
			pDlg->Create();
		}
		break;

	case DM_ERRORDECIDED:
		{
			MessageSendQueueItem *item = (MessageSendQueueItem *)lParam;
			item->hwndErrorDlg = nullptr;
			switch (wParam) {
			case MSGERROR_CANCEL:
				RemoveSendQueueItem(item);
				SetFocus(m_message.GetHwnd());
				break;
			case MSGERROR_RETRY:
				SendMessage(m_hwnd, DM_STARTMESSAGESENDING, 0, 0);
				SendSendQueueItem(item);
				break;
			}
		}
		break;

	case WM_MEASUREITEM:
		if (!MeasureMenuItem(wParam, lParam)) {
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);

			int ih = Chat_GetTextPixelSize(L"AQGgl'", g_Settings.UserListFont, false);
			int ih2 = Chat_GetTextPixelSize(L"AQGg'", g_Settings.UserListHeadingsFont, false);
			int font = ih > ih2 ? ih : ih2;
			int height = db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12);
			// make sure we have space for icon!
			if (db_get_b(0, CHAT_MODULE, "ShowContactStatus", 0))
				font = font > 16 ? font : 16;
			mis->itemHeight = height > font ? height : font;
		}
		return TRUE;

	case WM_DRAWITEM:
		if (!DrawMenuItem(wParam, lParam)) {
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);

			if (dis->hwndItem == GetDlgItem(m_hwnd, IDC_AVATAR)) {
				int avatarWidth = 0, avatarHeight = 0;
				int itemWidth = dis->rcItem.right - dis->rcItem.left + 1, itemHeight = dis->rcItem.bottom - dis->rcItem.top + 1;
				HDC hdcMem = CreateCompatibleDC(dis->hDC);
				HBITMAP hbmMem = CreateCompatibleBitmap(dis->hDC, itemWidth, itemHeight);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmMem);
				RECT rect;
				rect.top = 0;
				rect.left = 0;
				rect.right = itemWidth - 1;
				rect.bottom = itemHeight - 1;
				FillRect(hdcMem, &rect, GetSysColorBrush(COLOR_BTNFACE));

				if (m_hbmpAvatarPic && g_dat.flags.bShowAvatar) {
					BITMAP bminfo;
					GetObject(m_hbmpAvatarPic, sizeof(bminfo), &bminfo);
					if (bminfo.bmWidth != 0 && bminfo.bmHeight != 0) {
						avatarHeight = itemHeight;
						avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
						if (avatarWidth > itemWidth) {
							avatarWidth = itemWidth;
							avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
						}

						AVATARDRAWREQUEST adr = { sizeof(adr) };
						adr.hContact = m_hContact;
						adr.hTargetDC = hdcMem;
						adr.rcDraw.left = (itemWidth - avatarWidth) / 2;
						adr.rcDraw.top = (itemHeight - avatarHeight) / 2;
						adr.rcDraw.right = avatarWidth - 1;
						adr.rcDraw.bottom = avatarHeight - 1;
						adr.dwFlags = AVDRQ_DRAWBORDER | AVDRQ_HIDEBORDERONTRANSPARENCY;
						CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&adr);
					}
				}
				BitBlt(dis->hDC, 0, 0, itemWidth, itemHeight, hdcMem, 0, 0, SRCCOPY);
				hbmMem = (HBITMAP)SelectObject(hdcMem, hbmMem);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);
				return TRUE;
			}

			if (dis->CtlID == IDC_SRMM_NICKLIST) {
				int index = dis->itemID;
				USERINFO *ui = g_chatApi.SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, index);
				if (ui) {
					int x_offset = 2;

					int height = dis->rcItem.bottom - dis->rcItem.top;
					if (height & 1)
						height++;

					int offset = (height == 10) ? 0 : height / 2 - 5;
					HFONT hFont = (ui->iStatusEx == 0) ? g_Settings.UserListFont : g_Settings.UserListHeadingsFont;
					HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
					SetBkMode(dis->hDC, TRANSPARENT);

					if (dis->itemAction == ODA_FOCUS && dis->itemState & ODS_SELECTED)
						FillRect(dis->hDC, &dis->rcItem, g_chatApi.hListSelectedBkgBrush);
					else //if (dis->itemState & ODS_INACTIVE)
						FillRect(dis->hDC, &dis->rcItem, g_chatApi.hListBkgBrush);

					if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
						IcoLib_ReleaseIcon(hIcon);
						x_offset += 18;
					}
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset, g_chatApi.SM_GetStatusIcon(m_si, ui), 10, 10, 0, nullptr, DI_NORMAL);
					x_offset += 12;
					if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
						HICON hIcon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
						DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 3, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
						IcoLib_ReleaseIcon(hIcon);
						x_offset += 18;
					}

					SetTextColor(dis->hDC, ui->iStatusEx == 0 ? g_Settings.crUserListColor : g_Settings.crUserListHeadingsColor);
					TextOut(dis->hDC, dis->rcItem.left + x_offset, dis->rcItem.top, ui->pszNick, (int)mir_wstrlen(ui->pszNick));
					SelectObject(dis->hDC, hOldFont);
				}
				return TRUE;
			}
		}
		return Menu_DrawItem(lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SENDALL:
			int result;
			if (m_iSendAllConfirm == 0) {
				result = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONFIRM_SENDALL), m_hwnd, ConfirmSendAllDlgProc, (LPARAM)m_hwnd);
				if (result & 0x10000)
					m_iSendAllConfirm = result;
			}
			else result = m_iSendAllConfirm;

			if (LOWORD(result) == IDYES)
				onClick_Ok(nullptr);
			break;
		}
		break;

	case WM_NOTIFY:
		pNmhdr = (LPNMHDR)lParam;
		switch (pNmhdr->idFrom) {
		case IDC_SRMM_LOG:
			switch (pNmhdr->code) {
			case EN_MSGFILTER:
				{
					int result = InputAreaShortcuts(m_message.GetHwnd(), ((MSGFILTER *)lParam)->msg, ((MSGFILTER *)lParam)->wParam, ((MSGFILTER *)lParam)->lParam);
					if (result != -1) {
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
				}

				switch (((MSGFILTER *)lParam)->msg) {
				case WM_RBUTTONUP:
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;
			}
			break;
		
		case IDC_SRMM_MESSAGE:
			if (pNmhdr->code == EN_MSGFILTER && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
			break;

		case IDC_SRMM_NICKLIST:
			if (pNmhdr->code == TTN_NEEDTEXT) {
				LPNMTTDISPINFO lpttd = (LPNMTTDISPINFO)lParam;
				SESSION_INFO *parentdat = (SESSION_INFO *)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);

				POINT p;
				GetCursorPos(&p);
				ScreenToClient(m_nickList.GetHwnd(), &p);
				int item = LOWORD(m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y)));
				USERINFO *ui = g_chatApi.SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, item);
				if (ui != nullptr) {
					static wchar_t ptszBuf[1024];
					mir_snwprintf(ptszBuf, L"%s: %s\r\n%s: %s\r\n%s: %s",
						TranslateT("Nickname"), ui->pszNick,
						TranslateT("Unique ID"), ui->pszUID,
						TranslateT("Status"), g_chatApi.TM_WordToString(parentdat->pStatuses, ui->Status));
					lpttd->lpszText = ptszBuf;
				}
			}
			break;
		}
		break;

	case WM_CHAR:
		SetFocus(m_message.GetHwnd());
		m_message.SendMsg(msg, wParam, lParam);
		break;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}
