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
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "resource.h"
#include "tlen_list.h"
#include "tlen_iq.h"
#include "tlen_p2p_old.h"
#include "tlen_avatar.h"
#include "tlen_file.h"

DWORD_PTR TlenProtocol::GetCaps(int type, MCONTACT hContact)
{
	switch(type) {
	case PFLAGNUM_1:
		return PF1_IM|PF1_AUTHREQ|PF1_SERVERCLIST|PF1_MODEMSG|PF1_BASICSEARCH|PF1_SEARCHBYEMAIL|PF1_EXTSEARCH|PF1_EXTSEARCHUI|PF1_SEARCHBYNAME|PF1_FILE;//|PF1_VISLIST|PF1_INVISLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_HEAVYDND|PF2_FREECHAT;
	case PFLAGNUM_3:
		return PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_HEAVYDND|PF2_FREECHAT;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH|PF4_NOCUSTOMAUTH|PF4_SUPPORTTYPING|PF4_AVATARS|PF4_IMSENDOFFLINE|PF4_OFFLINEFILES;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR) Translate("Tlen login");
	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR) "jid";
	default:
		return 0;
	}
}

INT_PTR TlenProtocol::GetName(WPARAM wParam, LPARAM lParam)
{
	strncpy((char*)lParam, m_szModuleName, wParam);
	return 0;
}

int TlenRunSearch(TlenProtocol *proto) {
	int iqId = 0;
	if (!proto->isOnline) return 0;
	if (proto->searchQuery != NULL && proto->searchIndex < 10) {
		iqId = proto->searchID;
		TlenIqAdd(proto, iqId, IQ_PROC_GETSEARCH, TlenIqResultSearch);
		if (proto->searchIndex == 0) {
			TlenSend(proto, "<iq type='get' id='"TLEN_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'>%s</query></iq>", iqId, proto->searchQuery);
		} else {
			TlenSend(proto, "<iq type='get' id='"TLEN_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'>%s<f>%d</f></query></iq>", iqId, proto->searchQuery, proto->searchIndex * TLEN_MAX_SEARCH_RESULTS_PER_PAGE);
		}
		proto->searchIndex ++;
	}
	return iqId;
}

void TlenResetSearchQuery(TlenProtocol *proto) {
	if (proto->searchQuery != NULL) {
		mir_free(proto->searchQuery);
		proto->searchQuery = NULL;
	}
	proto->searchQueryLen = 0;
	proto->searchIndex = 0;
	proto->searchID = TlenSerialNext(proto);
}

HANDLE TlenProtocol::SearchBasic(const PROTOCHAR* id)
{
	int iqId = 0;
	if (!isOnline) return 0;
	if (id == NULL) return 0;
	char* id_A = mir_t2a(id);
	char *jid=TlenTextEncode(id_A);
	if (jid != NULL) {
		searchJID = mir_strdup(id_A);
		TlenResetSearchQuery(this);
		TlenStringAppend(&searchQuery, &searchQueryLen, "<i>%s</i>", jid);
		iqId = TlenRunSearch(this);
		mir_free(jid);
	}
	mir_free(id_A);
	return (HANDLE)iqId;
}

HANDLE TlenProtocol::SearchByEmail(const PROTOCHAR* email)
{
	int iqId = 0;

	if (!isOnline) return 0;
	if (email == NULL) return 0;

	char* email_A = mir_t2a(email);
	char *emailEnc=TlenTextEncode(email_A);
	if (emailEnc != NULL) {
		TlenResetSearchQuery(this);
		TlenStringAppend(&searchQuery, &searchQueryLen, "<email>%s</email>", emailEnc);
		iqId = TlenRunSearch(this);
		mir_free(emailEnc);
	}
	mir_free(email_A);
	return (HANDLE)iqId;
}

HANDLE TlenProtocol::SearchByName(const PROTOCHAR* nickT, const PROTOCHAR* firstNameT, const PROTOCHAR* lastNameT)
{
	if (!isOnline) return 0;

	char *nick = mir_t2a(nickT);
	char *firstName = mir_t2a(firstNameT);
	char *lastName = mir_t2a(lastNameT);

	char *p;
	int iqId = 0;

	TlenResetSearchQuery(this);

	if (nick != NULL && nick[0] != '\0') {
		if ((p=TlenTextEncode(nick)) != NULL) {
			TlenStringAppend(&searchQuery, &searchQueryLen, "<nick>%s</nick>", p);
			mir_free(p);
		}
	}
	if (firstName != NULL && firstName[0] != '\0') {
		if ((p=TlenTextEncode(firstName)) != NULL) {
			TlenStringAppend(&searchQuery, &searchQueryLen, "<first>%s</first>", p);
			mir_free(p);
		}
	}
	if (lastName != NULL && lastName[0] != '\0') {
		if ((p=TlenTextEncode(lastName)) != NULL) {
			TlenStringAppend(&searchQuery, &searchQueryLen, "<last>%s</last>", p);
			mir_free(p);
		}
	}

	iqId = TlenRunSearch(this);
	return (HANDLE)iqId;
}

HWND TlenProtocol::CreateExtendedSearchUI(HWND owner)
{
	return (HWND) CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADVSEARCH), owner, TlenAdvSearchDlgProc);
}

HWND TlenProtocol::SearchAdvanced(HWND owner)
{
	if (!isOnline) return 0;

	TlenResetSearchQuery(this);
	int iqId = TlenSerialNext(this);
	if ((searchQuery = TlenAdvSearchCreateQuery(owner, iqId)) != NULL) {
		iqId = TlenRunSearch(this);
	}
	return (HWND)iqId;
}


