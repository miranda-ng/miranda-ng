/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-15 Miranda NG project

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
#include "jabber_iq.h"
#include "jabber_caps.h"

#define GC_SERVER_LIST_SIZE 5

int JabberGcGetStatus(JABBER_GC_AFFILIATION a, JABBER_GC_ROLE r);
int JabberGcGetStatus(JABBER_RESOURCE_STATUS *r);

struct JabberGcRecentInfo
{
	ptrT room, server, nick, password;
	CJabberProto *ppro;

	JabberGcRecentInfo(CJabberProto* proto)
	{
		ppro = proto;
	}
	JabberGcRecentInfo(CJabberProto* proto, const TCHAR *_room, const TCHAR *_server, const TCHAR *_nick = NULL, const TCHAR *_password = NULL)
	{
		ppro = proto;
		fillData(_room, _server, _nick, _password);
	}
	JabberGcRecentInfo(CJabberProto* proto, const TCHAR *jid)
	{
		ppro = proto;
		fillData(jid);
	}
	JabberGcRecentInfo(CJabberProto* proto, int iRecent)
	{
		ppro = proto;
		loadRecent(iRecent);
	}

	~JabberGcRecentInfo()
	{
	}

	void cleanup()
	{
		room = server = nick = password = NULL;
	}

	BOOL equals(const TCHAR *room, const TCHAR *server, const TCHAR *nick = NULL, const TCHAR *password = NULL)
	{
		return
			null_strequals(this->room, room) &&
			null_strequals(this->server, server) &&
			null_strequals(this->nick, nick) &&
			null_strequals(this->password, password);
	}

	BOOL equalsnp(const TCHAR *room, const TCHAR *server, const TCHAR *nick = NULL)
	{
		return
			null_strequals(this->room, room) &&
			null_strequals(this->server, server) &&
			null_strequals(this->nick, nick);
	}

	void fillForm(HWND hwndDlg)
	{
		SetDlgItemText(hwndDlg, IDC_SERVER, server ? server : _T(""));
		SetDlgItemText(hwndDlg, IDC_ROOM, room ? room : _T(""));
		SetDlgItemText(hwndDlg, IDC_NICK, nick ? nick : _T(""));
		SetDlgItemText(hwndDlg, IDC_PASSWORD, password ? password : _T(""));
	}

	void fillData(const TCHAR *room, const TCHAR *server, const TCHAR *nick = NULL, const TCHAR *password = NULL)
	{
		this->room = mir_tstrdup(room);
		this->server = mir_tstrdup(server);
		this->nick = mir_tstrdup(nick);
		this->password = mir_tstrdup(password);
	}

	void fillData(const TCHAR *jid)
	{
		TCHAR *room, *server, *nick = NULL;
		room = NEWTSTR_ALLOCA(jid);
		server = _tcschr(room, _T('@'));
		if (server) {
			*server++ = 0;
			nick = _tcschr(server, _T('/'));
			if (nick) *nick++ = 0;
		}
		else {
			server = room;
			room = NULL;
		}

		fillData(room, server, nick);
	}

	BOOL loadRecent(int iRecent)
	{
		char setting[MAXMODULELABELLENGTH];
		mir_snprintf(setting, SIZEOF(setting), "rcMuc_%d_server", iRecent);
		server = ppro->getTStringA(setting);

		mir_snprintf(setting, SIZEOF(setting), "rcMuc_%d_room", iRecent);
		room = ppro->getTStringA(setting);

		mir_snprintf(setting, SIZEOF(setting), "rcMuc_%d_nick", iRecent);
		nick = ppro->getTStringA(setting);

		mir_snprintf(setting, SIZEOF(setting), "password_rcMuc_%d", iRecent);
		password = ppro->getTStringA(NULL, setting);

		return room || server || nick || password;
	}

	void saveRecent(int iRecent)
	{
		char setting[MAXMODULELABELLENGTH];

		mir_snprintf(setting, SIZEOF(setting), "rcMuc_%d_server", iRecent);
		if (server)
			ppro->setTString(setting, server);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, SIZEOF(setting), "rcMuc_%d_room", iRecent);
		if (room)
			ppro->setTString(setting, room);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, SIZEOF(setting), "rcMuc_%d_nick", iRecent);
		if (nick)
			ppro->setTString(setting, nick);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, SIZEOF(setting), "password_rcMuc_%d", iRecent);
		if (password)
			ppro->setTString(setting, password);
		else
			ppro->delSetting(setting);
	}

private:
	BOOL null_strequals(const TCHAR *str1, const TCHAR *str2)
	{
		if (!str1 && !str2) return TRUE;
		if (!str1 && str2 && !*str2) return TRUE;
		if (!str2 && str1 && !*str1) return TRUE;

		if (!str1 && str2) return FALSE;
		if (!str2 && str1) return FALSE;

		return !mir_tstrcmp(str1, str2);
	}
};

INT_PTR __cdecl CJabberProto::OnMenuHandleJoinGroupchat(WPARAM, LPARAM)
{
	GroupchatJoinRoomByJid(NULL, NULL);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	ptrT jid( getTStringA(hContact, "ChatRoomID"));
	if (jid == NULL)
		return 0;

	ptrT nick( getTStringA(hContact, "MyNick"));
	if (nick == NULL)
		if ((nick = getTStringA("Nick")) == NULL)
			return 0;

	ptrT password(getTStringA(hContact, "Password"));

	if (getWord(hContact, "Status", 0) != ID_STATUS_ONLINE) {
		TCHAR *p = _tcschr(jid, '@');
		if (p != NULL) {
			*p++ = 0;
			GroupchatJoinRoom(p, jid, nick, password);
		}
	}

	return 0;
}

