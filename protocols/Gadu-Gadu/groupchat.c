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

int gg_gc_event(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
INT_PTR gg_gc_openconf(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
INT_PTR gg_gc_clearignored(GGPROTO *gg, WPARAM wParam, LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////
// Inits Gadu-Gadu groupchat module using chat.dll
int gg_gc_init(GGPROTO *gg)
{
	if(ServiceExists(MS_GC_REGISTER))
	{
		char service[64];
		GCREGISTER gcr = {0};

		// Register Gadu-Gadu proto
		gcr.cbSize = sizeof(GCREGISTER);
		gcr.dwFlags = GC_TCHAR;
		gcr.iMaxText = 0;
		gcr.nColors = 0;
		gcr.pColors = 0;
		gcr.ptszModuleDispName = gg->proto.m_tszUserName;
		gcr.pszModule = GG_PROTO;
		CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);
		gg->hookGCUserEvent = HookProtoEvent(ME_GC_EVENT, gg_gc_event, gg);
		gg->gc_enabled = TRUE;
		// create & hook event
		mir_snprintf(service, 64, GG_GC_GETCHAT, GG_PROTO);
		gg_netlog(gg, "gg_gc_init(): Registered with groupchat plugin.");
	}
	else
		gg_netlog(gg, "gg_gc_init(): Cannot register with groupchat plugin !!!");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Groupchat menus initialization
void gg_gc_menus_init(GGPROTO *gg, HGENMENU hRoot)
{
	if(gg->gc_enabled)
	{
		CLISTMENUITEM mi = {0};
		char service[64];

		mi.cbSize = sizeof(mi);
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTHANDLE;
		mi.hParentMenu = hRoot;

		// Conferencing
		mir_snprintf(service, sizeof(service), GGS_OPEN_CONF, GG_PROTO);
		CreateProtoServiceFunction(service, gg_gc_openconf, gg);
		mi.position = 2000050001;
		mi.icolibItem = GetIconHandle(IDI_CONFERENCE);
		mi.pszName = LPGEN("Open &conference...");
		mi.pszService = service;
		gg->hMainMenu[0] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

		// Clear ignored conferences
		mir_snprintf(service, sizeof(service), GGS_CLEAR_IGNORED, GG_PROTO);
		CreateProtoServiceFunction(service, gg_gc_clearignored, gg);
		mi.position = 2000050002;
		mi.icolibItem = GetIconHandle(IDI_CLEAR_CONFERENCE);
		mi.pszName = LPGEN("&Clear ignored conferences");
		mi.pszService = service;
		gg->hMainMenu[1] = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Releases Gadu-Gadu groupchat module using chat.dll
int gg_gc_destroy(GGPROTO *gg)
{
	list_t l;
	for(l = gg->chats; l; l = l->next)
	{
		GGGC *chat = (GGGC *)l->data;
		if(chat->recipients) free(chat->recipients);
	}
	list_destroy(gg->chats, 1); gg->chats = NULL;
	LocalEventUnhook(gg->hookGCUserEvent);
	LocalEventUnhook(gg->hookGCMenuBuild);

	return 1;
}

GGGC *gg_gc_lookup(GGPROTO *gg, char *id)
{
	GGGC *chat;
	list_t l;

	for(l = gg->chats; l; l = l->next)
	{
		chat = (GGGC *)l->data;
		if(chat && !strcmp(chat->id, id))
			return chat;
	}

	return NULL;
}

int gg_gc_event(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK *)lParam;
	GGGC *chat = NULL;
	uin_t uin;

	// Check if we got our protocol, and fields are set
	if(!gch
		|| !gch->pDest
		|| !gch->pDest->pszID
		|| !gch->pDest->pszModule
		|| lstrcmpi(gch->pDest->pszModule, GG_PROTO)
		|| !(uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
		|| !(chat = gg_gc_lookup(gg, gch->pDest->pszID)))
		return 0;

	// Window terminated
	if(gch->pDest->iType == SESSION_TERMINATE)
	{
		HANDLE hContact = NULL;
		gg_netlog(gg, "gg_gc_event(): Terminating chat %x, id %s from chat window...", chat, gch->pDest->pszID);
		// Destroy chat entry
		free(chat->recipients);
		list_remove(&gg->chats, chat, 1);
		// Remove contact from contact list (duh!) should be done by chat.dll !!
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			DBVARIANT dbv;
			if(!DBGetContactSettingString(hContact, GG_PROTO, "ChatRoomID", &dbv))
			{
				if(dbv.pszVal && !strcmp(gch->pDest->pszID, dbv.pszVal))
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
				DBFreeVariant(&dbv);
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		return 1;
	}

	// Message typed / send only if online
	if(gg_isonline(gg) && (gch->pDest->iType == GC_USER_MESSAGE) && gch->pszText)
	{
		char id[32];
		DBVARIANT dbv;
		GCDEST gcdest = {GG_PROTO, gch->pDest->pszID, GC_EVENT_MESSAGE};
		GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
		int lc;

		UIN2ID(uin, id);

		gcevent.pszUID = id;
		gcevent.pszText = gch->pszText;
		if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_NICK, &dbv))
			gcevent.pszNick = dbv.pszVal;
		else
			gcevent.pszNick = Translate("Me");

		// Get rid of CRLF at back
		lc = (int)strlen(gch->pszText) - 1;
		while(lc >= 0 && (gch->pszText[lc] == '\n' || gch->pszText[lc] == '\r')) gch->pszText[lc --] = 0;
		gcevent.time = time(NULL);
		gcevent.bIsMe = 1;
		gcevent.dwFlags = GCEF_ADDTOLOG;
		gg_netlog(gg, "gg_gc_event(): Sending conference message to room %s, \"%s\".", gch->pDest->pszID, gch->pszText);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
		if(gcevent.pszNick == dbv.pszVal) DBFreeVariant(&dbv);
		EnterCriticalSection(&gg->sess_mutex);
		gg_send_message_confer(gg->sess, GG_CLASS_CHAT, chat->recipients_count, chat->recipients, gch->pszText);
		LeaveCriticalSection(&gg->sess_mutex);
		return 1;
	}

	// Privmessage selected
	if(gch->pDest->iType == GC_USER_PRIVMESS)
	{
		HANDLE hContact = NULL;
		if((uin = atoi(gch->pszUID)) && (hContact = gg_getcontact(gg, uin, 1, 0, NULL)))
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, (LPARAM)0);
	}
	gg_netlog(gg, "gg_gc_event(): Unhandled event %d, chat %x, uin %d, text \"%s\".", gch->pDest->iType, chat, uin, gch->pszText);

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
char *gg_gc_getchat(GGPROTO *gg, uin_t sender, uin_t *recipients, int recipients_count)
{
	list_t l; int i;
	GGGC *chat;
	char *senderName, status[256], *name, id[32];
	uin_t uin; DBVARIANT dbv;
	GCSESSION gcwindow;
	GCDEST gcdest = {GG_PROTO, 0, GC_EVENT_ADDGROUP};
	GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};

	gg_netlog(gg, "gg_gc_getchat(): Count %d.", recipients_count);
	if(!recipients) return NULL;

	// Look for existing chat
	for(l = gg->chats; l; l = l->next)
	{
		GGGC *chat = (GGGC *)l->data;
		if(!chat) continue;

		if(chat->recipients_count == recipients_count + (sender ? 1 : 0))
		{
			int i, j, found = 0, sok = (sender == 0);
			if(!sok) for(i = 0; i < chat->recipients_count; i++)
				if(sender == chat->recipients[i])
				{
					sok = 1;
					break;
				}
			if(sok)
				for(i = 0; i < chat->recipients_count; i++)
					for(j = 0; j < recipients_count; j++)
						if(recipients[j] == chat->recipients[i]) found++;
			// Found all recipients
			if(found == recipients_count)
			{
				if(chat->ignore)
					gg_netlog(gg, "gg_gc_getchat(): Ignoring existing id %s, size %d.", chat->id, chat->recipients_count);
				else
					gg_netlog(gg, "gg_gc_getchat(): Returning existing id %s, size %d.", chat->id, chat->recipients_count);
				return !(chat->ignore) ? chat->id : NULL;
			}
		}
	}

	// Make new uin list to chat mapping
	chat = (GGGC *)malloc(sizeof(GGGC));
	UIN2ID(gg->gc_id ++, chat->id); chat->ignore = FALSE;

	// Check groupchat policy (new) / only for incoming
	if(sender)
	{
		int unknown = (gg_getcontact(gg, sender, 0, 0, NULL) == NULL),
			unknownSender = unknown;
		for(i = 0; i < recipients_count; i++)
			if(!gg_getcontact(gg, recipients[i], 0, 0, NULL))
				unknown ++;
		if((DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 2) ||
		   (DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 2 &&
			recipients_count >= DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
		   (DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 2 &&
			unknown >= DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN)))
			chat->ignore = TRUE;
		if(!chat->ignore && ((DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_DEFAULT, GG_KEYDEF_GC_POLICY_DEFAULT) == 1) ||
		   (DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_TOTAL, GG_KEYDEF_GC_POLICY_TOTAL) == 1 &&
			recipients_count >= DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_TOTAL, GG_KEYDEF_GC_COUNT_TOTAL)) ||
		   (DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_POLICY_UNKNOWN, GG_KEYDEF_GC_POLICY_UNKNOWN) == 1 &&
			unknown >= DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_GC_COUNT_UNKNOWN, GG_KEYDEF_GC_COUNT_UNKNOWN))))
		{
			char *senderName = unknownSender ?
				Translate("Unknown") : ((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) gg_getcontact(gg, sender, 0, 0, NULL), 0));
			char error[256];
			mir_snprintf(error, sizeof(error), Translate("%s has initiated conference with %d participants (%d unknowns).\nDo you want do participate ?"),
				senderName, recipients_count + 1, unknown);
			chat->ignore = (MessageBox(
				NULL,
				error,
				GG_PROTONAME,
				MB_OKCANCEL | MB_ICONEXCLAMATION
				) != IDOK);
		}
		if(chat->ignore)
		{
			// Copy recipient list
			chat->recipients_count = recipients_count + (sender ? 1 : 0);
			chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
			for(i = 0; i < recipients_count; i++)
				chat->recipients[i] = recipients[i];
			if(sender) chat->recipients[i] = sender;
			gg_netlog(gg, "gg_gc_getchat(): Ignoring new chat %s, count %d.", chat->id, chat->recipients_count);
			list_add(&gg->chats, chat, 0);
			return NULL;
		}
	}

	// Create new chat window
	senderName = sender ? (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) gg_getcontact(gg, sender, 1, 0, NULL), 0) : NULL;
	mir_snprintf(status, 255, sender ? Translate("%s initiated the conference.") : Translate("This is my own conference."), senderName);
	gcwindow.cbSize		= sizeof(GCSESSION);
	gcwindow.iType		= GCW_CHATROOM;
	gcwindow.pszModule	= GG_PROTO;
	gcwindow.pszName	= sender ? senderName : Translate("Conference");
	gcwindow.pszID		= chat->id;
	gcwindow.pszStatusbarText = status;
	gcwindow.dwFlags = 0;
	gcwindow.dwItemData = (DWORD)chat;

	// Here we put nice new hash sign
	name = calloc(strlen(gcwindow.pszName) + 2, sizeof(char));
	*name = '#'; strcpy(name + 1, gcwindow.pszName);
	gcwindow.pszName = name;
	// Create new room
	if(CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM) &gcwindow))
	{
		gg_netlog(gg, "gg_gc_getchat(): Cannot create new chat window %s.", chat->id);
		free(name);
		free(chat);
		return NULL;
	}
	free(name);

	gcdest.pszID = chat->id;
	gcevent.pszUID = id;
	gcevent.dwFlags = GCEF_ADDTOLOG;
	gcevent.time = 0;

	// Add normal group
	gcevent.pszStatus = Translate("Participants");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
	gcdest.iType = GC_EVENT_JOIN;

	// Add myself
	if(uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
	{
		UIN2ID(uin, id);
		if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_NICK, &dbv))
			gcevent.pszNick = dbv.pszVal;
		else
			gcevent.pszNick = Translate("Me");
		gcevent.bIsMe = 1;
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
		gg_netlog(gg, "gg_gc_getchat(): Myself %s: %s (%s) to the list...", gcevent.pszUID, gcevent.pszNick, gcevent.pszStatus);
		if(gcevent.pszNick == dbv.pszVal) DBFreeVariant(&dbv);
	}
	else gg_netlog(gg, "gg_gc_getchat(): Myself adding failed with uin %d !!!", uin);

	// Copy recipient list
	chat->recipients_count = recipients_count + (sender ? 1 : 0);
	chat->recipients = (uin_t *)calloc(chat->recipients_count, sizeof(uin_t));
	for(i = 0; i < recipients_count; i++)
		chat->recipients[i] = recipients[i];
	if(sender) chat->recipients[i] = sender;

	// Add contacts
	for(i = 0; i < chat->recipients_count; i++)
	{
		HANDLE hContact = gg_getcontact(gg, chat->recipients[i], 1, 0, NULL);
		UIN2ID(chat->recipients[i], id);
		if(hContact && (name = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, 0)))
			gcevent.pszNick = name;
		else
			gcevent.pszNick = Translate("'Unknown'");
		gcevent.bIsMe = 0;
		gg_netlog(gg, "gg_gc_getchat(): Added %s: %s (%s) to the list...", gcevent.pszUID, gcevent.pszNick, gcevent.pszStatus);
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
	}
	gcdest.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gcevent);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gcevent);

	gg_netlog(gg, "gg_gc_getchat(): Returning new chat window %s, count %d.", chat->id, chat->recipients_count);
	list_add(&gg->chats, chat, 0);
	return chat->id;
}