static MCONTACT AddToListByJID(TlenProtocol *proto, const char *newJid, DWORD flags)
{
	MCONTACT hContact = TlenHContactFromJID(proto, newJid);
	if (hContact == NULL) {
		// not already there: add
		char *jid = mir_strdup(newJid); _strlwr(jid);
		hContact = (MCONTACT) CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM) proto->m_szModuleName);
		db_set_s(hContact, proto->m_szModuleName, "jid", jid);
		char *nick=TlenNickFromJID(newJid);
		if (nick == NULL)
			nick = mir_strdup(newJid);
		db_set_s(hContact, "CList", "MyHandle", nick);
		mir_free(nick);
		mir_free(jid);

		// Note that by removing or disable the "NotOnList" will trigger
		// the plugin to add a particular contact to the roster list.
		// See DBSettingChanged hook at the bottom part of this source file.
		// But the add module will delete "NotOnList". So we will not do it here.
		// Also because we need "MyHandle" and "Group" info, which are set after
		// PS_ADDTOLIST is called but before the add dialog issue deletion of
		// "NotOnList".
		// If temporary add, "NotOnList" won't be deleted, and that's expected.
		db_set_b(hContact, "CList", "NotOnList", 1);
		if (flags & PALF_TEMPORARY)
			db_set_b(hContact, "CList", "Hidden", 1);
	}
	else {
		// already exist
		// Set up a dummy "NotOnList" when adding permanently only
		if (!(flags&PALF_TEMPORARY))
			db_set_b(hContact, "CList", "NotOnList", 1);
	}

	return hContact;
}

MCONTACT TlenProtocol::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	TLEN_SEARCH_RESULT *jsr = (TLEN_SEARCH_RESULT*)psr;
	if (jsr->hdr.cbSize != sizeof(TLEN_SEARCH_RESULT))
		return NULL;
	return AddToListByJID(this, jsr->jid, flags);// wParam is flag e.g. PALF_TEMPORARY
}