INT_PTR __cdecl CJabberProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	ptrT jid(getTStringA(hContact, "ChatRoomID"));
	if (jid != NULL) {
		if (getWord(hContact, "Status", 0) != ID_STATUS_OFFLINE) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, jid);
			if (item != NULL)
				GcQuit(item, 200, NULL);
		}
	}
	return 0;
}

void CJabberProto::GroupchatJoinRoom(const TCHAR *server, const TCHAR *room, const TCHAR *nick, const TCHAR *password, bool autojoin)
{
	JabberGcRecentInfo info(this);

	bool found = false;
	for (int i = 0; i < 5; i++) {
		if (!info.loadRecent(i))
			continue;

		if (info.equals(room, server, nick, password)) {
			found = true;
			break;
		}
	}

	if (!found) {
		for (int i = 4; i--;) {
			if (info.loadRecent(i))
				info.saveRecent(i + 1);
		}

		info.fillData(room, server, nick, password);
		info.saveRecent(0);
	}

	TCHAR text[JABBER_MAX_JID_LEN + 1];
	mir_sntprintf(text, SIZEOF(text), _T("%s@%s/%s"), room, server, nick);

	JABBER_LIST_ITEM *item = ListAdd(LIST_CHATROOM, text);
	item->bAutoJoin = autojoin;
	replaceStrT(item->nick, nick);
	replaceStrT(item->password, info.password);

	int status = (m_iStatus == ID_STATUS_INVISIBLE) ? ID_STATUS_ONLINE : m_iStatus;
	XmlNode x(_T("x")); x << XATTR(_T("xmlns"), JABBER_FEAT_MUC);
	if (info.password && info.password[0])
		x << XCHILD(_T("password"), info.password);

	SendPresenceTo(status, text, x);
}

////////////////////////////////////////////////////////////////////////////////
// Join Dialog

static int sttTextLineHeight = 16;

struct RoomInfo
{
	enum Overlay { ROOM_WAIT, ROOM_FAIL, ROOM_BOOKMARK, ROOM_DEFAULT };
	Overlay	overlay;
	TCHAR	*line1, *line2;
};

static int sttRoomListAppend(HWND hwndList, RoomInfo::Overlay overlay, const TCHAR *line1, const TCHAR *line2, const TCHAR *name)
{
	RoomInfo *info = (RoomInfo *)mir_alloc(sizeof(RoomInfo));
	info->overlay = overlay;
	info->line1 = line1 ? mir_tstrdup(line1) : 0;
	info->line2 = line2 ? mir_tstrdup(line2) : 0;

	int id = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)name);
	SendMessage(hwndList, CB_SETITEMDATA, id, (LPARAM)info);
	SendMessage(hwndList, CB_SETITEMHEIGHT, id, sttTextLineHeight * 2);
	return id;
}

void CJabberProto::OnIqResultDiscovery(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!iqNode || !pInfo)
		return;

	HWND hwndList = (HWND)pInfo->GetUserData();
	SendMessage(hwndList, CB_SHOWDROPDOWN, FALSE, 0);
	SendMessage(hwndList, CB_RESETCONTENT, 0, 0);

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		HXML query = xmlGetChild(iqNode, "query");
		if (query == NULL) {
			sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
				TranslateT("Jabber Error"),
				TranslateT("Failed to retrieve room list from server."),
				_T(""));
		}
		else {
			bool found = false;
			HXML item;
			for (int i = 1; item = xmlGetNthChild(query, _T("item"), i); i++) {
				const TCHAR *jid = xmlGetAttrValue(item, _T("jid"));
				TCHAR *name = NEWTSTR_ALLOCA(jid);
				if (name) {
					if (TCHAR *p = _tcschr(name, _T('@')))
						*p = 0;
				}
				else name = _T("");

				sttRoomListAppend(hwndList,
					ListGetItemPtr(LIST_BOOKMARK, jid) ? RoomInfo::ROOM_BOOKMARK : RoomInfo::ROOM_DEFAULT,
					xmlGetAttrValue(item, _T("name")),
					jid, name);

				found = true;
			}

			if (!found) {
				sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
					TranslateT("Jabber Error"),
					TranslateT("No rooms available on server."),
					_T(""));
			}
		}
	}
	else if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR) {
		HXML errorNode = xmlGetChild(iqNode, "error");
		TCHAR *str = JabberErrorMsg(errorNode);
		sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
			TranslateT("Jabber Error"),
			str,
			_T(""));
		mir_free(str);
	}
	else 
		sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
			TranslateT("Jabber Error"),
			TranslateT("Room list request timed out."),
			_T(""));

	SendMessage(hwndList, CB_SHOWDROPDOWN, TRUE, 0);
}

