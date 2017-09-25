/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (ñ) 2012-17 Miranda NG project

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
	ptrW m_room, m_server, m_nick, m_password;
	CJabberProto *ppro;

	JabberGcRecentInfo(CJabberProto *proto)
	{
		ppro = proto;
	}
	JabberGcRecentInfo(CJabberProto *proto, const wchar_t *room, const wchar_t *server, const wchar_t *nick = nullptr, const wchar_t *password = nullptr)
	{
		ppro = proto;
		fillData(room, server, nick, password);
	}
	JabberGcRecentInfo(CJabberProto *proto, const wchar_t *jid)
	{
		ppro = proto;
		fillData(jid);
	}
	JabberGcRecentInfo(CJabberProto *proto, int iRecent)
	{
		ppro = proto;
		loadRecent(iRecent);
	}

	~JabberGcRecentInfo()
	{
	}

	void cleanup()
	{
		m_room = m_server = m_nick = m_password = nullptr;
	}

	BOOL equals(const wchar_t *room, const wchar_t *server, const wchar_t *nick = nullptr, const wchar_t *password = nullptr)
	{
		return
			null_strequals(m_room, room) &&
			null_strequals(m_server, server) &&
			null_strequals(m_nick, nick) &&
			null_strequals(m_password, password);
	}

	BOOL equalsnp(const wchar_t *room, const wchar_t *server, const wchar_t *nick = nullptr)
	{
		return
			null_strequals(m_room, room) &&
			null_strequals(m_server, server) &&
			null_strequals(m_nick, nick);
	}

	void fillForm(HWND hwndDlg)
	{
		SetDlgItemText(hwndDlg, IDC_SERVER, m_server ? m_server : L"");
		SetDlgItemText(hwndDlg, IDC_ROOM, m_room ? m_room : L"");
		SetDlgItemText(hwndDlg, IDC_NICK, m_nick ? m_nick : L"");
		SetDlgItemText(hwndDlg, IDC_PASSWORD, m_password ? m_password : L"");
	}

	void fillData(const wchar_t *room, const wchar_t *server, const wchar_t *nick = nullptr, const wchar_t *password = nullptr)
	{
		m_room = mir_wstrdup(room);
		m_server = mir_wstrdup(server);
		m_nick = mir_wstrdup(nick);
		m_password = mir_wstrdup(password);
	}

	void fillData(const wchar_t *jid)
	{
		wchar_t *room, *server, *nick = nullptr;
		room = NEWWSTR_ALLOCA(jid);
		server = wcschr(room, '@');
		if (server) {
			*server++ = 0;
			nick = wcschr(server, '/');
			if (nick) *nick++ = 0;
		}
		else {
			server = room;
			room = nullptr;
		}

		fillData(room, server, nick);
	}

	BOOL loadRecent(int iRecent)
	{
		char setting[MAXMODULELABELLENGTH];
		mir_snprintf(setting, "rcMuc_%d_server", iRecent);
		m_server = ppro->getWStringA(setting);

		mir_snprintf(setting, "rcMuc_%d_room", iRecent);
		m_room = ppro->getWStringA(setting);

		mir_snprintf(setting, "rcMuc_%d_nick", iRecent);
		m_nick = ppro->getWStringA(setting);

		mir_snprintf(setting, "password_rcMuc_%d", iRecent);
		m_password = ppro->getWStringA(0, setting);

		return m_room || m_server || m_nick || m_password;
	}

	void saveRecent(int iRecent)
	{
		char setting[MAXMODULELABELLENGTH];

		mir_snprintf(setting, "rcMuc_%d_server", iRecent);
		if (m_server)
			ppro->setWString(setting, m_server);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, "rcMuc_%d_room", iRecent);
		if (m_room)
			ppro->setWString(setting, m_room);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, "rcMuc_%d_nick", iRecent);
		if (m_nick)
			ppro->setWString(setting, m_nick);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, "password_rcMuc_%d", iRecent);
		if (m_password)
			ppro->setWString(setting, m_password);
		else
			ppro->delSetting(setting);
	}

private:
	BOOL null_strequals(const wchar_t *str1, const wchar_t *str2)
	{
		if (!str1 && !str2) return TRUE;
		if (!str1 && str2 && !*str2) return TRUE;
		if (!str2 && str1 && !*str1) return TRUE;

		if (!str1 && str2) return FALSE;
		if (!str2 && str1) return FALSE;

		return !mir_wstrcmp(str1, str2);
	}
};

