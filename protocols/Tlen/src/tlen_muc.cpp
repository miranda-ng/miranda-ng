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
static int TlenMUCSendMessage(TlenProtocol *proto, MUCCEVENT *event);
static int TlenMUCSendTopic(TlenProtocol *proto, MUCCEVENT *event);
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
	HCONTACT hContact;
	DBVARIANT dbv;
	if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
		mir_snprintf(jid, sizeof(jid), "%s@%s", id, dbv.pszVal);
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

BOOL TlenMUCInit(TlenProtocol *proto)
{
	proto->HookProtoEvent(ME_MUCC_EVENT, &TlenProtocol::MUCHandleEvent);
	return 0;
}

int TlenProtocol::MUCHandleEvent(WPARAM wParam, LPARAM lParam)
{
	HCONTACT hContact;
	int id;
	MUCCEVENT *mucce=(MUCCEVENT *) lParam;
	if (!strcmp(mucce->pszModule, m_szModuleName)) {
		switch (mucce->iType) {
		case MUCC_EVENT_INVITE:
			TlenMUCSendInvitation(this, mucce->pszID, mucce->pszNick);
			break;
		case MUCC_EVENT_MESSAGE:
			TlenMUCSendMessage(this, mucce);
			break;
		case MUCC_EVENT_TOPIC:
			TlenMUCSendTopic(this, mucce);
			break;
		case MUCC_EVENT_LEAVE:
			TlenMUCSendPresence(this, mucce->pszID, NULL, ID_STATUS_OFFLINE);
			break;
		case MUCC_EVENT_QUERY_GROUPS:
			TlenMUCSendQuery(this, 1, mucce->pszID, 0);
			break;
		case MUCC_EVENT_QUERY_ROOMS:
			TlenMUCSendQuery(this, 2, mucce->pszID, mucce->dwData);
			break;
		case MUCC_EVENT_QUERY_SEARCH:
			TlenMUCSendQuery(this, 3, mucce->pszName, 0);
			break;
		case MUCC_EVENT_QUERY_USERS:
			switch (mucce->dwFlags) {
			case MUCC_EF_USER_OWNER:
				id = 1;
				break;
			case MUCC_EF_USER_ADMIN:
				id = 2;
				break;
			case MUCC_EF_USER_MEMBER:
				id = 3;
				break;
			case MUCC_EF_USER_BANNED:
				id = 4;
				break;
			case MUCC_EF_USER_MODERATOR:
				id = 6;
				break;
			default:
				id = 0;
			}
			TlenMUCSendQuery(this, 4, mucce->pszID, id);
			break;
		case MUCC_EVENT_REGISTER_NICK:
			TlenMUCSendQuery(this, 6, mucce->pszNick, 0);
			break;
		case MUCC_EVENT_REMOVE_NICK:
			TlenMUCSendQuery(this, 6, mucce->pszNick, 1);
			break;
		case MUCC_EVENT_REGISTER_ROOM:
			id = TlenSerialNext(this);
			if (isOnline) {
				if (mucce->pszNick != NULL)
					TlenSend(this, "<p to='c' tp='c' id='"TLEN_IQID"%d' x='%d' n='%s' p='%s' nick='%s'/>", id, mucce->dwFlags | 0x10, mucce->pszName, mucce->pszID);
				else
					TlenSend(this, "<p to='c' tp='c' id='"TLEN_IQID"%d' x='%d' n='%s' p='%s'/>", id, mucce->dwFlags | 0x10, mucce->pszName, mucce->pszID);
			}
			break;
		case MUCC_EVENT_REMOVE_ROOM:
			if (isOnline) {
				TlenSend(this, "<p to='%s' type='d'/>", mucce->pszID);
				TlenListRemove(this, LIST_CHATROOM, mucce->pszID);
				//	TlenMUCSendPresence(mucce->pszID, NULL, ID_STATUS_OFFLINE);
			}
			break;
		case MUCC_EVENT_KICK_BAN:
			if (isOnline) {
				char *nick;
				nick = TlenResourceFromJID(mucce->pszUID);
				if (!isSelf(this, mucce->pszID, nick)) {
					char *reason = TlenTextEncode(mucce->pszText);
					TlenSend(this, "<p to='%s'><x><i i='%s' a='4' ex='%d' rs='%s'/></x></p>", mucce->pszID, nick, mucce->dwData, reason);
					mir_free(reason);
				}
				mir_free(nick);
			}
			break;
		case MUCC_EVENT_UNBAN:
			if (isOnline) {
				char *nick;
				nick = TlenResourceFromJID(mucce->pszUID);
				if (!isSelf(this, mucce->pszID, nick)) {
					TlenSend(this, "<p to='%s'><x><i i='%s' a='0'/></x></p>", mucce->pszID, nick);
				}
				mir_free(nick);
			}
			break;
		case MUCC_EVENT_SET_USER_ROLE:
			if (isOnline) {
				char *nick;
				nick = TlenResourceFromJID(mucce->pszUID);
				if (!isSelf(this, mucce->pszID, nick)) {
					if (mucce->dwFlags == MUCC_EF_USER_ADMIN) {
						id = 2;
					} else if (mucce->dwFlags == MUCC_EF_USER_MEMBER) {
						id = 3;
					} else {
						id = 0;
					}
					TlenSend(this, "<p to='%s'><x><i i='%s' a='%d' /></x></p>", mucce->pszID, nick, id);
				}
				mir_free(nick);
			}
			break;
		case MUCC_EVENT_QUERY_USER_NICKS:
			TlenMUCSendQuery(this, 7, mucce->pszID, 0);
			break;
		case MUCC_EVENT_QUERY_USER_ROOMS:
			TlenMUCSendQuery(this, 8, mucce->pszID, 0);
			break;
		case MUCC_EVENT_QUERY_CONTACTS:
			TlenMUCQueryContacts(this, mucce->pszID);
			break;
		case MUCC_EVENT_JOIN:
			if (isOnline) {
				if (mucce->pszID == NULL || strlen(mucce->pszID) == 0) {
					if (mucce->pszName == NULL || strlen(mucce->pszName) == 0) { // create a new chat room
						id = TlenSerialNext(this);
						TlenSend(this, "<p to='c' tp='c' id='"TLEN_IQID"%d'/>", id);
					}
					else TlenMUCSendQuery(this, 3, mucce->pszName, 0);  // find a chat room by name
				}
				else  // join existing chat room
					if (!TlenMUCCreateWindow(this, mucce->pszID, mucce->pszName, mucce->dwFlags, mucce->pszNick, NULL))
						TlenMUCSendPresence(this, mucce->pszID, mucce->pszNick, ID_STATUS_ONLINE);
			}
			break;
		case MUCC_EVENT_START_PRIV:
			if (isOnline) {
				TLEN_LIST_ITEM *item = TlenListGetItemPtr(this, LIST_CHATROOM, mucce->pszID);
				if (item != NULL) {
					char *nick = TlenResourceFromJID(mucce->pszUID);
					if (!isSelf(this, mucce->pszID, nick)) {
						if (nick[0] == '~' || item->nick != NULL) {
							char str[256];
							mir_snprintf(str, SIZEOF(str), "%s/%s", mucce->pszID, nick);
							hContact = TlenDBCreateContact(this, str, nick, TRUE); //(char *)mucce->pszUID
							db_set_b(hContact, m_szModuleName, "bChat", TRUE);
							CallService(MS_MSG_SENDMESSAGE, (WPARAM) hContact, (LPARAM) NULL);
						}
						else {
							DBVARIANT dbv;
							if (!db_get(NULL, m_szModuleName, "LoginServer", &dbv)) {
								char str[512];
								mir_snprintf(str, sizeof(str), "%s@%s", nick, dbv.pszVal);
								db_free(&dbv);
								hContact = TlenDBCreateContact(this, str, nick, TRUE);
								CallService(MS_MSG_SENDMESSAGE, (WPARAM) hContact, (LPARAM) NULL);
							}
						}
					}
					mir_free(nick);
				}
			}
			break;
		}
	}
	return 0;
}

