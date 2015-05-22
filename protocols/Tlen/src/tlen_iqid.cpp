/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
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
#include "resource.h"
#include "tlen_list.h"
#include "tlen_iq.h"
#include "tlen_muc.h"

// RECVED: authentication result
// ACTION: if successfully logged in, continue by requesting roster list and set my initial status
void TlenIqResultAuth(TlenProtocol *proto, XmlNode *iqNode)
{
	char *type=TlenXmlGetAttrValue(iqNode, "type");
	if (type == NULL)
		return;

	if (!mir_strcmp(type, "result")) {
		DBVARIANT dbv;

		if (db_get(NULL, proto->m_szModuleName, "Nick", &dbv))
			db_set_s(NULL, proto->m_szModuleName, "Nick", proto->threadData->username);
		else
			db_free(&dbv);
//		iqId = TlenSerialNext();
//		TlenIqAdd(iqId, IQ_PROC_NONE, TlenIqResultGetRoster);
//		TlenSend(info, "<iq type='get' id='"TLEN_IQID"%d'><query xmlns='jabber:iq:roster'/></iq>", iqId);

		TlenSend(proto, "<iq type='get' id='GetRoster'><query xmlns='jabber:iq:roster'/></iq>");
		TlenSend(proto, "<iq to='tcfg' type='get' id='TcfgGetAfterLoggedIn'></iq>");
	}
	// What to do if password error? etc...
	else if (!mir_strcmp(type, "error")) {
		char text[128];

		TlenSend(proto, "</s>");
		mir_snprintf(text, SIZEOF(text), Translate("Authentication failed for %s@%s."), proto->threadData->username, proto->threadData->server);
		MessageBoxA(NULL, text, Translate("Tlen Authentication"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
		ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
		proto->threadData = NULL;	// To disallow auto reconnect
	}
}

void TlenResultSetRoster(TlenProtocol *proto, XmlNode *queryNode) {
	DBVARIANT dbv;
	XmlNode *groupNode;
	MCONTACT hContact;
	char *name, *nick;

	for (int i=0; i<queryNode->numChild; i++) {
		XmlNode *itemNode = queryNode->child[i];
		if (!mir_strcmp(itemNode->name, "item")) {
			char *jid=TlenXmlGetAttrValue(itemNode, "jid");
			if (jid != NULL) {
				char *str = TlenXmlGetAttrValue(itemNode, "subscription");
				if (str == NULL)
					continue;
				else if (!mir_strcmp(str, "remove")) {
					if ((hContact = TlenHContactFromJID(proto, jid)) != NULL) {
						if (db_get_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
							db_set_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE);
					}
					TlenListRemove(proto, LIST_ROSTER, jid);
				} else {
					TLEN_LIST_ITEM *item = TlenListAdd(proto, LIST_ROSTER, jid);
					if (item != NULL) {
						if (!mir_strcmp(str, "both"))
							item->subscription = SUB_BOTH;
						else if (!mir_strcmp(str, "to"))
							item->subscription = SUB_TO;
						else if (!mir_strcmp(str, "from"))
							item->subscription = SUB_FROM;
						else
							item->subscription = SUB_NONE;
						if ((name=TlenXmlGetAttrValue(itemNode, "name")) != NULL) {
							nick = TlenTextDecode(name);
						} else {
							nick = TlenLocalNickFromJID(jid);
						}
						if (nick != NULL) {
							if (item->nick) mir_free(item->nick);
							item->nick = nick;

							if ((hContact=TlenHContactFromJID(proto, jid)) == NULL) {
								// Received roster has a new JID.
								// Add the jid (with empty resource) to Miranda contact list.
								hContact = TlenDBCreateContact(proto, jid, nick, FALSE);
							}
							db_set_s(hContact, "CList", "MyHandle", nick);
							if (item->group) mir_free(item->group);
							if ((groupNode=TlenXmlGetChild(itemNode, "group")) != NULL && groupNode->text != NULL) {
								item->group = TlenGroupDecode(groupNode->text);
								Clist_CreateGroup(0, _A2T(item->group));
								// Don't set group again if already correct, or Miranda may show wrong group count in some case
								if (!db_get(hContact, "CList", "Group", &dbv)) {
									if (mir_strcmp(dbv.pszVal, item->group))
										db_set_s(hContact, "CList", "Group", item->group);
									db_free(&dbv);
								} else
									db_set_s(hContact, "CList", "Group", item->group);
							} else {
								item->group = NULL;
								db_unset(hContact, "CList", "Group");
							}
						}
					}
				}
			}
		}
	}
}

// RECVED: roster information
// ACTION: populate LIST_ROSTER and create contact for any new rosters
void TlenIqResultRoster(TlenProtocol *proto, XmlNode *iqNode)
{
	char *type=TlenXmlGetAttrValue(iqNode, "type");
	if (type == NULL) return;
	XmlNode *queryNode=TlenXmlGetChild(iqNode, "query");
	if (queryNode == NULL) return;

	if (!mir_strcmp(type, "result")) {
		char *str = TlenXmlGetAttrValue(queryNode, "xmlns");
		if (str != NULL && !mir_strcmp(str, "jabber:iq:roster")) {
			DBVARIANT dbv;
			XmlNode *itemNode, *groupNode;
			TLEN_SUBSCRIPTION sub;
			TLEN_LIST_ITEM *item;
			char *jid, *name, *nick;
			int i, oldStatus;

			for (i=0; i<queryNode->numChild; i++) {
				itemNode = queryNode->child[i];
				if (!mir_strcmp(itemNode->name, "item")) {
					str = TlenXmlGetAttrValue(itemNode, "subscription");
					if (str == NULL) sub = SUB_NONE;
					else if (!mir_strcmp(str, "both")) sub = SUB_BOTH;
					else if (!mir_strcmp(str, "to")) sub = SUB_TO;
					else if (!mir_strcmp(str, "from")) sub = SUB_FROM;
					else sub = SUB_NONE;
					//if (str != NULL && (!mir_strcmp(str, "to") || !mir_strcmp(str, "both"))) {
					if ((jid=TlenXmlGetAttrValue(itemNode, "jid")) != NULL) {
						if ((name=TlenXmlGetAttrValue(itemNode, "name")) != NULL)
							nick = TlenTextDecode(name);
						else
							nick = TlenLocalNickFromJID(jid);
						
						if (nick != NULL) {
							MCONTACT hContact;
							item = TlenListAdd(proto, LIST_ROSTER, jid);
							if (item->nick) mir_free(item->nick);
							item->nick = nick;
							item->subscription = sub;
							if ((hContact=TlenHContactFromJID(proto, jid)) == NULL) {
								// Received roster has a new JID.
								// Add the jid (with empty resource) to Miranda contact list.
								hContact = TlenDBCreateContact(proto, jid, nick, FALSE);
							}
							db_set_s(hContact, "CList", "MyHandle", nick);
							if (item->group) mir_free(item->group);
							if ((groupNode=TlenXmlGetChild(itemNode, "group")) != NULL && groupNode->text != NULL) {
								item->group = TlenGroupDecode(groupNode->text);
								Clist_CreateGroup(0, _A2T(item->group));
								// Don't set group again if already correct, or Miranda may show wrong group count in some case
								if (!db_get(hContact, "CList", "Group", &dbv)) {
									if (mir_strcmp(dbv.pszVal, item->group))
										db_set_s(hContact, "CList", "Group", item->group);
									db_free(&dbv);
								}
								else db_set_s(hContact, "CList", "Group", item->group);
							}
							else {
								item->group = NULL;
								db_unset(hContact, "CList", "Group");
							}
							if (!db_get(hContact, proto->m_szModuleName, "AvatarHash", &dbv)) {
								if (item->avatarHash) mir_free(item->avatarHash);
								item->avatarHash = mir_strdup(dbv.pszVal);
								proto->debugLogA("Setting hash [%s] = %s", nick, item->avatarHash);
								db_free(&dbv);
							}
							item->avatarFormat = db_get_dw(hContact, proto->m_szModuleName, "AvatarFormat", PA_FORMAT_UNKNOWN);
						}
					}
				}
			}
			
			// Delete orphaned contacts (if roster sync is enabled)
			if (db_get_b(NULL, proto->m_szModuleName, "RosterSync", FALSE) == TRUE) {
				for (MCONTACT hContact = db_find_first(proto->m_szModuleName); hContact; ) {
					MCONTACT hNext = hContact = db_find_next(hContact, proto->m_szModuleName);
					ptrA jid( db_get_sa(hContact, proto->m_szModuleName, "jid"));
					if (jid != NULL) {
						if (!TlenListExist(proto, LIST_ROSTER, jid)) {
							proto->debugLogA("Syncing roster: deleting 0x%x", hContact);
							CallService(MS_DB_CONTACT_DELETE, hContact, 0);
						}
					}
					hContact = hNext;
				}
			}

			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_FLAGS;
			Menu_ModifyItem(proto->hMenuMUC, &mi);
			if (proto->hMenuChats != NULL)
				Menu_ModifyItem(proto->hMenuChats, &mi);

			proto->isOnline = TRUE;
			proto->debugLogA("Status changed via THREADSTART");
			oldStatus = proto->m_iStatus;
			TlenSendPresence(proto, proto->m_iDesiredStatus);
			ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, proto->m_iStatus);
		}
	}
}


// Tlen actually use jabber:iq:search for other users vCard or jabber:iq:register for own vCard
void TlenIqResultVcard(TlenProtocol *proto, XmlNode *iqNode)
{
	char text[128];
	MCONTACT hContact;
	char *nText;

//	TlenLog("<iq/> iqIdGetVcard (tlen)");
	char *type=TlenXmlGetAttrValue(iqNode, "type");
	if (type == NULL) return;

	if (!mir_strcmp(type, "result")) {
		DBVARIANT dbv;

		XmlNode *queryNode=TlenXmlGetChild(iqNode, "query");
		if (queryNode == NULL) return;
		XmlNode *itemNode=TlenXmlGetChild(queryNode, "item");
		if (itemNode == NULL) return;
		char *jid=TlenXmlGetAttrValue(itemNode, "jid");
		if (jid != NULL) {
			if (db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) return;
			if (strchr(jid, '@') != NULL) {
				strncpy_s(text, jid, _TRUNCATE);
			} else {
				mir_snprintf(text, SIZEOF(text),  "%s@%s", jid, dbv.pszVal);	// Add @tlen.pl
			}
			db_free(&dbv);
			if ((hContact=TlenHContactFromJID(proto, text)) == NULL) {
				if (db_get(NULL, proto->m_szModuleName, "LoginName", &dbv)) return;
				if (mir_strcmp(dbv.pszVal, jid)) {
					db_free(&dbv);
					return;
				}
				db_free(&dbv);
			}
		} else {
			hContact = NULL;
		}
		bool hasFirst = false, hasLast = false, hasNick = false, hasEmail = false, hasCity = false, hasAge = false,
			hasGender = false, hasSchool = false, hasLookFor = false, hasOccupation = false;
		for (int i=0; i<itemNode->numChild; i++) {
			XmlNode *n = itemNode->child[i];
			if (n == NULL || n->name == NULL) continue;
			if (!mir_strcmp(n->name, "first")) {
				if (n->text != NULL) {
					hasFirst = true;
					nText = TlenTextDecode(n->text);
					db_set_s(hContact, proto->m_szModuleName, "FirstName", nText);
					mir_free(nText);
				}
			}
			else if (!mir_strcmp(n->name, "last")) {
				if (n->text != NULL) {
					hasLast = true;
					nText = TlenTextDecode(n->text);
					db_set_s(hContact, proto->m_szModuleName, "LastName", nText);
					mir_free(nText);
				}
			}
			else if (!mir_strcmp(n->name, "nick")) {
				if (n->text != NULL) {
					hasNick = true;
					nText = TlenTextDecode(n->text);
					db_set_s(hContact, proto->m_szModuleName, "Nick", nText);
					mir_free(nText);
				}
			}
			else if (!mir_strcmp(n->name, "email")) {
				if (n->text != NULL) {
					hasEmail = true;
					nText = TlenTextDecode(n->text);
					db_set_s(hContact, proto->m_szModuleName, "e-mail", nText);
					mir_free(nText);
				}
			}
			else if (!mir_strcmp(n->name, "c")) {
				if (n->text != NULL) {
					hasCity = true;
					nText = TlenTextDecode(n->text);
					db_set_s(hContact, proto->m_szModuleName, "City", nText);
					mir_free(nText);
				}
			}
			else if (!mir_strcmp(n->name, "b")) {
				if (n->text != NULL) {
					WORD nAge = atoi(n->text);
					hasAge = true;
					db_set_w(hContact, proto->m_szModuleName, "Age", nAge);
				}
			}
			else if (!mir_strcmp(n->name, "s")) {
				if (n->text != NULL && n->text[1] == '\0' && (n->text[0] == '1' || n->text[0] == '2')) {
					hasGender = true;
					db_set_b(hContact, proto->m_szModuleName, "Gender", (BYTE) (n->text[0] == '1'?'M':'F'));
				}
			}
			else if (!mir_strcmp(n->name, "e")) {
				if (n->text != NULL) {
					hasSchool = true;
					nText = TlenTextDecode(n->text);
					db_set_s(hContact, proto->m_szModuleName, "School", nText);
					mir_free(nText);
				}
			}
			else if (!mir_strcmp(n->name, "j")) {
				if (n->text != NULL) {
					hasOccupation = true;
					WORD nOccupation = atoi(n->text);
					db_set_w(hContact, proto->m_szModuleName, "Occupation", nOccupation);
				}
			}
			else if (!mir_strcmp(n->name, "r")) {
				if (n->text != NULL) {
					WORD nLookFor = atoi(n->text);
					hasLookFor = true;
					db_set_w(hContact, proto->m_szModuleName, "LookingFor", nLookFor);
				}
			}
			else if (!mir_strcmp(n->name, "g")) { // voice chat enabled
				if (n->text != NULL) {
					BYTE bVoice = atoi(n->text);
					db_set_w(hContact, proto->m_szModuleName, "VoiceChat", bVoice);
				}
			}
			else if (!mir_strcmp(n->name, "v")) { // status visibility
				if (n->text != NULL) {
					BYTE bPublic = atoi(n->text);
					db_set_w(hContact, proto->m_szModuleName, "PublicStatus", bPublic);
				}
			}
		}
		if (!hasFirst)
			db_unset(hContact, proto->m_szModuleName, "FirstName");
		if (!hasLast)
			db_unset(hContact, proto->m_szModuleName, "LastName");
		// We are not removing "Nick"
//		if (!hasNick)
//			db_unset(hContact, m_szModuleName, "Nick");
		if (!hasEmail)
			db_unset(hContact, proto->m_szModuleName, "e-mail");
		if (!hasCity)
			db_unset(hContact, proto->m_szModuleName, "City");
		if (!hasAge)
			db_unset(hContact, proto->m_szModuleName, "Age");
		if (!hasGender)
			db_unset(hContact, proto->m_szModuleName, "Gender");
		if (!hasSchool)
			db_unset(hContact, proto->m_szModuleName, "School");
		if (!hasOccupation)
			db_unset(hContact, proto->m_szModuleName, "Occupation");
		if (!hasLookFor)
			db_unset(hContact, proto->m_szModuleName, "LookingFor");
		ProtoBroadcastAck(proto->m_szModuleName, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
	}
}

void TlenIqResultSearch(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *queryNode, *itemNode, *n;
	char *jid;
	int i, found = 0;
	TLEN_SEARCH_RESULT jsr = {0};
	DBVARIANT dbv = {0};

//	TlenLog("<iq/> iqIdGetSearch");
	char *type = TlenXmlGetAttrValue(iqNode, "type");
	if (type == NULL)
		return;
	char *str = TlenXmlGetAttrValue(iqNode, "id");
	if (str == NULL)
		return;
	int id = atoi(str+mir_strlen(TLEN_IQID));

	if (!mir_strcmp(type, "result")) {
		if ((queryNode=TlenXmlGetChild(iqNode, "query")) == NULL) return;
		if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
			jsr.hdr.cbSize = sizeof(TLEN_SEARCH_RESULT);
			jsr.hdr.flags = PSR_TCHAR;
			for (i=0; i<queryNode->numChild; i++) {
				itemNode = queryNode->child[i];
				if (!mir_strcmp(itemNode->name, "item")) {
					if ((jid=TlenXmlGetAttrValue(itemNode, "jid")) != NULL) {
						if (strchr(jid, '@') != NULL) {
							strncpy_s(jsr.jid, jid, _TRUNCATE);
						} else {
							mir_snprintf(jsr.jid, SIZEOF(jsr.jid), "%s@%s", jid, dbv.pszVal);
						}
						jsr.jid[sizeof(jsr.jid)-1] = '\0';
						jsr.hdr.id = mir_a2t(jid);
						if ((n=TlenXmlGetChild(itemNode, "nick")) != NULL && n->text != NULL){
							char* buf = TlenTextDecode(n->text);
							jsr.hdr.nick = mir_a2t(buf);
							mir_free(buf);
						} else {
							jsr.hdr.nick = mir_tstrdup(TEXT(""));
						}
						if ((n=TlenXmlGetChild(itemNode, "first")) != NULL && n->text != NULL){
							char* buf = TlenTextDecode(n->text);
							jsr.hdr.firstName = mir_a2t(buf);
							mir_free(buf);
						} else {
							jsr.hdr.firstName = mir_tstrdup(TEXT(""));
						}
						if ((n=TlenXmlGetChild(itemNode, "last")) != NULL && n->text != NULL){
							char* buf = TlenTextDecode(n->text);
							jsr.hdr.lastName = mir_a2t(buf);
							mir_free(buf);
						} else {
							jsr.hdr.lastName = mir_tstrdup(TEXT(""));
						}
						if ((n=TlenXmlGetChild(itemNode, "email"))!=NULL && n->text!=NULL){
							char* buf = TlenTextDecode(n->text);
							jsr.hdr.email = mir_a2t(buf);
							mir_free(buf);
						} else {
							jsr.hdr.email = mir_tstrdup(TEXT(""));
						}

						ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) id, (LPARAM) &jsr);
						found = 1;
						mir_free(jsr.hdr.id);
						mir_free(jsr.hdr.nick);
						mir_free(jsr.hdr.firstName);
						mir_free(jsr.hdr.lastName);
						mir_free(jsr.hdr.email);
					}
				}
			}
			if (proto->searchJID != NULL) {
				if (!found) {
					if (strchr(proto->searchJID, '@') != NULL) {
						strncpy_s(jsr.jid, proto->searchJID, _TRUNCATE);
					} else {
						mir_snprintf(jsr.jid, SIZEOF(jsr.jid), "%s@%s", proto->searchJID, dbv.pszVal);
					}
					jsr.jid[sizeof(jsr.jid)-1] = '\0';
					jsr.hdr.nick = mir_tstrdup(TEXT(""));
					jsr.hdr.firstName = mir_tstrdup(TEXT(""));
					jsr.hdr.lastName = mir_tstrdup(TEXT(""));
					jsr.hdr.email = mir_tstrdup(TEXT(""));
					jsr.hdr.id = mir_tstrdup(TEXT(""));
					ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) id, (LPARAM) &jsr);
					mir_free(jsr.hdr.nick);
					mir_free(jsr.hdr.firstName);
					mir_free(jsr.hdr.lastName);
					mir_free(jsr.hdr.email);
				}
				mir_free(proto->searchJID);
				proto->searchJID = NULL;
			}
			db_free(&dbv);
		}
		found = 0;
		if (queryNode->numChild == TLEN_MAX_SEARCH_RESULTS_PER_PAGE) {
			found = TlenRunSearch(proto);
		}
		if (!found) {
			ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) id, 0);
		}
	} else if (!mir_strcmp(type, "error")) {
		// ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE) id, 0);
		// There is no ACKRESULT_FAILED for ACKTYPE_SEARCH :) look at findadd.c
		// So we will just send a SUCCESS
		ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) id, 0);
	}
}