INT_PTR __cdecl CJabberProto::OnMenuHandleJoinGroupchat(WPARAM, LPARAM)
{
	GroupchatJoinRoomByJid(nullptr, nullptr);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	ptrW jid(getWStringA(hContact, "ChatRoomID"));
	if (jid == nullptr)
		return 0;

	ptrW nick(getWStringA(hContact, "MyNick"));
	if (nick == nullptr)
		if ((nick = getWStringA("Nick")) == nullptr)
			return 0;

	ptrW password(getWStringA(hContact, "Password"));

	if (getWord(hContact, "Status", 0) != ID_STATUS_ONLINE) {
		wchar_t *p = wcschr(jid, '@');
		if (p != nullptr) {
			*p++ = 0;
			GroupchatJoinRoom(p, jid, nick, password);
		}
	}

	return 0;
}

INT_PTR __cdecl CJabberProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	ptrW jid(getWStringA(hContact, "ChatRoomID"));
	if (jid != nullptr) {
		if (getWord(hContact, "Status", 0) != ID_STATUS_OFFLINE) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, jid);
			if (item != nullptr)
				GcQuit(item, 200, nullptr);
		}
	}
	return 0;
}

void CJabberProto::GroupchatJoinRoom(const wchar_t *server, const wchar_t *room, const wchar_t *nick, const wchar_t *password, bool autojoin)
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

	wchar_t text[JABBER_MAX_JID_LEN + 1];
	mir_snwprintf(text, L"%s@%s/%s", room, server, nick);

	JABBER_LIST_ITEM *item = ListAdd(LIST_CHATROOM, text);
	item->bAutoJoin = autojoin;
	replaceStrW(item->nick, nick);
	replaceStrW(item->password, info.m_password);

	int status = (m_iStatus == ID_STATUS_INVISIBLE) ? ID_STATUS_ONLINE : m_iStatus;
	XmlNode x(L"x"); x << XATTR(L"xmlns", JABBER_FEAT_MUC);
	if (info.m_password && info.m_password[0])
		x << XCHILD(L"password", info.m_password);

	SendPresenceTo(status, text, x);
}

////////////////////////////////////////////////////////////////////////////////
// Join Dialog

static int sttTextLineHeight = 16;

struct RoomInfo
{
	enum Overlay { ROOM_WAIT, ROOM_FAIL, ROOM_BOOKMARK, ROOM_DEFAULT };
	Overlay	overlay;
	wchar_t	*line1, *line2;
};

static int sttRoomListAppend(HWND hwndList, RoomInfo::Overlay overlay, const wchar_t *line1, const wchar_t *line2, const wchar_t *name)
{
	RoomInfo *info = (RoomInfo *)mir_alloc(sizeof(RoomInfo));
	info->overlay = overlay;
	info->line1 = line1 ? mir_wstrdup(line1) : 0;
	info->line2 = line2 ? mir_wstrdup(line2) : 0;

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
		HXML query = XmlGetChild(iqNode, "query");
		if (query == nullptr) {
			sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
				TranslateT("Jabber Error"),
				TranslateT("Failed to retrieve room list from server."),
				L"");
		}
		else {
			bool found = false;
			HXML item;
			for (int i = 1; item = XmlGetNthChild(query, L"item", i); i++) {
				const wchar_t *jid = XmlGetAttrValue(item, L"jid");
				wchar_t *name = NEWWSTR_ALLOCA(jid);
				if (name) {
					if (wchar_t *p = wcschr(name, '@'))
						*p = 0;
				}
				else name = L"";

				sttRoomListAppend(hwndList,
					ListGetItemPtr(LIST_BOOKMARK, jid) ? RoomInfo::ROOM_BOOKMARK : RoomInfo::ROOM_DEFAULT,
					XmlGetAttrValue(item, L"name"),
					jid, name);

				found = true;
			}

			if (!found) {
				sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
					TranslateT("Jabber Error"),
					TranslateT("No rooms available on server."),
					L"");
			}
		}
	}
	else if (pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR) {
		HXML errorNode = XmlGetChild(iqNode, "error");
		wchar_t *str = JabberErrorMsg(errorNode);
		sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
			TranslateT("Jabber Error"),
			str,
			L"");
		mir_free(str);
	}
	else
		sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
			TranslateT("Jabber Error"),
			TranslateT("Room list request timed out."),
			L"");

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
	for (int i = 0; i < _countof(ctrls); i++) {
		GetWindowRect(GetDlgItem(hwndDlg, ctrls[i]), &rc);
		MapWindowPoints(nullptr, hwndDlg, (LPPOINT)&rc, 2);
		SetWindowPos(GetDlgItem(hwndDlg, ctrls[i]), nullptr, rc.left, rc.top + offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	GetWindowRect(hwndDlg, &rc);
	SetWindowPos(hwndDlg, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top + offset, SWP_NOMOVE | SWP_NOZORDER);
}

class CJabberDlgGcJoin : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	CCtrlButton btnOk;

public:
	CJabberDlgGcJoin(CJabberProto *proto, wchar_t *jid);
	~CJabberDlgGcJoin();

protected:
	wchar_t *m_jid;

	void OnInitDialog();
	void OnDestroy();

	void OnBtnOk(CCtrlButton*);

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
};