int TlenMUCRecvInvitation(TlenProtocol *proto, const char *roomId, const char *roomName, const char *from, const char *reason)
{
	char *nick;
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
			mir_snprintf(jid, sizeof(jid), "%s@%s", from, dbv.pszVal);
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
			mir_snprintf(jid, sizeof(jid), "%s@%s", from, dbv.pszVal);
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
	if (!ignore) {
		if (ask) {
			MUCCEVENT mucce;
			mucce.cbSize = sizeof(MUCCEVENT);
			mucce.pszModule = proto->m_szModuleName;
			mucce.pszID = roomId;
			mucce.pszName = roomName;
			mucce.iType = MUCC_EVENT_INVITATION;
			mucce.pszUID = from;
			nick = getDisplayName(proto, from);
			mucce.pszNick = nick;
			mucce.pszText = reason;
			CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
			mir_free(nick);
		} else {
			if (!TlenMUCCreateWindow(proto, roomId, roomName, 0, NULL, NULL)) {
				TlenMUCSendPresence(proto, roomId, NULL, ID_STATUS_ONLINE);
			}
		}
	}
	return 0;
}

int TlenMUCRecvPresence(TlenProtocol *proto, const char *from, int status, int flags, const char *kick)
{
	char str[512];
//	if (TlenListExist(LIST_CHATROOM, from)) {
		char *nick, *roomId, *userId;
		MUCCEVENT mucce;
		roomId = TlenLoginFromJID(from);
		userId = TlenResourceFromJID(from);
		nick = getDisplayName(proto, userId);
		mucce.cbSize = sizeof(MUCCEVENT);
		mucce.pszModule = proto->m_szModuleName;
		mucce.pszID = roomId;
		mucce.iType = MUCC_EVENT_STATUS;
		mucce.pszUID = userId;//from;
		mucce.pszNick = nick;
		mucce.time = time(NULL);
		mucce.bIsMe = isSelf(proto, roomId, userId);
		mucce.dwData = status;
		mucce.dwFlags = 0;
		if (flags & USER_FLAGS_GLOBALOWNER) mucce.dwFlags |= MUCC_EF_USER_GLOBALOWNER;
		if (flags & USER_FLAGS_OWNER) mucce.dwFlags |= MUCC_EF_USER_OWNER;
		if (flags & USER_FLAGS_ADMIN) mucce.dwFlags |= MUCC_EF_USER_ADMIN;
		if (flags & USER_FLAGS_REGISTERED) mucce.dwFlags |= MUCC_EF_USER_REGISTERED;
		if (status == ID_STATUS_OFFLINE && mucce.bIsMe && kick != NULL) {
			mucce.iType = MUCC_EVENT_ERROR;
			mir_snprintf(str, SIZEOF(str), Translate("You have been kicked. Reason: %s "), kick);
			mucce.pszText = str;
		}
		CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
		mir_free(roomId);
		mir_free(userId);
		mir_free(nick);
//	}
	return 0;
}