static void sttJoinDlgShowRecentItems(HWND hwndDlg, int newCount)
{
	RECT rcTitle, rcLastItem;
	GetWindowRect(GetDlgItem(hwndDlg, IDC_TXT_RECENT), &rcTitle);
	GetWindowRect(GetDlgItem(hwndDlg, IDC_RECENT5), &rcLastItem);

	ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_RECENT), newCount ? SW_SHOW : SW_HIDE);

	int oldCount = 5;
	for (int idc = IDC_RECENT1; idc <= IDC_RECENT5; ++idc)
		ShowWindow(GetDlgItem(hwndDlg, idc), (idc - IDC_RECENT1 < newCount) ? SW_SHOW : SW_HIDE);

	int curRecentHeight = rcLastItem.bottom - rcTitle.top - (5 - oldCount) * (rcLastItem.bottom - rcLastItem.top);
	int newRecentHeight = rcLastItem.bottom - rcTitle.top - (5 - newCount) * (rcLastItem.bottom - rcLastItem.top);
	if (!newCount)
		newRecentHeight = 0;
	int offset = newRecentHeight - curRecentHeight;

	RECT rc;
	int ctrls[] = { IDC_BOOKMARKS, IDOK, IDCANCEL };
	for (int i = 0; i < SIZEOF(ctrls); i++) {
		GetWindowRect(GetDlgItem(hwndDlg, ctrls[i]), &rc);
		MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc, 2);
		SetWindowPos(GetDlgItem(hwndDlg, ctrls[i]), NULL, rc.left, rc.top + offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	GetWindowRect(hwndDlg, &rc);
	SetWindowPos(hwndDlg, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top + offset, SWP_NOMOVE | SWP_NOZORDER);
}

class CJabberDlgGcJoin: public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

public:
	CJabberDlgGcJoin(CJabberProto *proto, TCHAR *jid);
	~CJabberDlgGcJoin();

protected:
	TCHAR *m_jid;

	void OnInitDialog();
	void OnClose();
	void OnDestroy();
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

CJabberDlgGcJoin::CJabberDlgGcJoin(CJabberProto *proto, TCHAR *jid) :
	CSuper(proto, IDD_GROUPCHAT_JOIN, NULL),
	m_jid(mir_tstrdup(jid))
{
	m_autoClose = 0;
}

CJabberDlgGcJoin::~CJabberDlgGcJoin()
{
	mir_free(m_jid);
}

void CJabberDlgGcJoin::OnInitDialog()
{
	CSuper::OnInitDialog();

	WindowSetIcon(m_hwnd, m_proto, "group");

	JabberGcRecentInfo *info = NULL;
	if (m_jid)
		info = new JabberGcRecentInfo(m_proto, m_jid);
	else if(OpenClipboard(m_hwnd)) {
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);

		if (hData) {
			TCHAR *buf = (TCHAR *)GlobalLock(hData);
			if (buf && _tcschr(buf, _T('@')) && !_tcschr(buf, _T(' ')))
				info = new JabberGcRecentInfo(m_proto, buf);
			GlobalUnlock(hData);
		}
		CloseClipboard();
	}

	if (info) {
		info->fillForm(m_hwnd);
		delete info;
	}

	ptrT tszNick(m_proto->getTStringA("Nick"));
	if (tszNick == NULL)
		tszNick = JabberNickFromJID(m_proto->m_szJabberJID);
	SetDlgItemText(m_hwnd, IDC_NICK, tszNick);

	TEXTMETRIC tm = { 0 };
	HDC hdc = GetDC(m_hwnd);
	GetTextMetrics(hdc, &tm);
	ReleaseDC(m_hwnd, hdc);
	sttTextLineHeight = tm.tmHeight;
	SendDlgItemMessage(m_hwnd, IDC_ROOM, CB_SETITEMHEIGHT, -1, sttTextLineHeight - 1);

	LOGFONT lf = { 0 };
	HFONT hfnt = (HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_GETFONT, 0, 0);
	GetObject(hfnt, sizeof(lf), &lf);
	lf.lfWeight = FW_BOLD;
	SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);

	SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_proto->LoadIconEx("bookmarks"));
	SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BUTTONADDTOOLTIP, (WPARAM)"Bookmarks", 0);
	SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BUTTONSETASPUSHBTN, TRUE, 0);

	m_proto->ComboLoadRecentStrings(m_hwnd, IDC_SERVER, "joinWnd_rcSvr");

	int i;
	for (i = 0; i < 5; i++) {
		TCHAR jid[JABBER_MAX_JID_LEN];
		JabberGcRecentInfo info(m_proto);
		if (!info.loadRecent(i))
			break;

		mir_sntprintf(jid, SIZEOF(jid), _T("%s@%s (%s)"),
			info.room, info.server,
			info.nick ? info.nick : TranslateT("<no nick>"));
		SetDlgItemText(m_hwnd, IDC_RECENT1 + i, jid);
	}
	sttJoinDlgShowRecentItems(m_hwnd, i);
}

void CJabberDlgGcJoin::OnClose()
{
	CSuper::OnClose();
}

void CJabberDlgGcJoin::OnDestroy()
{
	g_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BM_SETIMAGE, IMAGE_ICON, 0));
	m_proto->m_pDlgJabberJoinGroupchat = NULL;
	DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_GETFONT, 0, 0));

	CSuper::OnDestroy();

	mir_free(m_jid); m_jid = NULL;
}

