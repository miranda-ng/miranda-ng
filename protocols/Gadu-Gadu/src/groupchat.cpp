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

#define GG_GC_GETCHAT "%s/GCGetChat"
#define GGS_OPEN_CONF "%s/OpenConf"
#define GGS_CLEAR_IGNORED "%s/ClearIgnored"

////////////////////////////////////////////////////////////////////////////////
// Inits Gadu-Gadu groupchat module using chat.dll

int GGPROTO::gc_init()
{
	if (ServiceExists(MS_GC_REGISTER))
	{
		char service[64];
		GCREGISTER gcr = {0};

		// Register Gadu-Gadu proto
		gcr.cbSize = sizeof(GCREGISTER);
		gcr.dwFlags = GC_TCHAR;
		gcr.iMaxText = 0;
		gcr.nColors = 0;
		gcr.pColors = 0;
		gcr.ptszModuleDispName = m_tszUserName;
		gcr.pszModule = m_szModuleName;
		CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);
		hookProtoEvent(ME_GC_EVENT, &GGPROTO::gc_event);
		gc_enabled = TRUE;
		// create & hook event
		mir_snprintf(service, 64, GG_GC_GETCHAT, m_szModuleName);
		netlog("gg_gc_init(): Registered with groupchat plugin.");
	}
	else
		netlog("gg_gc_init(): Cannot register with groupchat plugin !!!");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Groupchat menus initialization

void GGPROTO::gc_menus_init(HGENMENU hRoot)
{
	if (gc_enabled)
	{
		CLISTMENUITEM mi = {0};
		char service[64];

		mi.cbSize = sizeof(mi);
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTHANDLE;
		mi.hParentMenu = hRoot;

		// Conferencing
		mir_snprintf(service, sizeof(service), GGS_OPEN_CONF, m_szModuleName);
		createObjService(service, &GGPROTO::gc_openconf);
		mi.position = 2000050001;
		mi.icolibItem = GetIconHandle(IDI_CONFERENCE);
		mi.pszName = LPGEN("Open &conference...");
		mi.pszService = service;
		hMainMenu[0] = Menu_AddProtoMenuItem(&mi);

		// Clear ignored conferences
		mir_snprintf(service, sizeof(service), GGS_CLEAR_IGNORED, m_szModuleName);
		createObjService(service, &GGPROTO::gc_clearignored);
		mi.position = 2000050002;
		mi.icolibItem = GetIconHandle(IDI_CLEAR_CONFERENCE);
		mi.pszName = LPGEN("&Clear ignored conferences");
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
		if (chat->recipients) free(chat->recipients);
	}
	list_destroy(chats, 1); chats = NULL;
	return 1;
}