int TlenMUCRecvMessage(TlenProtocol *proto, const char *from, long timestamp, XmlNode *bodyNode)
{
//	if (TlenListExist(LIST_CHATROOM, from)) {
		char *localMessage;
		char *nick, *style, *roomId, *userId;
		int	 iStyle;
		MUCCEVENT mucce;
		roomId = TlenLoginFromJID(from);
		userId = TlenResourceFromJID(from);
		nick = getDisplayName(proto, userId);
		localMessage = TlenTextDecode(bodyNode->text);
		mucce.cbSize = sizeof(MUCCEVENT);
		mucce.iType = MUCC_EVENT_MESSAGE;
		mucce.pszID = roomId;
		mucce.pszModule = proto->m_szModuleName;
		mucce.pszText = localMessage;
		mucce.pszUID = userId;//from;
		mucce.pszNick = nick;
		mucce.time = timestamp;
		mucce.bIsMe = isSelf(proto, roomId, userId);
		mucce.dwFlags = 0;
		mucce.iFontSize = 0;
		style = TlenXmlGetAttrValue(bodyNode, "f");
		if (style != NULL) {
			iStyle = atoi(style);
			if (iStyle & 1) mucce.dwFlags |= MUCC_EF_FONT_BOLD;
			if (iStyle & 2) mucce.dwFlags |= MUCC_EF_FONT_ITALIC;
			if (iStyle & 4) mucce.dwFlags |= MUCC_EF_FONT_UNDERLINE;
		}
		style = TlenXmlGetAttrValue(bodyNode, "c");
		if (style != NULL && strlen(style) > 5) {
			iStyle = (stringToHex(style)<<16) | (stringToHex(style+2)<<8) | stringToHex(style+4);
		} else {
			iStyle = 0xFFFFFFFF;
		}
		mucce.color = (COLORREF) iStyle;
		style = TlenXmlGetAttrValue(bodyNode, "s");
		if (style != NULL) {
			iStyle = atoi(style);
		} else {
			iStyle = 0;
		}
		mucce.iFontSize = iStyle;
		style = TlenXmlGetAttrValue(bodyNode, "n");
		if (style != NULL) {
			iStyle = atoi(style)-1;
		} else {
			iStyle = 0;
		}
		mucce.iFont = iStyle;
		CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
		mir_free(roomId);
		mir_free(userId);
		mir_free(nick);
		mir_free(localMessage);
//	}
	return 0;
}
int TlenMUCRecvTopic(TlenProtocol *proto, const char *from, const char *subject)
{
//	if (TlenListExist(LIST_CHATROOM, from)) {
		MUCCEVENT mucce;
		mucce.cbSize = sizeof(MUCCEVENT);
		mucce.iType = MUCC_EVENT_TOPIC;
		mucce.pszID = from;
		mucce.pszModule = proto->m_szModuleName;
		mucce.pszText = subject;
		mucce.time = time(NULL);
		CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
//	}
	return 0;
}