INT_PTR CJabberDlgGcJoin::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR text[128];

	switch (msg) {
	case WM_DELETEITEM:
		{
			LPDELETEITEMSTRUCT lpdis = (LPDELETEITEMSTRUCT)lParam;
			if (lpdis->CtlID != IDC_ROOM)
				break;

			RoomInfo *info = (RoomInfo *)lpdis->itemData;
			mir_free(info->line1);
			mir_free(info->line2);
			mir_free(info);
		}
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
			if (lpmis->CtlID != IDC_ROOM)
				break;

			lpmis->itemHeight = 2*sttTextLineHeight;
			if (lpmis->itemID == -1)
				lpmis->itemHeight = sttTextLineHeight-1;

		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlID != IDC_ROOM)
				break;

			RoomInfo *info = (RoomInfo *)SendDlgItemMessage(m_hwnd, IDC_ROOM, CB_GETITEMDATA, lpdis->itemID, 0);
			COLORREF clLine1, clBack;

			if (lpdis->itemState & ODS_SELECTED) {
				FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
				clBack = GetSysColor(COLOR_HIGHLIGHT);
				clLine1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
			}
			else {
				FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_WINDOW));
				clBack = GetSysColor(COLOR_WINDOW);
				clLine1 = GetSysColor(COLOR_WINDOWTEXT);
			}
			COLORREF clLine2 = RGB(
					GetRValue(clLine1) * 0.66 + GetRValue(clBack) * 0.34,
					GetGValue(clLine1) * 0.66 + GetGValue(clBack) * 0.34,
					GetBValue(clLine1) * 0.66 + GetBValue(clBack) * 0.34);

			SetBkMode(lpdis->hDC, TRANSPARENT);

			RECT rc = lpdis->rcItem;
			rc.bottom -= (rc.bottom - rc.top) / 2;
			rc.left += 20;
			SetTextColor(lpdis->hDC, clLine1);
			DrawText(lpdis->hDC, info->line1, -1, &rc, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER|DT_WORD_ELLIPSIS);

			rc = lpdis->rcItem;
			rc.top += (rc.bottom - rc.top) / 2;
			rc.left += 20;
			SetTextColor(lpdis->hDC, clLine2);
			DrawText(lpdis->hDC, info->line2, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);

			DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("group"), 16, 16, 0, NULL, DI_NORMAL);
			switch (info->overlay) {
			case RoomInfo::ROOM_WAIT:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("disco_progress"), 16, 16, 0, NULL, DI_NORMAL);
				break;
			case RoomInfo::ROOM_FAIL:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("disco_fail"), 16, 16, 0, NULL, DI_NORMAL);
				break;
			case RoomInfo::ROOM_BOOKMARK:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("disco_ok"), 16, 16, 0, NULL, DI_NORMAL);
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SERVER:
			switch (HIWORD(wParam)) {
			case CBN_EDITCHANGE:
			case CBN_SELCHANGE:
				{
					int iqid = GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_ROOM), GWLP_USERDATA);
					if (iqid) {
						m_proto->m_iqManager.ExpireIq(iqid);
						SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_ROOM), GWLP_USERDATA, 0);
					}
					SendDlgItemMessage(m_hwnd, IDC_ROOM, CB_RESETCONTENT, 0, 0);
				}
				break;
			}
			break;

		case IDC_ROOM:
			switch (HIWORD(wParam)) {
			case CBN_DROPDOWN:
				if (!SendDlgItemMessage(m_hwnd, IDC_ROOM, CB_GETCOUNT, 0, 0)) {
					int iqid = GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_ROOM), GWLP_USERDATA);
					if (iqid) {
						m_proto->m_iqManager.ExpireIq(iqid);
						SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_ROOM), GWLP_USERDATA, 0);
					}

					SendDlgItemMessage(m_hwnd, IDC_ROOM, CB_RESETCONTENT, 0, 0);

					int len = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_SERVER)) + 1;
					TCHAR *server = (TCHAR*)_alloca(len * sizeof(TCHAR));
					GetDlgItemText(m_hwnd, IDC_SERVER, server, len);

					if (*server) {
						sttRoomListAppend(GetDlgItem(m_hwnd, IDC_ROOM), RoomInfo::ROOM_WAIT, TranslateT("Loading..."), TranslateT("Please wait for room list to download."), _T(""));

						CJabberIqInfo *pInfo = m_proto->AddIQ(&CJabberProto::OnIqResultDiscovery, JABBER_IQ_TYPE_GET, server, 0, -1, (void*)GetDlgItem(m_hwnd, IDC_ROOM));
						pInfo->SetTimeout(30000);
						XmlNodeIq iq(pInfo);
						iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
						m_proto->m_ThreadInfo->send(iq);

						SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_ROOM), GWLP_USERDATA, pInfo->GetIqId());
					}
					else
						sttRoomListAppend(GetDlgItem(m_hwnd, IDC_ROOM), RoomInfo::ROOM_FAIL,
							TranslateT("Jabber Error"),
							TranslateT("Please specify group chat directory first."),
							_T(""));
				}
				break;
			}
			break;

		case IDC_BOOKMARKS:
			{
				HMENU hMenu = CreatePopupMenu();

				LISTFOREACH(i, m_proto, LIST_BOOKMARK)
				{
					JABBER_LIST_ITEM *item = 0;
					if (item = m_proto->ListGetItemPtrFromIndex(i))
						if (!mir_tstrcmp(item->type, _T("conference")))
							AppendMenu(hMenu, MF_STRING, (UINT_PTR)item, item->name);
				}
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)-1, TranslateT("Bookmarks..."));
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));

				RECT rc; GetWindowRect(GetDlgItem(m_hwnd, IDC_BOOKMARKS), &rc);
				CheckDlgButton(m_hwnd, IDC_BOOKMARKS, BST_CHECKED);
				int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, NULL);
				CheckDlgButton(m_hwnd, IDC_BOOKMARKS, BST_UNCHECKED);
				DestroyMenu(hMenu);

				if (res == -1)
					m_proto->OnMenuHandleBookmarks(0, 0);
				else if (res) {
					JABBER_LIST_ITEM *item = (JABBER_LIST_ITEM *)res;
					TCHAR *room = NEWTSTR_ALLOCA(item->jid);
					if (room) {
						TCHAR *server = _tcschr(room, _T('@'));
						if (server) {
							*server++ = 0;

							SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_SERVER, CBN_EDITCHANGE), (LPARAM)GetDlgItem(m_hwnd, IDC_SERVER));

							SetDlgItemText(m_hwnd, IDC_SERVER, server);
							SetDlgItemText(m_hwnd, IDC_ROOM, room);
							SetDlgItemText(m_hwnd, IDC_NICK, item->nick);
							SetDlgItemText(m_hwnd, IDC_PASSWORD, item->password);
			}	}	}	}
			break;

		case IDC_RECENT1:
		case IDC_RECENT2:
		case IDC_RECENT3:
		case IDC_RECENT4:
		case IDC_RECENT5:
			{
				JabberGcRecentInfo info(m_proto, LOWORD(wParam) - IDC_RECENT1);
				info.fillForm(m_hwnd);
				if (GetAsyncKeyState(VK_CONTROL))
					break;
			}

			// fall through
		case IDOK:
			{
				GetDlgItemText(m_hwnd, IDC_SERVER, text, SIZEOF(text));
				TCHAR *server = NEWTSTR_ALLOCA(text), *room;

				m_proto->ComboAddRecentString(m_hwnd, IDC_SERVER, "joinWnd_rcSvr", server);

				GetDlgItemText(m_hwnd, IDC_ROOM, text, SIZEOF(text));
				room = NEWTSTR_ALLOCA(text);

				GetDlgItemText(m_hwnd, IDC_NICK, text, SIZEOF(text));
				TCHAR *nick = NEWTSTR_ALLOCA(text);

				GetDlgItemText(m_hwnd, IDC_PASSWORD, text, SIZEOF(text));
				TCHAR *password = NEWTSTR_ALLOCA(text);
				m_proto->GroupchatJoinRoom(server, room, nick, password);
			}
			// fall through
		case IDCANCEL:
			Close();
			break;
		}
		break;

	case WM_JABBER_CHECK_ONLINE:
		if (!m_proto->m_bJabberOnline)
			EndDialog(m_hwnd, 0);
		break;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