MCONTACT TlenProtocol::AddToListByEvent(int flags, int iContact, MEVENT hDbEvent)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return NULL;
	if ((dbei.pBlob=(PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return NULL;
	if (db_event_get(hDbEvent, &dbei)) {
		mir_free(dbei.pBlob);
		return NULL;
	}
	if (strcmp(dbei.szModule, m_szModuleName)) {
		mir_free(dbei.pBlob);
		return NULL;
	}

/*
	// EVENTTYPE_CONTACTS is when adding from when we receive contact list (not used in Tlen)
	// EVENTTYPE_ADDED is when adding from when we receive "You are added" (also not used in Tlen)
	// Tlen will only handle the case of EVENTTYPE_AUTHREQUEST
	// EVENTTYPE_AUTHREQUEST is when adding from the authorization request dialog
*/

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return NULL;
	}

	char *nick = (char *)dbei.pBlob + sizeof(DWORD)*2;
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	MCONTACT hContact = (MCONTACT) AddToListByJID(this, jid, flags);
	mir_free(dbei.pBlob);
	return hContact;
}

int TlenProtocol::Authorize(MEVENT hDbEvent)
{
	if (!isOnline)
		return 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)-1)
		return 1;
	if ((dbei.pBlob=(PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return 1;
	if (db_event_get(hDbEvent, &dbei)){
		mir_free(dbei.pBlob);
		return 1;
	}
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (strcmp(dbei.szModule, m_szModuleName)) {
		mir_free(dbei.pBlob);
		return 1;
	}

	char *nick = (char *)dbei.pBlob + sizeof(DWORD)*2;
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	TlenSend(this, "<presence to='%s' type='subscribed'/>", jid);

	// Automatically add this user to my roster if option is enabled
	if (db_get_b(NULL, m_szModuleName, "AutoAdd", TRUE) == TRUE) {
		MCONTACT hContact;
		TLEN_LIST_ITEM *item = TlenListGetItemPtr(this, LIST_ROSTER, jid);

		if (item == NULL || (item->subscription != SUB_BOTH && item->subscription != SUB_TO)) {
			debugLogA("Try adding contact automatically jid=%s", jid);
			if ((hContact=AddToListByJID(this, jid, 0)) != NULL) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and TlenDbSettingChanged().
				db_unset(hContact, "CList", "NotOnList");
			}
		}
	}

	mir_free(dbei.pBlob);
	return 0;
}

int TlenProtocol::AuthDeny(MEVENT hDbEvent, const PROTOCHAR* szReason)
{
	if (!isOnline)
		return 1;

	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return 1;
	if ((dbei.pBlob = (PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return 1;
	if (db_event_get(hDbEvent, &dbei)) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (strcmp(dbei.szModule, m_szModuleName)) {
		mir_free(dbei.pBlob);
		return 1;
	}

	char *nick = (char *)dbei.pBlob + sizeof(DWORD)*2;
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	TlenSend(this, "<presence to='%s' type='unsubscribed'/>", jid);
	TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item jid='%s' subscription='remove'/></query></iq>", jid);
	mir_free(dbei.pBlob);
	return 0;
}

static void TlenConnect(TlenProtocol *proto, int initialStatus)
{
	if (!proto->isConnected) {
		ThreadData *thread = (ThreadData *) mir_alloc(sizeof(ThreadData));
		memset(thread, 0, sizeof(ThreadData));
		thread->proto = proto;
		proto->m_iDesiredStatus = initialStatus;

		int oldStatus = proto->m_iStatus;
		proto->m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, proto->m_iStatus);
		thread->hThread = (HANDLE) forkthread((void (__cdecl *)(void*))TlenServerThread, 0, thread);
	}
}

int TlenProtocol::SetStatus(int iNewStatus)
{
	int oldStatus;
	HANDLE s;

	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (threadData) {
			if (isConnected) {
				TlenSendPresence(this, ID_STATUS_OFFLINE);
			}

			// TODO bug? s = proto;
			s = threadData->s;

			threadData = NULL;
			if (isConnected) {
				Sleep(200);
//				TlenSend(s, "</s>");
				// Force closing connection
				isConnected = FALSE;
				isOnline = FALSE;
				Netlib_CloseHandle(s);
			}
		}
		else {
			if (m_iStatus != ID_STATUS_OFFLINE) {
				oldStatus = m_iStatus;
				m_iStatus = ID_STATUS_OFFLINE;
				ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
			}
		}
	}
	else if (iNewStatus != m_iStatus) {
		if (!isConnected){
			TlenConnect(this, iNewStatus);
		} else {
			// change status
			oldStatus = m_iStatus;
			// send presence update
			TlenSendPresence(this, iNewStatus);
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		}
	}
	return 0;
}

INT_PTR TlenProtocol::GetStatus(WPARAM wParam, LPARAM lParam)
{
	return m_iStatus;
}

int TlenProtocol::SetAwayMsg(int iStatus, const PROTOCHAR* msg)
{
	char **szMsg;
	char *newModeMsg;

	newModeMsg = mir_t2a(msg);

	debugLogA("SetAwayMsg called, wParam=%d lParam=%s", iStatus, newModeMsg);

	EnterCriticalSection(&modeMsgMutex);

	switch (iStatus) {
	case ID_STATUS_ONLINE:
		szMsg = &modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		szMsg = &modeMsgs.szAway;
		break;
	case ID_STATUS_NA:
		szMsg = &modeMsgs.szNa;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		szMsg = &modeMsgs.szDnd;
		break;
	case ID_STATUS_FREECHAT:
		szMsg = &modeMsgs.szFreechat;
		break;
	case ID_STATUS_INVISIBLE:
		szMsg = &modeMsgs.szInvisible;
		break;
	default:
		LeaveCriticalSection(&modeMsgMutex);
		return 1;
	}

	if ((*szMsg == NULL && newModeMsg == NULL) ||
		(*szMsg != NULL && newModeMsg != NULL && !strcmp(*szMsg, newModeMsg))) {
		// Message is the same, no update needed
		if (newModeMsg != NULL) mir_free(newModeMsg);
	}
	else {
		// Update with the new mode message
		if (*szMsg != NULL) mir_free(*szMsg);
		*szMsg = newModeMsg;
		// Send a presence update if needed
		if (iStatus == m_iStatus) {
			TlenSendPresence(this, m_iStatus);
		}
	}

	LeaveCriticalSection(&modeMsgMutex);
	return 0;
}

int TlenProtocol::GetInfo(MCONTACT hContact, int infoType)
{
	DBVARIANT dbv;
	int iqId;
	char *nick, *pNick;

	if (!isOnline) return 1;
	if (hContact == NULL) {
		iqId = TlenSerialNext(this);
		TlenIqAdd(this, iqId, IQ_PROC_NONE, TlenIqResultVcard);
		TlenSend(this, "<iq type='get' id='"TLEN_IQID"%d' to='tuba'><query xmlns='jabber:iq:register'></query></iq>", iqId);
	} else {
		if (db_get(hContact, m_szModuleName, "jid", &dbv)) return 1;
		if ((nick=TlenNickFromJID(dbv.pszVal)) != NULL) {
			if ((pNick=TlenTextEncode(nick)) != NULL) {
				iqId = TlenSerialNext(this);
				TlenIqAdd(this, iqId, IQ_PROC_NONE, TlenIqResultVcard);
				TlenSend(this, "<iq type='get' id='"TLEN_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'><i>%s</i></query></iq>", iqId, pNick);
				mir_free(pNick);
			}
			mir_free(nick);
		}
		db_free(&dbv);
	}
	return 0;
}

int TlenProtocol::SetApparentMode(MCONTACT hContact, int mode)
{
	DBVARIANT dbv;
	int oldMode;
	char *jid;

	if (!isOnline) return 0;
	if (!db_get_b(NULL, m_szModuleName, "VisibilitySupport", FALSE)) return 0;
	if (mode != 0 && mode != ID_STATUS_ONLINE && mode != ID_STATUS_OFFLINE) return 1;
	oldMode = db_get_w(hContact, m_szModuleName, "ApparentMode", 0);
	if ((int) mode == oldMode) return 1;
	db_set_w(hContact, m_szModuleName, "ApparentMode", (WORD) mode);
	if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
		jid = dbv.pszVal;
		switch (mode) {
		case ID_STATUS_ONLINE:
			if (m_iStatus == ID_STATUS_INVISIBLE || oldMode == ID_STATUS_OFFLINE)
				TlenSend(this, "<presence to='%s'><show>available</show></presence>", jid);
			break;
		case ID_STATUS_OFFLINE:
			if (m_iStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE)
				TlenSend(this, "<presence to='%s' type='invisible'/>", jid);
			break;
		case 0:
			if (oldMode == ID_STATUS_ONLINE && m_iStatus == ID_STATUS_INVISIBLE)
				TlenSend(this, "<presence to='%s' type='invisible'/>", jid);
			else if (oldMode == ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_INVISIBLE)
				TlenSend(this, "<presence to='%s'><show>available</show></presence>", jid);
			break;
		}
		db_free(&dbv);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct SENDACKTHREADDATA
{
	__inline SENDACKTHREADDATA(TlenProtocol *_ppro, MCONTACT _hContact, int _msgid=0) :
		proto(_ppro), hContact(_hContact), msgid(_msgid)
		{}

	TlenProtocol *proto;
	MCONTACT hContact;
	int msgid;
};

static void __cdecl TlenSendMessageAckThread(void *ptr)
{
	SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;
	SleepEx(10, TRUE);
	ProtoBroadcastAck(data->proto->m_szModuleName, data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)data->msgid, 0);
	delete data;
}

static void __cdecl TlenSendMessageFailedThread(void *ptr)
{
	SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;
	SleepEx(10, TRUE);
	ProtoBroadcastAck(data->proto->m_szModuleName, data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, 0);
	delete data;
}

static void __cdecl TlenGetAwayMsgThread(void *ptr)
{
	DBVARIANT dbv;
	SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;

	Sleep(50);

	if (!db_get(data->hContact, data->proto->m_szModuleName, "jid", &dbv)) {
		TLEN_LIST_ITEM *item=TlenListGetItemPtr(data->proto, LIST_ROSTER, dbv.pszVal);
		if (item != NULL) {
			db_free(&dbv);
			ProtoBroadcastAck(data->proto->m_szModuleName, data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1,
				item->statusMessage==NULL ? (LPARAM)NULL : (LPARAM)(TCHAR*)_A2T(item->statusMessage));
		} else {
			ptrA ownJid(db_get_sa(NULL, data->proto->m_szModuleName, "jid"));
			if (!strcmp(ownJid, dbv.pszVal)){
				DBVARIANT dbv2;
				if (!db_get_s(data->hContact, "CList", "StatusMsg", &dbv2, DBVT_TCHAR)){
					data->proto->ProtoBroadcastAck(data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv2.ptszVal);
					db_free(&dbv2);
				} else {
					data->proto->ProtoBroadcastAck(data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, NULL);
				}
			}
			db_free(&dbv);
		}
	} else {
		data->proto->ProtoBroadcastAck(data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, NULL);
	}

	delete data;
}

INT_PTR TlenProtocol::SendAlert(WPARAM hContact, LPARAM lParam)
{
	DBVARIANT dbv;
	if (isOnline && !db_get(hContact, m_szModuleName, "jid", &dbv)) {
		TlenSend(this, "<m tp='a' to='%s'/>", dbv.pszVal);

		db_free(&dbv);
	}
	return 0;
}

int TlenProtocol::SendMsg(MCONTACT hContact, int flags, const char* msgRAW)
{
	DBVARIANT dbv;
	if (!isOnline || db_get(hContact, m_szModuleName, "jid", &dbv)) {
		forkthread(TlenSendMessageFailedThread, 0, new SENDACKTHREADDATA(this, hContact, 2));
		return 2;
	}

	char* msg;
	if (flags & PREF_UNICODE)
		msg = mir_u2a((wchar_t*)&msgRAW[strlen(msgRAW) + 1]);
	else if (flags & PREF_UTF)
		msg = mir_utf8decodeA(msgRAW);
	else
		msg = mir_strdup(msgRAW);

	TLEN_LIST_ITEM *item;
	char msgType[16];


	int id = TlenSerialNext(this);

	if (!strcmp(msg, "<alert>")) {
		TlenSend(this, "<m tp='a' to='%s'/>", dbv.pszVal);
		forkthread(TlenSendMessageAckThread, 0, new SENDACKTHREADDATA(this, hContact, id));
	}
	else if (!strcmp(msg, "<image>")) {
		TlenSend(this, "<message to='%s' type='%s' crc='%x' idt='%d'/>", dbv.pszVal, "pic", 0x757f044, id);
		forkthread(TlenSendMessageAckThread, 0, new SENDACKTHREADDATA(this, hContact, id));
	}
	else {
		char *msgEnc=TlenTextEncode(msg);
		if (msgEnc != NULL) {
			if (TlenListExist(this, LIST_CHATROOM, dbv.pszVal) && strchr(dbv.pszVal, '/') == NULL)
				strcpy(msgType, "groupchat");
			else if (db_get_b(hContact, m_szModuleName, "bChat", FALSE))
				strcpy(msgType, "privchat");
			else
				strcpy(msgType, "chat");

			if (!strcmp(msgType, "groupchat") || db_get_b(NULL, m_szModuleName, "MsgAck", FALSE) == FALSE) {
				if (!strcmp(msgType, "groupchat"))
					TlenSend(this, "<message to='%s' type='%s'><body>%s</body></message>", dbv.pszVal, msgType, msgEnc);
				else if (!strcmp(msgType, "privchat"))
					TlenSend(this, "<m to='%s'><b n='6' s='10' f='0' c='000000'>%s</b></m>", dbv.pszVal, msgEnc);
				else
					TlenSend(this, "<message to='%s' type='%s' id='"TLEN_IQID"%d'><body>%s</body><x xmlns='jabber:x:event'><composing/></x></message>", dbv.pszVal, msgType, id, msgEnc);

				forkthread(TlenSendMessageAckThread, 0, new SENDACKTHREADDATA(this, hContact, id));
			}
			else {
				if ((item=TlenListGetItemPtr(this, LIST_ROSTER, dbv.pszVal)) != NULL)
					item->idMsgAckPending = id;
				TlenSend(this, "<message to='%s' type='%s' id='"TLEN_IQID"%d'><body>%s</body><x xmlns='jabber:x:event'><offline/><delivered/><composing/></x></message>", dbv.pszVal, msgType, id, msgEnc);
			}
		}
		mir_free(msgEnc);
	}

	mir_free(msg);
	db_free(&dbv);
	return id;
}

/////////////////////////////////////////////////////////////////////////////////////////
// TlenGetAvatarInfo - retrieves the avatar info

INT_PTR TlenProtocol::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!tlenOptions.enableAvatars) return GAIR_NOAVATAR;
	BOOL downloadingAvatar = FALSE;
	char *avatarHash = NULL;
	TLEN_LIST_ITEM *item = NULL;
	DBVARIANT dbv;
	PROTO_AVATAR_INFORMATIONT* AI = ( PROTO_AVATAR_INFORMATIONT* )lParam;

	if (AI->hContact != NULL) {
		if (!db_get(AI->hContact, m_szModuleName, "jid", &dbv)) {
			item = TlenListGetItemPtr(this, LIST_ROSTER, dbv.pszVal);
			db_free(&dbv);
			if (item != NULL) {
				downloadingAvatar = item->newAvatarDownloading;
				avatarHash = item->avatarHash;
			}
		}
	}
	else if (threadData != NULL)
		avatarHash = threadData->avatarHash;

	if ((avatarHash == NULL || avatarHash[0] == '\0') && !downloadingAvatar)
		return GAIR_NOAVATAR;

	if (avatarHash != NULL && !downloadingAvatar) {
		TlenGetAvatarFileName(this, item, AI->filename, SIZEOF(AI->filename)-1);
		AI->format = ( AI->hContact == NULL ) ? threadData->avatarFormat : item->avatarFormat;
		return GAIR_SUCCESS;
	}

	/* get avatar */
	if (( wParam & GAIF_FORCE ) != 0 && AI->hContact != NULL && isOnline)
		return GAIR_WAITFOR;

	return GAIR_NOAVATAR;
}

HANDLE TlenProtocol::GetAwayMsg(MCONTACT hContact)
{
	SENDACKTHREADDATA *tdata = new SENDACKTHREADDATA(this, hContact, 0);
	forkthread((void (__cdecl *)(void*))TlenGetAwayMsgThread, 0, (void*)tdata);
	return (HANDLE)1;
}

int TlenProtocol::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt)
{
	return 0;
}

