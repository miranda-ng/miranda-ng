/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

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

#include "tlen.h"
#include "tlen_list.h"
#include "tlen_iq.h"
#include "resource.h"
#include "tlen_muc.h"

static int TlenMUCHandleEvent(void *ptr, WPARAM wParam, LPARAM lParam);
static int TlenMUCQueryContacts(TlenProtocol *proto, const char *roomId);
static int TlenMUCSendInvitation(TlenProtocol *proto, const char *roomID, const char *user);
static int TlenMUCSendPresence(TlenProtocol *proto, const char *roomID, const char *nick, int desiredStatus);
static int TlenMUCSendQuery(TlenProtocol *proto, int type, const char *parent, int page);

static int isSelf(TlenProtocol *proto, const char *roomID, const char *nick)
{
	TLEN_LIST_ITEM *item;
	int result;
	result=0;
	item = TlenListGetItemPtr(proto, LIST_CHATROOM, roomID);
	if (item != NULL) {
		if (item->nick == NULL) {
			if (!strcmp(nick, proto->threadData->username)) result = 1;
		} else if (nick[0] == '~') {
			if (!strcmp(nick+1, item->nick)) {
				result = 1;
			}
		}
	}
	return result;
}

static int stringToHex(const char *str)
{
	int i, val;
	val = 0;
	for (i=0;i<2;i++) {
		val <<= 4;
		if (str[i] >= 'A' && str[i] <= 'F') {
			val += 10 + str[i]-'A';
		} else if (str[i] >= '0' && str[i] <= '9') {
			val += str[i]-'0';
		}
	}
	return val;

}
static char *getDisplayName(TlenProtocol *proto, const char *id)
{
	char jid[256];
	MCONTACT hContact;
	DBVARIANT dbv;
	if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
		mir_snprintf(jid, SIZEOF(jid), "%s@%s", id, dbv.pszVal);
		db_free(&dbv);
		if (((hContact=TlenHContactFromJID(proto, jid)) != NULL) || !strcmp(id, proto->threadData->username)) {
			CONTACTINFO ci = { sizeof(ci) };
			ci.hContact = hContact;
			ci.szProto = (char *)proto->m_szModuleName;
			ci.dwFlag = CNF_DISPLAY;
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
				if (ci.type == CNFT_ASCIIZ) {
					if (ci.pszVal) {
						char* str = mir_t2a(ci.pszVal);
						mir_free(ci.pszVal);
						return str;
					}
				}
			}
		}
	}
	return mir_strdup(id);
}

int TlenMUCRecvInvitation(TlenProtocol *proto, const char *roomId, const char *roomName, const char *from, const char *reason)
{
	int	 ignore, ask, groupChatPolicy;
	if (roomId == NULL) return 1;
	groupChatPolicy = db_get_w(NULL, proto->m_szModuleName, "GroupChatPolicy", 0);
	ask = TRUE;
	ignore = FALSE;
	if (groupChatPolicy == TLEN_MUC_ASK) {
		ignore = FALSE;
		ask = TRUE;
	} else if (groupChatPolicy == TLEN_MUC_IGNORE_ALL) {
		ignore = TRUE;
	} else if (groupChatPolicy == TLEN_MUC_IGNORE_NIR) {
		char jid[256];
		DBVARIANT dbv;
		if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
			mir_snprintf(jid, SIZEOF(jid), "%s@%s", from, dbv.pszVal);
			db_free(&dbv);
		} else {
			strcpy(jid, from);
		}
		ignore = !IsAuthorized(proto, jid);
		ask = TRUE;
	} else if (groupChatPolicy == TLEN_MUC_ACCEPT_IR) {
		char jid[256];
		TLEN_LIST_ITEM *item;
		DBVARIANT dbv;
		if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
			mir_snprintf(jid, SIZEOF(jid), "%s@%s", from, dbv.pszVal);
			db_free(&dbv);
		} else {
			strcpy(jid, from);
		}
		item = TlenListGetItemPtr(proto, LIST_ROSTER, jid);
		ask = !IsAuthorized(proto, jid);
		ignore = FALSE;
	} else if (groupChatPolicy == TLEN_MUC_ACCEPT_ALL) {
		ask = FALSE;
		ignore = FALSE;
	}
	return 0;
}

