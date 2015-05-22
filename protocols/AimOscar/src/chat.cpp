/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2009 Boris Krasnovskiy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"

static const COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void CAimProto::chat_register(void)
{
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.nColors = 16;
	gcr.pColors = (COLORREF*)crCols;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CAimProto::OnGCEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &CAimProto::OnGCMenuHook );
}

void CAimProto::chat_start(const char* id, unsigned short exchange)
{
	TCHAR* idt = mir_a2t(id);

	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = idt;
	gcw.ptszID = idt;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, idt, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszStatus = TranslateT("Me");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.ptszStatus = TranslateT("Others");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE,   (LPARAM)&gce);

	setWord(find_chat_contact(id), "Exchange", exchange);

	mir_free(idt);
}

void CAimProto::chat_event(const char* id, const char* sn, int evt, const TCHAR* msg)
{
	TCHAR* idt = mir_a2t(id);
	TCHAR* snt = mir_a2t(sn);

	MCONTACT hContact = contact_from_sn(sn);
	TCHAR* nick = hContact ? (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, 
		WPARAM(hContact), GCDNF_TCHAR) : snt;

	GCDEST gcd = { m_szModuleName, idt, evt };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pDest = &gcd;
	gce.ptszNick = nick;
	gce.ptszUID = snt;
	gce.bIsMe = _stricmp(sn, username) == 0;
	gce.ptszStatus = gce.bIsMe ? TranslateT("Me") : TranslateT("Others");
	gce.ptszText = msg;
	gce.time = time(NULL);
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	mir_free(snt);
	mir_free(idt);
}

void CAimProto::chat_leave(const char* id)
{
	TCHAR* idt = mir_a2t(id);

	GCDEST gcd = { m_szModuleName, idt, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.pDest = &gcd;
	CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);

	mir_free(idt);
}


int CAimProto::OnGCEvent(WPARAM, LPARAM lParam) 
{
	GCHOOK *gch = (GCHOOK*) lParam;
	if (!gch) return 1;

	if (mir_strcmp(gch->pDest->pszModule, m_szModuleName)) return 0;

	char* id = mir_t2a(gch->pDest->ptszID);
	chat_list_item* item = find_chat_by_id(id);

	if (item == NULL) return 0;

	switch (gch->pDest->iType) 
	{
		case GC_SESSION_TERMINATE: 
			aim_sendflap(item->hconn,0x04,0,NULL,item->seqno);
			Netlib_Shutdown(item->hconn);
			break;

		case GC_USER_MESSAGE:
			if (gch->ptszText && mir_tstrlen(gch->ptszText)) 
				aim_chat_send_message(item->hconn, item->seqno, T2Utf(gch->ptszText));
			break;

		case GC_USER_CHANMGR: 
			DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, invite_to_chat_dialog, 
				LPARAM(new invite_chat_param(item->id, this)));
			break;

		case GC_USER_PRIVMESS:
			{
				char* sn = mir_t2a(gch->ptszUID);
				MCONTACT hContact = contact_from_sn(sn);
				mir_free(sn);
				CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			}
			break;

		case GC_USER_LOGMENU:
			switch(gch->dwData) 
			{
			case 10:
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, invite_to_chat_dialog, 
					LPARAM(new invite_chat_param(item->id, this)));
				break;

			case 20:
				chat_leave(id);
				break;
			}
			break;
		
		case GC_USER_NICKLISTMENU: 
			{
				char *sn = mir_t2a(gch->ptszUID);
				MCONTACT hContact = contact_from_sn(sn);
				mir_free(sn);

				switch (gch->dwData) 
				{
				case 10:
					CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
					break;

				case 20:
					CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);
					break;

				case 110:
					chat_leave(id);
					break;
				}
			}
			break;

		case GC_USER_TYPNOTIFY: 
			break;
	}
	mir_free(id);

	return 0;
}

