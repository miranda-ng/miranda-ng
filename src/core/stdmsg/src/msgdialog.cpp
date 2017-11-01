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

#define ENTERCLICKTIME   1000   //max time in ms during which a double-tap on enter will cause a send

int SendMessageDirect(const wchar_t *szMsg, MCONTACT hContact)
{
	if (hContact == 0)
		return 0;

	int flags = 0;
	if (Utils_IsRtl(szMsg))
		flags |= PREF_RTL;

	T2Utf sendBuffer(szMsg);
	if (!mir_strlen(sendBuffer))
		return 0;

	if (db_mc_isMeta(hContact))
		hContact = db_mc_getSrmmSub(hContact);

	int sendId = ProtoChainSend(hContact, PSS_MESSAGE, flags, (LPARAM)sendBuffer);
	msgQueue_add(hContact, sendId, sendBuffer.detach(), flags);
	return sendId;
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

static void SetEditorText(HWND hwnd, const wchar_t* txt)
{
	SetWindowText(hwnd, txt);
	SendMessage(hwnd, EM_SETSEL, -1, -1);
}

/////////////////////////////////////////////////////////////////////////////////////////

CSrmmWindow::CSrmmWindow(CTabbedWindow *pOwner, MCONTACT hContact) :
	CSuper(IDD_MSG),
	m_splitter(this, IDC_SPLITTERY),
	m_avatar(this, IDC_AVATAR),
	m_cmdList(20),
	m_bNoActivate(g_dat.bDoNotStealFocus),
	m_pOwner(pOwner)
{
	m_hContact = hContact;

	m_btnOk.OnClick = Callback(this, &CSrmmWindow::onClick_Ok);
	m_splitter.OnChange = Callback(this, &CSrmmWindow::OnSplitterMoved);
}

void CSrmmWindow::OnInitDialog()
{
	CSuper::OnInitDialog();

	m_bIsMeta = db_mc_isMeta(m_hContact) != 0;
	m_hTimeZone = TimeZone_CreateByContact(m_hContact, 0, TZF_KNOWNONLY);
	m_wMinute = 61;

	NotifyEvent(MSG_WINDOW_EVT_OPENING);
	if (m_wszInitialText) {
		m_message.SetText(m_wszInitialText);

		int len = GetWindowTextLength(m_message.GetHwnd());
		PostMessage(m_message.GetHwnd(), EM_SETSEL, len, len);
		mir_free(m_wszInitialText);
	}

	m_szProto = GetContactProto(m_hContact);

	// avatar stuff
	m_limitAvatarH = db_get_b(0, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ? db_get_dw(0, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT) : 0;

	if (m_hContact && m_szProto != nullptr)
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
	else
		m_wStatus = ID_STATUS_OFFLINE;
	m_wOldStatus = m_wStatus;
	m_splitterPos = (int)db_get_dw(g_dat.bSavePerContact ? m_hContact : 0, SRMMMOD, "splitterPos", (DWORD)-1);
	m_cmdListInd = -1;
	m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
	SetTimer(m_hwnd, TIMERID_TYPE, 1000, nullptr);

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rc);
	POINT pt = { 0, (rc.top + rc.bottom) / 2 };
	ScreenToClient(m_hwnd, &pt);
	m_originalSplitterPos = pt.y;
	if (m_splitterPos == -1)
		m_splitterPos = m_originalSplitterPos;

	GetWindowRect(m_message.GetHwnd(), &m_minEditInit);
	SendMessage(m_hwnd, DM_UPDATESIZEBAR, 0, 0);

	m_avatar.Enable(false);

	m_log.SendMsg(EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_SCROLL);
	m_log.SendMsg(EM_AUTOURLDETECT, TRUE, 0);

	if (m_hContact && m_szProto) {
		int nMax = CallProtoService(m_szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, m_hContact);
		if (nMax)
			m_message.SendMsg(EM_LIMITTEXT, nMax, 0);

		// get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF
		m_log.SendMsg(EM_LIMITTEXT, sizeof(wchar_t) * 0x7FFFFFFF, 0);
	}

	if (m_hContact) {
		int historyMode = db_get_b(0, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);
		// This finds the first message to display, it works like shit
		m_hDbEventFirst = db_event_firstUnread(m_hContact);
		switch (historyMode) {
		case LOADHISTORY_COUNT:
			for (int i = db_get_w(0, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT); i--;) {
				MEVENT hPrevEvent;
				if (m_hDbEventFirst == 0)
					hPrevEvent = db_event_last(m_hContact);
				else
					hPrevEvent = db_event_prev(m_hContact, m_hDbEventFirst);
				if (hPrevEvent == 0)
					break;

				DBEVENTINFO dbei = {};
				m_hDbEventFirst = hPrevEvent;
				db_event_get(hPrevEvent, &dbei);
				if (!DbEventIsShown(&dbei))
					i++;
			}
			break;

		case LOADHISTORY_TIME:
			DBEVENTINFO dbei = {};
			if (m_hDbEventFirst == 0)
				dbei.timestamp = (DWORD)time(nullptr);
			else
				db_event_get(m_hDbEventFirst, &dbei);

			DWORD firstTime = dbei.timestamp - 60 * db_get_w(0, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
			for (;;) {
				MEVENT hPrevEvent;
				if (m_hDbEventFirst == 0)
					hPrevEvent = db_event_last(m_hContact);
				else
					hPrevEvent = db_event_prev(m_hContact, m_hDbEventFirst);
				if (hPrevEvent == 0)
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
			DBEVENTINFO dbei = {};
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
				m_message.SetText(dbv.ptszVal);
				m_btnOk.Enable(true);
				UpdateReadChars();
				PostMessage(m_message.GetHwnd(), EM_SETSEL, -1, -1);
			}
			db_free(&dbv);
		}
	}
	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_CHANGE);

	int flag = m_bNoActivate ? RWPF_HIDDEN : 0;
	if (Utils_RestoreWindowPosition(m_hwnd, g_dat.bSavePerContact ? m_hContact : 0, SRMMMOD, "", flag)) {
		if (g_dat.bSavePerContact) {
			if (Utils_RestoreWindowPosition(m_hwnd, 0, SRMMMOD, "", flag | RWPF_NOMOVE))
				SetWindowPos(m_hwnd, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
		}
		else SetWindowPos(m_hwnd, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	if (m_bNoActivate) {
		SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
	}
	else {
		SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetForegroundWindow(m_hwnd);
		SetFocus(m_message.GetHwnd());
	}

	SendMessage(m_hwnd, DM_GETAVATAR, 0, 0);
	NotifyEvent(MSG_WINDOW_EVT_OPEN);
}

void CSrmmWindow::OnDestroy()
{
	NotifyEvent(MSG_WINDOW_EVT_CLOSING);

	// save string from the editor
	if (m_hContact) {
		ptrW msg(m_message.GetText());
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

	for (int i = 0; i < m_cmdList.getCount(); i++)
		mir_free(m_cmdList[i]);
	m_cmdList.destroy();

	MCONTACT hContact = (g_dat.bSavePerContact) ? m_hContact : 0;
	db_set_dw(hContact ? m_hContact : 0, SRMMMOD, "splitterPos", m_splitterPos);

	if (m_hFont) {
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}

	WINDOWPLACEMENT wp = { sizeof(wp) };
	GetWindowPlacement(m_hwnd, &wp);
	if (!m_windowWasCascaded) {
		db_set_dw(hContact, SRMMMOD, "x", wp.rcNormalPosition.left);
		db_set_dw(hContact, SRMMMOD, "y", wp.rcNormalPosition.top);
	}
	db_set_dw(hContact, SRMMMOD, "width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	db_set_dw(hContact, SRMMMOD, "height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	Window_FreeIcon_IcoLib(m_hwnd);

	CSuper::OnDestroy();

	// a temporary contact should be destroyed after removing window from the window list to prevent recursion
	if (m_hContact && g_dat.bDeleteTempCont)
		if (db_get_b(m_hContact, "CList", "NotOnList", 0))
			db_delete_contact(m_hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmWindow::onClick_Ok(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	ptrA msgText(m_message.GetRichTextRtf(true));
	ptrW temp(mir_utf8decodeW(msgText));
	if (!temp[0])
		return;

	int sendId = SendMessageDirect(rtrimw(temp), m_hContact);
	if (sendId) {
		m_cmdList.insert(temp.detach());

		m_cmdListInd = -1;
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

		m_btnOk.Enable(false);
		SetFocus(m_message.GetHwnd());

		m_message.SetText(L"");

		if (!g_Settings.bTabsEnable) {
			if (g_dat.bAutoClose)
				::PostMessage(m_hwndParent, WM_CLOSE, 0, 0);
			else if (g_dat.bAutoMin)
				::ShowWindow(m_hwndParent, SW_MINIMIZE);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmWindow::OnOptionsApplied(bool bUpdateAvatar)
{
	CustomButtonData *cbd;
	for (int i = 0; cbd = Srmm_GetNthButton(i); i++) {
		HWND hwndButton = GetDlgItem(m_hwnd, cbd->m_dwButtonCID);
		if (hwndButton == nullptr)
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

	ShowWindow(GetDlgItem(m_hwnd, IDCANCEL), SW_HIDE);
	m_splitter.Show();
	m_btnOk.Show(g_dat.bSendButton);
	m_btnOk.Enable(GetWindowTextLength(m_message.GetHwnd()) != 0);
	if (m_avatarPic == nullptr || !g_dat.bShowAvatar)
		m_avatar.Hide();
	SendMessage(m_hwnd, DM_UPDATETITLE, 0, 0);
	Resize();

	if (m_hBkgBrush)
		DeleteObject(m_hBkgBrush);

	COLORREF colour = db_get_dw(0, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
	m_hBkgBrush = CreateSolidBrush(colour);
	m_log.SendMsg(EM_SETBKGNDCOLOR, 0, colour);
	m_message.SendMsg(EM_SETBKGNDCOLOR, 0, colour);

	// avatar stuff
	m_avatarPic = nullptr;
	m_limitAvatarH = 0;
	if (CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_AVATARS)
		m_limitAvatarH = db_get_b(0, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ?
		db_get_dw(0, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT) : 0;

	if (bUpdateAvatar)
		SendMessage(m_hwnd, DM_GETAVATAR, 0, 0);

	InvalidateRect(m_message.GetHwnd(), nullptr, FALSE);

	LOGFONT lf;
	CHARFORMAT cf = {};
	if (m_hFont)
		DeleteObject(m_hFont);
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &cf.crTextColor);
	m_hFont = CreateFontIndirect(&lf);
	m_message.SendMsg(WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));

	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_ALL, (WPARAM)&cf);

	// configure message history for proper RTL formatting
	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(pf2));
	pf2.cbSize = sizeof(pf2);

	pf2.wEffects = PFE_RTLPARA;
	pf2.dwMask = PFM_RTLPARA;
	ClearLog();
	m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	pf2.wEffects = 0;
	m_log.SendMsg(EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
	m_log.SendMsg(EM_SETLANGOPTIONS, 0, m_log.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);

	SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);
	SendMessage(m_hwnd, DM_UPDATEWINICON, 0, 0);
}

void CSrmmWindow::OnSplitterMoved(CSplitter *pSplitter)
{
	RECT rc, rcLog;
	GetClientRect(m_hwnd, &rc);
	GetWindowRect(m_log.GetHwnd(), &rcLog);

	int oldSplitterY = m_splitterPos;
	m_splitterPos = rc.bottom - pSplitter->GetPos() + 23;
	GetWindowRect(m_message.GetHwnd(), &rc);
	if (rc.bottom - rc.top + (m_splitterPos - oldSplitterY) < m_minEditBoxSize.cy)
		m_splitterPos = oldSplitterY + m_minEditBoxSize.cy - (rc.bottom - rc.top);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmWindow::CloseTab()
{
	if (g_Settings.bTabsEnable) {
		SendMessage(GetParent(m_hwndParent), GC_REMOVETAB, 0, (LPARAM)this);
		Close();
	}
	else SendMessage(m_hwndParent, WM_CLOSE, 0, 0);
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
	if (!db_get_b(m_hContact, SRMMMOD, SRMSGSET_TYPING, db_get_b(0, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)))
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

void CSrmmWindow::ScrollToBottom()
{
	if (!(GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL))
		return;

	SCROLLINFO si = {};
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	GetScrollInfo(m_log.GetHwnd(), SB_VERT, &si);
	si.fMask = SIF_POS;
	si.nPos = si.nMax - si.nPage;
	SetScrollInfo(m_log.GetHwnd(), SB_VERT, &si, TRUE);
	m_log.SendMsg(WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
}

void CSrmmWindow::ShowAvatar()
{
	if (g_dat.bShowAvatar) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, getActiveContact(), 0);
		if (ace && (INT_PTR)ace != CALLSERVICE_NOTFOUND && (ace->dwFlags & AVS_BITMAP_VALID) && !(ace->dwFlags & AVS_HIDEONCLIST))
			m_avatarPic = ace->hbmPic;
		else
			m_avatarPic = nullptr;
	}
	else m_avatarPic = nullptr;

	SendMessage(m_hwnd, DM_UPDATESIZEBAR, 0, 0);
	SendMessage(m_hwnd, DM_AVATARSIZECHANGE, 0, 0);
}

void CSrmmWindow::ShowTime()
{
	if (m_hTimeZone) {
		SYSTEMTIME st;
		GetSystemTime(&st);
		if (m_wMinute != st.wMinute) {
			wchar_t buf[32];
			unsigned i = g_dat.bShowReadChar ? 2 : 1;

			TimeZone_PrintDateTime(m_hTimeZone, L"t", buf, _countof(buf), 0);
			SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, i, (LPARAM)buf);
			m_wMinute = st.wMinute;
		}
	}
}

void CSrmmWindow::SetupStatusBar()
{
	int i = 0, statwidths[4];
	int icons_width = GetStatusIconsCount(m_hContact) * (GetSystemMetrics(SM_CXSMICON) + 2) + SB_GRIP_WIDTH;

	RECT rc;
	GetWindowRect(m_pOwner->m_hwndStatus, &rc);
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
	SendMessage(m_pOwner->m_hwndStatus, SB_SETPARTS, i, (LPARAM)statwidths);

	UpdateReadChars();
	ShowTime();
	SendMessage(m_hwnd, DM_STATUSICONCHANGE, 0, 0);
}

void CSrmmWindow::SetStatusText(const wchar_t *wszText, HICON hIcon)
{
	SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hIcon);
	SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)(wszText == nullptr ? L"" : wszText));
}

void CSrmmWindow::UpdateReadChars()
{
	if (g_dat.bShowReadChar) {
		wchar_t buf[32];
		int len = GetWindowTextLength(m_message.GetHwnd());

		mir_snwprintf(buf, L"%d", len);
		SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSrmmWindow::Resizer(UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_SRMM_LOG:
		if (!g_dat.bShowButtons)
			urc->rcItem.top -= m_lineHeight;
		urc->rcItem.bottom -= m_splitterPos - m_originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_SPLITTERY:
		urc->rcItem.top -= m_splitterPos - m_originalSplitterPos;
		urc->rcItem.bottom -= m_splitterPos - m_originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_SRMM_MESSAGE:
		if (!g_dat.bSendButton)
			urc->rcItem.right = urc->dlgNewSize.cx - urc->rcItem.left;
		if (g_dat.bShowAvatar && m_avatarPic)
			urc->rcItem.left = m_avatarWidth + 4;

		urc->rcItem.top -= m_splitterPos - m_originalSplitterPos;
		if (!g_dat.bSendButton)
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDCANCEL:
	case IDOK:
		urc->rcItem.top -= m_splitterPos - m_originalSplitterPos;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_AVATAR:
		urc->rcItem.top = urc->rcItem.bottom - (m_avatarHeight + 2);
		urc->rcItem.right = urc->rcItem.left + (m_avatarWidth + 2);
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CSrmmWindow::WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KEYDOWN) {
		bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		bool isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool isAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
		if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
			return FALSE;
	}
	
	return CSuper::WndProc_Log(msg, wParam, lParam);
}

LRESULT CSrmmWindow::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isShift, isCtrl, isAlt;

	switch (msg) {
	case WM_DROPFILES:
		SendMessage(m_hwnd, WM_DROPFILES, wParam, lParam);
		break;

	case WM_CHAR:
		if (GetWindowLongPtr(m_message.GetHwnd(), GWL_STYLE) & ES_READONLY)
			break;

		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) { //ctrl-a
			m_message.SendMsg(EM_SETSEL, 0, -1);
			return 0;
		}

		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w
			CloseTab();
			return 0;
		}
		break;

	case WM_KEYDOWN:
		isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		isAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;

		if (wParam == VK_RETURN) {
			if (!isShift && isCtrl != g_dat.bSendOnEnter) {
				onClick_Ok(&m_btnOk);
				return 0;
			}
			if (g_dat.bSendOnDblEnter) {
				if (m_iLastEnterTime + ENTERCLICKTIME < GetTickCount())
					m_iLastEnterTime = GetTickCount();
				else {
					m_message.SendMsg(WM_KEYDOWN, VK_BACK, 0);
					m_message.SendMsg(WM_KEYUP, VK_BACK, 0);
					onClick_Ok(&m_btnOk);
					return 0;
				}
			}
		}
		else m_iLastEnterTime = 0;

		if (wParam == VK_INSERT && isShift || wParam == 'V' && isCtrl) { // ctrl-v (paste clean text)
			m_message.SendMsg(WM_PASTE, 0, 0);
			return 0;
		}
								 
		if (wParam == VK_UP && isCtrl && g_dat.bCtrlSupport && !g_dat.bAutoClose) {
			if (m_cmdList.getCount()) {
				if (m_cmdListInd < 0) {
					m_cmdListInd = m_cmdList.getCount() - 1;
					SetEditorText(m_message.GetHwnd(), m_cmdList[m_cmdListInd]);
				}
				else if (m_cmdListInd > 0) {
					SetEditorText(m_message.GetHwnd(), m_cmdList[--m_cmdListInd]);
				}
			}
			m_btnOk.Enable(GetWindowTextLength(m_message.GetHwnd()) != 0);
			UpdateReadChars();
			return 0;
		}

		if (wParam == VK_DOWN && isCtrl && g_dat.bCtrlSupport && !g_dat.bAutoClose) {
			if (m_cmdList.getCount() && m_cmdListInd >= 0) {
				if (m_cmdListInd < m_cmdList.getCount() - 1)
					SetEditorText(m_message.GetHwnd(), m_cmdList[++m_cmdListInd]);
				else {
					m_cmdListInd = -1;
					SetEditorText(m_message.GetHwnd(), m_cmdList[m_cmdList.getCount() - 1]);
				}
			}

			m_btnOk.Enable(GetWindowTextLength(m_message.GetHwnd()) != 0);
			UpdateReadChars();
		}

		if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
			return FALSE;

		if (wParam == VK_TAB && isCtrl && !isShift) { // CTRL-TAB (switch tab/window)
			if (g_Settings.bTabsEnable) {
				SendMessage(GetParent(GetParent(m_hwnd)), GC_SWITCHNEXTTAB, 0, 0);
				return TRUE;
			}
		}

		if (wParam == VK_TAB && isCtrl && isShift) { // CTRL_SHIFT-TAB (switch tab/window)
			if (g_Settings.bTabsEnable) {
				SendMessage(GetParent(GetParent(m_hwnd)), GC_SWITCHPREVTAB, 0, 0);
				return TRUE;
			}
		}

		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MOUSEWHEEL:
	case WM_KILLFOCUS:
		m_iLastEnterTime = 0;
		break;

	case WM_SYSCHAR:
		m_iLastEnterTime = 0;
		if ((wParam == 's' || wParam == 'S') && GetKeyState(VK_MENU) & 0x8000) {
			onClick_Ok(&m_btnOk);
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
		{
			static const CHARRANGE all = { 0, -1 };

			MessageWindowPopupData mwpd = {};
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = m_hContact;
			mwpd.hwnd = m_message.GetHwnd();

			HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));

			mwpd.hMenu = GetSubMenu(hMenu, 1);
			TranslateMenu(mwpd.hMenu);

			CHARRANGE sel;
			m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(mwpd.hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
			}
			if (!m_message.SendMsg(EM_CANUNDO, 0, 0))
				EnableMenuItem(mwpd.hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);

			if (!m_message.SendMsg(EM_CANREDO, 0, 0))
				EnableMenuItem(mwpd.hMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);

			if (!m_message.SendMsg(EM_CANPASTE, 0, 0)) {
				if (!IsClipboardFormatAvailable(CF_HDROP))
					EnableMenuItem(mwpd.hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(mwpd.hMenu, IDM_PASTESEND, MF_BYCOMMAND | MF_GRAYED);
			}

			if (lParam == 0xFFFFFFFF) {
				m_message.SendMsg(EM_POSFROMCHAR, (WPARAM)&mwpd.pt, sel.cpMax);
				ClientToScreen(m_message.GetHwnd(), &mwpd.pt);
			}
			else {
				mwpd.pt.x = GET_X_LPARAM(lParam);
				mwpd.pt.y = GET_Y_LPARAM(lParam);
			}

			// First notification
			NotifyEventHooks(pci->hevWinPopup, 0, (LPARAM)&mwpd);

			// Someone added items?
			if (GetMenuItemCount(mwpd.hMenu) > 0) {
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
				mwpd.selection = TrackPopupMenu(mwpd.hMenu, TPM_RETURNCMD, mwpd.pt.x, mwpd.pt.y, 0, m_message.GetHwnd(), nullptr);
			}

			// Second notification
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(pci->hevWinPopup, 0, (LPARAM)&mwpd);

			switch (mwpd.selection) {
			case IDM_UNDO:
				m_message.SendMsg(WM_UNDO, 0, 0);
				break;

			case IDM_REDO:
				m_message.SendMsg(EM_REDO, 0, 0);
				break;

			case IDM_CUT:
				m_message.SendMsg(WM_CUT, 0, 0);
				break;

			case IDM_COPY:
				m_message.SendMsg(WM_COPY, 0, 0);
				break;

			case IDM_PASTE:
				m_message.SendMsg(WM_PASTE, 0, 0);
				break;

			case IDM_PASTESEND:
				m_message.SendMsg(EM_PASTESPECIAL, CF_TEXT, 0);
				onClick_Ok(&m_btnOk);
				break;

			case IDM_DELETE:
				m_message.SendMsg(EM_REPLACESEL, TRUE, 0);
				break;

			case IDM_SELECTALL:
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				break;

			case IDM_CLEAR:
				m_message.SetText(L"");
				break;
			}
			DestroyMenu(hMenu);
			return 0;
		}

	case WM_PASTE:
		if (IsClipboardFormatAvailable(CF_HDROP)) {
			if (OpenClipboard(m_message.GetHwnd())) {
				HANDLE hDrop = GetClipboardData(CF_HDROP);
				if (hDrop)
					m_message.SendMsg(WM_DROPFILES, (WPARAM)hDrop, 0);
				CloseClipboard();
			}
		}
		else m_message.SendMsg(EM_PASTESPECIAL, CF_TEXT, 0);
		return 0;
	}

	return CSuper::WndProc_Message(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CSrmmWindow::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ENLINK *pLink;
	CHARRANGE sel;
	RECT rc;

	switch (uMsg) {
	case WM_CONTEXTMENU:
		if (m_pOwner->m_hwndStatus == (HWND)wParam) {
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2.x = pt.x; pt2.y = pt.y;
			ScreenToClient(m_pOwner->m_hwndStatus, &pt);

			// no popup menu for status icons - this is handled via NM_RCLICK notification and the plugins that added the icons
			SendMessage(m_pOwner->m_hwndStatus, SB_GETRECT, SendMessage(m_pOwner->m_hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
			if (pt.x >= rc.left)
				break;

			HMENU hMenu = Menu_BuildContactMenu(m_hContact);
			TrackPopupMenu(hMenu, 0, pt2.x, pt2.y, 0, m_hwnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;

	case WM_DROPFILES: // Mod from tabsrmm
		if (m_szProto == nullptr) break;
		if (!(CallProtoService(m_szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_FILESEND)) break;
		if (m_wStatus == ID_STATUS_OFFLINE) break;
		if (m_hContact != 0) {
			wchar_t szFilename[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			int fileCount = DragQueryFile(hDrop, -1, nullptr, 0), totalCount = 0, i;
			wchar_t **ppFiles = nullptr;
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
		if (m_avatarPic == nullptr || !g_dat.bShowAvatar) {
			m_avatarWidth = 50;
			m_avatarHeight = 50;
			m_avatar.Hide();
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
			m_avatar.Show();
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
		GetWindowRect(m_message.GetHwnd(), &rc);
		if (rc.bottom - rc.top < m_minEditBoxSize.cy)
			m_splitter.OnChange(&m_splitter);

		Resize();
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
			Window_FreeIcon_IcoLib(m_pOwner->GetHwnd());

			if (m_szProto) {
				WORD wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
				Window_SetProtoIcon_IcoLib(m_pOwner->GetHwnd(), m_szProto, wStatus);
				break;
			}
		}
		Window_SetSkinIcon_IcoLib(m_pOwner->GetHwnd(), SKINICON_EVENT_MESSAGE);
		break;

	case DM_USERNAMETOCLIP:
		if (m_hContact) {
			ptrW id(Contact_GetInfo(CNF_UNIQUEID, m_hContact, m_szProto));
			if (id != nullptr && OpenClipboard(m_hwnd)) {
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
		if (m_nTypeSecs)
			break;

		if (m_lastMessage) {
			wchar_t date[64], time[64], fmt[128];
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"d", date, _countof(date), 0);
			TimeZone_PrintTimeStamp(nullptr, m_lastMessage, L"t", time, _countof(time), 0);
			mir_snwprintf(fmt, TranslateT("Last message received on %s at %s."), date, time);
			SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)fmt);
		}
		else SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)L"");

		SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, 0);
		break;

	case DM_OPTIONSAPPLIED:
		OnOptionsApplied(wParam != 0);
		break;

	case DM_UPDATETITLE:
		wchar_t newtitle[256];
		if (m_hContact && m_szProto) {
			m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
			wchar_t *contactName = pcli->pfnGetContactDisplayName(m_hContact, 0);

			wchar_t *szStatus = pcli->pfnGetStatusModeDescription(m_szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE), 0);
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
			SetWindowText(m_pOwner->GetHwnd(), newtitle);
			Resize();
		}
		break;

	case DM_NEWTIMEZONE:
		m_hTimeZone = TimeZone_CreateByContact(m_hContact, 0, TZF_KNOWNONLY);
		m_wMinute = 61;
		Resize();
		break;

	case DM_CASCADENEWWINDOW:
		if ((HWND)wParam != m_hwnd) {
			RECT rcThis, rcNew;
			GetWindowRect(m_pOwner->GetHwnd(), &rcThis);
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

		SetFocus(m_message.GetHwnd());
		// fall through
	case WM_MOUSEACTIVATE:
		SendMessage(m_hwnd, DM_UPDATETITLE, 0, 0);
		if (KillTimer(m_hwnd, TIMERID_FLASHWND))
			FlashWindow(m_pOwner->GetHwnd(), FALSE);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			RECT rcWindow, rcLog;
			GetWindowRect(m_hwnd, &rcWindow);
			GetWindowRect(m_log.GetHwnd(), &rcLog);
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

			SetupStatusBar();

			if (GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) {
				SCROLLINFO si = {};
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
				GetScrollInfo(m_log.GetHwnd(), SB_VERT, &si);
				bottomScroll = (si.nPos + (int)si.nPage + 5) >= si.nMax;
			}

			CDlgBase::DlgProc(uMsg, 0, 0);
			SetButtonsPos(m_hwnd, false);

			// The statusbar sometimes draws over these 2 controls so redraw them
			RedrawWindow(m_btnOk.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			if (g_dat.bShowAvatar && m_avatarPic)
				RedrawWindow(m_avatar.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);

			if (bottomScroll)
				ScrollToBottom();
		}
		return TRUE;

	case DM_APPENDTOLOG:
		StreamInEvents(wParam, 1, 1);
		break;

	case DM_REMAKELOG:
		StreamInEvents(m_hDbEventFirst, -1, 0);
		break;

	case HM_DBEVENTADDED:
		if (wParam == m_hContact) {
			MEVENT hDbEvent = lParam;
			if (m_hDbEventFirst == 0)
				m_hDbEventFirst = hDbEvent;

			DBEVENTINFO dbei = {};
			db_event_get(hDbEvent, &dbei);
			bool isMessage = (dbei.eventType == EVENTTYPE_MESSAGE), isSent = ((dbei.flags & DBEF_SENT) != 0);
			if (DbEventIsShown(&dbei)) {
				// Sounds *only* for sent messages, not for custom events
				if (isMessage && !isSent) {
					if (GetForegroundWindow() == m_pOwner->GetHwnd())
						Skin_PlaySound("RecvMsgActive");
					else
						Skin_PlaySound("RecvMsgInactive");
				}
				if (isMessage && !isSent) {
					m_lastMessage = dbei.timestamp;
					SendMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
				}
				if (hDbEvent != m_hDbEventFirst && db_event_next(m_hContact, hDbEvent) == 0)
					SendMessage(m_hwnd, DM_APPENDTOLOG, hDbEvent, 0);
				else
					SendMessage(m_hwnd, DM_REMAKELOG, 0, 0);

				// Flash window *only* for messages, not for custom events
				if (isMessage && !isSent) {
					if (GetActiveWindow() == m_pOwner->GetHwnd() && GetForegroundWindow() == m_pOwner->GetHwnd()) {
						if (GetWindowLongPtr(m_log.GetHwnd(), GWL_STYLE) & WS_VSCROLL) {
							SCROLLINFO si = {};
							si.cbSize = sizeof(si);
							si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
							GetScrollInfo(m_log.GetHwnd(), SB_VERT, &si);
							if ((si.nPos + (int)si.nPage + 5) < si.nMax)
								SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
						}
					}
					else SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
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
			FlashWindow(m_pOwner->GetHwnd(), TRUE);
			if (m_nFlash > 2 * g_dat.nFlashMax) {
				KillTimer(m_hwnd, TIMERID_FLASHWND);
				FlashWindow(m_pOwner->GetHwnd(), FALSE);
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
					if (GetForegroundWindow() == m_pOwner->GetHwnd())
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

					SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
					SendMessage(m_pOwner->m_hwndStatus, SB_SETICON, 0, (LPARAM)hTyping);
					if (g_dat.bShowTypingWin && GetForegroundWindow() != m_pOwner->GetHwnd()) {
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
		switch (LOWORD(wParam)) {
		case IDC_USERMENU:
			if (GetKeyState(VK_SHIFT) & 0x8000)    // copy user name
				SendMessage(m_hwnd, DM_USERNAMETOCLIP, 0, 0);
			else {
				HMENU hMenu = Menu_BuildContactMenu(m_hContact);
				GetWindowRect(GetDlgItem(m_hwnd, LOWORD(wParam)), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, m_hwnd, nullptr);
				DestroyMenu(hMenu);
			}
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

		case IDC_SRMM_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int len = GetWindowTextLength(m_message.GetHwnd());
				UpdateReadChars();
				m_btnOk.Enable(len != 0);
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
		HCURSOR hCur;
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_SRMM_LOG:
			switch (((LPNMHDR)lParam)->code) {
			case EN_MSGFILTER:
				switch (((MSGFILTER *)lParam)->msg) {
				case WM_LBUTTONDOWN:
					hCur = GetCursor();
					if (hCur == LoadCursor(nullptr, IDC_SIZENS) || hCur == LoadCursor(nullptr, IDC_SIZEWE) || hCur == LoadCursor(nullptr, IDC_SIZENESW) || hCur == LoadCursor(nullptr, IDC_SIZENWSE)) {
						SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;

				case WM_MOUSEMOVE:
					hCur = GetCursor();
					if (hCur == LoadCursor(nullptr, IDC_SIZENS) || hCur == LoadCursor(nullptr, IDC_SIZEWE) || hCur == LoadCursor(nullptr, IDC_SIZENESW) || hCur == LoadCursor(nullptr, IDC_SIZENWSE))
						SetCursor(LoadCursor(nullptr, IDC_ARROW));
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

					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr)) {
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
						ClearLog();
						m_hDbEventFirst = 0;
						break;
					}
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;

			case EN_VSCROLL:
				if (LOWORD(wParam) == IDC_SRMM_LOG && GetWindowLongPtr((HWND)lParam, GWL_STYLE) & WS_VSCROLL) {
					SCROLLINFO si = {};
					si.cbSize = sizeof(si);
					si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
					GetScrollInfo((HWND)lParam, SB_VERT, &si);
					if ((si.nPos + (int)si.nPage + 5) >= si.nMax)
						if (KillTimer(m_hwnd, TIMERID_FLASHWND))
							FlashWindow(m_pOwner->GetHwnd(), FALSE);
				}
				break;
			}
		}
		break;

	case DM_CLOSETAB:
		CloseTab();
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(m_pOwner->m_hwndStatus, SB_SETTEXT, (SBT_OWNERDRAW | (SendMessage(m_pOwner->m_hwndStatus, SB_GETPARTS, 0, 0) - 1)), 0);
		break;
	}
	
	return CSuper::DlgProc(uMsg, wParam, lParam);
}