HANDLE TlenProtocol::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	if (!isOnline) return 0;

	TLEN_FILE_TRANSFER *ft = (TLEN_FILE_TRANSFER *) hTransfer;
	ft->szSavePath = mir_strdup(mir_t2a(szPath));	//TODO convert to PROTOCHAR*
	TLEN_LIST_ITEM *item = TlenListAdd(this, LIST_FILE, ft->iqId);
	if (item != NULL) {
		item->ft = ft;
	}
	char *nick = TlenNickFromJID(ft->jid);
	if (ft->newP2P) {
		TlenSend(this, "<iq to='%s'><query xmlns='p2p'><fs t='%s' e='5' i='%s' v='1'/></query></iq>", ft->jid, ft->jid, ft->iqId);
	} else {
		TlenSend(this, "<f t='%s' i='%s' e='5' v='1'/>", nick, ft->iqId);
	}
	mir_free(nick);
	return (HANDLE)hTransfer;
}

int TlenProtocol::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason)
{
	if (!isOnline) return 1;

	TLEN_FILE_TRANSFER *ft = (TLEN_FILE_TRANSFER *) hTransfer;
	char *nick = TlenNickFromJID(ft->jid);
	if (ft->newP2P) {
		TlenSend(this, "<f i='%s' e='4' t='%s'/>", ft->iqId, nick);\
	} else {
		TlenSend(this, "<f i='%s' e='4' t='%s'/>", ft->iqId, nick);\
	}
	mir_free(nick);
	TlenP2PFreeFileTransfer(ft);
	return 0;
}