CJabberDlgGcJoin::CJabberDlgGcJoin(CJabberProto *proto, wchar_t *jid) :
	CSuper(proto, IDD_GROUPCHAT_JOIN, nullptr),
	btnOk(this, IDOK),
	m_jid(mir_wstrdup(jid))
{
	btnOk.OnClick = Callback(this, &CJabberDlgGcJoin::OnBtnOk);
}

CJabberDlgGcJoin::~CJabberDlgGcJoin()
{
	mir_free(m_jid);
}

void CJabberDlgGcJoin::OnInitDialog()
{
	CSuper::OnInitDialog();

	Window_SetIcon_IcoLib(m_hwnd, g_GetIconHandle(IDI_GROUP));

	JabberGcRecentInfo *pInfo = nullptr;
	if (m_jid)
		pInfo = new JabberGcRecentInfo(m_proto, m_jid);
	else if (OpenClipboard(m_hwnd)) {
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);

		if (hData) {
			wchar_t *buf = (wchar_t *)GlobalLock(hData);
			if (buf && wcschr(buf, '@') && !wcschr(buf, ' '))
				pInfo = new JabberGcRecentInfo(m_proto, buf);
			GlobalUnlock(hData);
		}
		CloseClipboard();
	}

	if (pInfo) {
		pInfo->fillForm(m_hwnd);
		delete pInfo;
	}

	ptrW tszNick(m_proto->getWStringA("Nick"));
	if (tszNick == nullptr)
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
		wchar_t jid[JABBER_MAX_JID_LEN];
		JabberGcRecentInfo info(m_proto);
		if (!info.loadRecent(i))
			break;

		mir_snwprintf(jid, L"%s@%s (%s)", info.m_room, info.m_server, info.m_nick ? info.m_nick : TranslateT("<no nick>"));
		SetDlgItemText(m_hwnd, IDC_RECENT1 + i, jid);
	}
	sttJoinDlgShowRecentItems(m_hwnd, i);
}

void CJabberDlgGcJoin::OnDestroy()
{
	IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BM_SETIMAGE, IMAGE_ICON, 0));
	m_proto->m_pDlgJabberJoinGroupchat = nullptr;
	DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_GETFONT, 0, 0));

	CSuper::OnDestroy();

	mir_free(m_jid); m_jid = nullptr;
}

void CJabberDlgGcJoin::OnBtnOk(CCtrlButton*)
{
	wchar_t text[128];
	GetDlgItemText(m_hwnd, IDC_SERVER, text, _countof(text));
	wchar_t *server = NEWWSTR_ALLOCA(text), *room;

	m_proto->ComboAddRecentString(m_hwnd, IDC_SERVER, "joinWnd_rcSvr", server);

	GetDlgItemText(m_hwnd, IDC_ROOM, text, _countof(text));
	room = NEWWSTR_ALLOCA(text);

	GetDlgItemText(m_hwnd, IDC_NICK, text, _countof(text));
	wchar_t *nick = NEWWSTR_ALLOCA(text);

	GetDlgItemText(m_hwnd, IDC_PASSWORD, text, _countof(text));
	wchar_t *password = NEWWSTR_ALLOCA(text);
	m_proto->GroupchatJoinRoom(server, room, nick, password);
}

