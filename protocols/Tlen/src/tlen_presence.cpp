/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
Copyright (C) 2004-2009  Piotr Piastucki

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
#include "tlen_avatar.h"

void TlenProcessPresence(XmlNode *node, TlenProtocol *proto)
{
	MCONTACT hContact;
	XmlNode *showNode, *statusNode;
	TLEN_LIST_ITEM *item;
	char *from, *type, *nick, *show;
	int status, laststatus = ID_STATUS_OFFLINE;
	char *p;

	if ((from=TlenXmlGetAttrValue(node, "from")) != NULL) {
		if (TlenListExist(proto, LIST_CHATROOM, from)); //TlenGroupchatProcessPresence(node, userdata);

		else {
			type = TlenXmlGetAttrValue(node, "type");
			item = TlenListGetItemPtr(proto, LIST_ROSTER, from);
			if (item != NULL) {
				if (proto->tlenOptions.enableAvatars) {
					TlenProcessPresenceAvatar(proto, node, item);
				}
			}
			if (type == NULL || (!strcmp(type, "available"))) {
				if ((nick=TlenLocalNickFromJID(from)) != NULL) {
					if ((hContact=TlenHContactFromJID(proto, from)) == NULL)
						hContact = TlenDBCreateContact(proto, from, nick, FALSE);
					if (!TlenListExist(proto, LIST_ROSTER, from)) {
						proto->debugLogA("Receive presence online from %s (who is not in my roster)", from);
						TlenListAdd(proto, LIST_ROSTER, from);
					}
					status = ID_STATUS_ONLINE;
					if ((showNode=TlenXmlGetChild(node, "show")) != NULL) {
						if ((show=showNode->text) != NULL) {
							if (!strcmp(show, "away")) status = ID_STATUS_AWAY;
							else if (!strcmp(show, "xa")) status = ID_STATUS_NA;
							else if (!strcmp(show, "dnd")) status = ID_STATUS_DND;
							else if (!strcmp(show, "chat")) status = ID_STATUS_FREECHAT;
							else if (!strcmp(show, "unavailable")) {
								// Always show invisible (on old Tlen client) as invisible (not offline)
								status = ID_STATUS_OFFLINE;
							}
						}
					}

					statusNode = TlenXmlGetChild(node, "status");
					if (statusNode)
						p = TlenTextDecode(statusNode->text);
					else
						p = NULL;
					TlenListAddResource(proto, LIST_ROSTER, from, status, statusNode?p:NULL);
					if (p != NULL && *p) {
						char* statusMsg_utf8 = mir_utf8encode(p);
						db_set_utf(hContact, "CList", "StatusMsg", statusMsg_utf8);
						mir_free(statusMsg_utf8);
						mir_free(p);
					} else {
						db_unset(hContact, "CList", "StatusMsg");
					}
					// Determine status to show for the contact and request version information
					if (item != NULL) {
						laststatus = item->status;
						item->status = status;
					}
					if (strchr(from, '@') != NULL || db_get_b(NULL, proto->m_szModuleName, "ShowTransport", TRUE) == TRUE) {
						if (db_get_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != status)
							db_set_w(hContact, proto->m_szModuleName, "Status", (WORD) status);
					}
					if (item != NULL) {
						if (!item->infoRequested) {
							int iqId = TlenSerialNext(proto);
							item->infoRequested = TRUE;
							TlenSend( proto, "<iq type='get' id='"TLEN_IQID"%d'><query xmlns='jabber:iq:info' to='%s'></query></iq>", iqId, from);
						}
						if (proto->tlenOptions.enableVersion && !item->versionRequested) {
							item->versionRequested = TRUE;
							if (proto->m_iStatus != ID_STATUS_INVISIBLE) {
								TlenSend( proto, "<message to='%s' type='iq'><iq type='get'><query xmlns='jabber:iq:version'/></iq></message>", from );
							}
						}
					}
					proto->debugLogA("%s (%s) online, set contact status to %d", nick, from, status);
					mir_free(nick);
				}
			}
			else if (!strcmp(type, "unavailable")) {
				if (!TlenListExist(proto, LIST_ROSTER, from)) {
					proto->debugLogA("Receive presence offline from %s (who is not in my roster)", from);
					TlenListAdd(proto, LIST_ROSTER, from);
				}
				else {
					TlenListRemoveResource(proto, LIST_ROSTER, from);
				}
				status = ID_STATUS_OFFLINE;
				statusNode = TlenXmlGetChild(node, "status");
				if (statusNode) {
					if (proto->tlenOptions.offlineAsInvisible) {
						status = ID_STATUS_INVISIBLE;
					}
					p = TlenTextDecode(statusNode->text);
				}
				else
					p = NULL;
				TlenListAddResource(proto, LIST_ROSTER, from, status, p);
				if ((hContact=TlenHContactFromJID(proto, from)) != NULL) {
					if (p != NULL && *p) {
						char* statusMsg_utf8 = mir_utf8encode(p);
						db_set_utf(hContact, "CList", "StatusMsg", statusMsg_utf8);
						mir_free(statusMsg_utf8);
						mir_free(p);
					} else {
						db_unset(hContact, "CList", "StatusMsg");
					}
				}
				if ((item=TlenListGetItemPtr(proto, LIST_ROSTER, from)) != NULL) {
					// Determine status to show for the contact based on the remaining resources
					item->status = status;
					item->versionRequested = FALSE;
					item->infoRequested = FALSE;
				}
				if ((hContact=TlenHContactFromJID(proto, from)) != NULL) {
					if (strchr(from, '@') != NULL || db_get_b(NULL, proto->m_szModuleName, "ShowTransport", TRUE) == TRUE) {
						if (db_get_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != status)
							db_set_w(hContact, proto->m_szModuleName, "Status", (WORD) status);
					}
					if (item != NULL && item->isTyping) {
						item->isTyping = FALSE;
						CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
					}
					proto->debugLogA("%s offline, set contact status to %d", from, status);
				}
			}
			else if (!strcmp(type, "subscribe")) {
				if (strchr(from, '@') == NULL) {
					// automatically send authorization allowed to agent/transport
					TlenSend(proto, "<presence to='%s' type='subscribed'/>", from);
				}
				else if ((nick=TlenNickFromJID(from)) != NULL) {
					proto->debugLogA("%s (%s) requests authorization", nick, from);
					TlenDBAddAuthRequest(proto, from, nick);
					mir_free(nick);
				}
			}
			else if (!strcmp(type, "subscribed")) {
				if ((item=TlenListGetItemPtr(proto, LIST_ROSTER, from)) != NULL) {
					if (item->subscription == SUB_FROM) item->subscription = SUB_BOTH;
					else if (item->subscription == SUB_NONE) {
						item->subscription = SUB_TO;
					}
				}
			}
		}
	}
}