int TlenProtocol::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename) {
	return 0;
}

int TlenProtocol::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	TLEN_FILE_TRANSFER *ft = (TLEN_FILE_TRANSFER *) hTransfer;
	debugLogA("Invoking FileCancel()");
	if (ft->s != NULL) {
		ft->state = FT_ERROR;
		Netlib_CloseHandle(ft->s);
		ft->s = NULL;
		if (ft->hFileEvent != NULL) {
			HANDLE hEvent = ft->hFileEvent;
			ft->hFileEvent = NULL;
			SetEvent(hEvent);
		}
	} else {
		TlenP2PFreeFileTransfer(ft);
	}
	return 0;
}

HANDLE TlenProtocol::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	int i, j;
	struct _stat statbuf;
	DBVARIANT dbv;
	char *nick, *p, idStr[10];

	if (!isOnline) return 0;
//	if (db_get_w(ccs->hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) return 0;
	if (db_get(hContact, m_szModuleName, "jid", &dbv)) return 0;
	TLEN_FILE_TRANSFER *ft = TlenFileCreateFT(this, dbv.pszVal);
	for (ft->fileCount=0; ppszFiles[ft->fileCount]; ft->fileCount++);
	ft->files = (char **) mir_alloc(sizeof(char *) * ft->fileCount);
	ft->filesSize = (long *) mir_alloc(sizeof(long) * ft->fileCount);
	ft->allFileTotalSize = 0;
	for (i=j=0; i<ft->fileCount; i++) {
		char* ppszFiles_i_A = mir_t2a(ppszFiles[i]);
		if (_stat(ppszFiles_i_A, &statbuf))
			debugLogA("'%s' is an invalid filename", ppszFiles[i]);
		else {
			ft->filesSize[j] = statbuf.st_size;
			ft->files[j++] = mir_strdup(ppszFiles_i_A);
			ft->allFileTotalSize += statbuf.st_size;
		}
		mir_free(ppszFiles_i_A);
	}
	ft->fileCount = j;
	ft->szDescription = mir_t2a(szDescription);
	ft->hContact = hContact;
	ft->currentFile = 0;
	db_free(&dbv);

	int id = TlenSerialNext(this);
	mir_snprintf(idStr, SIZEOF(idStr), "%d", id);
	TLEN_LIST_ITEM *item = TlenListAdd(this, LIST_FILE, idStr);
	if (item != NULL) {
		ft->iqId = mir_strdup(idStr);
		nick = TlenNickFromJID(ft->jid);
		item->ft = ft;
		if (tlenOptions.useNewP2P) {
			TlenSend(this, "<iq to='%s'><query xmlns='p2p'><fs t='%s' e='1' i='%s' c='%d' s='%d' v='%d'/></query></iq>",
				ft->jid, ft->jid, idStr, ft->fileCount, ft->allFileTotalSize, ft->fileCount);

			ft->newP2P = TRUE;
		} else {
			if (ft->fileCount == 1) {
				char* ppszFiles_0_A = mir_t2a(ppszFiles[0]);
				if ((p=strrchr(ppszFiles_0_A, '\\')) != NULL) {
					p++;
				} else {
					p = ppszFiles_0_A;
				}
				p = TlenTextEncode(p);
				TlenSend(this, "<f t='%s' n='%s' e='1' i='%s' c='1' s='%d' v='1'/>", nick, p, idStr, ft->allFileTotalSize);
				mir_free(ppszFiles[0]);
				mir_free(p);
			} else {
				TlenSend(this, "<f t='%s' e='1' i='%s' c='%d' s='%d' v='1'/>", nick, idStr, ft->fileCount, ft->allFileTotalSize);
			}
		}
		mir_free(nick);
	}

	return (HANDLE) ft;
}