INT_PTR CJabberDlgGcJoin::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
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

			lpmis->itemHeight = 2 * sttTextLineHeight;
			if (lpmis->itemID == -1)
				lpmis->itemHeight = sttTextLineHeight - 1;

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
			DrawText(lpdis->hDC, info->line1, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);

			rc = lpdis->rcItem;
			rc.top += (rc.bottom - rc.top) / 2;
			rc.left += 20;
			SetTextColor(lpdis->hDC, clLine2);
			DrawText(lpdis->hDC, info->line2, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS);

			DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("group"), 16, 16, 0, nullptr, DI_NORMAL);
			switch (info->overlay) {
			case RoomInfo::ROOM_WAIT:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("disco_progress"), 16, 16, 0, nullptr, DI_NORMAL);
				break;
			case RoomInfo::ROOM_FAIL:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("disco_fail"), 16, 16, 0, nullptr, DI_NORMAL);
				break;
			case RoomInfo::ROOM_BOOKMARK:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, m_proto->LoadIconEx("disco_ok"), 16, 16, 0, nullptr, DI_NORMAL);
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
					wchar_t *server = (wchar_t*)_alloca(len * sizeof(wchar_t));
					GetDlgItemText(m_hwnd, IDC_SERVER, server, len);

					if (*server) {
						sttRoomListAppend(GetDlgItem(m_hwnd, IDC_ROOM), RoomInfo::ROOM_WAIT, TranslateT("Loading..."), TranslateT("Please wait for room list to download."), L"");

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
						L"");
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
						if (!mir_wstrcmp(item->type, L"conference"))
							AppendMenu(hMenu, MF_STRING, (UINT_PTR)item, item->name);
				}
				AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)-1, TranslateT("Bookmarks..."));
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));

				RECT rc; GetWindowRect(GetDlgItem(m_hwnd, IDC_BOOKMARKS), &rc);
				CheckDlgButton(m_hwnd, IDC_BOOKMARKS, BST_CHECKED);
				int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
				CheckDlgButton(m_hwnd, IDC_BOOKMARKS, BST_UNCHECKED);
				DestroyMenu(hMenu);

				if (res == -1)
					m_proto->OnMenuHandleBookmarks(0, 0);
				else if (res) {
					JABBER_LIST_ITEM *item = (JABBER_LIST_ITEM *)res;
					wchar_t *room = NEWWSTR_ALLOCA(item->jid);
					if (room) {
						wchar_t *server = wcschr(room, '@');
						if (server) {
							*server++ = 0;

							SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_SERVER, CBN_EDITCHANGE), (LPARAM)GetDlgItem(m_hwnd, IDC_SERVER));

							SetDlgItemText(m_hwnd, IDC_SERVER, server);
							SetDlgItemText(m_hwnd, IDC_ROOM, room);
							SetDlgItemText(m_hwnd, IDC_NICK, item->nick);
							SetDlgItemText(m_hwnd, IDC_PASSWORD, item->password);
						}
					}
				}
			}
			break;

		case IDC_RECENT1:
		case IDC_RECENT2:
		case IDC_RECENT3:
		case IDC_RECENT4:
		case IDC_RECENT5:
			JabberGcRecentInfo info(m_proto, LOWORD(wParam) - IDC_RECENT1);
			info.fillForm(m_hwnd);
			if (GetAsyncKeyState(VK_CONTROL))
				break;

			OnBtnOk(nullptr);
			Close();
		}
		break;

	case WM_JABBER_CHECK_ONLINE:
		if (!m_proto->m_bJabberOnline)
			EndDialog(m_hwnd, 0);
		break;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

void CJabberProto::GroupchatJoinRoomByJid(HWND, wchar_t *jid)
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
	JabberGroupchatChangeNicknameParam(CJabberProto* ppro_, const wchar_t *jid_) :
		ppro(ppro_),
		jid(mir_wstrdup(jid_))
		{}

	~JabberGroupchatChangeNicknameParam()
	{	mir_free(jid);
	}

	CJabberProto *ppro;
	wchar_t *jid;
};

