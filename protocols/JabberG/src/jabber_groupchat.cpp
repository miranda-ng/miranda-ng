/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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
	ptrA m_room, m_server, m_nick, m_password;
	CJabberProto *ppro;

	JabberGcRecentInfo(CJabberProto *proto)
	{
		ppro = proto;
	}
	JabberGcRecentInfo(CJabberProto *proto, const char *room, const char *server, const char *nick = nullptr, const char *password = nullptr)
	{
		ppro = proto;
		fillData(room, server, nick, password);
	}
	JabberGcRecentInfo(CJabberProto *proto, const char *jid)
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

	BOOL equals(const char *room, const char *server, const char *nick = nullptr, const char *password = nullptr)
	{
		return
			null_strequals(m_room, room) &&
			null_strequals(m_server, server) &&
			null_strequals(m_nick, nick) &&
			null_strequals(m_password, password);
	}

	BOOL equalsnp(const char *room, const char *server, const char *nick = nullptr)
	{
		return
			null_strequals(m_room, room) &&
			null_strequals(m_server, server) &&
			null_strequals(m_nick, nick);
	}

	void fillForm(HWND hwndDlg)
	{
		SetDlgItemTextUtf(hwndDlg, IDC_SERVER, m_server ? m_server : "");
		SetDlgItemTextUtf(hwndDlg, IDC_ROOM, m_room ? m_room : "");
		SetDlgItemTextUtf(hwndDlg, IDC_NICK, m_nick ? m_nick : "");
		SetDlgItemTextUtf(hwndDlg, IDC_PASSWORD, m_password ? m_password : "");
	}

	void fillData(const char *room, const char *server, const char *nick = nullptr, const char *password = nullptr)
	{
		m_room = mir_strdup(room);
		m_server = mir_strdup(server);
		m_nick = mir_strdup(nick);
		m_password = mir_strdup(password);
	}

	void fillData(const char *jid)
	{
		char *room, *server, *nick = nullptr;
		room = NEWSTR_ALLOCA(jid);
		server = strchr(room, '@');
		if (server) {
			*server++ = 0;
			nick = strchr(server, '/');
			if (nick)
				*nick++ = 0;
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
		m_server = ppro->getUStringA(setting);

		mir_snprintf(setting, "rcMuc_%d_room", iRecent);
		m_room = ppro->getUStringA(setting);

		mir_snprintf(setting, "rcMuc_%d_nick", iRecent);
		m_nick = ppro->getUStringA(setting);

		mir_snprintf(setting, "password_rcMuc_%d", iRecent);
		m_password = ppro->getUStringA(setting);

		return m_room || m_server || m_nick || m_password;
	}

	void saveRecent(int iRecent)
	{
		char setting[MAXMODULELABELLENGTH];

		mir_snprintf(setting, "rcMuc_%d_server", iRecent);
		if (m_server)
			ppro->setUString(setting, m_server);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, "rcMuc_%d_room", iRecent);
		if (m_room)
			ppro->setUString(setting, m_room);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, "rcMuc_%d_nick", iRecent);
		if (m_nick)
			ppro->setUString(setting, m_nick);
		else
			ppro->delSetting(setting);

		mir_snprintf(setting, "password_rcMuc_%d", iRecent);
		if (m_password)
			ppro->setUString(setting, m_password);
		else
			ppro->delSetting(setting);
	}

private:
	bool null_strequals(const char *str1, const char *str2)
	{
		if (!str1 && !str2) return true;
		if (!str1 && str2 && !*str2) return true;
		if (!str2 && str1 && !*str1) return true;

		if (!str1 && str2) return false;
		if (!str2 && str1) return false;

		return !mir_strcmp(str1, str2);
	}
};

INT_PTR __cdecl CJabberProto::OnMenuHandleJoinGroupchat(WPARAM, LPARAM)
{
	GroupchatJoinRoomByJid(nullptr, nullptr);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	ptrA jid(getUStringA(hContact, "ChatRoomID"));
	if (jid == nullptr)
		return 0;

	ptrA nick(getUStringA(hContact, "MyNick"));
	if (nick == nullptr)
		if ((nick = getUStringA("Nick")) == nullptr)
			return 0;

	ptrA password(getUStringA(hContact, "Password"));

	if (getWord(hContact, "Status", 0) != ID_STATUS_ONLINE) {
		char *p = strchr(jid, '@');
		if (p != nullptr) {
			*p++ = 0;
			GroupchatJoinRoom(p, jid, nick, password);
		}
	}

	return 0;
}

INT_PTR __cdecl CJabberProto::OnLeaveChat(WPARAM hContact, LPARAM)
{
	ptrA jid(getUStringA(hContact, "ChatRoomID"));
	if (jid != nullptr) {
		if (getWord(hContact, "Status", 0) != ID_STATUS_OFFLINE) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, jid);
			if (item != nullptr)
				GcQuit(item, 200, nullptr);
		}
	}
	return 0;
}