int TlenProtocol::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList){
	return 0;
}

int TlenProtocol::SendUrl(MCONTACT hContact, int flags, const char* urlt){
	return 0;
}

int TlenProtocol::RecvMsg(MCONTACT hContact, PROTORECVEVENT* evt)
{
	return Proto_RecvMessage(hContact, evt);
}

int TlenProtocol::RecvFile(MCONTACT hContact, PROTOFILEEVENT* evt)
{
	return Proto_RecvFile(hContact, evt);
}

int TlenProtocol::RecvUrl(MCONTACT hContact, PROTORECVEVENT*)
{
	return 0;
}

static char* settingToChar( DBCONTACTWRITESETTING* cws )
{
	switch( cws->value.type ) {
	case DBVT_ASCIIZ:
			return mir_strdup( cws->value.pszVal );
	case DBVT_UTF8:
			return mir_utf8decode(mir_strdup(cws->value.pszVal), NULL);
	}
	return NULL;
}

int TlenProtocol::TlenDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT) wParam;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	// no action for hContact == NULL or when offline
	if (hContact == NULL) return 0;
	if (!isConnected) return 0;

	if (!strcmp(cws->szModule, "CList")) {
		DBVARIANT dbv;
		TLEN_LIST_ITEM *item;
		char *nick, *jid, *group;

		char *szProto = GetContactProto(hContact);
		if (szProto == NULL || strcmp(szProto, m_szModuleName)) return 0;
		// A contact's group is changed
		if (!strcmp(cws->szSetting, "Group")) {
			if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
				if ((item=TlenListGetItemPtr(this, LIST_ROSTER, dbv.pszVal)) != NULL) {
					db_free(&dbv);
					if (!db_get(hContact, "CList", "MyHandle", &dbv)) {
						nick = TlenTextEncode(dbv.pszVal);
						db_free(&dbv);
					}
					else if (!db_get(hContact, this->m_szModuleName, "Nick", &dbv)) {
						nick = TlenTextEncode(dbv.pszVal);
						db_free(&dbv);
					}
					else nick = TlenNickFromJID(item->jid);

					if (nick != NULL) {
						// Note: we need to compare with item->group to prevent infinite loop
						if (cws->value.type == DBVT_DELETED && item->group != NULL) {
							debugLogA("Group set to nothing");
							TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'></item></query></iq>", nick, item->jid);
						}
						else if (cws->value.pszVal != NULL) {
							char *newGroup = settingToChar(cws);
							if (item->group == NULL || strcmp(newGroup, item->group)) {
								debugLogA("Group set to %s", newGroup);
								if ((group=TlenGroupEncode(newGroup)) != NULL) {
									TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, item->jid, group);
									mir_free(group);
								}
							}
							mir_free(newGroup);
						}
						mir_free(nick);
					}
				}
				else {
					db_free(&dbv);
				}
			}
		}
		// A contact is renamed
		else if (!strcmp(cws->szSetting, "MyHandle")) {
			char *newNick;

//			hContact = (MCONTACT) wParam;
//			szProto = GetContactProto(hContact);
//			if (szProto == NULL || strcmp(szProto, proto->m_szModuleName)) return 0;

			if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
				jid = dbv.pszVal;
				if ((item=TlenListGetItemPtr(this, LIST_ROSTER, dbv.pszVal)) != NULL) {
					if (cws->value.type == DBVT_DELETED) {
						newNick = mir_strdup((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_NOMYHANDLE));
					} else if (cws->value.pszVal != NULL) {
						newNick = settingToChar(cws);
					} else {
						newNick = NULL;
					}
					// Note: we need to compare with item->nick to prevent infinite loop
					if (newNick != NULL && (item->nick == NULL || (item->nick != NULL && strcmp(item->nick, newNick)))) {
						if ((nick=TlenTextEncode(newNick)) != NULL) {
							debugLogA("Nick set to %s", newNick);
							if (item->group != NULL && (group=TlenGroupEncode(item->group)) != NULL) {
								TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, jid, group);
								mir_free(group);
							} else {
								TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'></item></query></iq>", nick, jid);
							}
							mir_free(nick);
						}
					}
					if (newNick != NULL) mir_free(newNick);
				}
				db_free(&dbv);
			}
		}
		// A temporary contact has been added permanently
		else if (!strcmp(cws->szSetting, "NotOnList")) {
			char *jid, *nick, *pGroup;

			if (cws->value.type==DBVT_DELETED || (cws->value.type==DBVT_BYTE && cws->value.bVal==0)) {
				if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
					jid = mir_strdup(dbv.pszVal);
					db_free(&dbv);
					debugLogA("Add %s permanently to list", jid);
					if (!db_get(hContact, "CList", "MyHandle", &dbv)) {
						nick = TlenTextEncode(dbv.pszVal); //Utf8Encode
						db_free(&dbv);
					}
					else {
						nick = TlenNickFromJID(jid);
					}
					if (nick != NULL) {
						debugLogA("jid=%s nick=%s", jid, nick);
						if (!db_get(hContact, "CList", "Group", &dbv)) {
							if ((pGroup=TlenGroupEncode(dbv.pszVal)) != NULL) {
								TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, jid, pGroup);
								TlenSend(this, "<presence to='%s' type='subscribe'/>", jid);
								mir_free(pGroup);
							}
							db_free(&dbv);
						}
						else {
							TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'/></query></iq>", nick, jid);
							TlenSend(this, "<presence to='%s' type='subscribe'/>", jid);
						}
						mir_free(nick);
						db_unset(hContact, "CList", "Hidden");
					}
					mir_free(jid);
				}
			}
		}
	}

	return 0;
}