static VOID CALLBACK JabberGroupchatChangeNickname(void* arg)
{
	JabberGroupchatChangeNicknameParam *param = (JabberGroupchatChangeNicknameParam*)arg;
	if (param == nullptr)
		return;

	JABBER_LIST_ITEM *item = param->ppro->ListGetItemPtr(LIST_CHATROOM, param->jid);
	if (item != nullptr) {
		CMStringW szBuffer, szTitle;
		szTitle.Format(TranslateT("Change nickname in <%s>"), item->name ? item->name : item->jid);
		if (item->nick)
			szBuffer = item->nick;

		if (param->ppro->EnterString(szBuffer, szTitle, ESF_COMBO, "gcNick_")) {
			replaceStrW(item->nick, szBuffer);
			param->ppro->SendPresenceTo(param->ppro->m_iStatus, CMStringW(FORMAT, L"%s/%s", item->jid, szBuffer.c_str()), nullptr);
		}
	}

	delete param;
}

static int sttGetStatusCode(HXML node)
{
	HXML statusNode = XmlGetChild(node, "status");
	if (statusNode == nullptr)
		return -1;

	const wchar_t *statusCode = XmlGetAttrValue(statusNode, L"code");
	if (statusCode == nullptr)
		return -1;

	return _wtol(statusCode);
}

void CJabberProto::RenameParticipantNick(JABBER_LIST_ITEM *item, const wchar_t *oldNick, HXML itemNode)
{
	const wchar_t *jid = XmlGetAttrValue(itemNode, L"jid");
	const wchar_t *newNick = XmlGetAttrValue(itemNode, L"nick");
	if (newNick == nullptr)
		return;

	pResourceStatus r(item->findResource(oldNick));
	if (r == nullptr)
		return;

	r->m_tszResourceName = mir_wstrdup(newNick);

	if (!mir_wstrcmp(item->nick, oldNick)) {
		replaceStrW(item->nick, newNick);

		MCONTACT hContact = HContactFromJID(item->jid);
		if (hContact != 0)
			setWString(hContact, "MyNick", newNick);
	}

	Chat_ChangeUserId(m_szModuleName, item->jid, oldNick, newNick);

	GCEVENT gce = { m_szModuleName, item->jid, GC_EVENT_NICK };
	if (jid != nullptr)
		gce.ptszUserInfo = jid;
	gce.time = time(0);
	gce.ptszNick = oldNick;
	gce.ptszUID = newNick;
	gce.ptszText = newNick;
	Chat_Event(&gce);
}