void CJabberProto::GroupchatJoinRoom(const char *server, const char *room, const char *nick, const char *password, bool autojoin)
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

	char text[JABBER_MAX_JID_LEN + 1];
	mir_snprintf(text, "%s@%s/%s", room, server, nick);

	JABBER_LIST_ITEM *item = ListAdd(LIST_CHATROOM, text);
	item->bAutoJoin = autojoin;
	replaceStr(item->nick, nick);
	replaceStr(item->password, info.m_password);

	int status = (m_iStatus == ID_STATUS_INVISIBLE) ? ID_STATUS_ONLINE : m_iStatus;
	XmlNode x("x"); x << XATTR("xmlns", JABBER_FEAT_MUC);
	if (mir_strlen(info.m_password))
		x << XCHILD("password", info.m_password);
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
	info->line1 = mir_wstrdup(line1);
	info->line2 = mir_wstrdup(line2);

	int id = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)name);
	SendMessage(hwndList, CB_SETITEMDATA, id, (LPARAM)info);
	SendMessage(hwndList, CB_SETITEMHEIGHT, id, sttTextLineHeight * 2);
	return id;
}

void CJabberProto::OnIqResultDiscovery(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!iqNode || !pInfo)
		return;

	HWND hwndList = (HWND)pInfo->GetUserData();
	SendMessage(hwndList, CB_SHOWDROPDOWN, FALSE, 0);
	SendMessage(hwndList, CB_RESETCONTENT, 0, 0);

	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) {
		auto *query = XmlFirstChild(iqNode, "query");
		if (query == nullptr) {
			sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
				TranslateT("Jabber Error"),
				TranslateT("Failed to retrieve room list from server."),
				L"");
		}
		else {
			bool found = false;
			for (auto *item : TiXmlFilter(query, "item")) {
				const char *jid = XmlGetAttr(item, "jid");
				char *name = NEWSTR_ALLOCA(jid);
				if (name) {
					if (char *p = strchr(name, '@'))
						*p = 0;
				}
				else name = "";

				sttRoomListAppend(hwndList,
					ListGetItemPtr(LIST_BOOKMARK, jid) ? RoomInfo::ROOM_BOOKMARK : RoomInfo::ROOM_DEFAULT,
					Utf2T(XmlGetAttr(item, "name")), Utf2T(jid), Utf2T(name));

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
		sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
			TranslateT("Jabber Error"),
			JabberErrorMsg(iqNode),
			L"");
	}
	else
		sttRoomListAppend(hwndList, RoomInfo::ROOM_FAIL,
			TranslateT("Jabber Error"),
			TranslateT("Room list request timed out."),
			L"");

	SendMessage(hwndList, CB_SHOWDROPDOWN, TRUE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CJabberDlgGcJoin : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;

	char *m_jid;
	int m_iqid = 0;

	struct CRoomCombo : public CCtrlCombo
	{
		CRoomCombo(CDlgBase *pDlg, int id) :
			CCtrlCombo(pDlg, id)
		{}

		BOOL OnMeasureItem(MEASUREITEMSTRUCT *lpmis) override
		{
			lpmis->itemHeight = 2 * sttTextLineHeight;
			if (lpmis->itemID == -1)
				lpmis->itemHeight = sttTextLineHeight - 1;
			return FALSE;
		}

		BOOL OnDrawItem(DRAWITEMSTRUCT *lpdis) override
		{
			RoomInfo *info = (RoomInfo *)GetItemData(lpdis->itemID);
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

			DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, g_plugin.getIcon(IDI_GROUP), 16, 16, 0, nullptr, DI_NORMAL);
			switch (info->overlay) {
			case RoomInfo::ROOM_WAIT:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, g_plugin.getIcon(IDI_DISCO_PROGRESS), 16, 16, 0, nullptr, DI_NORMAL);
				break;
			case RoomInfo::ROOM_FAIL:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, g_plugin.getIcon(IDI_DISCO_FAIL), 16, 16, 0, nullptr, DI_NORMAL);
				break;
			case RoomInfo::ROOM_BOOKMARK:
				DrawIconEx(lpdis->hDC, lpdis->rcItem.left + 1, lpdis->rcItem.top + 1, g_plugin.getIcon(IDI_DISCO_OK), 16, 16, 0, nullptr, DI_NORMAL);
				break;
			}
			return FALSE;
		}

		BOOL OnDeleteItem(DELETEITEMSTRUCT *lpdis) override
		{
			RoomInfo *info = (RoomInfo *)lpdis->itemData;
			mir_free(info->line1);
			mir_free(info->line2);
			mir_free(info);
			return FALSE;
		}
	};

	CRoomCombo cmbRoom;
	CCtrlCombo cmbServer;
	CCtrlMButton btnBookmarks;
	CCtrlHyperlink h1, h2, h3, h4, h5;