GGGC* GGPROTO::gc_lookup(char *id)
{
	GGGC *chat;
	list_t l;

	for(l = chats; l; l = l->next)
	{
		chat = (GGGC *)l->data;
		if (chat && !strcmp(chat->id, id))
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
		|| !gch->pDest->pszID
		|| !gch->pDest->pszModule
		|| lstrcmpiA(gch->pDest->pszModule, m_szModuleName)
		|| !(uin = db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0))
		|| !(chat = gc_lookup(gch->pDest->pszID)))
		return 0;

	// Window terminated
	if (gch->pDest->iType == SESSION_TERMINATE)
	{
		HANDLE hContact = NULL;
		netlog("gg_gc_event(): Terminating chat %x, id %s from chat window...", chat, gch->pDest->pszID);
		// Destroy chat entry
		free(chat->recipients);
		list_remove(&chats, chat, 1);
		// Remove contact from contact list (duh!) should be done by chat.dll !!
		hContact = db_find_first();
		while (hContact)
		{
			DBVARIANT dbv;
			if (!db_get_s(hContact, m_szModuleName, "ChatRoomID", &dbv, DBVT_ASCIIZ))
			{
				if (dbv.pszVal && !strcmp(gch->pDest->pszID, dbv.pszVal))
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
				DBFreeVariant(&dbv);
			}
			hContact = db_find_next(hContact);
		}
		return 1;
	}

	// Message typed / send only if online
	if (isonline() && (gch->pDest->iType == GC_USER_MESSAGE) && gch->ptszText)
	{
		char id[32];
		DBVARIANT dbv;
		GCDEST gcdest = {m_szModuleName, gch->pDest->pszID, GC_EVENT_MESSAGE};
		GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
		int lc;

		UIN2ID(uin, id);

		gcevent.pszUID = id;
		gcevent.ptszText = gch->ptszText;
		TCHAR* pszValT;
		if (!db_get_s(NULL, m_szModuleName, GG_KEY_NICK, &dbv, DBVT_ASCIIZ)){
			pszValT = mir_a2t(dbv.pszVal);
			gcevent.ptszNick = pszValT;
			DBFreeVariant(&dbv);
		} else {
			gcevent.ptszNick = TranslateT("Me");
		}

		// Get rid of CRLF at back
		lc = (int)_tcslen(gch->ptszText) - 1;
		while(lc >= 0 && (gch->ptszText[lc] == '\n' || gch->ptszText[lc] == '\r')) gch->ptszText[lc --] = 0;
		char* pszText = mir_t2a(gch->ptszText);

		gcevent.time = time(NULL);
		gcevent.bIsMe = 1;
		gcevent.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
		netlog("gg_gc_event(): Sending conference message to room %s, \"%s\".", gch->pDest->pszID, pszText);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
		if (pszValT != NULL) mir_free(pszValT);;
		
		EnterCriticalSection(&sess_mutex);
		gg_send_message_confer(sess, GG_CLASS_CHAT, chat->recipients_count, chat->recipients, (BYTE*)pszText);
		LeaveCriticalSection(&sess_mutex);
		mir_free(pszText);
		
		return 1;
	}

	// Privmessage selected
	if (gch->pDest->iType == GC_USER_PRIVMESS)
	{
		HANDLE hContact = NULL;
		if ((uin = atoi(gch->pszUID)) && (hContact = getcontact(uin, 1, 0, NULL)))
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, (LPARAM)0);
	}
	char* pszText = mir_t2a(gch->ptszText);
	netlog("gg_gc_event(): Unhandled event %d, chat %x, uin %d, text \"%s\".", gch->pDest->iType, chat, uin, pszText);
	mir_free(pszText);

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