void CJabberProto::GroupchatJoinRoomByJid(HWND, TCHAR *jid)
{
	if (m_pDlgJabberJoinGroupchat)
		SetForegroundWindow(m_pDlgJabberJoinGroupchat->GetHwnd());
	else {
		m_pDlgJabberJoinGroupchat = new CJabberDlgGcJoin(this, jid);
		m_pDlgJabberJoinGroupchat->Show();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGroupchatProcessPresence - handles the group chat presence packet

struct JabberGroupchatChangeNicknameParam
{
	JabberGroupchatChangeNicknameParam(CJabberProto* ppro_, const TCHAR *jid_) :
		ppro(ppro_),
		jid(mir_tstrdup(jid_))
		{}

	~JabberGroupchatChangeNicknameParam()
	{	mir_free(jid);
	}

	CJabberProto *ppro;
	TCHAR *jid;
};

static VOID CALLBACK JabberGroupchatChangeNickname(void* arg)
{
	JabberGroupchatChangeNicknameParam *param = (JabberGroupchatChangeNicknameParam*)arg;
	if (param == NULL)
		return;

	JABBER_LIST_ITEM *item = param->ppro->ListGetItemPtr(LIST_CHATROOM, param->jid);
	if (item != NULL) {
		CMString szBuffer, szTitle;
		szTitle.Format(TranslateT("Change nickname in <%s>"), item->name ? item->name : item->jid);
		if (item->nick)
			szBuffer = item->nick;

		if (param->ppro->EnterString(szBuffer, szTitle, ESF_COMBO, "gcNick_")) {
			replaceStrT(item->nick, szBuffer);
			param->ppro->SendPresenceTo(param->ppro->m_iStatus, CMString(FORMAT, _T("%s/%s"), item->jid, szBuffer), NULL);
		}
	}

	delete param;
}

static int sttGetStatusCode(HXML node)
{
	HXML statusNode = xmlGetChild(node, "status");
	if (statusNode == NULL)
		return -1;

	const TCHAR *statusCode = xmlGetAttrValue(statusNode, _T("code"));
	if (statusCode == NULL)
		return -1;

	return _ttol(statusCode);
}

void CJabberProto::RenameParticipantNick(JABBER_LIST_ITEM *item, const TCHAR *oldNick, HXML itemNode)
{
	const TCHAR *jid = xmlGetAttrValue(itemNode, _T("jid"));
	const TCHAR *newNick = xmlGetAttrValue(itemNode, _T("nick"));
	if (newNick == NULL)
		return;

	pResourceStatus r(item->findResource(oldNick));
	if (r == NULL)
		return;

	r->m_tszResourceName = mir_tstrdup(newNick);

	if (!mir_tstrcmp(item->nick, oldNick)) {
		replaceStrT(item->nick, newNick);

		MCONTACT hContact = HContactFromJID(item->jid);
		if (hContact != NULL)
			setTString(hContact, "MyNick", newNick);
	}

	GCDEST gcd = { m_szModuleName, item->jid, GC_EVENT_CHUID };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszNick = oldNick;
	gce.ptszText = newNick;
	if (jid != NULL)
		gce.ptszUserInfo = jid;
	gce.time = time(0);
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

	gcd.iType = GC_EVENT_NICK;
	gce.ptszNick = oldNick;
	gce.ptszUID = newNick;
	gce.ptszText = newNick;
	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
}

void CJabberProto::GroupchatProcessPresence(HXML node)
{
	const TCHAR *from;

	if (!node || !xmlGetName(node) || mir_tstrcmp(xmlGetName(node), _T("presence"))) return;
	if ((from = xmlGetAttrValue(node, _T("from"))) == NULL) return;

	const TCHAR *resource = _tcschr(from, '/');
	if (resource == NULL || *++resource == '\0')
		return;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, from);
	if (item == NULL)
		return;

	pResourceStatus r(item->findResource(resource));

	HXML nNode = xmlGetChildByTag(node, "nick", "xmlns", JABBER_FEAT_NICK);
	const TCHAR *cnick = xmlGetText(nNode);
	const TCHAR *nick = cnick ? cnick : (r && r->m_tszNick ? r->m_tszNick : resource);

	// process custom nick change
	if (cnick && r && r->m_tszNick && mir_tstrcmp(cnick, r->m_tszNick))
		r->m_tszNick = mir_tstrdup(cnick);

	HXML xNode = xmlGetChildByTag(node, "x", "xmlns", JABBER_FEAT_MUC_USER);
	HXML itemNode = xmlGetChild(xNode, "item");

	const TCHAR *type = xmlGetAttrValue(node, _T("type"));

	// entering room or a usual room presence
	if (type == NULL || !mir_tstrcmp(type, _T("available"))) {
		if (ptrT(JabberNickFromJID(from)) == NULL)
			return;

		GcLogCreate(item);
		item->iChatState = 0;

		// Update status of room participant
		int status = ID_STATUS_ONLINE;
		LPCTSTR ptszShow = xmlGetText(xmlGetChild(node, "show"));
		if (ptszShow) {
			if (!mir_tstrcmp(ptszShow, _T("away"))) status = ID_STATUS_AWAY;
			else if (!mir_tstrcmp(ptszShow, _T("xa"))) status = ID_STATUS_NA;
			else if (!mir_tstrcmp(ptszShow, _T("dnd"))) status = ID_STATUS_DND;
			else if (!mir_tstrcmp(ptszShow, _T("chat"))) status = ID_STATUS_FREECHAT;
		}

		LPCTSTR str = xmlGetText(xmlGetChild(node, "status"));

		char priority = 0;
		if (LPCTSTR ptszPriority = xmlGetText(xmlGetChild(node, "priority")))
			priority = (char)_ttoi(ptszPriority);

		bool bStatusChanged = false, bRoomCreated = false, bAffiliationChanged = false, bRoleChanged = false;
		int  newRes = ListAddResource(LIST_CHATROOM, from, status, str, priority, cnick) ? GC_EVENT_JOIN : 0;

		if (pResourceStatus oldRes = ListFindResource(LIST_CHATROOM, from))
		if ((oldRes->m_iStatus != status) || lstrcmp_null(oldRes->m_tszStatusMessage, str))
			bStatusChanged = true;

		// Check additional MUC info for this user
		if (itemNode != NULL) {
			if (r == NULL)
				r = item->findResource(resource);
			if (r != NULL) {
				JABBER_GC_AFFILIATION affiliation = r->m_affiliation;
				JABBER_GC_ROLE role = r->m_role;

				if ((str = xmlGetAttrValue(itemNode, _T("affiliation"))) != NULL) {
					if (!mir_tstrcmp(str, _T("owner")))       affiliation = AFFILIATION_OWNER;
					else if (!mir_tstrcmp(str, _T("admin")))       affiliation = AFFILIATION_ADMIN;
					else if (!mir_tstrcmp(str, _T("member")))      affiliation = AFFILIATION_MEMBER;
					else if (!mir_tstrcmp(str, _T("none")))	     affiliation = AFFILIATION_NONE;
					else if (!mir_tstrcmp(str, _T("outcast")))     affiliation = AFFILIATION_OUTCAST;
				}
				if ((str = xmlGetAttrValue(itemNode, _T("role"))) != NULL) {
					if (!mir_tstrcmp(str, _T("moderator")))   role = ROLE_MODERATOR;
					else if (!mir_tstrcmp(str, _T("participant"))) role = ROLE_PARTICIPANT;
					else if (!mir_tstrcmp(str, _T("visitor")))     role = ROLE_VISITOR;
					else                                        role = ROLE_NONE;
				}

				if ((role != ROLE_NONE) && (JabberGcGetStatus(r) != JabberGcGetStatus(affiliation, role))) {
					GcLogUpdateMemberStatus(item, resource, nick, NULL, GC_EVENT_REMOVESTATUS, NULL);
					if (!newRes) newRes = GC_EVENT_ADDSTATUS;
				}

				if (affiliation != r->m_affiliation) {
					r->m_affiliation = affiliation;
					bAffiliationChanged = true;
				}

				if (role != r->m_role) {
					r->m_role = role;
					if (r->m_role != ROLE_NONE)
						bRoleChanged = true;
				}

				if (str = xmlGetAttrValue(itemNode, _T("jid")))
					r->m_tszRealJid = mir_tstrdup(str);
			}
		}

		if (sttGetStatusCode(xNode) == 201)
			bRoomCreated = true;

		// show status change if needed
		if (bStatusChanged)
		if (pResourceStatus res = ListFindResource(LIST_CHATROOM, from))
			GcLogShowInformation(item, res, INFO_STATUS);

		// Update groupchat log window
		GcLogUpdateMemberStatus(item, resource, nick, str, newRes, NULL);
		if (r && bAffiliationChanged) GcLogShowInformation(item, r, INFO_AFFILIATION);
		if (r && bRoleChanged) GcLogShowInformation(item, r, INFO_ROLE);

		// update clist status
		MCONTACT hContact = HContactFromJID(from);
		if (hContact != NULL)
			setWord(hContact, "Status", status);

		// Check <created/>
		if (bRoomCreated) {
			HXML n = xmlGetChild(node, "created");
			if (n != NULL && (str = xmlGetAttrValue(n, _T("xmlns"))) != NULL && !mir_tstrcmp(str, JABBER_FEAT_MUC_OWNER))
				// A new room just created by me
				// Request room config
				m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, item->jid))
					<< XQUERY(JABBER_FEAT_MUC_OWNER));
		}
	}

	// leaving room
	else if (!mir_tstrcmp(type, _T("unavailable"))) {
		const TCHAR *str = 0;
		if (xNode != NULL && item->nick != NULL) {
			HXML reasonNode = xmlGetChild(itemNode, "reason");
			str = xmlGetAttrValue(itemNode, _T("jid"));

			int iStatus = sttGetStatusCode(xNode);
			if (iStatus == 301 && r != NULL)
				GcLogShowInformation(item, r, INFO_BAN);

			if (!mir_tstrcmp(resource, item->nick)) {
				switch (iStatus) {
				case 301:
				case 307:
					GcQuit(item, iStatus, reasonNode);
					return;

				case 303:
					RenameParticipantNick(item, resource, itemNode);
					return;
				}
			}
			else {
				switch (iStatus) {
				case 303:
					RenameParticipantNick(item, resource, itemNode);
					return;

				case 301:
				case 307:
				case 322:
					ListRemoveResource(LIST_CHATROOM, from);
					GcLogUpdateMemberStatus(item, resource, nick, str, GC_EVENT_KICK, reasonNode, iStatus);
					return;
				}
			}
		}

		HXML statusNode = xmlGetChild(node, "status");
		GcLogUpdateMemberStatus(item, resource, nick, str, GC_EVENT_PART, statusNode);
		ListRemoveResource(LIST_CHATROOM, from);

		MCONTACT hContact = HContactFromJID(from);
		if (hContact != NULL)
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
	}

	// processing room errors
	else if (!mir_tstrcmp(type, _T("error"))) {
		int errorCode = 0;
		HXML errorNode = xmlGetChild(node, "error");
		ptrT str(JabberErrorMsg(errorNode, &errorCode));

		if (errorCode == JABBER_ERROR_CONFLICT) {
			ptrT newNick(getTStringA("GcAltNick"));
			if (++item->iChatState == 1 && newNick != NULL && newNick[0] != 0) {
				replaceStrT(item->nick, newNick);
				TCHAR text[1024] = { 0 };
				mir_sntprintf(text, SIZEOF(text), _T("%s/%s"), item->jid, newNick);
				SendPresenceTo(m_iStatus, text, NULL);
			}
			else {
				CallFunctionAsync(JabberGroupchatChangeNickname, new JabberGroupchatChangeNicknameParam(this, from));
				item->iChatState = 0;
			}
			return;
		}

		MsgPopup(NULL, str, TranslateT("Jabber Error"));

		if (item != NULL && !item->bChatActive)
			ListRemove(LIST_CHATROOM, from);
	}
}