/* change status and status msg on own contact on contact list (if present) */
void setOwnStatusOnCList(TlenProtocol *proto, int status, char *statusMsg)
{
	ptrA ownJid(db_get_sa(NULL, proto->m_szModuleName, "jid"));
	MCONTACT hContact = TlenHContactFromJID(proto, ownJid);
	if(hContact){
		if (db_get_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != status)
			db_set_w(hContact, proto->m_szModuleName, "Status", (WORD)status);
		if (statusMsg != NULL && *statusMsg) {
			char* statusMsg_utf8 = mir_utf8encode(statusMsg);
			db_set_utf(hContact, "CList", "StatusMsg", statusMsg_utf8);
			mir_free(statusMsg_utf8);
		} else {
			db_unset(hContact, "CList", "StatusMsg");
		}
	}
}

static void TlenSendPresenceTo(TlenProtocol *proto, int status, char *to)
{
	char *showBody, *statusMsg, *presenceType;
	char *ptr = NULL;

	if (!proto->isOnline) return;

	// Send <presence/> update for status (we won't handle ID_STATUS_OFFLINE here)
	// Note: tlenModeMsg is already encoded using TlenTextEncode()
	EnterCriticalSection(&proto->modeMsgMutex);

	showBody = NULL;
	statusMsg = NULL;
	presenceType = NULL;
	switch (status) {
	case ID_STATUS_ONLINE:
		showBody = "available";
		statusMsg = proto->modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		showBody = "away";
		statusMsg = proto->modeMsgs.szAway;
		break;
	case ID_STATUS_NA:
		showBody = "xa";
		statusMsg = proto->modeMsgs.szNa;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		showBody = "dnd";
		statusMsg = proto->modeMsgs.szDnd;
		break;
	case ID_STATUS_FREECHAT:
		showBody = "chat";
		statusMsg = proto->modeMsgs.szFreechat;
		break;
	case ID_STATUS_INVISIBLE:
		presenceType = "invisible";
		statusMsg = proto->modeMsgs.szInvisible;
		break;
	case ID_STATUS_OFFLINE:
		presenceType = "unavailable";
		if (db_get_b(NULL, proto->m_szModuleName, "LeaveOfflineMessage", FALSE)) {
			int offlineMessageOption = db_get_w(NULL, proto->m_szModuleName, "OfflineMessageOption", 0);
			if (offlineMessageOption == 0) {
				switch (proto->m_iStatus) {
					case ID_STATUS_ONLINE:
						ptr = mir_strdup(proto->modeMsgs.szOnline);
						break;
					case ID_STATUS_AWAY:
					case ID_STATUS_ONTHEPHONE:
					case ID_STATUS_OUTTOLUNCH:
						ptr = mir_strdup(proto->modeMsgs.szAway);
						break;
					case ID_STATUS_NA:
						ptr = mir_strdup(proto->modeMsgs.szNa);
						break;
					case ID_STATUS_DND:
					case ID_STATUS_OCCUPIED:
						ptr = mir_strdup(proto->modeMsgs.szDnd);
						break;
					case ID_STATUS_FREECHAT:
						ptr = mir_strdup(proto->modeMsgs.szFreechat);
						break;
					case ID_STATUS_INVISIBLE:
						ptr = mir_strdup(proto->modeMsgs.szInvisible);
						break;
				}
			} else if (offlineMessageOption == 99) {

			} else if (offlineMessageOption < 7) {
				DBVARIANT dbv;
				const char *statusNames[] = {"OnDefault", "AwayDefault", "NaDefault", "DndDefault", "FreeChatDefault", "InvDefault"};
				if (!db_get(NULL, "SRAway", statusNames[offlineMessageOption-1], &dbv)) {
					int i;
					char substituteStr[128];
					ptr = mir_strdup(dbv.pszVal);
					db_free(&dbv);
					for (i=0;ptr[i];i++) {
						if (ptr[i] != '%') continue;
						if (!_strnicmp(ptr+i,"%time%",6))
							GetTimeFormatA(LOCALE_USER_DEFAULT,TIME_NOSECONDS,NULL,NULL,substituteStr,sizeof(substituteStr));
						else if (!_strnicmp(ptr+i,"%date%",6))
							GetDateFormatA(LOCALE_USER_DEFAULT,DATE_SHORTDATE,NULL,NULL,substituteStr,sizeof(substituteStr));
						else continue;
						if (strlen(substituteStr)>6) ptr=(char*)mir_realloc(ptr,strlen(ptr)+1+strlen(substituteStr)-6);
						MoveMemory(ptr+i+strlen(substituteStr),ptr+i+6,strlen(ptr)-i-5);
						CopyMemory(ptr+i,substituteStr,strlen(substituteStr));
					}
				}
			}
		}
		mir_free(statusMsg);
		statusMsg = ptr;
		break;
	default:
		// Should not reach here
		break;
	}
	proto->m_iStatus = status;
	if (presenceType) {
		if (statusMsg != NULL && *statusMsg)
			TlenSend(proto, "<presence type='%s'><status>%s</status></presence>", presenceType, ptrA(TlenTextEncode(statusMsg)));
		else
			TlenSend(proto, "<presence type='%s'></presence>", presenceType);
	} else {
		if (statusMsg != NULL && *statusMsg)
			TlenSend(proto, "<presence><show>%s</show><status>%s</status></presence>", showBody, ptrA(TlenTextEncode(statusMsg)));
		else
			TlenSend(proto, "<presence><show>%s</show></presence>", showBody);
	}

	setOwnStatusOnCList(proto, proto->m_iStatus, statusMsg);

	LeaveCriticalSection(&proto->modeMsgMutex);
}



void TlenSendPresence(TlenProtocol *proto, int statusIn)
{
	int statusOut;
	switch (statusIn) {
		case ID_STATUS_ONLINE:
		case ID_STATUS_OFFLINE:
		case ID_STATUS_NA:
		case ID_STATUS_FREECHAT:
		case ID_STATUS_INVISIBLE:
			statusOut = statusIn;
			break;
		case ID_STATUS_AWAY:
		case ID_STATUS_ONTHEPHONE:
		case ID_STATUS_OUTTOLUNCH:
		default:
			statusOut = ID_STATUS_AWAY;
			break;
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
			statusOut = ID_STATUS_DND;
			break;
	}
	TlenSendPresenceTo(proto, statusOut, NULL);
}