public:
	CJabberDlgGcJoin(CJabberProto *proto, char *jid) :
		CSuper(proto, IDD_GROUPCHAT_JOIN),
		m_jid(mir_strdup(jid)),
		h1(this, IDC_RECENT1),
		h2(this, IDC_RECENT2),
		h3(this, IDC_RECENT3),
		h4(this, IDC_RECENT4),
		h5(this, IDC_RECENT5),
		cmbRoom(this, IDC_ROOM),
		cmbServer(this, IDC_SERVER),
		btnBookmarks(this, IDC_BOOKMARKS, g_plugin.getIcon(IDI_BOOKMARKS), "Bookmarks")
	{
		btnBookmarks.OnClick = Callback(this, &CJabberDlgGcJoin::onClick_Bookmarks);

		cmbRoom.OnDropdown = Callback(this, &CJabberDlgGcJoin::onDrop_Room);
		cmbServer.OnChange = cmbServer.OnSelChanged = Callback(this, &CJabberDlgGcJoin::onChange_Server);

		h1.OnClick = h2.OnClick = h3.OnClick = h4.OnClick = h5.OnClick = Callback(this, &CJabberDlgGcJoin::onClick_Link);
	}
	
	~CJabberDlgGcJoin()
	{
		mir_free(m_jid);
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_GROUP));

		JabberGcRecentInfo *pInfo = nullptr;
		if (m_jid)
			pInfo = new JabberGcRecentInfo(m_proto, m_jid);
		else if (OpenClipboard(m_hwnd)) {
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);

			if (hData) {
				wchar_t *buf = (wchar_t *)GlobalLock(hData);
				if (buf && wcschr(buf, '@') && !wcschr(buf, ' '))
					pInfo = new JabberGcRecentInfo(m_proto, T2Utf(buf).get());
				GlobalUnlock(hData);
			}
			CloseClipboard();
		}

		if (pInfo) {
			pInfo->fillForm(m_hwnd);
			delete pInfo;
		}

		ptrA tszNick(m_proto->getUStringA("Nick"));
		if (tszNick == nullptr)
			tszNick = JabberNickFromJID(m_proto->m_szJabberJID);
		SetDlgItemTextUtf(m_hwnd, IDC_NICK, tszNick);

		TEXTMETRIC tm = { 0 };
		HDC hdc = GetDC(m_hwnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(m_hwnd, hdc);
		sttTextLineHeight = tm.tmHeight;
		cmbRoom.SendMsg(CB_SETITEMHEIGHT, -1, sttTextLineHeight - 1);

		LOGFONT lf = { 0 };
		HFONT hfnt = (HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_GETFONT, 0, 0);
		GetObject(hfnt, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_SETFONT, (WPARAM)CreateFontIndirect(&lf), TRUE);

		btnBookmarks.MakeFlat();
		btnBookmarks.MakePush();

		m_proto->ComboLoadRecentStrings(m_hwnd, IDC_SERVER, "joinWnd_rcSvr");

		CCtrlHyperlink *links[] = { &h1, &h2, &h3, &h4, &h5 };
		for (auto *it : links)
			it->Hide();

		int newCount = 0;
		for (auto *it : links) {
			JabberGcRecentInfo info(m_proto);
			if (!info.loadRecent(newCount))
				break;

			char jid[JABBER_MAX_JID_LEN];
			mir_snprintf(jid, "%s@%s (%s)", info.m_room.get(), info.m_server.get(), info.m_nick ? info.m_nick.get() : TranslateU("<no nick>"));
			SetDlgItemTextUtf(m_hwnd, it->GetCtrlId(), jid);

			it->Show();
			newCount++;
		}

		RECT rcTitle, rcLastItem;
		GetWindowRect(GetDlgItem(m_hwnd, IDC_TXT_RECENT), &rcTitle);
		GetWindowRect(h5.GetHwnd(), &rcLastItem);

		ShowWindow(GetDlgItem(m_hwnd, IDC_TXT_RECENT), newCount ? SW_SHOW : SW_HIDE);

		int curRecentHeight = rcLastItem.bottom - rcTitle.top;
		int newRecentHeight = rcLastItem.bottom - rcTitle.top - (5 - newCount) * (rcLastItem.bottom - rcLastItem.top);
		if (!newCount)
			newRecentHeight = 0;
		int offset = newRecentHeight - curRecentHeight;

		RECT rc;
		int ctrls[] = { IDC_BOOKMARKS, IDOK, IDCANCEL };
		for (auto &it : ctrls) {
			GetWindowRect(GetDlgItem(m_hwnd, it), &rc);
			MapWindowPoints(nullptr, m_hwnd, (LPPOINT)&rc, 2);
			SetWindowPos(GetDlgItem(m_hwnd, it), nullptr, rc.left, rc.top + offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		GetWindowRect(m_hwnd, &rc);
		SetWindowPos(m_hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top + offset, SWP_NOMOVE | SWP_NOZORDER);
		return true;
	}

	bool OnApply() override
	{
		wchar_t text[128];
		cmbServer.GetText(text, _countof(text));
		T2Utf server(text);

		m_proto->ComboAddRecentString(m_hwnd, IDC_SERVER, "joinWnd_rcSvr", text);

		cmbRoom.GetText(text, _countof(text));
		T2Utf room(text);

		GetDlgItemText(m_hwnd, IDC_NICK, text, _countof(text));
		T2Utf nick(text);

		GetDlgItemText(m_hwnd, IDC_PASSWORD, text, _countof(text));
		T2Utf password(text);
		m_proto->GroupchatJoinRoom(server, room, nick, password);
		return true;
	}

	void OnDestroy() override
	{
		IcoLib_ReleaseIcon((HICON)SendDlgItemMessage(m_hwnd, IDC_BOOKMARKS, BM_SETIMAGE, IMAGE_ICON, 0));
		m_proto->m_pDlgJabberJoinGroupchat = nullptr;
		DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_TXT_RECENT, WM_GETFONT, 0, 0));

		CSuper::OnDestroy();

		mir_free(m_jid); m_jid = nullptr;
	}

	void OnProtoCheckOnline(WPARAM, LPARAM) override
	{
		if (!m_proto->m_bJabberOnline)
			EndModal(0);
	}

	void onDrop_Room(CCtrlCombo*)
	{
		if (cmbRoom.GetCount())
			return;

		if (m_iqid) {
			m_proto->m_iqManager.ExpireIq(m_iqid);
			m_iqid = 0;
		}

		cmbRoom.ResetContent();

		int len = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_SERVER)) + 1;
		wchar_t *server = (wchar_t *)_alloca(len * sizeof(wchar_t));
		GetDlgItemText(m_hwnd, IDC_SERVER, server, len);

		if (*server) {
			sttRoomListAppend(cmbRoom.GetHwnd(), RoomInfo::ROOM_WAIT, TranslateT("Loading..."), TranslateT("Please wait for room list to download."), L"");

			CJabberIqInfo *pInfo = m_proto->AddIQ(&CJabberProto::OnIqResultDiscovery, JABBER_IQ_TYPE_GET, T2Utf(server), (void *)cmbRoom.GetHwnd());
			pInfo->SetTimeout(30000);
			XmlNodeIq iq(pInfo);
			iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
			m_proto->m_ThreadInfo->send(iq);

			m_iqid = pInfo->GetIqId();
		}
		else sttRoomListAppend(cmbRoom.GetHwnd(), RoomInfo::ROOM_FAIL, TranslateT("Jabber Error"), TranslateT("Please specify group chat directory first."), L"");
	}

	void onChange_Server(CCtrlCombo*)
	{
		if (m_iqid) {
			m_proto->m_iqManager.ExpireIq(m_iqid);
			m_iqid = 0;
		}

		cmbRoom.ResetContent();
	}

	void onClick_Bookmarks(CCtrlButton *pButton)
	{
		HMENU hMenu = CreatePopupMenu();

		LISTFOREACH(i, m_proto, LIST_BOOKMARK)
		{
			JABBER_LIST_ITEM *item = nullptr;
			if (item = m_proto->ListGetItemPtrFromIndex(i))
				if (!mir_strcmp(item->type, "conference"))
					AppendMenu(hMenu, MF_STRING, (UINT_PTR)item, item->name);
		}
		AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
		AppendMenu(hMenu, MF_STRING, (UINT_PTR)-1, TranslateT("Bookmarks..."));
		AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));

		RECT rc; 
		GetWindowRect(pButton->GetHwnd(), &rc);
		CheckDlgButton(m_hwnd, IDC_BOOKMARKS, BST_CHECKED);
		int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, m_hwnd, nullptr);
		CheckDlgButton(m_hwnd, IDC_BOOKMARKS, BST_UNCHECKED);
		DestroyMenu(hMenu);

		if (res == -1)
			m_proto->OnMenuHandleBookmarks(0, 0);
		else if (res) {
			JABBER_LIST_ITEM *item = (JABBER_LIST_ITEM *)res;
			if (char *room = NEWSTR_ALLOCA(item->jid)) {
				if (char *server = strchr(room, '@')) {
					*server++ = 0;

					SendMessage(m_hwnd, WM_COMMAND, MAKEWPARAM(IDC_SERVER, CBN_EDITCHANGE), (LPARAM)GetDlgItem(m_hwnd, IDC_SERVER));

					SetDlgItemTextUtf(m_hwnd, IDC_SERVER, server);
					SetDlgItemTextUtf(m_hwnd, IDC_ROOM, room);
					SetDlgItemTextUtf(m_hwnd, IDC_NICK, item->nick);
					SetDlgItemTextUtf(m_hwnd, IDC_PASSWORD, item->password);
				}
			}
		}
	}

	void onClick_Link(CCtrlHyperlink *pLink)
	{
		JabberGcRecentInfo info(m_proto, pLink->GetCtrlId() - IDC_RECENT1);
		info.fillForm(m_hwnd);
		if (!GetAsyncKeyState(VK_CONTROL)) {
			OnApply();
			Close();
		}
	}
};