int TlenProtocol::TlenContactDeleted(WPARAM wParam, LPARAM lParam)
{
	if (!isOnline)	// should never happen
		return 0;

	char *szProto = GetContactProto(wParam);
	if (szProto == NULL || strcmp(szProto, m_szModuleName))
		return 0;

	DBVARIANT dbv;
	if (!db_get(wParam, m_szModuleName, "jid", &dbv)) {
		char *p, *q;

		char *jid = dbv.pszVal;
		if ((p=strchr(jid, '@')) != NULL) {
			if ((q=strchr(p, '/')) != NULL)
				*q = '\0';
		}

		// Remove from roster, server also handles the presence unsubscription process.
		if (TlenListExist(this, LIST_ROSTER, jid))
			TlenSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item jid='%s' subscription='remove'/></query></iq>", jid);

		db_free(&dbv);
	}
	return 0;
}

int TlenProtocol::UserIsTyping(MCONTACT hContact, int type)
{
	DBVARIANT dbv;

	if (!isOnline) return 0;
	if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
		TLEN_LIST_ITEM *item = TlenListGetItemPtr(this, LIST_ROSTER, dbv.pszVal);
		if (item != NULL /*&& item->wantComposingEvent == TRUE*/) {
			switch (type) {
			case PROTOTYPE_SELFTYPING_OFF:
				TlenSend(this, "<m tp='u' to='%s'/>", dbv.pszVal);
				break;
			case PROTOTYPE_SELFTYPING_ON:
				TlenSend(this, "<m tp='t' to='%s'/>", dbv.pszVal);
				break;
			}
		}
		db_free(&dbv);
	}
	return 0;
}

INT_PTR TlenProtocol::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR* buf = (TCHAR*)wParam;
	int  size = ( int )lParam;
	if ( buf == NULL || size <= 0 )
		return -1;

	TlenGetAvatarFileName(this, NULL, buf, size);
	return 0;
}

static INT_PTR CALLBACK TlenChangeAvatarDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{
			HICON hIcon = GetIcolibIcon(IDI_TLEN);
			SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) hIcon);
			ReleaseIcolibIcon(hIcon);
		}
		CheckDlgButton(hwndDlg, IDC_PUBLICAVATAR, BST_CHECKED);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				int result = LOWORD(wParam);
				if (IsDlgButtonChecked(hwndDlg, IDC_PUBLICAVATAR)) {
					result |= 0x10000;
				}
				EndDialog(hwndDlg, result);
			}
			return TRUE;
		}
		break;
	}
	return 0;
}

INT_PTR TlenProtocol::SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!isOnline){
		PUShowMessageT(TranslateT("You need to be connected to Tlen account to set avatar."), SM_WARNING);
		return 1;
	}
	TCHAR* szFileName = ( TCHAR* )lParam;
	TCHAR tFileName[ MAX_PATH ];
	if (szFileName != NULL) {
		int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_USER_CHANGEAVATAR), NULL, TlenChangeAvatarDlgProc, (LPARAM) NULL);
		TlenGetAvatarFileName(this, NULL, tFileName, MAX_PATH);
		if ( CopyFile( szFileName, tFileName, FALSE ) == FALSE )
			return 1;

		char* tFileNameA = mir_t2a(tFileName); //TODO - drop io.h
		int fileIn = open( tFileNameA, O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
		if ( fileIn != -1 ) {
			long  dwPngSize = filelength(fileIn);
			BYTE* pResult = (BYTE *)mir_alloc(dwPngSize);
			if (pResult != NULL) {
				read( fileIn, pResult, dwPngSize );
				close( fileIn );
				TlenUploadAvatar(this, pResult, dwPngSize, (result & 0x10000) != 0);
				mir_free(pResult);
			}
		} else debugLogA("SetMyAvatar open error");
		mir_free(tFileName);
		mir_free(tFileNameA);
	}
	else TlenRemoveAvatar(this);

	return 0;
}