void CJabberProto::GroupchatProcessMessage(HXML node)
{
	HXML n, m;
	const TCHAR *from, *type, *p, *nick, *resource;
	JABBER_LIST_ITEM *item;

	if (!xmlGetName(node) || mir_tstrcmp(xmlGetName(node), _T("message"))) return;
	if ((from = xmlGetAttrValue(node, _T("from"))) == NULL) return;
	if ((item = ListGetItemPtr(LIST_CHATROOM, from)) == NULL) return;

	if ((type = xmlGetAttrValue(node, _T("type"))) == NULL) return;
	if (!mir_tstrcmp(type, _T("error")))
		return;

	GCDEST gcd = { m_szModuleName, item->jid, 0 };

	const TCHAR *msgText = NULL;

	resource = _tcschr(from, '/');
	if (resource != NULL && *++resource == '\0')
		resource = NULL;

	if ((n = xmlGetChild(node, "subject")) != NULL) {
		msgText = xmlGetText(n);
		if (msgText == NULL || msgText[0] == '\0')
			return;

		gcd.iType = GC_EVENT_TOPIC;

		if (resource == NULL && (m = xmlGetChild(node, "body")) != NULL) {
			const TCHAR *tmpnick = xmlGetText(m);
			if (tmpnick == NULL || *tmpnick == 0)
				return;

			const TCHAR *tmptr = _tcsstr(tmpnick, _T("has set the subject to:")); //ejabberd
			if (tmptr == NULL)
				tmptr = _tcsstr(tmpnick, TranslateT("has set the subject to:")); //ejabberd
			if (tmptr != NULL && *tmptr != 0) {
				*(TCHAR*)(--tmptr) = 0;
				resource = tmpnick;
			}
		}
		item->getTemp()->m_tszStatusMessage = mir_tstrdup(msgText);
	}
	else {
		if ((n = xmlGetChildByTag(node, "body", "xml:lang", m_tszSelectedLang)) == NULL)
			if ((n = xmlGetChild(node, "body")) == NULL)
				return;

		msgText = xmlGetText(n);
		if (msgText == NULL)
			return;

		if (resource == NULL)
			gcd.iType = GC_EVENT_INFORMATION;
		else if (_tcsncmp(msgText, _T("/me "), 4) == 0 && mir_tstrlen(msgText) > 4) {
			msgText += 4;
			gcd.iType = GC_EVENT_ACTION;
		}
		else gcd.iType = GC_EVENT_MESSAGE;
	}

	GcLogCreate(item);

	time_t msgTime = 0;
	if (!JabberReadXep203delay(node, msgTime)) {
		HXML xDelay = xmlGetChildByTag(node, "x", "xmlns", _T("jabber:x:delay"));
		if (xDelay && (p = xmlGetAttrValue(xDelay, _T("stamp"))) != NULL)
			msgTime = JabberIsoToUnixTime(p);
	}

	bool isHistory = msgTime != 0;
	time_t now = time(NULL);
	if (!msgTime || msgTime > now)
		msgTime = now;

	if (resource != NULL) {
		pResourceStatus r(item->findResource(resource));
		nick = (r && r->m_tszNick) ? r->m_tszNick : resource;
	}
	else nick = NULL;

	CMString tszText(msgText);
	tszText.Replace(_T("%"), _T("%%"));

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszUID = resource;
	gce.ptszNick = nick;
	gce.time = msgTime;
	gce.ptszText = tszText;
	gce.bIsMe = nick == NULL ? FALSE : (mir_tstrcmp(resource, item->nick) == 0);

	if (!isHistory)
		gce.dwFlags |= GCEF_ADDTOLOG;

	if (m_options.GcLogChatHistory && isHistory)
		gce.dwFlags |= GCEF_NOTNOTIFY;

	CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);

	item->bChatActive = 2;

	if (gcd.iType == GC_EVENT_TOPIC) {
		gce.dwFlags &= ~GCEF_ADDTOLOG;
		gcd.iType = GC_EVENT_SETSBTEXT;
		CallServiceSync(MS_GC_EVENT, NULL, (LPARAM)&gce);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Accepting groupchat invitations

class CGroupchatInviteAcceptDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
	CCtrlButton m_accept;
	JABBER_GROUPCHAT_INVITE_INFO* m_info;

public:
	CGroupchatInviteAcceptDlg(CJabberProto *ppro, JABBER_GROUPCHAT_INVITE_INFO* pInfo) :
		CSuper(ppro, IDD_GROUPCHAT_INVITE_ACCEPT, NULL),
		m_info(pInfo),
		m_accept(this, IDC_ACCEPT)
	{
		m_accept.OnClick = Callback(this, &CGroupchatInviteAcceptDlg::OnCommand_Accept);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		TCHAR buf[256];
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Group chat invitation to\n%s"), m_info->roomJid);
		SetDlgItemText(m_hwnd, IDC_HEADERBAR, buf);

		SetDlgItemText(m_hwnd, IDC_FROM, m_info->from);

		if (m_info->reason != NULL)
			SetDlgItemText(m_hwnd, IDC_REASON, m_info->reason);

		SetDlgItemText(m_hwnd, IDC_NICK, ptrT(JabberNickFromJID(m_proto->m_szJabberJID)));

		WindowSetIcon(m_hwnd, m_proto, "group");

		SetFocus(GetDlgItem(m_hwnd, IDC_NICK));
	}

	void OnCommand_Accept(CCtrlButton*)
	{
		TCHAR text[128];
		GetDlgItemText(m_hwnd, IDC_NICK, text, SIZEOF(text));
		m_proto->AcceptGroupchatInvite(m_info->roomJid, text, m_info->password);
		EndDialog(m_hwnd, 0);
	}
};

