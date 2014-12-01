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

int GGPROTO::gc_init()
{
	if (ServiceExists(MS_GC_REGISTER)) {
		char service[64];

		// Register Gadu-Gadu proto
		GCREGISTER gcr = { sizeof(gcr) };
		gcr.ptszDispName = m_tszUserName;
		gcr.pszModule = m_szModuleName;
		CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

		HookProtoEvent(ME_GC_EVENT, &GGPROTO::gc_event);

		gc_enabled = TRUE;
		// create & hook event
		mir_snprintf(service, 64, GG_GC_GETCHAT, m_szModuleName);
		debugLogA("gc_init(): Registered with groupchat plugin.");
	}
	else debugLogA("gc_init(): Cannot register with groupchat plugin !!!");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Groupchat menus initialization

void GGPROTO::gc_menus_init(HGENMENU hRoot)
{
	if (gc_enabled) {
		char service[64];

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
		mi.hParentMenu = hRoot;

		// Conferencing
		mir_snprintf(service, SIZEOF(service), "%s%s", m_szModuleName, GGS_OPEN_CONF);
		CreateProtoService(GGS_OPEN_CONF, &GGPROTO::gc_openconf);
		mi.position = 2000050001;
		mi.icolibItem = iconList[14].hIcolib;
		mi.ptszName = LPGENT("Open &conference...");
		mi.pszService = service;
		hMainMenu[0] = Menu_AddProtoMenuItem(&mi);

		// Clear ignored conferences
		mir_snprintf(service, SIZEOF(service), "%s%s", m_szModuleName, GGS_CLEAR_IGNORED);
		CreateProtoService(GGS_CLEAR_IGNORED, &GGPROTO::gc_clearignored);
		mi.position = 2000050002;
		mi.icolibItem = iconList[15].hIcolib;
		mi.ptszName = LPGENT("&Clear ignored conferences");
		mi.pszService = service;
		hMainMenu[1] = Menu_AddProtoMenuItem(&mi);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Releases Gadu-Gadu groupchat module using chat.dll

int GGPROTO::gc_destroy()
{
	list_t l;
	for(l = chats; l; l = l->next)
	{
		GGGC *chat = (GGGC *)l->data;
		free(chat->recipients);
	}
	list_destroy(chats, 1); chats = NULL;
	return 1;
}

GGGC* GGPROTO::gc_lookup(const TCHAR *id)
{
	GGGC *chat;
	list_t l;

	for(l = chats; l; l = l->next)
	{
		chat = (GGGC *)l->data;
		if (chat && !_tcscmp(chat->id, id))
			return chat;
	}

	return NULL;
}

int GGPROTO::gc_event(WPARAM wParam, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	GGGC *chat = NULL;
	uin_t uin;

	// Check if we got our protocol, and fields are set
	if (!gch
		|| !gch->pDest
		|| !gch->pDest->ptszID
		|| !gch->pDest->pszModule
		|| mir_strcmpi(gch->pDest->pszModule, m_szModuleName)
		|| !(uin = getDword(GG_KEY_UIN, 0))
		|| !(chat = gc_lookup(gch->pDest->ptszID)))
		return 0;

	// Window terminated (Miranda exit)
	if (gch->pDest->iType == SESSION_TERMINATE)
	{
		debugLog(_T("gc_event(): Terminating chat %x, id %s from chat window..."), chat, gch->pDest->ptszID);
		// Destroy chat entry
		free(chat->recipients);
		list_remove(&chats, chat, 1);

		// Remove contact from contact list (duh!) should be done by chat.dll !!
		for (MCONTACT hContact = db_find_first(); hContact; ) {
			MCONTACT hNext = db_find_next(hContact);
			DBVARIANT dbv;
			if (!getTString(hContact, "ChatRoomID", &dbv)) {
				if (dbv.ptszVal && !_tcscmp(gch->pDest->ptszID, dbv.ptszVal))
					CallService(MS_DB_CONTACT_DELETE, hContact, 0);
				db_free(&dbv);
			}
			hContact = hNext;
		}
		return 1;
	}

	// Message typed / send only if online
	if (isonline() && (gch->pDest->iType == GC_USER_MESSAGE) && gch->ptszText) {
		TCHAR id[32];
		UIN2IDT(uin, id);
		DBVARIANT dbv;

		GCDEST gcd = { m_szModuleName, gch->pDest->ptszID, GC_EVENT_MESSAGE };
		GCEVENT gce = { sizeof(gce), &gcd };
		gce.ptszUID = id;
		gce.ptszText = gch->ptszText;
		TCHAR* nickT;
		if (!getTString(GG_KEY_NICK, &dbv)){
			nickT = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		}
		else nickT = mir_tstrdup(TranslateT("Me"));
		gce.ptszNick = nickT;

		// Get rid of CRLF at back
		int lc = (int)_tcslen(gch->ptszText) - 1;
		while(lc >= 0 && (gch->ptszText[lc] == '\n' || gch->ptszText[lc] == '\r'))
			gch->ptszText[lc --] = 0;

		gce.time = time(NULL);
		gce.bIsMe = 1;
		gce.dwFlags = GCEF_ADDTOLOG;
		debugLog(_T("gc_event(): Sending conference message to room %s, \"%s\"."), gch->pDest->ptszID, gch->ptszText);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		mir_free(nickT);
		
		char* pszText_utf8 = mir_utf8encodeT(gch->ptszText);
		gg_EnterCriticalSection(&sess_mutex, "gc_event", 57, "sess_mutex", 1);
		gg_send_message_confer(sess, GG_CLASS_CHAT, chat->recipients_count, chat->recipients, (BYTE*)pszText_utf8);
		gg_LeaveCriticalSection(&sess_mutex, "gc_event", 57, 1, "sess_mutex", 1);
		mir_free(pszText_utf8);
		
		return 1;
	}

	// Privmessage selected
	if (gch->pDest->iType == GC_USER_PRIVMESS)
	{
		MCONTACT hContact = NULL;
		if ((uin = _ttoi(gch->ptszUID)) && (hContact = getcontact(uin, 1, 0, NULL)))
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
	}
	debugLog(_T("gc_event(): Unhandled event %d, chat %x, uin %d, text \"%s\"."), gch->pDest->iType, chat, uin, gch->ptszText);

	return 0;
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

TCHAR* GGPROTO::gc_getchat(uin_t sender, uin_t *recipients, int recipients_count)
{
	list_t l; int i;
	GGGC *chat;
	TCHAR id[32];
	uin_t uin; DBVARIANT dbv;
	GCDEST gcd = { m_szModuleName, 0, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };

	debugLogA("gc_getchat(): Count %d.", recipients_count);
	if (!recipients) return NULL;

	// Look for existing chat
	for(l = chats; l; l = l->next)
	{
		GGGC *chat = (GGGC *)l->data;
		if (!chat) continue;

		if (chat->recipients_count == recipients_count + (sender ? 1 : 0))
		{
			int i, j, found = 0, sok = (sender == 0);
			if (!sok) for(i = 0; i < chat->recipients_count; i++)
				if (sender == chat->recipients[i])
				{
					sok = 1;
					break;
				}
			if (sok)
				for(i = 0; i < chat->recipients_count; i++)
					for(j = 0; j < recipients_count; j++)
						if (recipients[j] == chat->recipients[i]) found++;
			// Found all recipients
			if (found == recipients_count)
			{
				if (chat->ignore)
					debugLog(_T("gc_getchat(): Ignoring existing id %s, size %d."), chat->id, chat->recipients_count);
				else
					debugLog(_T("gc_getchat(): Returning existing id %s, size %d."), chat->id, chat->recipients_count);
				return !(chat->ignore) ? chat->id : NULL;
			}
		}
	}

	// Make new uin list to chat mapping
	chat = (GGGC *)malloc(sizeof(GGGC));
	UIN2IDT(gc_id ++, chat->id);
	chat->ignore = FALSE;

	// Check groupchat policy (new) / only for incoming
	if (sender)
	{
		int unknown = (getcontact(sender, 0, 0, NULL) == NULL),
			unknownSender = unknown;
		for(i = 0; i < recipients_count; i++)
			if (!getcontact(recipients[i], 0, 0, NULL))
				unknown ++;
		if ((getWord(GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 2) ||
		   (getWord(GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 2 &&
			recipients_count >= getWord(GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
		   (getWord(GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 2 &&
			unknown >= getWord(GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN)))
			chat->ignore = TRUE;
		if (!chat->ignore && ((getWord(GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 1) ||
		   (getWord(GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 1 &&
			recipients_count >= getWord(GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
		   (getWord(GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 1 &&
			unknown >= getWord(GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN))))
		{
			TCHAR *senderName = unknownSender ?
				TranslateT("Unknown") : pcli->pfnGetContactDisplayName(getcontact(sender, 0, 0, NULL), 0);
			TCHAR error[256];
			mir_sntprintf(error, SIZEOF(error), TranslateT("%s has initiated conference with %d participants (%d unknowns).\nDo you want to participate?"),
				senderName, recipients_count + 1, unknown);
			chat->ignore = MessageBox(NULL, error, m_tszUserName, MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK;
		}
		if (chat->ignore)
		{
			// Copy recipient list
			chat->recipients_count = recipients_count + (sender ? 1 : 0);
			chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
			for(i = 0; i < recipients_count; i++)
				chat->recipients[i] = recipients[i];
			if (sender) chat->recipients[i] = sender;
			debugLog(_T("gc_getchat(): Ignoring new chat %s, count %d."), chat->id, chat->recipients_count);
			list_add(&chats, chat, 0);
			return NULL;
		}
	}

	// Create new chat window
	TCHAR status[256];
	TCHAR *senderName = sender ? pcli->pfnGetContactDisplayName(getcontact(sender, 1, 0, NULL), 0) : NULL;
	mir_sntprintf(status, 255, (sender) ? TranslateT("%s initiated the conference.") : TranslateT("This is my own conference."), senderName);

	GCSESSION gcwindow = { sizeof(gcwindow) };
	gcwindow.iType = GCW_CHATROOM;
	gcwindow.pszModule = m_szModuleName;
	gcwindow.ptszName = sender ? senderName : TranslateT("Conference");
	gcwindow.ptszID = chat->id;
	gcwindow.dwItemData = (DWORD)chat;
	gcwindow.ptszStatusbarText = status;

	// Here we put nice new hash sign
	TCHAR *name = (TCHAR*)calloc(_tcslen(gcwindow.ptszName) + 2, sizeof(TCHAR));
	*name = '#'; _tcscpy(name + 1, gcwindow.ptszName);
	gcwindow.ptszName = name;

	// Create new room
	if (CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM) &gcwindow)) {
		debugLog(_T("gc_getchat(): Cannot create new chat window %s."), chat->id);
		free(name);
		free(chat);
		return NULL;
	}
	free(name);

	gcd.ptszID = chat->id;
	gce.ptszUID = id;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.time = 0;

	// Add normal group
	gce.ptszStatus = TranslateT("Participants");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	gcd.iType = GC_EVENT_JOIN;

	// Add myself
	if (uin = getDword(GG_KEY_UIN, 0))
	{
		UIN2IDT(uin, id);

		TCHAR* nickT;
		if (!getTString(GG_KEY_NICK, &dbv)) {
			nickT = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		} else {
			nickT = mir_tstrdup(TranslateT("Me"));
		}
		gce.ptszNick = nickT;

		gce.bIsMe = 1;
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		mir_free(nickT);
		debugLog(_T("gc_getchat(): Myself %s: %s (%s) to the list..."), gce.ptszUID, gce.ptszNick, gce.ptszStatus);
	}
	else debugLogA("gc_getchat(): Myself adding failed with uin %d !!!", uin);

	// Copy recipient list
	chat->recipients_count = recipients_count + (sender ? 1 : 0);
	chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
	for(i = 0; i < recipients_count; i++)
		chat->recipients[i] = recipients[i];
	if (sender) chat->recipients[i] = sender;

	// Add contacts
	for(i = 0; i < chat->recipients_count; i++) {
		MCONTACT hContact = getcontact(chat->recipients[i], 1, 0, NULL);
		UIN2IDT(chat->recipients[i], id);
		if (hContact && (name = pcli->pfnGetContactDisplayName(hContact, 0)) != NULL)
			gce.ptszNick = name;
		else
			gce.ptszNick = TranslateT("'Unknown'");
		gce.bIsMe = 0;
		gce.dwFlags = 0;
		debugLog(_T("gc_getchat(): Added %s: %s (%s) to the list..."), gce.ptszUID, gce.ptszNick, gce.ptszStatus);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}
	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

	debugLog(_T("gc_getchat(): Returning new chat window %s, count %d."), chat->id, chat->recipients_count);
	list_add(&chats, chat, 0);
	return chat->id;
}

static MCONTACT gg_getsubcontact(GGPROTO* gg, MCONTACT hContact)
{
	char* szProto = GetContactProto(hContact);
	if (szProto && !mir_strcmp(szProto, META_PROTO)) {
		int nSubContacts = db_mc_getSubCount(hContact), i;
		for (i = 0; i < nSubContacts; i++) {
			MCONTACT hMetaContact = db_mc_getSub(hContact, i);
			szProto = GetContactProto(hMetaContact);
			if (szProto && !mir_strcmp(szProto, gg->m_szModuleName))
				return hMetaContact;
		}
	}
	return NULL;
}

static void gg_gc_resetclistopts(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, TRUE, 0);
}

static int gg_gc_countcheckmarks(HWND hwndList)
{
	int count = 0;
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
			count++;
	}
	return count;
}

#define HM_SUBCONTACTSCHANGED (WM_USER + 100)

static INT_PTR CALLBACK gg_gc_openconfdlg(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			CLCINFOITEM cii = {0};
			HANDLE hMetaContactsEvent;

			SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)lParam);
			TranslateDialogDefault(hwndDlg);
			WindowSetIcon(hwndDlg, "conference");
			gg_gc_resetclistopts(GetDlgItem(hwndDlg, IDC_CLIST));

			// Hook MetaContacts event (if available)
			hMetaContactsEvent = HookEventMessage(ME_MC_SUBCONTACTSCHANGED, hwndDlg, HM_SUBCONTACTSCHANGED);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hMetaContactsEvent);
		}
		return TRUE;

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);
					GGPROTO* gg = (GGPROTO*)GetWindowLongPtr(hwndDlg, DWLP_USER);
					int count = 0, i = 0;
					// Check if connected
					if (!gg->isonline())
					{
						MessageBox(NULL,
							TranslateT("You have to be connected to open new conference."),
							gg->m_tszUserName, MB_OK | MB_ICONSTOP);
					}
					else if (hwndList && (count = gg_gc_countcheckmarks(hwndList)) >= 2)
					{
						// Create new participiants table
						TCHAR* chat;
						uin_t* participants = (uin_t*)calloc(count, sizeof(uin_t));
						gg->debugLogA("gg_gc_openconfdlg(): WM_COMMAND IDOK Opening new conference for %d contacts.", count);
						for (MCONTACT hContact = db_find_first(); hContact && i < count; hContact = db_find_next(hContact)) {
							HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
							if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
								MCONTACT hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
								participants[i++] = db_get_dw(hMetaContact ? hMetaContact : hContact, gg->m_szModuleName, GG_KEY_UIN, 0);
							}
						}
						if (count > i) i = count;
						chat = gg->gc_getchat(0, participants, count);
						if (chat)
						{
							GCDEST gcd = { gg->m_szModuleName, chat, GC_EVENT_CONTROL };
							GCEVENT gce = { sizeof(gce), &gcd };
							CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
						}
						free(participants);
					}
				}

				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
			}
			break;
		}

		case WM_NOTIFY:
		{
			switch(((NMHDR*)lParam)->idFrom)
			{
				case IDC_CLIST:
				{
					switch(((NMHDR*)lParam)->code)
					{
						case CLN_OPTIONSCHANGED:
							gg_gc_resetclistopts(GetDlgItem(hwndDlg, IDC_CLIST));
							break;

						case CLN_NEWCONTACT:
						case CLN_CONTACTMOVED:
						case CLN_LISTREBUILT:
						{
							char* szProto;
							uin_t uin;
							GGPROTO* gg = (GGPROTO*)GetWindowLongPtr(hwndDlg, DWLP_USER);

							if (!gg) break;

							// Delete non-gg contacts
							for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
								MCONTACT hItem = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, hContact, 0);
								if (hItem) {
									MCONTACT hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
									if (hMetaContact) {
										szProto = gg->m_szModuleName;
										uin = (uin_t)gg->getDword(hMetaContact, GG_KEY_UIN, 0);
									}
									else {
										szProto = GetContactProto(hContact);
										uin = (uin_t)gg->getDword(hContact, GG_KEY_UIN, 0);
									}

									if (szProto == NULL || mir_strcmp(szProto, gg->m_szModuleName) || !uin || uin == gg->getDword(GG_KEY_UIN, 0))
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
			}
			break;
		}

		case HM_SUBCONTACTSCHANGED:
		{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);
			SendMessage(hwndList, CLM_AUTOREBUILD, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), gg_gc_countcheckmarks(hwndList) >= 2);
			break;
		}

		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;

		case WM_DESTROY:
		{
			HANDLE hMetaContactsEvent = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (hMetaContactsEvent) UnhookEvent(hMetaContactsEvent);
			WindowFreeIcon(hwndDlg);
			break;
		}
	}

	return FALSE;
}

INT_PTR GGPROTO::gc_clearignored(WPARAM wParam, LPARAM lParam)
{
	list_t l = chats; BOOL cleared = FALSE;
	while(l)
	{
		GGGC *chat = (GGGC *)l->data;
		l = l->next;
		if (chat->ignore)
		{
			free(chat->recipients);
			list_remove(&chats, chat, 1);
			cleared = TRUE;
		}
	}
	MessageBox( NULL,
		cleared ?
			TranslateT("All ignored conferences are now unignored and the conference policy will act again.") :
			TranslateT("There are no ignored conferences."),
		m_tszUserName, MB_OK | MB_ICONINFORMATION
	);

	return 0;
}

INT_PTR GGPROTO::gc_openconf(WPARAM wParam, LPARAM lParam)
{
	// Check if connected
	if (!isonline())
	{
		MessageBox(NULL,
			TranslateT("You have to be connected to open new conference."),
			m_tszUserName, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CONFERENCE), NULL, gg_gc_openconfdlg, (LPARAM)this);
	return 1;
}

int GGPROTO::gc_changenick(MCONTACT hContact, TCHAR *ptszNick)
{
	list_t l;
	uin_t uin = getDword(hContact, GG_KEY_UIN, 0);
	if (!uin || !ptszNick) return 0;

	debugLogA("gc_changenick(): Nickname for uin %d changed. Lookup for chats having this nick", uin);
	// Lookup for chats having this nick
	for(l = chats; l; l = l->next) {
		GGGC *chat = (GGGC *)l->data;
		if (chat->recipients && chat->recipients_count)
			for(int i = 0; i < chat->recipients_count; i++)
				// Rename this window if it's exising in the chat
				if (chat->recipients[i] == uin)
				{
					TCHAR id[32];
					UIN2IDT(uin, id);
					
					GCDEST gcd = { m_szModuleName, chat->id, GC_EVENT_NICK };
					GCEVENT gce = { sizeof(gce), &gcd };
					gce.pDest = &gcd;
					gce.ptszUID = id;
					gce.ptszText = ptszNick;

					debugLog(_T("gc_changenick(): Found room %s with uin %d, sending nick change %s."), chat->id, uin, id);
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

					break;
				}
	}

	return 1;
}