int TlenMUCRecvError(TlenProtocol *proto, const char *from, XmlNode *errorNode)
{
	int errCode;
	char str[512];
	TLEN_LIST_ITEM *item;
	MUCCEVENT mucce;
	mucce.cbSize = sizeof(MUCCEVENT);
	mucce.iType = MUCC_EVENT_ERROR;
	mucce.pszID = from;
	mucce.pszModule = proto->m_szModuleName;
	errCode = atoi(TlenXmlGetAttrValue(errorNode, "code"));
	switch (errCode) {
		case 403:
			mir_snprintf(str, SIZEOF(str), Translate("You cannot join this chat room, because you are banned."));
			break;
		case 404:
			mir_snprintf(str, SIZEOF(str), Translate("Chat room not found."));
			break;
		case 407:
			mir_snprintf(str, SIZEOF(str), Translate("This is a private chat room and you are not one of the members."));
			break;
		case 408:
			mir_snprintf(str, SIZEOF(str), Translate("You cannot send any message unless you join this chat room."));
			break;
		case 410:
			mir_snprintf(str, SIZEOF(str), Translate("Chat room with already created."));
			break;
		case 411:
			mir_snprintf(str, SIZEOF(str), Translate("Nickname '%s' is already registered."),
				TlenXmlGetAttrValue(errorNode, "n"));
			break;
		case 412:
			mir_snprintf(str, SIZEOF(str), Translate("Nickname already in use, please try another one. Hint: '%s' is free."),
				TlenXmlGetAttrValue(errorNode, "free"));
			break;
		case 413:
			mir_snprintf(str, SIZEOF(str), Translate("You cannot register more than %s nicknames."),
				TlenXmlGetAttrValue(errorNode, "num"));
			break;
		case 414:
			mir_snprintf(str, SIZEOF(str), Translate("You cannot create more than %s chat rooms."),
				TlenXmlGetAttrValue(errorNode, "num"));
			break;
		case 415:
			mir_snprintf(str, SIZEOF(str), Translate("You cannot join more than %s chat rooms."),
				TlenXmlGetAttrValue(errorNode, "num"));
			break;
		case 601:
			mir_snprintf(str, SIZEOF(str), Translate("Anonymous nicknames are not allowed in this chat room."));
			break;
		default:
			mir_snprintf(str, SIZEOF(str), Translate("Unknown error code: %d"), errCode);
			break;
	}
	mucce.pszText = str;
	CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
	if (proto->isOnline) {
		switch (errCode) {
			case 412:
				item = TlenListGetItemPtr(proto, LIST_CHATROOM, from);
				if (item != NULL) {
					mucce.iType = MUCC_EVENT_JOIN;
					mucce.dwFlags = MUCC_EF_ROOM_NICKNAMES;
					mucce.pszModule = proto->m_szModuleName;
					mucce.pszID = from;
					mucce.pszName = item->roomName;
					mucce.pszNick = TlenXmlGetAttrValue(errorNode, "free");
					CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
				}
				break;
			case 601:
				item = TlenListGetItemPtr(proto, LIST_CHATROOM, from);
				if (item != NULL) {
					mucce.iType = MUCC_EVENT_JOIN;
					mucce.dwFlags = 0;
					mucce.pszModule = proto->m_szModuleName;
					mucce.pszID = from;
					mucce.pszName = item->roomName;
					mucce.pszNick = NULL;
					CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
				}
				break;
		}
	}
	return 1;
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
		mir_snprintf(str, sizeof(str), "%s/%s", roomID, nick);
	} else {
		mir_snprintf(str, sizeof(str), "%s", roomID);
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

static int TlenMUCSendMessage(TlenProtocol *proto, MUCCEVENT *event)
{
	char *msg, *jid;
	int style;

	if (!proto->isOnline) {
		return 1;
	}
	if ((msg = TlenTextEncode(event->pszText)) != NULL) {
		if ((jid = TlenTextEncode(event->pszID)) != NULL) {
			style = 0;
			if (event->dwFlags & MUCC_EF_FONT_BOLD) style |=1;
			if (event->dwFlags & MUCC_EF_FONT_ITALIC) style |=2;
			if (event->dwFlags & MUCC_EF_FONT_UNDERLINE) style |=4;
			TlenSend(proto, "<m to='%s'><b n='%d' s='%d' f='%d' c='%06X'>%s</b></m>", jid, event->iFont+1, event->iFontSize, style, event->color, msg);
			mir_free(jid);
		}
		mir_free(msg);
	}
	return 0;
}

static int TlenMUCSendTopic(TlenProtocol *proto, MUCCEVENT *event)
{
	char *msg, *jid;
	if (!proto->isOnline) {
		return 1;
	}
	if ((msg = TlenTextEncode(event->pszText)) != NULL) {
		if ((jid = TlenTextEncode(event->pszID)) != NULL) {
			TlenSend(proto, "<m to='%s'><subject>%s</subject></m>", jid, msg);
			mir_free(jid);
		}
		mir_free(msg);
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

int TlenMUCCreateWindow(TlenProtocol *proto, const char *roomID, const char *roomName, int roomFlags, const char *nick, const char *iqId)
{
	TLEN_LIST_ITEM *item;
	MUCCWINDOW mucw;
	if (!proto->isOnline || roomID == NULL) {
		return 1;
	}
	if (TlenListExist(proto, LIST_CHATROOM, roomID)) {
		return 0;
	}
	item = TlenListAdd(proto, LIST_CHATROOM, roomID);
	if (roomName != NULL) {
		item->roomName = mir_strdup(roomName);
	}
	if (nick != NULL) {
		item->nick = mir_strdup(nick);
	}
	mucw.cbSize = sizeof(MUCCWINDOW);
	mucw.iType = MUCC_WINDOW_CHATROOM;
	mucw.pszModule = proto->m_szModuleName;
	mucw.pszModuleName = proto->m_szModuleName;
	mucw.pszID = roomID;
	mucw.pszName = roomName;
	mucw.pszNick = nick;
	mucw.dwFlags = roomFlags;
	mucw.pszStatusbarText = "hello";
	CallService(MS_MUCC_NEW_WINDOW, 0, (LPARAM) &mucw);
	if (iqId != NULL) {
		item = TlenListGetItemPtr(proto, LIST_INVITATIONS, iqId);
		if (item !=NULL) {
			TlenMUCSendInvitation(proto, roomID, item->nick);
		}
		TlenListRemove(proto, LIST_INVITATIONS, iqId);
	}
	return 0;
}

static void TlenMUCFreeQueryResult(MUCCQUERYRESULT *result)
{	int i;
	for (i=0; i<result->iItemsNum; i++) {
		if (result->pItems[i].pszID != NULL) {
			mir_free((char *) result->pItems[i].pszID);
		}
		if (result->pItems[i].pszName != NULL) {
			mir_free((char *) result->pItems[i].pszName);
		}
		if (result->pItems[i].pszNick != NULL) {
			mir_free((char *) result->pItems[i].pszNick);
		}
		if (result->pItems[i].pszText != NULL) {
			mir_free((char *) result->pItems[i].pszText);
		}
	}
	mir_free((MUCCQUERYITEM *)result->pItems);
}

void TlenIqResultChatGroups(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *lNode, *itemNode;
	char *p, *n, *id, *f;
	int i, j;
	MUCCQUERYRESULT queryResult;

	if ((lNode=TlenXmlGetChild(iqNode, "l")) == NULL) return;
	p = TlenXmlGetAttrValue(iqNode, "p");
	if (p == NULL) {
		p="";
	}
	p = TlenTextDecode(p);
	queryResult.cbSize = sizeof (MUCCQUERYRESULT);
	queryResult.iType = MUCC_EVENT_QUERY_GROUPS;
	queryResult.pszModule = proto->m_szModuleName;
	queryResult.pszParent = p;
	queryResult.pItems = (MUCCQUERYITEM*)mir_alloc(sizeof(MUCCQUERYITEM) * lNode->numChild);
	memset(queryResult.pItems, 0, sizeof(MUCCQUERYITEM) * lNode->numChild);
	for (i=j=0; i<lNode->numChild; i++) {
		itemNode = lNode->child[i];
		if (!strcmp(itemNode->name, "i")) {
			queryResult.pItems[j].iCount = 0;
			if ((f = TlenXmlGetAttrValue(itemNode, "f")) != NULL) {
				queryResult.pItems[j].iCount = !strcmp(f, "3");
			}
			n = TlenXmlGetAttrValue(itemNode, "n");
			id = TlenXmlGetAttrValue(itemNode, "i");
			if (n != NULL && id != NULL) {
				queryResult.pItems[j].pszID =  TlenTextDecode(id);
				queryResult.pItems[j].pszName = TlenTextDecode(n);
				j++;
			}
		}
	}
	queryResult.iItemsNum = j;
	CallService(MS_MUCC_QUERY_RESULT, 0, (LPARAM) &queryResult);
	TlenMUCFreeQueryResult(&queryResult);
	mir_free(p);
}

void TlenIqResultChatRooms(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *lNode, *itemNode;
	char *id, *c, *n, *x, *p, *px, *pn;
	int i, j;
	MUCCQUERYRESULT queryResult;

	if ((lNode=TlenXmlGetChild(iqNode, "l")) == NULL) return;
	if ((p = TlenXmlGetAttrValue(iqNode, "p")) == NULL) return;
	pn = TlenXmlGetAttrValue(lNode, "n");
	if (pn == NULL) pn = "0";
	px = TlenXmlGetAttrValue(lNode, "x");
	if (px == NULL) px = "0";
	p = TlenTextDecode(p);
	queryResult.cbSize = sizeof (MUCCQUERYRESULT);
	queryResult.iType = MUCC_EVENT_QUERY_ROOMS;
	queryResult.pszModule = proto->m_szModuleName;
	queryResult.pszParent = p;
	queryResult.pItems = (MUCCQUERYITEM*)mir_alloc(sizeof(MUCCQUERYITEM) * lNode->numChild);
	memset(queryResult.pItems, 0, sizeof(MUCCQUERYITEM) * lNode->numChild);
	queryResult.iPage = atoi(pn);
	queryResult.iLastPage = atoi(px)&2?1:0;
	for (i=j=0; i<lNode->numChild; i++) {
		itemNode = lNode->child[i];
		if (!strcmp(itemNode->name, "i")) {
			n = TlenXmlGetAttrValue(itemNode, "n");
			c = TlenXmlGetAttrValue(itemNode, "c");
			x = TlenXmlGetAttrValue(itemNode, "x");
			if ((id=TlenXmlGetAttrValue(itemNode, "i")) != NULL) {
				queryResult.pItems[j].pszID =  TlenTextDecode(id);
				queryResult.pItems[j].pszName = TlenTextDecode(n);
				queryResult.pItems[j].iCount = atoi(c);
				queryResult.pItems[j].dwFlags = atoi(x);
				j++;
			}
		}
	}
	queryResult.iItemsNum = j;
	CallService(MS_MUCC_QUERY_RESULT, 0, (LPARAM) &queryResult);
	TlenMUCFreeQueryResult(&queryResult);
	mir_free(p);
}
void TlenIqResultUserRooms(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *lNode, *itemNode;
	char *id, *n;
	int i, j;
	MUCCQUERYRESULT queryResult;
	if ((lNode=TlenXmlGetChild(iqNode, "l")) == NULL) return;
	queryResult.cbSize = sizeof (MUCCQUERYRESULT);
	queryResult.iType = MUCC_EVENT_QUERY_USER_ROOMS;
	queryResult.pszModule = proto->m_szModuleName;
	queryResult.pItems = (MUCCQUERYITEM*)mir_alloc(sizeof(MUCCQUERYITEM) * lNode->numChild);
	memset(queryResult.pItems, 0, sizeof(MUCCQUERYITEM) * lNode->numChild);
	for (i=j=0; i<lNode->numChild; i++) {
		itemNode = lNode->child[i];
		if (!strcmp(itemNode->name, "i")) {
			n = TlenXmlGetAttrValue(itemNode, "n");
			id = TlenXmlGetAttrValue(itemNode, "i");
			if (n != NULL && id != NULL) {
				queryResult.pItems[j].pszID =  TlenTextDecode(id);
				queryResult.pItems[j].pszName = TlenTextDecode(n);
				j++;
			}
		}
	}
	queryResult.iItemsNum = j;
	CallService(MS_MUCC_QUERY_RESULT, 0, (LPARAM) &queryResult);
	TlenMUCFreeQueryResult(&queryResult);
}
void TlenIqResultUserNicks(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *lNode, *itemNode;
	char *n;
	int i, j;
	MUCCQUERYRESULT queryResult;
	if ((lNode=TlenXmlGetChild(iqNode, "l")) == NULL) return;
	queryResult.cbSize = sizeof (MUCCQUERYRESULT);
	queryResult.iType = MUCC_EVENT_QUERY_USER_NICKS;
	queryResult.pszModule = proto->m_szModuleName;
	queryResult.pItems = (MUCCQUERYITEM*)mir_alloc(sizeof(MUCCQUERYITEM) * lNode->numChild);
	memset(queryResult.pItems, 0, sizeof(MUCCQUERYITEM) * lNode->numChild);
	for (i=j=0; i<lNode->numChild; i++) {
		itemNode = lNode->child[i];
		if (!strcmp(itemNode->name, "i")) {
			n = TlenXmlGetAttrValue(itemNode, "n");
			queryResult.pItems[j].pszID =  NULL;//TlenTextDecode(n);
			queryResult.pItems[j].pszName = TlenTextDecode(n);
			j++;
		}
	}
	queryResult.iItemsNum = j;
	CallService(MS_MUCC_QUERY_RESULT, 0, (LPARAM) &queryResult);
	TlenMUCFreeQueryResult(&queryResult);
}
void TlenIqResultChatRoomUsers(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *lNode, *itemNode;
	char *id, *n;
	int i, j;
	MUCCQUERYRESULT queryResult;
	if ((lNode=TlenXmlGetChild(iqNode, "l")) == NULL) return;
	if ((id=TlenXmlGetAttrValue(iqNode, "from")) == NULL) return;
	queryResult.cbSize = sizeof (MUCCQUERYRESULT);
	queryResult.iType = MUCC_EVENT_QUERY_USERS;
	queryResult.pszModule = proto->m_szModuleName;
	queryResult.pszParent = id;
	queryResult.pItems = (MUCCQUERYITEM*)mir_alloc(sizeof(MUCCQUERYITEM) * lNode->numChild);
	memset(queryResult.pItems, 0, sizeof(MUCCQUERYITEM) * lNode->numChild);
	for (i=j=0; i<lNode->numChild; i++) {
		itemNode = lNode->child[i];
		if (!strcmp(itemNode->name, "i")) {
			id = TlenXmlGetAttrValue(itemNode, "i");
			if (id != NULL) {
				queryResult.pItems[j].pszID =  TlenTextDecode(id);
				n = TlenXmlGetAttrValue(itemNode, "n");
				if (n != NULL) {
					queryResult.pItems[j].pszName = TlenTextDecode(n);
				}
				n = TlenXmlGetAttrValue(itemNode, "a");
				if (n != NULL) {
					queryResult.pItems[j].pszNick = TlenTextDecode(n);
				}
				n = TlenXmlGetAttrValue(itemNode, "r");
				if (n != NULL) {
					queryResult.pItems[j].pszText = TlenTextDecode(n);
				}
				n = TlenXmlGetAttrValue(itemNode, "e");
				if (n != NULL) {
					queryResult.pItems[j].iCount = atoi(n);
				}
				n = TlenXmlGetAttrValue(itemNode, "s");
				if (n != NULL) {
					queryResult.pItems[j].dwFlags = atoi(n);
				}
				j++;
			}
		}
	}
	queryResult.iItemsNum = j;
	CallService(MS_MUCC_QUERY_RESULT, 0, (LPARAM) &queryResult);
	TlenMUCFreeQueryResult(&queryResult);
}

void TlenIqResultRoomSearch(TlenProtocol *proto, XmlNode *iqNode)
{
	char *iqId, *id;
	TLEN_LIST_ITEM *item;
	iqId=TlenXmlGetAttrValue(iqNode, "id");
	item=TlenListGetItemPtr(proto, LIST_SEARCH, iqId);
	if ((id=TlenXmlGetAttrValue(iqNode, "i")) != NULL) {
		MUCCEVENT mucce;
		id = TlenTextDecode(id);
		mucce.cbSize = sizeof(MUCCEVENT);
		mucce.iType = MUCC_EVENT_JOIN;
		mucce.pszModule = proto->m_szModuleName;
		mucce.pszID = id;
		mucce.pszName = id;
		if (item != NULL) {
			mucce.pszName = item->roomName;
		}
		mucce.pszNick = NULL;
		mucce.dwFlags = MUCC_EF_ROOM_NICKNAMES;
		CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
		mir_free(id);
	}
	TlenListRemove(proto, LIST_SEARCH, iqId);
}

void TlenIqResultRoomInfo(TlenProtocol *proto, XmlNode *iqNode)
{
	char *id, *name, *group, *flags;
	if ((id=TlenXmlGetAttrValue(iqNode, "from")) != NULL) {
		if ((name=TlenXmlGetAttrValue(iqNode, "n")) != NULL) {
			MUCCEVENT mucce;
			group = TlenXmlGetAttrValue(iqNode, "cn");
			flags = TlenXmlGetAttrValue(iqNode, "x");
			id = TlenTextDecode(id);
			name = TlenTextDecode(name);
			mucce.cbSize = sizeof(MUCCEVENT);
			mucce.iType = MUCC_EVENT_ROOM_INFO;
			mucce.pszModule = proto->m_szModuleName;
			mucce.pszID = id;
			mucce.pszName = name;
			mucce.dwFlags = atoi(flags);
			CallService(MS_MUCC_EVENT, 0, (LPARAM) &mucce);
			mir_free(id);
			mir_free(name);
		}
	}
}


typedef struct {
	TlenProtocol *proto;
	char *roomId;
} MUCSENDQUERYTHREADDATA;

static void __cdecl TlenMUCCSendQueryResultThread(void *ptr)
{
	DBVARIANT dbv;
	MUCSENDQUERYTHREADDATA* threadData = (MUCSENDQUERYTHREADDATA*)ptr;
	char *szProto = threadData->proto->m_szModuleName;

	MUCCQUERYRESULT queryResult;
	queryResult.cbSize = sizeof (MUCCQUERYRESULT);
	queryResult.iType = MUCC_EVENT_QUERY_CONTACTS;
	queryResult.pszModule = szProto;
	queryResult.pszParent = threadData->roomId;
	queryResult.iItemsNum = 0;


	for (HCONTACT hContact = db_find_first(szProto); hContact; hContact = db_find_next(hContact, szProto)) {
		if ( db_get_b(hContact, szProto, "bChat", FALSE))
			continue;

		if ( !db_get_s(hContact, szProto, "jid", &dbv)) {
			if (strcmp(dbv.pszVal, "b73@tlen.pl"))
				queryResult.iItemsNum++;

			db_free(&dbv);
		}
	}

	queryResult.pItems = (MUCCQUERYITEM*)mir_alloc(sizeof(MUCCQUERYITEM) * queryResult.iItemsNum);
	memset(queryResult.pItems, 0, sizeof(MUCCQUERYITEM) * queryResult.iItemsNum);
	queryResult.iItemsNum = 0;

	for (HCONTACT hContact = db_find_first(szProto); hContact; hContact = db_find_next(hContact, szProto)) {
		if ( db_get_b(hContact, szProto, "bChat", FALSE))
			continue;

		if (!db_get(hContact, szProto, "jid", &dbv)) {
			if (strcmp(dbv.pszVal, "b73@tlen.pl")) {
				queryResult.pItems[queryResult.iItemsNum].pszID = mir_strdup(dbv.pszVal);
				queryResult.pItems[queryResult.iItemsNum].pszName = mir_strdup((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, 0));
				queryResult.iItemsNum++;
			}
			db_free(&dbv);
		}
	}

	CallService(MS_MUCC_QUERY_RESULT, 0, (LPARAM) &queryResult);
	TlenMUCFreeQueryResult(&queryResult);
	mir_free(threadData->roomId);
	mir_free(threadData);
}


static int TlenMUCQueryContacts(TlenProtocol *proto, const char *roomId) {

	MUCSENDQUERYTHREADDATA *threadData = (MUCSENDQUERYTHREADDATA *)mir_alloc(sizeof(MUCSENDQUERYTHREADDATA));
	threadData->proto = proto;
	threadData->roomId = mir_strdup(roomId);
	forkthread(TlenMUCCSendQueryResultThread, 0, (void *)threadData);
	return 1;
}

INT_PTR TlenProtocol::MUCMenuHandleMUC(WPARAM wParam, LPARAM lParam)
{
	if (!isOnline)
		return 1;

	TlenSend(this, "<p to='c' tp='c' id='"TLEN_IQID"%d'/>", TlenSerialNext(this));
	return 0;
}

INT_PTR TlenProtocol::MUCMenuHandleChats(WPARAM wParam, LPARAM lParam)
{
	if (!isOnline)
		return 1;

	MUCCWINDOW mucw;
	mucw.cbSize = sizeof(MUCCWINDOW);
	mucw.iType = MUCC_WINDOW_CHATLIST;
	mucw.pszModule = m_szModuleName;
	mucw.pszModuleName = m_szModuleName;
	CallService(MS_MUCC_NEW_WINDOW, 0, (LPARAM) &mucw);
	return 0;
}

INT_PTR TlenProtocol::MUCContactMenuHandleMUC(WPARAM wParam, LPARAM lParam)
{
	HCONTACT hContact;
	DBVARIANT dbv;
	TLEN_LIST_ITEM *item;
	if (!isOnline)
		return 1;

	if ((hContact=(HCONTACT)wParam) != NULL && isOnline) {
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