void __cdecl CJabberProto::GroupchatInviteAcceptThread(JABBER_GROUPCHAT_INVITE_INFO *inviteInfo)
{
	CGroupchatInviteAcceptDlg(this, inviteInfo).DoModal();

	mir_free(inviteInfo->roomJid);
	mir_free(inviteInfo->from);
	mir_free(inviteInfo->reason);
	mir_free(inviteInfo->password);
	mir_free(inviteInfo);
}

void CJabberProto::GroupchatProcessInvite(const TCHAR *roomJid, const TCHAR *from, const TCHAR *reason, const TCHAR *password)
{
	if (roomJid == NULL)
		return;

	if (ListGetItemPtr(LIST_CHATROOM, roomJid))
		return;

	if (m_options.AutoAcceptMUC == FALSE) {
		JABBER_GROUPCHAT_INVITE_INFO* inviteInfo = (JABBER_GROUPCHAT_INVITE_INFO *)mir_alloc(sizeof(JABBER_GROUPCHAT_INVITE_INFO));
		inviteInfo->roomJid = mir_tstrdup(roomJid);
		inviteInfo->from = mir_tstrdup(from);
		inviteInfo->reason = mir_tstrdup(reason);
		inviteInfo->password = mir_tstrdup(password);
		ForkThread((MyThreadFunc)&CJabberProto::GroupchatInviteAcceptThread, inviteInfo);
	}
	else AcceptGroupchatInvite(roomJid, ptrT(JabberNickFromJID(m_szJabberJID)), password);
}

void CJabberProto::AcceptGroupchatInvite(const TCHAR *roomJid, const TCHAR *reason, const TCHAR *password)
{
	TCHAR room[256], *server, *p;
	_tcsncpy_s(room, roomJid, _TRUNCATE);
	p = _tcstok(room, _T("@"));
	server = _tcstok(NULL, _T("@"));
	GroupchatJoinRoom(server, p, reason, password);
}