int CAimProto::OnGCMenuHook(WPARAM, LPARAM lParam) 
{
	GCMENUITEMS *gcmi= (GCMENUITEMS*)lParam;

	if ( gcmi == NULL || _stricmp(gcmi->pszModule, m_szModuleName )) return 0;

	if ( gcmi->Type == MENU_ON_LOG ) 
	{
		static const struct gc_item Items[] = {
			{ TranslateT("&Invite user..."), 10, MENU_ITEM, FALSE },
			{ TranslateT("&Leave chat session"), 20, MENU_ITEM, FALSE }
		};
		gcmi->nItems = SIZEOF(Items);
		gcmi->Item = (gc_item*)Items;
	}
	else if ( gcmi->Type == MENU_ON_NICKLIST ) 
	{
		char* sn = mir_t2a(gcmi->pszUID);
		if ( !mir_strcmp(username, sn)) 
		{
			static const struct gc_item Items[] = {
				{ TranslateT("User &details"), 10, MENU_ITEM, FALSE },
				{ TranslateT("User &history"), 20, MENU_ITEM, FALSE },
				{ _T(""), 100, MENU_SEPARATOR, FALSE },
				{ TranslateT("&Leave chat session"), 110, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		else {
			static const struct gc_item Items[] = {
				{ TranslateT("User &details"), 10, MENU_ITEM, FALSE },
				{ TranslateT("User &history"), 20, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}	
		mir_free(sn);
	}

	return 0;
}


void   __cdecl CAimProto::chatnav_request_thread( void* param )
{
	chatnav_param* par = (chatnav_param*)param;

	if (wait_conn(hChatNavConn, hChatNavEvent, 0x0d))
	{
		if (par->isroom)
			aim_chatnav_create(hChatNavConn, chatnav_seqno, par->id, par->exchange);
		else
			aim_chatnav_room_info(hChatNavConn, chatnav_seqno, par->id, par->exchange, par->instance);
	}
	delete par;
}

chat_list_item* CAimProto::find_chat_by_cid(unsigned short cid)
{
	chat_list_item* item = NULL;
	for(int i=0; i<chat_rooms.getCount(); ++i)
	{
		if (chat_rooms[i].cid == cid)
		{
			item = &chat_rooms[i];
			break;
		}
	}
	return item;
}

chat_list_item* CAimProto::find_chat_by_id(char* id)
{
	chat_list_item* item = NULL;
	for(int i=0; i<chat_rooms.getCount(); ++i)
	{
		if (mir_strcmp(chat_rooms[i].id, id) == 0)
		{
			item = &chat_rooms[i];
			break;
		}
	}
	return item;
}

chat_list_item* CAimProto::find_chat_by_conn(HANDLE conn)
{
	chat_list_item* item = NULL;
	for(int i=0; i<chat_rooms.getCount(); ++i)
	{
		if (chat_rooms[i].hconn == conn) 
		{
			item = &chat_rooms[i];
			break;
		}
	}
	return item;
}

void CAimProto::remove_chat_by_ptr(chat_list_item* item)
{
	for(int i=0; i<chat_rooms.getCount(); ++i)
	{
		if (&chat_rooms[i] == item) 
		{
			chat_rooms.remove(i);
			break;
		}
	}
}

void CAimProto::shutdown_chat_conn(void)
{
	for(int i=0; i<chat_rooms.getCount(); ++i)
	{
		chat_list_item& item = chat_rooms[i];
		if (item.hconn)
		{
			aim_sendflap(item.hconn,0x04,0,NULL,item.seqno);
			Netlib_Shutdown(item.hconn);
		}
	}
}

void CAimProto::close_chat_conn(void)
{
	for(int i=0; i<chat_rooms.getCount(); ++i)
	{
		chat_list_item& item = chat_rooms[i];
		if (item.hconn)
		{
			Netlib_CloseHandle(item.hconn);
			item.hconn = NULL;
		}
	}
}