INT_PTR TlenProtocol::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		{
			POINT* size = (POINT*)lParam;
			if ( size )
				size->x = size->y = 64;
		}
		return 0;
	case AF_PROPORTION:
		return PIP_SQUARE;
	case AF_FORMATSUPPORTED:
		return (lParam == PA_FORMAT_PNG) ? 1 : 0;
	case AF_ENABLED:
		return tlenOptions.enableAvatars;
	case AF_DONTNEEDDELAYS:
		return 1;
	case AF_MAXFILESIZE:
		return 10 * 1024;
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	default:
		return 0;
	}
}

int TlenProtocol::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	//TlenProtocol *proto = (TlenProtocol *)ptr;
	switch( iEventType ) {
	case EV_PROTO_ONLOAD:    return OnModulesLoaded(0, 0);
	case EV_PROTO_ONOPTIONS: return OptionsInit(wParam, lParam);
	case EV_PROTO_ONEXIT:    return PreShutdown(0, 0);

	case EV_PROTO_ONRENAME:
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_NAME | CMIF_TCHAR;
		mi.ptszName = m_tszUserName;
		Menu_ModifyItem(hMenuRoot, &mi);
		/* FIXME: Rename network user as well */
	}
	return 1;
}

// PSS_ADDED
int TlenProtocol::AuthRecv(MCONTACT hContact, PROTORECVEVENT* evt)
{
	return 1;
}

// PSS_AUTHREQUEST
int TlenProtocol::AuthRequest(MCONTACT hContact, const PROTOCHAR* szMessage)
{
	return 1;
}

int TlenProtocol::RecvContacts(MCONTACT hContact, PROTORECVEVENT* evt)
{
	return 1;
}

extern INT_PTR CALLBACK TlenAccMgrUIDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR TlenProtocol::AccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, TlenAccMgrUIDlgProc, (LPARAM)this);
}

void TlenInitServicesVTbl(TlenProtocol *proto)
{
	proto->CreateProtoService(PS_GETNAME,        &TlenProtocol::GetName);
	proto->CreateProtoService(PS_GETAVATARINFOT, &TlenProtocol::GetAvatarInfo);
	proto->CreateProtoService(PS_SEND_NUDGE,     &TlenProtocol::SendAlert);
	proto->CreateProtoService(PS_GETAVATARCAPS,  &TlenProtocol::GetAvatarCaps);
	proto->CreateProtoService(PS_SETMYAVATART,   &TlenProtocol::SetMyAvatar);
	proto->CreateProtoService(PS_GETMYAVATART,   &TlenProtocol::GetMyAvatar);
	proto->CreateProtoService(PS_GETSTATUS,      &TlenProtocol::GetStatus);
	proto->CreateProtoService(PS_CREATEACCMGRUI, &TlenProtocol::AccMgrUI);
}

TlenProtocol::TlenProtocol( const char *aProtoName, const TCHAR *aUserName) :
	PROTO<TlenProtocol>(aProtoName, aUserName)
{
	TlenInitServicesVTbl(this);

	InitializeCriticalSection(&modeMsgMutex);
	InitializeCriticalSection(&csSend);

	hTlenNudge = CreateProtoEvent("/Nudge");

	HookProtoEvent(ME_OPT_INITIALISE,            &TlenProtocol::OptionsInit);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &TlenProtocol::TlenDbSettingChanged);
	HookProtoEvent(ME_DB_CONTACT_DELETED,        &TlenProtocol::TlenContactDeleted);
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &TlenProtocol::PrebuildContactMenu);

	DBVARIANT dbv;
	if (!db_get(NULL, m_szModuleName, "LoginServer", &dbv))
		db_free(&dbv);
	else
		db_set_s(NULL, m_szModuleName, "LoginServer", "tlen.pl");

	if (!db_get(NULL, m_szModuleName, "ManualHost", &dbv))
		db_free(&dbv);
	else
		db_set_s(NULL, m_szModuleName, "ManualHost", "s1.tlen.pl");

	TlenLoadOptions(this);

	TlenWsInit(this);
	TlenSerialInit(this);
	TlenIqInit(this);
	TlenListInit(this);

	initMenuItems();
}

TlenProtocol::~TlenProtocol()
{
	TlenVoiceCancelAll(this);
	TlenFileCancelAll(this);
	if (hTlenNudge)
		DestroyHookableEvent(hTlenNudge);
	TlenListUninit(this);
	TlenIqUninit(this);
	TlenSerialUninit(this);
	DeleteCriticalSection(&modeMsgMutex);
	DeleteCriticalSection(&csSend);
	TlenWsUninit(this);

	mir_free(modeMsgs.szOnline);
	mir_free(modeMsgs.szAway);
	mir_free(modeMsgs.szNa);
	mir_free(modeMsgs.szDnd);
	mir_free(modeMsgs.szFreechat);
	mir_free(modeMsgs.szInvisible);
}