void CJabberProto::GroupchatJoinRoomByJid(HWND parent, char *jid)
{
	if (m_pDlgJabberJoinGroupchat)
		SetForegroundWindow(m_pDlgJabberJoinGroupchat->GetHwnd());
	else {
		m_pDlgJabberJoinGroupchat = new CJabberDlgGcJoin(this, jid);
		m_pDlgJabberJoinGroupchat->SetParent(parent);
		m_pDlgJabberJoinGroupchat->Show();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGroupchatProcessPresence - handles the group chat presence packet

struct JabberGroupchatChangeNicknameParam
{
	JabberGroupchatChangeNicknameParam(CJabberProto* ppro_, const char *jid_) :
		ppro(ppro_),
		jid(mir_strdup(jid_))
	{}

	~JabberGroupchatChangeNicknameParam()
	{
		mir_free(jid);
	}

	CJabberProto *ppro;
	char *jid;
};

static VOID CALLBACK JabberGroupchatChangeNickname(void* arg)
{
	JabberGroupchatChangeNicknameParam *param = (JabberGroupchatChangeNicknameParam*)arg;
	if (param == nullptr)
		return;

	JABBER_LIST_ITEM *item = param->ppro->ListGetItemPtr(LIST_CHATROOM, param->jid);
	if (item != nullptr) {
		CMStringW szBuffer, szTitle;
		szTitle.Format(TranslateT("Change nickname in <%s>"), item->name ? item->name : Utf2T(item->jid).get());
		if (item->nick)
			szBuffer = Utf2T(item->nick);

		if (param->ppro->EnterString(szBuffer, szTitle, ESF_COMBO, "gcNick_")) {
			T2Utf newNick(szBuffer);
			replaceStr(item->nick, newNick);
			param->ppro->SendPresenceTo(param->ppro->m_iStatus, MakeJid(item->jid, newNick));
		}
	}

	delete param;
}

static int sttGetStatusCode(const TiXmlElement *node)
{
	if (node == nullptr)
		return -1;

	auto *statusNode = XmlFirstChild(node, "status");
	if (statusNode == nullptr)
		return -1;

	const char *statusCode = XmlGetAttr(statusNode, "code");
	if (statusCode == nullptr)
		return -1;

	return atol(statusCode);
}

void CJabberProto::RenameParticipantNick(JABBER_LIST_ITEM *item, const char *oldNick, const TiXmlElement *itemNode)
{
	const char *jid = XmlGetAttr(itemNode, "jid");
	const char *newNick = XmlGetAttr(itemNode, "nick");
	if (newNick == nullptr)
		return;

	pResourceStatus r(item->findResource(oldNick));
	if (r == nullptr)
		return;

	r->m_szResourceName = mir_strdup(newNick);

	if (!mir_strcmp(item->nick, oldNick)) {
		replaceStr(item->nick, newNick);

		MCONTACT hContact = HContactFromJID(item->jid);
		if (hContact != 0)
			setUString(hContact, "MyNick", newNick);
	}

	Chat_ChangeUserId(m_szModuleName, Utf2T(item->jid), Utf2T(oldNick), Utf2T(newNick));

	GCEVENT gce = { m_szModuleName, item->jid, GC_EVENT_NICK };
	gce.dwFlags = GCEF_UTF8;
	gce.pszID.a = item->jid;
	gce.pszUserInfo.a = jid;
	gce.time = time(0);
	gce.pszNick.a = oldNick;
	gce.pszUID.a = newNick;
	gce.pszText.a = newNick;
	Chat_Event(&gce);
}

void CJabberProto::GroupchatProcessPresence(const TiXmlElement *node)
{
	const char *from;

	if (!node || !node->Name() || mir_strcmp(node->Name(), "presence")) return;
	if ((from = XmlGetAttr(node, "from")) == nullptr) return;

	const char *resource = strchr(from, '/');
	if (resource == nullptr || *++resource == '\0')
		return;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, from);
	if (item == nullptr)
		return;

	pResourceStatus r(item->findResource(resource));

	const char *cnick = nullptr;
	if (auto *n = XmlGetChildByTag(node, "nick", "xmlns", JABBER_FEAT_NICK))
		cnick = n->GetText();

	const char *nick = cnick ? cnick : (r && r->m_szNick ? r->m_szNick.get() : resource);

	const TiXmlElement *itemNode = nullptr;
	auto *xNode = XmlGetChildByTag(node, "x", "xmlns", JABBER_FEAT_MUC_USER);
	if (xNode)
		itemNode = XmlFirstChild(xNode, "item");

	// entering room or a usual room presence
	const char *type = XmlGetAttr(node, "type");
	if (type == nullptr || !mir_strcmp(type, "available")) {
		if (ptrA(JabberNickFromJID(from)) == nullptr)
			return;

		GcInit(item);
		item->iChatState = 0;

		// Update status of room participant
		int status = ID_STATUS_ONLINE;
		if (auto *pszStatus = XmlGetChildText(node, "show")) {
			if (!mir_strcmp(pszStatus, "away")) status = ID_STATUS_AWAY;
			else if (!mir_strcmp(pszStatus, "xa")) status = ID_STATUS_NA;
			else if (!mir_strcmp(pszStatus, "dnd")) status = ID_STATUS_DND;
			else if (!mir_strcmp(pszStatus, "chat")) status = ID_STATUS_FREECHAT;
		}

		// process custom nick change
		if (cnick && r && r->m_szNick && mir_strcmp(cnick, r->m_szNick))
			r->m_szNick = mir_strdup(cnick);

		const char *str = XmlGetChildText(node, "status");
		int priority = XmlGetChildInt(node, "priority");
		int newRes = ListAddResource(LIST_CHATROOM, from, status, str, priority, cnick) ? GC_EVENT_JOIN : 0;

		bool bStatusChanged = false, bRoomCreated = false, bAffiliationChanged = false, bRoleChanged = false;
		if (pResourceStatus oldRes = ListFindResource(LIST_CHATROOM, from))
			if ((oldRes->m_iStatus != status) || mir_strcmp(oldRes->m_szStatusMessage, str))
				bStatusChanged = true;

		// Check additional MUC info for this user
		if (itemNode != nullptr) {
			if (r == nullptr)
				r = item->findResource(resource);
			if (r != nullptr) {
				JABBER_GC_AFFILIATION affiliation = r->m_affiliation;
				JABBER_GC_ROLE role = r->m_role;

				if ((str = XmlGetAttr(itemNode, "affiliation")) != nullptr) {
					if (!mir_strcmp(str, "owner"))        affiliation = AFFILIATION_OWNER;
					else if (!mir_strcmp(str, "admin"))   affiliation = AFFILIATION_ADMIN;
					else if (!mir_strcmp(str, "member"))  affiliation = AFFILIATION_MEMBER;
					else if (!mir_strcmp(str, "none"))	  affiliation = AFFILIATION_NONE;
					else if (!mir_strcmp(str, "outcast")) affiliation = AFFILIATION_OUTCAST;
				}
				if ((str = XmlGetAttr(itemNode, "role")) != nullptr) {
					if (!mir_strcmp(str, "moderator"))        role = ROLE_MODERATOR;
					else if (!mir_strcmp(str, "participant")) role = ROLE_PARTICIPANT;
					else if (!mir_strcmp(str, "visitor"))     role = ROLE_VISITOR;
					else                                      role = ROLE_NONE;
				}

				if (r->m_role != ROLE_NONE && JabberGcGetStatus(r) != JabberGcGetStatus(affiliation, role)) {
					GcLogUpdateMemberStatus(item, resource, nick, nullptr, GC_EVENT_REMOVESTATUS, nullptr);
					if (!newRes)
						newRes = GC_EVENT_ADDSTATUS;
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

				if (str = XmlGetAttr(itemNode, "jid"))
					r->m_szRealJid = mir_strdup(str);

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

		// A new room just created by me. Request room config
		if (bRoomCreated)
			m_ThreadInfo->send(XmlNodeIq(AddIQ(&CJabberProto::OnIqResultGetMuc, JABBER_IQ_TYPE_GET, item->jid)) << XQUERY(JABBER_FEAT_MUC_OWNER));
	}

	// leaving room
	else if (!mir_strcmp(type, "unavailable")) {
		const char *str = nullptr;
		if (xNode != nullptr && item->nick != nullptr) {
			auto *reasonNode = XmlFirstChild(itemNode, "reason");
			str = XmlGetAttr(itemNode, "jid");

			int iStatus = sttGetStatusCode(xNode);
			if (iStatus == 301 && r != nullptr)
				GcLogShowInformation(item, r, INFO_BAN);

			if (!mir_strcmp(resource, item->nick)) {
				switch (iStatus) {
				case 301:
				case 307:
				case 321:
				case 322:
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
				case 321:
				case 322:
					ListRemoveResource(LIST_CHATROOM, from);
					GcLogUpdateMemberStatus(item, resource, nick, str, GC_EVENT_KICK, reasonNode, iStatus);
					return;
				}
			}
		}

		auto *statusNode = XmlFirstChild(node, "status");
		GcLogUpdateMemberStatus(item, resource, nick, str, GC_EVENT_PART, statusNode);
		ListRemoveResource(LIST_CHATROOM, from);

		MCONTACT hContact = HContactFromJID(from);
		if (hContact != 0)
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
	}

	// processing room errors
	else if (!mir_strcmp(type, "error")) {
		int errorCode = 0;
		CMStringW str(JabberErrorMsg(node, &errorCode));

		if (errorCode == JABBER_ERROR_CONFLICT) {
			ptrA newNick(getUStringA("GcAltNick"));
			if (++item->iChatState == 1 && newNick != nullptr && newNick[0] != 0) {
				replaceStr(item->nick, newNick);
				SendPresenceTo(m_iStatus, MakeJid(item->jid, newNick));
			}
			else {
				CallFunctionAsync(JabberGroupchatChangeNickname, new JabberGroupchatChangeNicknameParam(this, from));
				item->iChatState = 0;
			}
			return;
		}

		MsgPopup(0, str, TranslateT("Error"));

		if (item != nullptr && !item->si)
			ListRemove(LIST_CHATROOM, from);
	}
}

void CJabberProto::GroupchatProcessMessage(const TiXmlElement *node)
{
	const TiXmlElement *n, *m;
	const char *from, *type, *p, *nick;
	JABBER_LIST_ITEM *item;
	CMStringW imgLink;

	if (!node->Name() || mir_strcmp(node->Name(), "message")) return;
	if ((from = XmlGetAttr(node, "from")) == nullptr) return;
	if ((item = ListGetItemPtr(LIST_CHATROOM, from)) == nullptr) return;

	if ((type = XmlGetAttr(node, "type")) == nullptr) return;
	if (!mir_strcmp(type, "error"))
		return;

	GCEVENT gce = { m_szModuleName, item->jid, 0 };
	gce.dwFlags = GCEF_UTF8;

	const char *resource = strchr(from, '/'), *msgText;
	if (resource != nullptr && *++resource == '\0')
		resource = nullptr;

	for (auto *x : TiXmlFilter(node, "x"))
		if (!mir_strcmp(XmlGetAttr(x, "xmlns"), JABBER_FEAT_MUC_USER))
			if (XmlGetChildByTag(x, "status", "code", "104"))
				SendGetVcard(item->hContact);

	if ((n = XmlFirstChild(node, "subject")) != nullptr) {
		msgText = n->GetText();
		if (msgText == nullptr || msgText[0] == '\0')
			return;

		gce.iType = GC_EVENT_TOPIC;

		if (resource == nullptr && (m = XmlFirstChild(node, "body")) != nullptr) {
			const char *tmpnick = m->GetText();
			if (tmpnick == nullptr || *tmpnick == 0)
				return;

			const char *tmptr = strstr(tmpnick, "has set the subject to:"); //ejabberd
			if (tmptr == nullptr)
				tmptr = strstr(tmpnick, TranslateU("has set the subject to:")); //ejabberd
			if (tmptr != nullptr && *tmptr != 0) {
				*(wchar_t*)(--tmptr) = 0;
				resource = tmpnick;
			}
		}
		
		item->getTemp()->m_szStatusMessage = mir_strdup(msgText);
	}
	else {
		imgLink = ExtractImage(node);

		if ((n = XmlGetChildByTag(node, "body", "xml:lang", m_tszSelectedLang)) == nullptr)
			if ((n = XmlFirstChild(node, "body")) == nullptr)
				return;

		msgText = n->GetText();
		if (msgText == nullptr)
			return;

		if (resource == nullptr)
			gce.iType = GC_EVENT_INFORMATION;
		else if (strncmp(msgText, "/me ", 4) == 0 && mir_strlen(msgText) > 4) {
			msgText += 4;
			gce.iType = GC_EVENT_ACTION;
		}
		else gce.iType = GC_EVENT_MESSAGE;
	}

	GcInit(item);

	time_t msgTime = 0;
	if (!JabberReadXep203delay(node, msgTime)) {
		auto *xDelay = XmlGetChildByTag(node, "x", "xmlns", "jabber:x:delay");
		if (xDelay && (p = XmlGetAttr(xDelay, "stamp")) != nullptr)
			msgTime = JabberIsoToUnixTime(p);
	}

	time_t now = time(0);
	if (!msgTime || msgTime > now)
		msgTime = now;

	// ugly hack to cut out group chat history: 2 seconds delay to skip all old messages
	// unfortunately there's no other way to detect if that message came online or from history
	if (now - item->iChatInitTime > 2)
		item->bChatLogging = true;

	if (resource != nullptr) {
		pResourceStatus r(item->findResource(resource));
		nick = (r && r->m_szNick) ? r->m_szNick.get() : resource;
	}
	else nick = nullptr;

	CMStringA szText(msgText);
	szText.Replace("%", "%%");
	szText += imgLink;

	gce.pszUID.a = resource;
	gce.pszNick.a = nick;
	gce.time = msgTime;
	gce.pszText.a = szText;
	gce.bIsMe = nick == nullptr ? FALSE : (mir_strcmp(resource, item->nick) == 0);

	// if we log chat events, add them to log window
	if (item->bChatLogging)
		gce.dwFlags |= GCEF_ADDTOLOG;
	else if (m_bGcLogChatHistory) // else we need to suppress disk logging and sounds/popups
		gce.dwFlags |= GCEF_NOTNOTIFY;

	Chat_Event(&gce);

	if (gce.iType == GC_EVENT_TOPIC)
		Chat_SetStatusbarText(m_szModuleName, Utf2T(item->jid), Utf2T(szText));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Accepting groupchat invitations

class CGroupchatInviteAcceptDlg : public CJabberDlgBase
{
	typedef CJabberDlgBase CSuper;
	CMStringA m_roomJid, m_from, m_reason, m_password;

public:
	CGroupchatInviteAcceptDlg(CJabberProto *ppro, const char *roomJid, const char *from, const char *reason, const char *password) :
		CSuper(ppro, IDD_GROUPCHAT_INVITE_ACCEPT),
		m_roomJid(roomJid), m_from(from), m_reason(reason), m_password(password)
	{
	}

	bool OnInitDialog() override
	{
		CSuper::OnInitDialog();

		wchar_t buf[256];
		mir_snwprintf(buf, TranslateT("Group chat invitation to\n%s"), Utf2T(m_roomJid).get());
		SetDlgItemText(m_hwnd, IDC_HEADERBAR, buf);

		SetDlgItemTextUtf(m_hwnd, IDC_FROM, m_from);
		SetDlgItemTextUtf(m_hwnd, IDC_REASON, m_reason);
		SetDlgItemTextUtf(m_hwnd, IDC_NICK, JabberNickFromJID(m_proto->m_szJabberJID));

		Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_GROUP));

		SetFocus(GetDlgItem(m_hwnd, IDC_NICK));
		return true;
	}

	bool OnApply() override
	{
		wchar_t text[128];
		GetDlgItemText(m_hwnd, IDC_NICK, text, _countof(text));
		m_proto->AcceptGroupchatInvite(m_roomJid, T2Utf(text), m_password);
		return true;
	}
};

static void __stdcall sttShowDialog(void *pArg)
{
	CGroupchatInviteAcceptDlg *pDlg = (CGroupchatInviteAcceptDlg*)pArg;
	pDlg->Show();
}

void CJabberProto::GroupchatProcessInvite(const char *roomJid, const char *from, const char *reason, const char *password)
{
	if (roomJid == nullptr)
		return;

	if (ListGetItemPtr(LIST_CHATROOM, roomJid))
		return;

	if (m_bAutoAcceptMUC) {
		ptrA nick(getUStringA(HContactFromJID(m_szJabberJID), "MyNick"));
		if (nick == nullptr)
			nick = getUStringA("Nick");
		if (nick == nullptr)
			nick = JabberNickFromJID(m_szJabberJID);
		AcceptGroupchatInvite(roomJid, nick, password);
	}
	else CallFunctionAsync(sttShowDialog, new CGroupchatInviteAcceptDlg(this, roomJid, from, reason, password));
}

void CJabberProto::AcceptGroupchatInvite(const char *roomJid, const char *reason, const char *password)
{
	char room[256];
	strncpy_s(room, roomJid, _TRUNCATE);
	char *p = strtok(room, "@");
	char *server = strtok(nullptr, "@");
	GroupchatJoinRoom(server, p, reason, password);
}

void CJabberProto::GroupchatSendMsg(JABBER_LIST_ITEM *pItem, const char *msg)
{
	if (!m_bJabberOnline)
		return;

	char szId[100];
	int64_t id = (_time64(nullptr) << 16) + (GetTickCount() & 0xFFFF);
	_i64toa(id, szId, 36);

	m_ThreadInfo->send(
		XmlNode("message") << XATTR("id", szId) << XATTR("to", pItem->jid) << XATTR("type", "groupchat")
		<< XCHILD("body", msg));
}