char* GGPROTO::gc_getchat(uin_t sender, uin_t *recipients, int recipients_count)
{
	list_t l; int i;
	GGGC *chat;
	char id[32];
	uin_t uin; DBVARIANT dbv;
	GCDEST gcdest = {m_szModuleName, 0, GC_EVENT_ADDGROUP};
	GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};

	netlog("gg_gc_getchat(): Count %d.", recipients_count);
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
					netlog("gg_gc_getchat(): Ignoring existing id %s, size %d.", chat->id, chat->recipients_count);
				else
					netlog("gg_gc_getchat(): Returning existing id %s, size %d.", chat->id, chat->recipients_count);
				return !(chat->ignore) ? chat->id : NULL;
			}
		}
	}

	// Make new uin list to chat mapping
	chat = (GGGC *)malloc(sizeof(GGGC));
	UIN2ID(gc_id ++, chat->id); chat->ignore = FALSE;

	// Check groupchat policy (new) / only for incoming
	if (sender)
	{
		int unknown = (getcontact(sender, 0, 0, NULL) == NULL),
			unknownSender = unknown;
		for(i = 0; i < recipients_count; i++)
			if (!getcontact(recipients[i], 0, 0, NULL))
				unknown ++;
		if ((db_get_w(NULL, m_szModuleName, GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 2) ||
		   (db_get_w(NULL, m_szModuleName, GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 2 &&
			recipients_count >= db_get_w(NULL, m_szModuleName, GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
		   (db_get_w(NULL, m_szModuleName, GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 2 &&
			unknown >= db_get_w(NULL, m_szModuleName, GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN)))
			chat->ignore = TRUE;
		if (!chat->ignore && ((db_get_w(NULL, m_szModuleName, GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 1) ||
		   (db_get_w(NULL, m_szModuleName, GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 1 &&
			recipients_count >= db_get_w(NULL, m_szModuleName, GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
		   (db_get_w(NULL, m_szModuleName, GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 1 &&
			unknown >= db_get_w(NULL, m_szModuleName, GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN))))
		{
			TCHAR *senderName = unknownSender ?
				TranslateT("Unknown") : pcli->pfnGetContactDisplayName(getcontact(sender, 0, 0, NULL), 0);
			TCHAR error[256];
			mir_sntprintf(error, SIZEOF(error), TranslateT("%s has initiated conference with %d participants (%d unknowns).\nDo you want do participate ?"),
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
			netlog("gg_gc_getchat(): Ignoring new chat %s, count %d.", chat->id, chat->recipients_count);
			list_add(&chats, chat, 0);
			return NULL;
		}
	}

	// Create new chat window
	TCHAR status[256];
	TCHAR *senderName = sender ? pcli->pfnGetContactDisplayName(getcontact(sender, 1, 0, NULL), 0) : NULL;
	mir_sntprintf(status, 255, (sender) ? TranslateT("%s initiated the conference.") : TranslateT("This is my own conference."), senderName);
	GCSESSION gcwindow = { 0 };
	gcwindow.cbSize    = sizeof(GCSESSION);
	gcwindow.iType     = GCW_CHATROOM;
	gcwindow.pszModule = m_szModuleName;
	gcwindow.ptszName  = sender ? senderName : TranslateT("Conference");
	gcwindow.pszID     = chat->id;
	gcwindow.dwFlags   = GC_TCHAR;
	gcwindow.dwItemData = (DWORD)chat;
	gcwindow.ptszStatusbarText = status;

	// Here we put nice new hash sign
	TCHAR *name = (TCHAR*)calloc(_tcslen(gcwindow.ptszName) + 2, sizeof(TCHAR));
	*name = '#'; _tcscpy(name + 1, gcwindow.ptszName);
	gcwindow.ptszName = name;
	// Create new room
	if (CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM) &gcwindow))
	{
		netlog("gg_gc_getchat(): Cannot create new chat window %s.", chat->id);
		free(name);
		free(chat);
		return NULL;
	}
	free(name);

	gcdest.pszID = chat->id;
	gcevent.pszUID = id;
	gcevent.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
	gcevent.time = 0;

	// Add normal group
	gcevent.ptszStatus = TranslateT("Participants");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
	gcdest.iType = GC_EVENT_JOIN;

	// Add myself
	if (uin = db_get_dw(NULL, m_szModuleName, GG_KEY_UIN, 0))
	{
		UIN2ID(uin, id);
		if (!db_get_s(NULL, m_szModuleName, GG_KEY_NICK, &dbv, DBVT_ASCIIZ)) {
			gcevent.ptszNick = mir_a2t(dbv.pszVal);
			db_free(&dbv);
		}
		else gcevent.ptszNick = TranslateT("Me");
		gcevent.bIsMe = 1;
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
		netlog("gg_gc_getchat(): Myself %s: %S (%S) to the list...", gcevent.pszUID, gcevent.ptszNick, gcevent.ptszStatus);
	}
	else netlog("gg_gc_getchat(): Myself adding failed with uin %d !!!", uin);

	// Copy recipient list
	chat->recipients_count = recipients_count + (sender ? 1 : 0);
	chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
	for(i = 0; i < recipients_count; i++)
		chat->recipients[i] = recipients[i];
	if (sender) chat->recipients[i] = sender;

	// Add contacts
	for(i = 0; i < chat->recipients_count; i++) {
		HANDLE hContact = getcontact(chat->recipients[i], 1, 0, NULL);
		UIN2ID(chat->recipients[i], id);
		if (hContact && (name = pcli->pfnGetContactDisplayName(hContact, 0)) != NULL)
			gcevent.ptszNick = name;
		else
			gcevent.ptszNick = TranslateT("'Unknown'");
		gcevent.bIsMe = 0;
		gcevent.dwFlags = GC_TCHAR;
		netlog("gg_gc_getchat(): Added %s: %S (%S) to the list...", gcevent.pszUID, gcevent.ptszNick, gcevent.pszStatus);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
	}
	gcdest.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gcevent);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gcevent);

	netlog("gg_gc_getchat(): Returning new chat window %s, count %d.", chat->id, chat->recipients_count);
	list_add(&chats, chat, 0);
	return chat->id;
}

static HANDLE gg_getsubcontact(GGPROTO* gg, HANDLE hContact)
{
	char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char* szMetaProto = (char*)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	if (szProto && szMetaProto && (INT_PTR)szMetaProto != CALLSERVICE_NOTFOUND && !lstrcmpA(szProto, szMetaProto))
	{
		int nSubContacts = (int)CallService(MS_MC_GETNUMCONTACTS, (WPARAM)hContact, 0), i;
		HANDLE hMetaContact;
		for (i = 0; i < nSubContacts; i++)
		{
			hMetaContact = (HANDLE)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hContact, i);
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hMetaContact, 0);
			if (szProto && !lstrcmpA(szProto, gg->m_szModuleName))
				return hMetaContact;
		}
	}
	return NULL;
}

static void gg_gc_resetclistopts(HWND hwndList)
{
	int i;
	SendMessage(hwndList, CLM_SETLEFTMARGIN, 2, 0);
	SendMessage(hwndList, CLM_SETBKBITMAP, 0, (LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList, CLM_SETBKCOLOR, GetSysColor(COLOR_WINDOW), 0);
	SendMessage(hwndList, CLM_SETGREYOUTFLAGS, 0, 0);
	SendMessage(hwndList, CLM_SETINDENT, 10, 0);
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, (WPARAM)TRUE, 0);
	for (i = 0; i <= FONTID_MAX; i++)
		SendMessage(hwndList, CLM_SETTEXTCOLOR, i, GetSysColor(COLOR_WINDOWTEXT));
}

static int gg_gc_countcheckmarks(HWND hwndList)
{
	int count = 0;
	HANDLE hItem, hContact = db_find_first();
	while (hContact)
	{
		hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
		if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
			count++;
		hContact = db_find_next(hContact);
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
						char* chat;
						uin_t* participants = (uin_t*)calloc(count, sizeof(uin_t));
						HANDLE hItem, hContact = db_find_first();
						gg->netlog("gg_gc_getchat(): Opening new conference for %d contacts.", count);
						while (hContact && i < count)
						{
							hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
							if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
							{
								HANDLE hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
								participants[i++] = db_get_dw(hMetaContact ? hMetaContact : hContact, gg->m_szModuleName, GG_KEY_UIN, 0);
							}
							hContact = db_find_next(hContact);
						}
						if (count > i) i = count;
						chat = gg->gc_getchat(0, participants, count);
						if (chat)
						{
							GCDEST gcdest = {gg->m_szModuleName, chat, GC_EVENT_CONTROL};
							GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
							CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gcevent);
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
							HANDLE hContact;
							HANDLE hItem;
							char* szProto;
							uin_t uin;
							GGPROTO* gg = (GGPROTO*)GetWindowLongPtr(hwndDlg, DWLP_USER);

							if (!gg) break;

							// Delete non-gg contacts
							hContact = db_find_first();
							while (hContact)
							{
								hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, (WPARAM)hContact, 0);
								if (hItem)
								{
									HANDLE hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
									if (hMetaContact)
									{
										szProto = gg->m_szModuleName;
										uin = (uin_t)db_get_dw(hMetaContact, gg->m_szModuleName, GG_KEY_UIN, 0);
									}
									else
									{
										szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
										uin = (uin_t)db_get_dw(hContact, gg->m_szModuleName, GG_KEY_UIN, 0);
									}

									if (szProto == NULL || lstrcmpA(szProto, gg->m_szModuleName) || !uin || uin == db_get_dw(NULL, gg->m_szModuleName, GG_KEY_UIN, 0))
										SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_DELETEITEM, (WPARAM)hItem, 0);
								}
								hContact = db_find_next(hContact);
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
			if (chat->recipients) free(chat->recipients);
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

int GGPROTO::gc_changenick(HANDLE hContact, char *pszNick)
{
	list_t l;
	uin_t uin = db_get_dw(hContact, m_szModuleName, GG_KEY_UIN, 0);
	if (!uin || !pszNick) return 0;

	netlog("gg_gc_changenick(): Nickname for uin %d changed to %s.", uin, pszNick);
	// Lookup for chats having this nick
	for(l = chats; l; l = l->next) {
		GGGC *chat = (GGGC *)l->data;
		if (chat->recipients && chat->recipients_count)
			for(int i = 0; i < chat->recipients_count; i++)
				// Rename this window if it's exising in the chat
				if (chat->recipients[i] == uin)
				{
					char id[32];
					GCEVENT gce = {sizeof(GCEVENT)};
					GCDEST gcd;

					UIN2ID(uin, id);
					gcd.iType = GC_EVENT_NICK;
					gcd.pszModule = m_szModuleName;
					gce.pDest = &gcd;
					gcd.pszID = chat->id;
					gce.pszUID = id;
					gce.pszText = pszNick;
					netlog("gg_gc_changenick(): Found room %s with uin %d, sending nick change %s.", chat->id, uin, id);
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

					break;
				}
	}

	return 1;
}
