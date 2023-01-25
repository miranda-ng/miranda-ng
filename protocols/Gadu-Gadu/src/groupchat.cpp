////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"
#include "m_metacontacts.h"

#define GG_GC_GETCHAT "/GCGetChat"
#define GGS_OPEN_CONF "/OpenConf"
#define GGS_CLEAR_IGNORED "/ClearIgnored"

////////////////////////////////////////////////////////////////////////////////
// Inits Gadu-Gadu groupchat module using chat.dll
//
int GaduProto::gc_init()
{
	char service[64];

	// Register Gadu-Gadu groupchats
	GCREGISTER gcr = {};
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &GaduProto::gc_event);

	gc_enabled = TRUE;
	// create & hook event
	mir_snprintf(service, GG_GC_GETCHAT, m_szModuleName);
	debugLogA("gc_init(): Registered with groupchat plugin.");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Groupchat menus initialization
//
void GaduProto::gc_menus_init(HGENMENU hRoot)
{
	if (gc_enabled) {
		CMenuItem mi(&g_plugin);
		mi.flags = CMIF_UNICODE;
		mi.root = hRoot;

		// Conferencing
		mi.pszService = GGS_OPEN_CONF;
		CreateProtoService(mi.pszService, &GaduProto::gc_openconf);
		mi.position = 2000050001;
		mi.hIcolibItem = iconList[14].hIcolib;
		mi.name.w = LPGENW("Open &conference...");
		hMainMenu[0] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

		// Clear ignored conferences
		mi.pszService = GGS_CLEAR_IGNORED;
		CreateProtoService(mi.pszService, &GaduProto::gc_clearignored);
		mi.position = 2000050002;
		mi.hIcolibItem = iconList[15].hIcolib;
		mi.name.w = LPGENW("&Clear ignored conferences");
		hMainMenu[1] = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Releases Gadu-Gadu groupchat module using chat.dll
//
int GaduProto::gc_destroy()
{
	list_t l;
	for (l = chats; l; l = l->next) {
		GGGC *chat = (GGGC *)l->data;
		free(chat->recipients);
	}
	list_destroy(chats, 1); chats = nullptr;

	return 1;
}

GGGC* GaduProto::gc_lookup(const wchar_t *id)
{
	GGGC *chat;
	list_t l;

	for (l = chats; l; l = l->next) {
		chat = (GGGC *)l->data;
		if (chat && !mir_wstrcmp(chat->id, id))
			return chat;
	}

	return nullptr;
}

int GaduProto::gc_event(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	GGGC *chat = nullptr;
	uin_t uin;

	// Check if we got our protocol, and fields are set
	if (!gch || !gch->si->ptszID || !gch->si->pszModule
		|| mir_strcmpi(gch->si->pszModule, m_szModuleName)
		|| !(uin = getDword(gch->si->hContact, GG_KEY_UIN))
		|| !(chat = gc_lookup(gch->si->ptszID)))
		return 0;

	// Window terminated (Miranda exit)
	if (gch->iType == SESSION_TERMINATE) {
		debugLogW(L"gc_event(): Terminating chat %x, id %s from chat window...", chat, gch->si->ptszID);
		// Destroy chat entry
		free(chat->recipients);
		list_remove(&chats, chat, 1);

		// Remove contact from contact list (duh!) should be done by chat.dll !!
		db_delete_contact(gch->si->hContact);
		return 1;
	}

	// Message typed / send only if online
	if (isonline() && (gch->iType == GC_USER_MESSAGE) && gch->ptszText) {
		wchar_t id[32];
		UIN2IDT(uin, id);
		DBVARIANT dbv;

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
		gce.pszID.w = gch->si->ptszID;
		gce.pszUID.w = id;
		gce.pszText.w = gch->ptszText;
		wchar_t* nickT;
		if (!getWString(GG_KEY_NICK, &dbv)) {
			nickT = mir_wstrdup(dbv.pwszVal);
			db_free(&dbv);
		}
		else
			nickT = mir_wstrdup(TranslateT("Me"));

		gce.pszNick.w = nickT;

		// Get rid of CRLF at back
		int lc = (int)mir_wstrlen(gch->ptszText) - 1;
		while (lc >= 0 && (gch->ptszText[lc] == '\n' || gch->ptszText[lc] == '\r'))
			gch->ptszText[lc--] = 0;

		gce.time = time(0);
		gce.bIsMe = 1;
		gce.dwFlags = GCEF_ADDTOLOG;
		debugLogW(L"gc_event(): Sending conference message to room %s, \"%s\".", gch->si->ptszID, gch->ptszText);
		Chat_Event(&gce);
		mir_free(nickT);

		T2Utf pszText_utf8(gch->ptszText);
		gg_EnterCriticalSection(&sess_mutex, "gc_event", 57, "sess_mutex", 1);
		gg_send_message_confer(m_sess, GG_CLASS_CHAT, chat->recipients_count, chat->recipients, pszText_utf8);
		gg_LeaveCriticalSection(&sess_mutex, "gc_event", 57, 1, "sess_mutex", 1);
		return 1;
	}

	// Privmessage selected
	if (gch->iType == GC_USER_PRIVMESS) {
		MCONTACT hContact = NULL;
		if ((uin = _wtoi(gch->ptszUID)) && (hContact = getcontact(uin, 1, 0, nullptr)))
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
	}

	debugLogW(L"gc_event(): Unhandled event %d, chat %x, uin %d, text \"%s\".", gch->iType, chat, uin, gch->ptszText);
	return 1;
}

typedef struct _gg_gc_echat
{
	uin_t sender;
	uin_t *recipients;
	int recipients_count;
	char * chat_id;
} gg_gc_echat;

////////////////////////////////////////////////////////////////////////////////
// This is main groupchat initialization routine
//
wchar_t* GaduProto::gc_getchat(uin_t sender, uin_t *recipients, int recipients_count)
{
	list_t l;
	GGGC *chat;

	debugLogA("gc_getchat(): Count %d.", recipients_count);
	if (!recipients)
		return nullptr;

	// Look for existing chat
	for (l = chats; l; l = l->next) {
		chat = (GGGC *)l->data;
		if (!chat) continue;

		if (chat->recipients_count == recipients_count + (sender ? 1 : 0)) {
			int sok = (sender == 0);
			if (!sok) {
				for (int i = 0; i < chat->recipients_count; i++) {
					if (sender == chat->recipients[i]) {
						sok = 1;
						break;
					}
				}
			}

			int found = 0;
			if (sok)
				for (int i = 0; i < chat->recipients_count; i++)
					for (int j = 0; j < recipients_count; j++)
						if (recipients[j] == chat->recipients[i])
							++found;

			// Found all recipients
			if (found == recipients_count) {
				if (chat->ignore)
					debugLogW(L"gc_getchat(): Ignoring existing id %s, size %d.", chat->id, chat->recipients_count);
				else
					debugLogW(L"gc_getchat(): Returning existing id %s, size %d.", chat->id, chat->recipients_count);
				return !(chat->ignore) ? chat->id : nullptr;
			}
		}
	}

	// Make new uin list to chat mapping
	chat = (GGGC *)malloc(sizeof(GGGC));
	UIN2IDT(gc_id++, chat->id);
	chat->ignore = FALSE;

	// Check groupchat policy (new) / only for incoming
	if (sender) {
		int unknown = (getcontact(sender, 0, 0, nullptr) == NULL),
			unknownSender = unknown;
		for (int i = 0; i < recipients_count; i++)
			if (!getcontact(recipients[i], 0, 0, nullptr))
				++unknown;

		if ((getWord(GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 2) ||
			(getWord(GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 2 &&
				recipients_count >= getWord(GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
				(getWord(GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 2 &&
					unknown >= getWord(GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN))) {
			chat->ignore = TRUE;
		}

		if (!chat->ignore && ((getWord(GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 1) ||
			(getWord(GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 1 &&
				recipients_count >= getWord(GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
				(getWord(GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 1 &&
					unknown >= getWord(GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN)))) {
			wchar_t *senderName = unknownSender ?
				TranslateT("Unknown") : Clist_GetContactDisplayName(getcontact(sender, 0, 0, nullptr));
			wchar_t error[256];
			mir_snwprintf(error, TranslateT("%s has initiated conference with %d participants (%d unknowns).\nDo you want to participate?"),
				senderName, recipients_count + 1, unknown);
			chat->ignore = MessageBox(nullptr, error, m_tszUserName, MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK;
		}

		if (chat->ignore) {
			// Copy recipient list
			chat->recipients_count = recipients_count + 1;
			chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
			int i = 0;
			for (; i < recipients_count; i++)
				chat->recipients[i] = recipients[i];
			if (sender) chat->recipients[i] = sender;
			debugLogW(L"gc_getchat(): Ignoring new chat %s, count %d.", chat->id, chat->recipients_count);
			list_add(&chats, chat, 0);
			return nullptr;
		}
	}

	// Create new chat window
	wchar_t status[256];
	wchar_t *senderName;
	if (sender) {
		senderName = Clist_GetContactDisplayName(getcontact(sender, 1, 0, nullptr));
		mir_snwprintf(status, TranslateT("%s initiated the conference."), senderName);
	}
	else {
		senderName = nullptr;
		mir_snwprintf(status, TranslateT("This is my own conference."));
	}

	// Create new room
	CMStringW wszTitle(L"#"); wszTitle.Append(sender ? senderName : TranslateT("Conference"));
	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, chat->id, wszTitle, chat);
	if (!si)
		return nullptr;

	Chat_SetStatusbarText(m_szModuleName, chat->id, status);

	// Add normal group
	Chat_AddGroup(si, TranslateT("Participants"));

	wchar_t id[32];

	GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
	gce.pszID.w = chat->id;
	gce.pszUID.w = id;
	gce.dwFlags = GCEF_ADDTOLOG;

	// Add myself
	uin_t uin = getDword(GG_KEY_UIN, 0);
	if (uin) {
		UIN2IDT(uin, id);

		ptrW nickT(getWStringA(GG_KEY_NICK));
		if (nickT == NULL)
			nickT = mir_wstrdup(TranslateT("Me"));

		gce.pszNick.w = nickT;
		gce.bIsMe = 1;
		Chat_Event(&gce);
		mir_free(nickT);
		debugLogW(L"gc_getchat(): Myself %s: %s (%s) to the list...", gce.pszUID.w, gce.pszNick.w, gce.pszStatus.w);
	}
	else
		debugLogA("gc_getchat(): Myself adding failed with uin %d !!!", uin);

	// Copy recipient list
	chat->recipients_count = recipients_count + (sender ? 1 : 0);
	chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
	int i;
	for (i = 0; i < recipients_count; i++)
		chat->recipients[i] = recipients[i];
	if (sender)
		chat->recipients[i] = sender;

	// Add contacts
	for (i = 0; i < chat->recipients_count; i++) {
		MCONTACT hContact = getcontact(chat->recipients[i], 1, 0, nullptr);
		UIN2IDT(chat->recipients[i], id);
		if (hContact)
			gce.pszNick.w = Clist_GetContactDisplayName(hContact);
		else
			gce.pszNick.w = TranslateT("'Unknown'");
		gce.bIsMe = 0;
		gce.dwFlags = 0;
		debugLogW(L"gc_getchat(): Added %s: %s (%s) to the list...", gce.pszUID.w, gce.pszNick.w, gce.pszStatus.w);
		Chat_Event(&gce);
	}

	Chat_Control(m_szModuleName, chat->id, SESSION_INITDONE);
	Chat_Control(m_szModuleName, chat->id, SESSION_ONLINE);

	debugLogW(L"gc_getchat(): Returning new chat window %s, count %d.", chat->id, chat->recipients_count);
	list_add(&chats, chat, 0);

	return chat->id;
}

static MCONTACT gg_getsubcontact(GaduProto* gg, MCONTACT hContact)
{
	char* szProto = Proto_GetBaseAccountName(hContact);
	if (szProto && !mir_strcmp(szProto, META_PROTO)) {
		int nSubContacts = db_mc_getSubCount(hContact), i;
		for (i = 0; i < nSubContacts; i++) {
			MCONTACT hMetaContact = db_mc_getSub(hContact, i);
			szProto = Proto_GetBaseAccountName(hMetaContact);
			if (szProto && !mir_strcmp(szProto, gg->m_szModuleName))
				return hMetaContact;
		}
	}

	return 0;
}

static void gg_gc_resetclistopts(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, TRUE, 0);
}

static int gg_gc_countcheckmarks(HWND hwndList)
{
	int count = 0;
	for (auto &hContact : Contacts()) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
			++count;
	}

	return count;
}

#define HM_SUBCONTACTSCHANGED (WM_USER + 100)

static INT_PTR CALLBACK gg_gc_openconfdlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		{
			SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)lParam);
			TranslateDialogDefault(hwndDlg);
			Window_SetIcon_IcoLib(hwndDlg, g_plugin.getIconHandle(IDI_CONFERENCE));
			gg_gc_resetclistopts(GetDlgItem(hwndDlg, IDC_CLIST));

			// Hook MetaContacts event (if available)
			HANDLE hMetaContactsEvent = HookEventMessage(ME_MC_SUBCONTACTSCHANGED, hwndDlg, HM_SUBCONTACTSCHANGED);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hMetaContactsEvent);
		}
		return TRUE;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDOK:
				{
					HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);
					GaduProto* gg = (GaduProto*)GetWindowLongPtr(hwndDlg, DWLP_USER);
					int count = 0, i = 0;
					// Check if connected
					if (!gg->isonline()) {
						MessageBox(nullptr,
							TranslateT("You have to be connected to open new conference."),
							gg->m_tszUserName, MB_OK | MB_ICONSTOP);
					}
					else if (hwndList && (count = gg_gc_countcheckmarks(hwndList)) >= 2) {
						// Create new participiants table
						uin_t* participants = (uin_t*)calloc(count, sizeof(uin_t));
						gg->debugLogA("gg_gc_openconfdlg(): WM_COMMAND IDOK Opening new conference for %d contacts.", count);
						for (auto &hContact : Contacts()) {
							HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
							if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
								MCONTACT hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
								participants[i++] = db_get_dw(hMetaContact ? hMetaContact : hContact, gg->m_szModuleName, GG_KEY_UIN, 0);
							}
						}
						if (count > i)
							i = count;

						wchar_t *chat = gg->gc_getchat(0, participants, count);
						if (chat)
							Chat_Control(gg->m_szModuleName, chat, WINDOW_VISIBLE);

						free(participants);
					}
				}
				// fall through
			case IDCANCEL:
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_CLIST:
			switch (((NMHDR*)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				gg_gc_resetclistopts(GetDlgItem(hwndDlg, IDC_CLIST));
				break;

			case CLN_NEWCONTACT:
			case CLN_CONTACTMOVED:
			case CLN_LISTREBUILT:
				{
					char* szProto;
					uin_t uin;
					GaduProto* gg = (GaduProto*)GetWindowLongPtr(hwndDlg, DWLP_USER);

					if (!gg)
						break;

					// Delete non-gg contacts
					for (auto &hContact : Contacts()) {
						MCONTACT hItem = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
						if (hItem) {
							MCONTACT hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
							if (hMetaContact) {
								szProto = gg->m_szModuleName;
								uin = (uin_t)gg->getDword(hMetaContact, GG_KEY_UIN, 0);
							}
							else {
								szProto = Proto_GetBaseAccountName(hContact);
								uin = (uin_t)gg->getDword(hContact, GG_KEY_UIN, 0);
							}

							if (szProto == nullptr || mir_strcmp(szProto, gg->m_szModuleName) || !uin || uin == gg->getDword(GG_KEY_UIN, 0))
								SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_DELETEITEM, (WPARAM)hItem, 0);
						}
					}
				}
				break;

			case CLN_CHECKCHANGED:
				EnableWindow(GetDlgItem(hwndDlg, IDOK), gg_gc_countcheckmarks(GetDlgItem(hwndDlg, IDC_CLIST)) >= 2);
				break;
			}
			break;
		}
		break;

	case HM_SUBCONTACTSCHANGED:
		{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);
			SendMessage(hwndList, CLM_AUTOREBUILD, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), gg_gc_countcheckmarks(hwndList) >= 2);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		HANDLE hMetaContactsEvent = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (hMetaContactsEvent)
			UnhookEvent(hMetaContactsEvent);
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}

	return FALSE;
}

INT_PTR GaduProto::gc_clearignored(WPARAM, LPARAM)
{
	list_t l = chats;
	BOOL cleared = FALSE;
	while (l) {
		GGGC *chat = (GGGC *)l->data;
		l = l->next;
		if (chat->ignore) {
			free(chat->recipients);
			list_remove(&chats, chat, 1);
			cleared = TRUE;
		}
	}
	MessageBox(nullptr,
		cleared ?
		TranslateT("All ignored conferences are now unignored and the conference policy will act again.") :
		TranslateT("There are no ignored conferences."),
		m_tszUserName, MB_OK | MB_ICONINFORMATION);

	return 0;
}

INT_PTR GaduProto::gc_openconf(WPARAM, LPARAM)
{
	// Check if connected
	if (!isonline()) {
		MessageBox(nullptr,
			TranslateT("You have to be connected to open new conference."),
			m_tszUserName, MB_OK | MB_ICONSTOP);
		return 0;
	}

	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONFERENCE), nullptr, gg_gc_openconfdlg, (LPARAM)this);
	return 1;
}

int GaduProto::gc_changenick(MCONTACT hContact, wchar_t *ptszNick)
{
	list_t l;
	uin_t uin = getDword(hContact, GG_KEY_UIN, 0);
	if (!uin || !ptszNick) return 0;

	debugLogA("gc_changenick(): Nickname for uin %d changed. Lookup for chats having this nick", uin);
	// Lookup for chats having this nick
	for (l = chats; l; l = l->next) {
		GGGC *chat = (GGGC *)l->data;
		if (chat->recipients && chat->recipients_count)
			for (int i = 0; i < chat->recipients_count; i++)
				// Rename this window if it's exising in the chat
				if (chat->recipients[i] == uin) {
					wchar_t id[32];
					UIN2IDT(uin, id);

					GCEVENT gce = { m_szModuleName, 0, GC_EVENT_NICK };
					gce.pszID.w = chat->id;
					gce.pszUID.w = id;
					gce.pszText.w = ptszNick;

					debugLogW(L"gc_changenick(): Found room %s with uin %d, sending nick change %s.", chat->id, uin, id);
					Chat_Event(&gce);

					break;
				}
	}

	return 1;
}