static HANDLE gg_getsubcontact(GGPROTO* gg, HANDLE hContact)
{
	char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char* szMetaProto = (char*)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	if (szProto && szMetaProto && (INT_PTR)szMetaProto != CALLSERVICE_NOTFOUND && !lstrcmp(szProto, szMetaProto))
	{
		int nSubContacts = (int)CallService(MS_MC_GETNUMCONTACTS, (WPARAM)hContact, 0), i;
		HANDLE hMetaContact;
		for (i = 0; i < nSubContacts; i++)
		{
			hMetaContact = (HANDLE)CallService(MS_MC_GETSUBCONTACT, (WPARAM)hContact, i);
			szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hMetaContact, 0);
			if (szProto && !lstrcmp(szProto, GG_PROTO))
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
	HANDLE hItem, hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
		if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
			count++;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
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
					if (!gg_isonline(gg))
					{
						MessageBox(NULL,
							Translate("You have to be connected to open new conference."),
							GG_PROTONAME, MB_OK | MB_ICONSTOP
						);
					}
					else if (hwndList && (count = gg_gc_countcheckmarks(hwndList)) >= 2)
					{
						// Create new participiants table
						char* chat;
						uin_t* participants = calloc(count, sizeof(uin_t));
						HANDLE hItem, hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
						gg_netlog(gg, "gg_gc_getchat(): Opening new conference for %d contacts.", count);
						while (hContact && i < count)
						{
							hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
							if (hItem && SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
							{
								HANDLE hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
								participants[i++] = DBGetContactSettingDword(hMetaContact ? hMetaContact : hContact, GG_PROTO, GG_KEY_UIN, 0);
							}
							hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
						}
						if (count > i) i = count;
						chat = gg_gc_getchat(gg, 0, participants, count);
						if (chat)
						{
							GCDEST gcdest = {GG_PROTO, chat, GC_EVENT_CONTROL};
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
							hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
							while (hContact)
							{
								hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_FINDCONTACT, (WPARAM)hContact, 0);
								if (hItem)
								{
									HANDLE hMetaContact = gg_getsubcontact(gg, hContact); // MetaContacts support
									if (hMetaContact)
									{
										szProto = GG_PROTO;
										uin = (uin_t)DBGetContactSettingDword(hMetaContact, GG_PROTO, GG_KEY_UIN, 0);
									}
									else
									{
										szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
										uin = (uin_t)DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0);
									}

									if (szProto == NULL || lstrcmp(szProto, GG_PROTO) || !uin || uin == DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0))
										SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_DELETEITEM, (WPARAM)hItem, 0);
								}
								hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
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

INT_PTR gg_gc_clearignored(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	list_t l = gg->chats; BOOL cleared = FALSE;
	while(l)
	{
		GGGC *chat = (GGGC *)l->data;
		l = l->next;
		if(chat->ignore)
		{
			if(chat->recipients) free(chat->recipients);
			list_remove(&gg->chats, chat, 1);
			cleared = TRUE;
		}
	}
	MessageBox(
		NULL,
		cleared ?
			Translate("All ignored conferences are now unignored and the conference policy will act again.") :
			Translate("There are no ignored conferences."),
		GG_PROTONAME,
		MB_OK | MB_ICONINFORMATION
	);

	return 0;
}

INT_PTR gg_gc_openconf(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	// Check if connected
	if (!gg_isonline(gg))
	{
		MessageBox(NULL,
			Translate("You have to be connected to open new conference."),
			GG_PROTONAME, MB_OK | MB_ICONSTOP
		);
		return 0;
	}

	CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CONFERENCE), NULL, gg_gc_openconfdlg, (LPARAM)gg);
	return 1;
}

int gg_gc_changenick(GGPROTO *gg, HANDLE hContact, char *pszNick)
{
	list_t l;
	uin_t uin = DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0);
	if(!uin || !pszNick) return 0;

	gg_netlog(gg, "gg_gc_changenick(): Nickname for uin %d changed to %s.", uin, pszNick);
	// Lookup for chats having this nick
	for(l = gg->chats; l; l = l->next)
	{
		int i;
		GGGC *chat = (GGGC *)l->data;
		if(chat->recipients && chat->recipients_count)
			for(i = 0; i < chat->recipients_count; i++)
				// Rename this window if it's exising in the chat
				if(chat->recipients[i] == uin)
				{
					char id[32];
					GCEVENT gce = {sizeof(GCEVENT)};
					GCDEST gcd;

					UIN2ID(uin, id);
					gcd.iType = GC_EVENT_NICK;
					gcd.pszModule = GG_PROTO;
					gce.pDest = &gcd;
					gcd.pszID = chat->id;
					gce.pszUID = id;
					gce.pszText = pszNick;
					gg_netlog(gg, "gg_gc_changenick(): Found room %s with uin %d, sending nick change %s.", chat->id, uin, id);
					CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

					break;
				}
	}

	return 1;
}