void GetConfigItem(XmlNode *node, char *dest, BOOL bMethod, int *methodDest) {
	mir_strcpy(dest, node->text);
	TlenUrlDecode(dest);
	if (bMethod) {
		char *method = TlenXmlGetAttrValue(node, "method");
		if (method != NULL && !strcmpi(method, "POST")) {
			*methodDest = REQUEST_POST;
		} else {
			*methodDest = REQUEST_GET;
		}
	}
}

void TlenIqResultTcfg(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *queryNode, *miniMailNode, *node;

	char *type=TlenXmlGetAttrValue(iqNode, "type");
	if (type == NULL) return;
	if (!mir_strcmp(type, "result")) {
		if ((queryNode=TlenXmlGetChild(iqNode, "query")) == NULL) return;
		if ((miniMailNode=TlenXmlGetChild(queryNode, "mini-mail")) == NULL) return;
		if ((node=TlenXmlGetChild(miniMailNode, "base")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.mailBase, FALSE, NULL);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "msg")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.mailMsg, TRUE, &proto->threadData->tlenConfig.mailMsgMthd);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "index")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.mailIndex, TRUE, &proto->threadData->tlenConfig.mailIndexMthd);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "login")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.mailLogin, TRUE, &proto->threadData->tlenConfig.mailLoginMthd);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "compose")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.mailCompose, TRUE, &proto->threadData->tlenConfig.mailComposeMthd);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "avatar-get")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.avatarGet, TRUE, &proto->threadData->tlenConfig.avatarGetMthd);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "avatar-upload")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.avatarUpload, TRUE, &proto->threadData->tlenConfig.avatarUploadMthd);
		}
		if ((node=TlenXmlGetChild(miniMailNode, "avatar-remove")) != NULL) {
			GetConfigItem(node, proto->threadData->tlenConfig.avatarRemove, TRUE, &proto->threadData->tlenConfig.avatarRemoveMthd);
		}
	}
}