void CJabberProto::GroupchatProcessPresence(HXML node)
{
	const wchar_t *from;

	if (!node || !XmlGetName(node) || mir_wstrcmp(XmlGetName(node), L"presence")) return;
	if ((from = XmlGetAttrValue(node, L"from")) == nullptr) return;

	const wchar_t *resource = wcschr(from, '/');
	if (resource == nullptr || *++resource == '\0')
		return;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, from);
	if (item == nullptr)
		return;

	pResourceStatus r(item->findResource(resource));

	HXML nNode = XmlGetChildByTag(node, "nick", "xmlns", JABBER_FEAT_NICK);
	const wchar_t *cnick = XmlGetText(nNode);
	const wchar_t *nick = cnick ? cnick : (r && r->m_tszNick ? r->m_tszNick : resource);

	// process custom nick change
	if (cnick && r && r->m_tszNick && mir_wstrcmp(cnick, r->m_tszNick))
		r->m_tszNick = mir_wstrdup(cnick);

	HXML xNode = XmlGetChildByTag(node, "x", "xmlns", JABBER_FEAT_MUC_USER);
	HXML itemNode = XmlGetChild(xNode, "item");

	const wchar_t *type = XmlGetAttrValue(node, L"type");

	// entering room or a usual room presence
	if (type == nullptr || !mir_wstrcmp(type, L"available")) {
		if (ptrW(JabberNickFromJID(from)) == nullptr)
			return;

		GcInit(item);
		item->iChatState = 0;

		// Update status of room participant
		int status = ID_STATUS_ONLINE;
		LPCTSTR ptszShow = XmlGetText(XmlGetChild(node, "show"));
		if (ptszShow) {
			if (!mir_wstrcmp(ptszShow, L"away")) status = ID_STATUS_AWAY;
			else if (!mir_wstrcmp(ptszShow, L"xa")) status = ID_STATUS_NA;
			else if (!mir_wstrcmp(ptszShow, L"dnd")) status = ID_STATUS_DND;
			else if (!mir_wstrcmp(ptszShow, L"chat")) status = ID_STATUS_FREECHAT;
		}

		LPCTSTR str = XmlGetText(XmlGetChild(node, "status"));

		char priority = 0;
		if (LPCTSTR ptszPriority = XmlGetText(XmlGetChild(node, "priority")))
			priority = (char)_wtoi(ptszPriority);

		bool bStatusChanged = false, bRoomCreated = false, bAffiliationChanged = false, bRoleChanged = false;
		int  newRes = ListAddResource(LIST_CHATROOM, from, status, str, priority, cnick) ? GC_EVENT_JOIN : 0;

		if (pResourceStatus oldRes = ListFindResource(LIST_CHATROOM, from))
			if ((oldRes->m_iStatus != status) || lstrcmp_null(oldRes->m_tszStatusMessage, str))
				bStatusChanged = true;

		// Check additional MUC info for this user
		if (itemNode != nullptr) {
			if (r == nullptr)
				r = item->findResource(resource);
			if (r != nullptr) {
				JABBER_GC_AFFILIATION affiliation = r->m_affiliation;
				JABBER_GC_ROLE role = r->m_role;

				if ((str = XmlGetAttrValue(itemNode, L"affiliation")) != nullptr) {
					if (!mir_wstrcmp(str, L"owner"))       affiliation = AFFILIATION_OWNER;
					else if (!mir_wstrcmp(str, L"admin"))       affiliation = AFFILIATION_ADMIN;
					else if (!mir_wstrcmp(str, L"member"))      affiliation = AFFILIATION_MEMBER;
					else if (!mir_wstrcmp(str, L"none"))	     affiliation = AFFILIATION_NONE;
					else if (!mir_wstrcmp(str, L"outcast"))     affiliation = AFFILIATION_OUTCAST;
				}
				if ((str = XmlGetAttrValue(itemNode, L"role")) != nullptr) {
					if (!mir_wstrcmp(str, L"moderator"))   role = ROLE_MODERATOR;
					else if (!mir_wstrcmp(str, L"participant")) role = ROLE_PARTICIPANT;
					else if (!mir_wstrcmp(str, L"visitor"))     role = ROLE_VISITOR;
					else                                        role = ROLE_NONE;
				}

				if ((role != ROLE_NONE) && (JabberGcGetStatus(r) != JabberGcGetStatus(affiliation, role))) {
					GcLogUpdateMemberStatus(item, resource, nick, nullptr, GC_EVENT_REMOVESTATUS, nullptr);
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

				if (str = XmlGetAttrValue(itemNode, L"jid"))
					r->m_tszRealJid = mir_wstrdup(str);

				// XEP-0115: Entity Capabilities
				OnProcessPresenceCapabilites(node, r);
			}
		}

		if (sttGetStatusCode(xNode) == 201)
			bRoomCreated = true;

		// show status change if needed
		if (bStatusChanged)
			if (pResourceStatus res = ListFindResource(LIST_CHATROOM, from))
				GcLogShowInformation(item, res, INFO_STATUS);

		// Update groupchat log window
		GcLogUpdateMemberStatus(item, resource, nick, str, newRes, nullptr);
		if (r && bAffiliationChanged) GcLogShowInformation(item, r, INFO_AFFILIATION);
		if (r && bRoleChanged) GcLogShowInformation(item, r, INFO_ROLE);

		// update clist status
		MCONTACT hContact = HContactFromJID(from, false);
		if (hContact != 0)
			setWord(hContact, "Status", status);

		// Check <created/>
		if (bRoomCreated) {
			HXML n = XmlGetChild(node, "created");
			if (n != nullptr && (str = XmlGetAttrValue(n, L"xmlns")) != nullptr && !mir_wstrcmp(str, JABBER_FEAT_MUC_OWNER))
				// A new room just created by me
				// Request room config
				m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, item->jid))
					<< XQUERY(JABBER_FEAT_MUC_OWNER));
		}
	}

	// leaving room
	else if (!mir_wstrcmp(type, L"unavailable")) {
		const wchar_t *str = 0;
		if (xNode != nullptr && item->nick != nullptr) {
			HXML reasonNode = XmlGetChild(itemNode, "reason");
			str = XmlGetAttrValue(itemNode, L"jid");

			int iStatus = sttGetStatusCode(xNode);
			if (iStatus == 301 && r != nullptr)
				GcLogShowInformation(item, r, INFO_BAN);

			if (!mir_wstrcmp(resource, item->nick)) {
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

		HXML statusNode = XmlGetChild(node, "status");
		GcLogUpdateMemberStatus(item, resource, nick, str, GC_EVENT_PART, statusNode);
		ListRemoveResource(LIST_CHATROOM, from);

		MCONTACT hContact = HContactFromJID(from);
		if (hContact != 0)
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
	}

	// processing room errors
	else if (!mir_wstrcmp(type, L"error")) {
		int errorCode = 0;
		HXML errorNode = XmlGetChild(node, "error");
		ptrW str(JabberErrorMsg(errorNode, &errorCode));

		if (errorCode == JABBER_ERROR_CONFLICT) {
			ptrW newNick(getWStringA("GcAltNick"));
			if (++item->iChatState == 1 && newNick != nullptr && newNick[0] != 0) {
				replaceStrW(item->nick, newNick);
				wchar_t text[1024] = { 0 };
				mir_snwprintf(text, L"%s/%s", item->jid, newNick);
				SendPresenceTo(m_iStatus, text, nullptr);
			}
			else {
				CallFunctionAsync(JabberGroupchatChangeNickname, new JabberGroupchatChangeNicknameParam(this, from));
				item->iChatState = 0;
			}
			return;
		}

		MsgPopup(0, str, TranslateT("Jabber Error"));

		if (item != nullptr && !item->bChatActive)
			ListRemove(LIST_CHATROOM, from);
	}
}

void CJabberProto::GroupchatProcessMessage(HXML node)
{
	HXML n, m;
	const wchar_t *from, *type, *p, *nick, *resource;
	JABBER_LIST_ITEM *item;
	CMStringW imgLink;

	if (!XmlGetName(node) || mir_wstrcmp(XmlGetName(node), L"message")) return;
	if ((from = XmlGetAttrValue(node, L"from")) == nullptr) return;
	if ((item = ListGetItemPtr(LIST_CHATROOM, from)) == nullptr) return;

	if ((type = XmlGetAttrValue(node, L"type")) == nullptr) return;
	if (!mir_wstrcmp(type, L"error"))
		return;

	GCEVENT gce = { m_szModuleName, item->jid, 0 };

	const wchar_t *msgText = nullptr;

	resource = wcschr(from, '/');
	if (resource != nullptr && *++resource == '\0')
		resource = nullptr;

	if ((n = XmlGetChild(node, "subject")) != nullptr) {
		msgText = XmlGetText(n);
		if (msgText == nullptr || msgText[0] == '\0')
			return;

		gce.iType = GC_EVENT_TOPIC;

		if (resource == nullptr && (m = XmlGetChild(node, "body")) != nullptr) {
			const wchar_t *tmpnick = XmlGetText(m);
			if (tmpnick == nullptr || *tmpnick == 0)
				return;

			const wchar_t *tmptr = wcsstr(tmpnick, L"has set the subject to:"); //ejabberd
			if (tmptr == nullptr)
				tmptr = wcsstr(tmpnick, TranslateT("has set the subject to:")); //ejabberd
			if (tmptr != nullptr && *tmptr != 0) {
				*(wchar_t*)(--tmptr) = 0;
				resource = tmpnick;
			}
		}
		item->getTemp()->m_tszStatusMessage = mir_wstrdup(msgText);
	}
	else {
		imgLink = ExtractImage(node);

		if ((n = XmlGetChildByTag(node, "body", "xml:lang", m_tszSelectedLang)) == nullptr)
			if ((n = XmlGetChild(node, "body")) == nullptr)
				return;

		msgText = XmlGetText(n);
		if (msgText == nullptr)
			return;

		if (resource == nullptr)
			gce.iType = GC_EVENT_INFORMATION;
		else if (wcsncmp(msgText, L"/me ", 4) == 0 && mir_wstrlen(msgText) > 4) {
			msgText += 4;
			gce.iType = GC_EVENT_ACTION;
		}
		else gce.iType = GC_EVENT_MESSAGE;
	}

	GcInit(item);

	time_t msgTime = 0;
	if (!JabberReadXep203delay(node, msgTime)) {
		HXML xDelay = XmlGetChildByTag(node, "x", "xmlns", L"jabber:x:delay");
		if (xDelay && (p = XmlGetAttrValue(xDelay, L"stamp")) != nullptr)
			msgTime = JabberIsoToUnixTime(p);
	}

	bool isHistory = msgTime != 0;
	time_t now = time(nullptr);
	if (!msgTime || msgTime > now)
		msgTime = now;

	if (resource != nullptr) {
		pResourceStatus r(item->findResource(resource));
		nick = (r && r->m_tszNick) ? r->m_tszNick : resource;
	}
	else nick = nullptr;

	CMStringW tszText(msgText);
	tszText.Replace(L"%", L"%%");
	tszText += imgLink;

	gce.ptszUID = resource;
	gce.ptszNick = nick;
	gce.time = msgTime;
	gce.ptszText = tszText;
	gce.bIsMe = nick == nullptr ? FALSE : (mir_wstrcmp(resource, item->nick) == 0);

	if (!isHistory)
		gce.dwFlags |= GCEF_ADDTOLOG;

	if (m_options.GcLogChatHistory && isHistory)
		gce.dwFlags |= GCEF_NOTNOTIFY;

	Chat_Event(&gce);

	item->bChatActive = 2;

	if (gce.iType == GC_EVENT_TOPIC)
		Chat_SetStatusbarText(m_szModuleName, item->jid, tszText);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Accepting groupchat invitations

class CGroupchatInviteAcceptDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
	CCtrlButton m_accept;
	CMStringW m_roomJid, m_from, m_reason, m_password;

public:
	CGroupchatInviteAcceptDlg(CJabberProto *ppro, const wchar_t *roomJid, const wchar_t *from, const wchar_t *reason, const wchar_t *password) :
		CSuper(ppro, IDD_GROUPCHAT_INVITE_ACCEPT, nullptr),
		m_roomJid(roomJid), m_from(from), m_reason(reason), m_password(password),
		m_accept(this, IDC_ACCEPT)
	{
		m_accept.OnClick = Callback(this, &CGroupchatInviteAcceptDlg::OnCommand_Accept);
	}

	void OnInitDialog()
	{
		CSuper::OnInitDialog();

		wchar_t buf[256];
		mir_snwprintf(buf, TranslateT("Group chat invitation to\n%s"), m_roomJid.c_str());
		SetDlgItemText(m_hwnd, IDC_HEADERBAR, buf);

		SetDlgItemText(m_hwnd, IDC_FROM, m_from);
		SetDlgItemText(m_hwnd, IDC_REASON, m_reason);
		SetDlgItemText(m_hwnd, IDC_NICK, ptrW(JabberNickFromJID(m_proto->m_szJabberJID)));

		Window_SetIcon_IcoLib(m_hwnd, g_GetIconHandle(IDI_GROUP));

		SetFocus(GetDlgItem(m_hwnd, IDC_NICK));
	}

	void OnCommand_Accept(CCtrlButton*)
	{
		wchar_t text[128];
		GetDlgItemText(m_hwnd, IDC_NICK, text, _countof(text));
		m_proto->AcceptGroupchatInvite(m_roomJid, text, m_password);
		EndDialog(m_hwnd, 0);
	}
};

static void __stdcall sttShowDialog(void *pArg)
{
	CGroupchatInviteAcceptDlg *pDlg = (CGroupchatInviteAcceptDlg*)pArg;
	pDlg->Show();
}

void CJabberProto::GroupchatProcessInvite(const wchar_t *roomJid, const wchar_t *from, const wchar_t *reason, const wchar_t *password)
{
	if (roomJid == nullptr)
		return;

	if (ListGetItemPtr(LIST_CHATROOM, roomJid))
		return;

	if (m_options.AutoAcceptMUC) {
		ptrW nick(getWStringA(HContactFromJID(m_szJabberJID), "MyNick"));
		if (nick == nullptr)
			nick = getWStringA("Nick");
		if (nick == nullptr)
			nick = JabberNickFromJID(m_szJabberJID);
		AcceptGroupchatInvite(roomJid, nick, password);
	}
	else CallFunctionAsync(sttShowDialog, new CGroupchatInviteAcceptDlg(this, roomJid, from, reason, password));
}

void CJabberProto::AcceptGroupchatInvite(const wchar_t *roomJid, const wchar_t *reason, const wchar_t *password)
{
	wchar_t room[256], *server, *p;
	wcsncpy_s(room, roomJid, _TRUNCATE);
	p = wcstok(room, L"@");
	server = wcstok(nullptr, L"@");
	GroupchatJoinRoom(server, p, reason, password);
}