static int TlenMUCSendInvitation(TlenProtocol *proto, const char *roomID, const char *user)
{
	if (!proto->isOnline) {
		return 1;
	}
	TlenSend(proto, "<m to='%s'><x><inv to='%s'><r></r></inv></x></m>", roomID, user);
	return 0;
}

static int TlenMUCSendPresence(TlenProtocol *proto, const char *roomID, const char *nick, int desiredStatus)
{
	char str[512];
	char *jid;
	TLEN_LIST_ITEM *item;
	if (!proto->isOnline) {
		return 1;
	}
	if (nick != NULL) {
		mir_snprintf(str, SIZEOF(str), "%s/%s", roomID, nick);
	} else {
		mir_snprintf(str, SIZEOF(str), "%s", roomID);
	}
	if ((jid = TlenTextEncode(str)) != NULL) {
		switch (desiredStatus) {
			case ID_STATUS_ONLINE:
				TlenSend(proto, "<p to='%s'/>", jid);
				item = TlenListGetItemPtr(proto, LIST_CHATROOM, roomID);
				if (item != NULL) {
					if (item->nick != NULL) mir_free(item->nick);
					item->nick = NULL;
					if (nick != NULL) {
						item->nick = mir_strdup(nick);
					}
				}
				break;
			default:
				item = TlenListGetItemPtr(proto, LIST_CHATROOM, roomID);
				if (item != NULL) {
					TlenSend(proto, "<p to='%s'><s>unavailable</s></p>", jid);
					TlenListRemove(proto, LIST_CHATROOM, roomID);
				}
				break;
		}
		mir_free(jid);
	}
	return 0;
}

static int TlenMUCSendQuery(TlenProtocol *proto, int type, const char *parent, int page)
{
	if (!proto->isOnline) {
		return 1;
	}
	if (type == 3) { // find chat room by name
		char serialId[32];
		TLEN_LIST_ITEM *item;
		mir_snprintf(serialId, SIZEOF(serialId), TLEN_IQID"%d", TlenSerialNext(proto));
		item = TlenListAdd(proto, LIST_SEARCH, serialId);
		item->roomName = mir_strdup(parent);
		TlenSend(proto, "<iq to='c' type='3' n='%s' id='%s'/>", parent, serialId);
	} else {
		if (parent == NULL) {
			TlenSend(proto, "<iq to='c' type='%d'/>", type);
		} else { // 1 - groups, 2 - chat rooms, 7 - user nicks, 8 - user rooms
			if (type == 1 || (type == 2 && page == 0) || type == 7 || type == 8) {
				TlenSend(proto, "<iq to='c' type='%d' p='%s'/>", type, parent);
			} else if (type == 2) {
				TlenSend(proto, "<iq to='c' type='%d' p='%s' n='%d'/>", type, parent, page);
			} else if (type == 6) {
				if (page) {
					TlenSend(proto, "<iq to='c' type='%d' n='%s' k='u'/>", type, parent);
				} else {
					TlenSend(proto, "<iq to='c' type='%d' n='%s'/>", type, parent);
				}
			} else if (type == 4) { // list of users, admins etc.
				TlenSend(proto, "<iq to='%s' type='%d' k='%d'/>", parent, type, page);
			}
		}
	}
	return 0;
}

INT_PTR TlenProtocol::MUCMenuHandleMUC(WPARAM wParam, LPARAM lParam)
{
	if (!isOnline)
		return 1;

	TlenSend(this, "<p to='c' tp='c' id='"TLEN_IQID"%d'/>", TlenSerialNext(this));
	return 0;
}

INT_PTR TlenProtocol::MUCContactMenuHandleMUC(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact;
	DBVARIANT dbv;
	TLEN_LIST_ITEM *item;
	if (!isOnline)
		return 1;

	if ((hContact=wParam) != NULL && isOnline) {
		if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
			char serialId[32];
			mir_snprintf(serialId, SIZEOF(serialId), TLEN_IQID"%d", TlenSerialNext(this));
			item = TlenListAdd(this, LIST_INVITATIONS, serialId);
			item->nick = mir_strdup(dbv.pszVal);
			TlenSend(this, "<p to='c' tp='c' id='%s'/>", serialId);
			db_free(&dbv);
		}
	}
	return 0;
}