void TlenIqResultVersion(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *queryNode = TlenXmlGetChild(iqNode, "query");
	if (queryNode != NULL) {
		char *from = TlenXmlGetAttrValue(iqNode, "from");
		if (from != NULL ) {
			TLEN_LIST_ITEM *item = TlenListGetItemPtr(proto, LIST_ROSTER, from);
			if (item != NULL) {
				XmlNode *n;
				if ( item->software ) mir_free( item->software );
				if ( item->version ) mir_free( item->version );
				if ( item->system ) mir_free( item->system );
				if (( n=TlenXmlGetChild( queryNode, "name" )) != NULL && n->text ) {
					item->software = TlenTextDecode( n->text );
				} else
					item->software = NULL;
				if (( n=TlenXmlGetChild( queryNode, "version" )) != NULL && n->text )
					item->version = TlenTextDecode( n->text );
				else
					item->version = NULL;
				if (( n=TlenXmlGetChild( queryNode, "os" )) != NULL && n->text )
					item->system = TlenTextDecode( n->text );
				else
					item->system = NULL;
				MCONTACT hContact = TlenHContactFromJID(proto, item->jid);
				if (hContact != NULL) {
					if (item->software != NULL) {
						db_set_s(hContact, proto->m_szModuleName, "MirVer", item->software);
					} else {
						db_unset(hContact, proto->m_szModuleName, "MirVer");
					}
				}
			}
		}
	}
}

void TlenIqResultInfo(TlenProtocol *proto, XmlNode *iqNode)
{
	XmlNode *queryNode = TlenXmlGetChild(iqNode, "query");
	if (queryNode != NULL) {
		char *from=TlenXmlGetAttrValue(queryNode, "from");
		if (from != NULL ) {
			TLEN_LIST_ITEM *item=TlenListGetItemPtr(proto, LIST_ROSTER, from);
			if (item != NULL) {
				XmlNode *version = TlenXmlGetChild(queryNode, "version");
				if (version != NULL) {
					item->protocolVersion = TlenTextDecode(version->text);
					MCONTACT hContact=TlenHContactFromJID(proto, item->jid);
					if (hContact != NULL) {
						if (item->software == NULL) {
							char str[128];
							mir_snprintf(str, SIZEOF(str), "Tlen Protocol %s", item->protocolVersion);
							db_set_s(hContact, proto->m_szModuleName, "MirVer", str);
						}
					}
				}
			}
		}
	}
}

