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

#include "jabber.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "resource.h"
#include "jabber_list.h"
#include "jabber_iq.h"
#include "tlen_p2p_old.h"
#include "tlen_avatar.h"
#include "tlen_file.h"

extern int TlenOnModulesLoaded(void *ptr, WPARAM wParam, LPARAM lParam);
extern int TlenOptionsInit(void *ptr, WPARAM wParam, LPARAM lParam);
extern int TlenPreShutdown(void *ptr, WPARAM wParam, LPARAM lParam);
extern int TlenSystemModulesLoaded(void *ptr, WPARAM wParam, LPARAM lParam);
extern int TlenPrebuildContactMenu(void *ptr, WPARAM wParam, LPARAM lParam);

DWORD_PTR __cdecl TlenProtocol::GetCaps(int type, HANDLE hContact)
{
	if (type == PFLAGNUM_1)
		return PF1_IM|PF1_AUTHREQ|PF1_SERVERCLIST|PF1_MODEMSG|PF1_BASICSEARCH|PF1_SEARCHBYEMAIL|PF1_EXTSEARCH|PF1_EXTSEARCHUI|PF1_SEARCHBYNAME|PF1_FILE;//|PF1_VISLIST|PF1_INVISLIST;
	if (type == PFLAGNUM_2)
		return PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_HEAVYDND|PF2_FREECHAT;
	if (type == PFLAGNUM_3)
		return PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_HEAVYDND|PF2_FREECHAT;
	if (type == PFLAGNUM_4)
		return PF4_FORCEAUTH|PF4_NOCUSTOMAUTH|PF4_SUPPORTTYPING|PF4_AVATARS|PF4_IMSENDOFFLINE|PF4_OFFLINEFILES;
	if (type == PFLAG_UNIQUEIDTEXT)
		return (DWORD_PTR) Translate("Tlen login");
	if (type == PFLAG_UNIQUEIDSETTING)
		return (DWORD_PTR) "jid";
	return 0;
}

INT_PTR TlenGetName(void *ptr, LPARAM wParam, LPARAM lParam)
{
	TlenProtocol *proto = (TlenProtocol *)ptr;
	strncpy((char*)lParam, proto->m_szModuleName, wParam);
	return 0;
}

int TlenRunSearch(TlenProtocol *proto) {
	int iqId = 0;
	if (!proto->isOnline) return 0;
	if (proto->searchQuery != NULL && proto->searchIndex < 10) {
		iqId = proto->searchID;
		JabberIqAdd(proto, iqId, IQ_PROC_GETSEARCH, JabberIqResultSearch);
		if (proto->searchIndex == 0) {
			JabberSend(proto, "<iq type='get' id='"JABBER_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'>%s</query></iq>", iqId, proto->searchQuery);
		} else {
			JabberSend(proto, "<iq type='get' id='"JABBER_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'>%s<f>%d</f></query></iq>", iqId, proto->searchQuery, proto->searchIndex * TLEN_MAX_SEARCH_RESULTS_PER_PAGE);
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
	proto->searchID = JabberSerialNext(proto);
}

HANDLE __cdecl TlenProtocol::SearchBasic(const PROTOCHAR* id)
{
	char *jid;
	int iqId = 0;
	if (!isOnline) return 0;
	if (id == NULL) return 0;
	char* id_A = mir_t2a(id);
	if ((jid=JabberTextEncode(id_A)) != NULL) {
		searchJID = mir_strdup(id_A);
		TlenResetSearchQuery(this);
		JabberStringAppend(&searchQuery, &searchQueryLen, "<i>%s</i>", jid);
		iqId = TlenRunSearch(this);
		mir_free(jid);
	}
	mir_free(id_A);
	return (HANDLE)iqId;
}

HANDLE __cdecl TlenProtocol::SearchByEmail(const PROTOCHAR* email)
{
	char *emailEnc;
	int iqId = 0;

	if (!isOnline) return 0;
	if (email == NULL) return 0;

	char* email_A = mir_t2a(email);
	if ((emailEnc=JabberTextEncode(email_A)) != NULL) {
		TlenResetSearchQuery(this);
		JabberStringAppend(&searchQuery, &searchQueryLen, "<email>%s</email>", emailEnc);
		iqId = TlenRunSearch(this);
		mir_free(emailEnc);
	}
	mir_free(email_A);
	return (HANDLE)iqId;
}

HANDLE __cdecl TlenProtocol::SearchByName(const PROTOCHAR* nickT, const PROTOCHAR* firstNameT, const PROTOCHAR* lastNameT)
{

	char* nick = mir_t2a(nickT);
	char* firstName = mir_t2a(firstNameT);
	char* lastName = mir_t2a(lastNameT);

	char *p;
	int iqId = 0;

	if (!isOnline) return 0;

	TlenResetSearchQuery(this);

	if (nick != NULL && nick[0] != '\0') {
		if ((p=JabberTextEncode(nick)) != NULL) {
			JabberStringAppend(&searchQuery, &searchQueryLen, "<nick>%s</nick>", p);
			mir_free(p);
		}
	}
	if (firstName != NULL && firstName[0] != '\0') {
		if ((p=JabberTextEncode(firstName)) != NULL) {
			JabberStringAppend(&searchQuery, &searchQueryLen, "<first>%s</first>", p);
			mir_free(p);
		}
	}
	if (lastName != NULL && lastName[0] != '\0') {
		if ((p=JabberTextEncode(lastName)) != NULL) {
			JabberStringAppend(&searchQuery, &searchQueryLen, "<last>%s</last>", p);
			mir_free(p);
		}
	}

	iqId = TlenRunSearch(this);
	return (HANDLE)iqId;
}

HWND __cdecl TlenProtocol::CreateExtendedSearchUI(HWND owner)
{
	return (HWND) CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADVSEARCH), owner, TlenAdvSearchDlgProc);
}

HWND __cdecl TlenProtocol::SearchAdvanced(HWND owner)
{
	int iqId;
	if (!isOnline) return 0;

	TlenResetSearchQuery(this);
	iqId = JabberSerialNext(this);
	if ((searchQuery = TlenAdvSearchCreateQuery(owner, iqId)) != NULL) {
		iqId = TlenRunSearch(this);
	}
	return (HWND)iqId;
}


static HANDLE AddToListByJID(TlenProtocol *proto, const char *newJid, DWORD flags)
{
	HANDLE hContact;
	char *jid, *nick;

	if ((hContact=JabberHContactFromJID(proto, newJid)) == NULL) {
		// not already there: add
		jid = mir_strdup(newJid); _strlwr(jid);
		hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) proto->m_szModuleName);
		db_set_s(hContact, proto->m_szModuleName, "jid", jid);
		if ((nick=JabberNickFromJID(newJid)) == NULL)
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

HANDLE __cdecl TlenProtocol::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	HANDLE hContact;
	JABBER_SEARCH_RESULT *jsr = (JABBER_SEARCH_RESULT*)psr;
	if (jsr->hdr.cbSize != sizeof(JABBER_SEARCH_RESULT))
		return (int) NULL;
	hContact = AddToListByJID(this, jsr->jid, flags);	// wParam is flag e.g. PALF_TEMPORARY
	return hContact;
}

HANDLE __cdecl TlenProtocol::AddToListByEvent( int flags, int iContact, HANDLE hDbEvent )
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == (DWORD)(-1))
		return (HANDLE) NULL;
	if ((dbei.pBlob=(PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return (HANDLE) NULL;
	if (db_event_get(hDbEvent, &dbei)) {
		mir_free(dbei.pBlob);
		return (HANDLE) NULL;
	}
	if (strcmp(dbei.szModule, m_szModuleName)) {
		mir_free(dbei.pBlob);
		return (HANDLE) NULL;
	}

/*
	// EVENTTYPE_CONTACTS is when adding from when we receive contact list (not used in Jabber)
	// EVENTTYPE_ADDED is when adding from when we receive "You are added" (also not used in Jabber)
	// Jabber will only handle the case of EVENTTYPE_AUTHREQUEST
	// EVENTTYPE_AUTHREQUEST is when adding from the authorization request dialog
*/

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return (HANDLE) NULL;
	}

	char *nick = (char *)dbei.pBlob + sizeof(DWORD)*2;
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *jid = lastName + strlen(lastName) + 1;

	HANDLE hContact = (HANDLE) AddToListByJID(this, jid, flags);
	mir_free(dbei.pBlob);
	return hContact;
}

int __cdecl TlenProtocol::Authorize(HANDLE hDbEvent)
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

	JabberSend(this, "<presence to='%s' type='subscribed'/>", jid);

	// Automatically add this user to my roster if option is enabled
	if (db_get_b(NULL, m_szModuleName, "AutoAdd", TRUE) == TRUE) {
		HANDLE hContact;
		JABBER_LIST_ITEM *item;

		if ((item=JabberListGetItemPtr(this, LIST_ROSTER, jid)) == NULL || (item->subscription != SUB_BOTH && item->subscription != SUB_TO)) {
			JabberLog(this, "Try adding contact automatically jid=%s", jid);
			if ((hContact=AddToListByJID(this, jid, 0)) != NULL) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				db_unset(hContact, "CList", "NotOnList");
			}
		}
	}

	mir_free(dbei.pBlob);
	return 0;
}

int __cdecl TlenProtocol::AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason)
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

	JabberSend(this, "<presence to='%s' type='unsubscribed'/>", jid);
	JabberSend(this, "<iq type='set'><query xmlns='jabber:iq:roster'><item jid='%s' subscription='remove'/></query></iq>", jid);
	mir_free(dbei.pBlob);
	return 0;
}

static void TlenConnect(TlenProtocol *proto, int initialStatus)
{
	if (!proto->isConnected) {
		ThreadData *thread;
		int oldStatus;

		thread = (ThreadData *) mir_alloc(sizeof(ThreadData));
		memset(thread, 0, sizeof(ThreadData));
		thread->proto = proto;
		proto->m_iDesiredStatus = initialStatus;

		oldStatus = proto->m_iStatus;
		proto->m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(proto->m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, proto->m_iStatus);
		thread->hThread = (HANDLE) JabberForkThread((void (__cdecl *)(void*))JabberServerThread, 0, thread);
	}
}

int __cdecl TlenProtocol::SetStatus(int iNewStatus)
{
	int oldStatus;
	HANDLE s;

	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (threadData) {
			if (isConnected) {
				JabberSendPresence(this, ID_STATUS_OFFLINE);
			}

			// TODO bug? s = proto;
			s = threadData->s;

			threadData = NULL;
			if (isConnected) {
				Sleep(200);
//				JabberSend(s, "</s>");
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
		if (!isConnected)
			TlenConnect(this, iNewStatus);
		else {
			// change status
			oldStatus = m_iStatus;
			// send presence update
			JabberSendPresence(this, iNewStatus);
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, m_iStatus);
		}
	}
	return 0;
}

INT_PTR TlenGetStatus(void *ptr, LPARAM wParam, LPARAM lParam)
{
	TlenProtocol *proto = (TlenProtocol *)ptr;
	return proto->m_iStatus;
}


int __cdecl TlenProtocol::SetAwayMsg(int iStatus, const PROTOCHAR* msg)
{
	char **szMsg;
	char *newModeMsg;

	JabberLog(this, "SetAwayMsg called, wParam=%d lParam=%s", iStatus, msg);

	newModeMsg = JabberTextEncode(mir_t2a(msg)); //TODO TCHAR

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
			JabberSendPresence(this, m_iStatus);
		}
	}

	LeaveCriticalSection(&modeMsgMutex);
	return 0;
}

int __cdecl TlenProtocol::GetInfo(HANDLE hContact, int infoType)
{
	DBVARIANT dbv;
	int iqId;
	char *nick, *pNick;

	if (!isOnline) return 1;
	if (hContact == NULL) {
		iqId = JabberSerialNext(this);
		JabberIqAdd(this, iqId, IQ_PROC_NONE, TlenIqResultVcard);
		JabberSend(this, "<iq type='get' id='"JABBER_IQID"%d' to='tuba'><query xmlns='jabber:iq:register'></query></iq>", iqId);
	} else {
		if (db_get(hContact, m_szModuleName, "jid", &dbv)) return 1;
		if ((nick=JabberNickFromJID(dbv.pszVal)) != NULL) {
			if ((pNick=JabberTextEncode(nick)) != NULL) {
				iqId = JabberSerialNext(this);
				JabberIqAdd(this, iqId, IQ_PROC_NONE, TlenIqResultVcard);
				JabberSend(this, "<iq type='get' id='"JABBER_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'><i>%s</i></query></iq>", iqId, pNick);
				mir_free(pNick);
			}
			mir_free(nick);
		}
		db_free(&dbv);
	}
	return 0;
}

int __cdecl TlenProtocol::SetApparentMode(HANDLE hContact, int mode)
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
				JabberSend(this, "<presence to='%s'><show>available</show></presence>", jid);
			break;
		case ID_STATUS_OFFLINE:
			if (m_iStatus != ID_STATUS_INVISIBLE || oldMode == ID_STATUS_ONLINE)
				JabberSend(this, "<presence to='%s' type='invisible'/>", jid);
			break;
		case 0:
			if (oldMode == ID_STATUS_ONLINE && m_iStatus == ID_STATUS_INVISIBLE)
				JabberSend(this, "<presence to='%s' type='invisible'/>", jid);
			else if (oldMode == ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_INVISIBLE)
				JabberSend(this, "<presence to='%s'><show>available</show></presence>", jid);
			break;
		}
		db_free(&dbv);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct SENDACKTHREADDATA
{
	__inline SENDACKTHREADDATA(TlenProtocol *_ppro, HANDLE _hContact, int _msgid=0) :
		proto(_ppro), hContact(_hContact), msgid(_msgid)
		{}

	TlenProtocol *proto;
	HANDLE hContact;
	int msgid;
};

static void __cdecl JabberSendMessageAckThread(void *ptr)
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
	JABBER_LIST_ITEM *item;
	SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;
	if (!db_get(data->hContact, data->proto->m_szModuleName, "jid", &dbv)) {
		if ((item=JabberListGetItemPtr(data->proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
			db_free(&dbv);
			ProtoBroadcastAck(data->proto->m_szModuleName, data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1,
				item->statusMessage==NULL ? (LPARAM)NULL : (LPARAM)(TCHAR*)_A2T(item->statusMessage));
			return;
		}
		else db_free(&dbv);
	}
	ProtoBroadcastAck(data->proto->m_szModuleName, data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)(TCHAR*)TEXT(""));
	delete data;
}

INT_PTR TlenSendAlert(void *ptr, LPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = ( HANDLE )wParam;
	DBVARIANT dbv;
	TlenProtocol *proto = (TlenProtocol *)ptr;
	if (proto->isOnline && !db_get(hContact, proto->m_szModuleName, "jid", &dbv)) {
		JabberSend(proto, "<m tp='a' to='%s'/>", dbv.pszVal);

		db_free(&dbv);
	}
	return 0;
}

int __cdecl TlenProtocol::SendMsg(HANDLE hContact, int flags, const char* msgRAW)
{
	DBVARIANT dbv;
	char *msgEnc;
	JABBER_LIST_ITEM *item;
	char msgType[16];

	if (!isOnline || db_get(hContact, m_szModuleName, "jid", &dbv)) {
		JabberForkThread(TlenSendMessageFailedThread, 0, new SENDACKTHREADDATA(this, hContact, 2));
		return 2;
	}

	char* msg;
	if (flags & PREF_UNICODE)
		msg = mir_u2a((wchar_t*)&msgRAW[strlen(msgRAW) + 1]);
	else if (flags & PREF_UTF)
		msg = mir_utf8decodeA(msgRAW);
	else
		msg = mir_strdup(msgRAW);


	int id = JabberSerialNext(this);

	if (!strcmp(msg, "<alert>")) {
		JabberSend(this, "<m tp='a' to='%s'/>", dbv.pszVal);
		JabberForkThread(JabberSendMessageAckThread, 0, new SENDACKTHREADDATA(this, hContact, id));
	}
	else if (!strcmp(msg, "<image>")) {
		JabberSend(this, "<message to='%s' type='%s' crc='%x' idt='%d'/>", dbv.pszVal, "pic", 0x757f044, id);
		JabberForkThread(JabberSendMessageAckThread, 0, new SENDACKTHREADDATA(this, hContact, id));
	}
	else {
		if ((msgEnc=JabberTextEncode(msg)) != NULL) {
			if (JabberListExist(this, LIST_CHATROOM, dbv.pszVal) && strchr(dbv.pszVal, '/') == NULL)
				strcpy(msgType, "groupchat");
			else if (db_get_b(hContact, m_szModuleName, "bChat", FALSE))
				strcpy(msgType, "privchat");
			else
				strcpy(msgType, "chat");

			if (!strcmp(msgType, "groupchat") || db_get_b(NULL, m_szModuleName, "MsgAck", FALSE) == FALSE) {
				SENDACKTHREADDATA *tdata = (SENDACKTHREADDATA*) mir_alloc(sizeof(SENDACKTHREADDATA));
				tdata->proto = this;
				tdata->hContact = hContact;
				if (!strcmp(msgType, "groupchat"))
					JabberSend(this, "<message to='%s' type='%s'><body>%s</body></message>", dbv.pszVal, msgType, msgEnc);
				else if (!strcmp(msgType, "privchat"))
					JabberSend(this, "<m to='%s'><b n='6' s='10' f='0' c='000000'>%s</b></m>", dbv.pszVal, msgEnc);
				else
					JabberSend(this, "<message to='%s' type='%s' id='"JABBER_IQID"%d'><body>%s</body><x xmlns='jabber:x:event'><composing/></x></message>", dbv.pszVal, msgType, id, msgEnc);

				JabberForkThread(JabberSendMessageAckThread, 0, new SENDACKTHREADDATA(this, hContact, id));
			}
			else {
				if ((item=JabberListGetItemPtr(this, LIST_ROSTER, dbv.pszVal)) != NULL)
					item->idMsgAckPending = id;
				JabberSend(this, "<message to='%s' type='%s' id='"JABBER_IQID"%d'><body>%s</body><x xmlns='jabber:x:event'><offline/><delivered/><composing/></x></message>", dbv.pszVal, msgType, id, msgEnc);
			}
		}
		mir_free(msgEnc);
	}

	mir_free(msg);
	db_free(&dbv);
	return id;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarInfo - retrieves the avatar info

static INT_PTR TlenGetAvatarInfo(void *ptr, LPARAM wParam, LPARAM lParam)
{
	BOOL downloadingAvatar = FALSE;
	char *avatarHash = NULL;
	JABBER_LIST_ITEM *item = NULL;
	DBVARIANT dbv;
	TlenProtocol *proto = (TlenProtocol *)ptr;
	PROTO_AVATAR_INFORMATIONT* AI = ( PROTO_AVATAR_INFORMATIONT* )lParam;
	if (!proto->tlenOptions.enableAvatars) return GAIR_NOAVATAR;

	if (AI->hContact != NULL) {
		if (!db_get(AI->hContact, proto->m_szModuleName, "jid", &dbv)) {
			item = JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal);
			db_free(&dbv);
			if (item != NULL) {
				downloadingAvatar = item->newAvatarDownloading;
				avatarHash = item->avatarHash;
			}
		}
	} else {
		if (proto->threadData != NULL) {
			avatarHash = proto->threadData->avatarHash;
		}
	}
	if ((avatarHash == NULL || avatarHash[0] == '\0') && !downloadingAvatar) {
		return GAIR_NOAVATAR;
	}
	if (avatarHash != NULL && !downloadingAvatar) {
		TlenGetAvatarFileName(proto, item, AI->filename, sizeof(AI->filename));
		AI->format = ( AI->hContact == NULL ) ? proto->threadData->avatarFormat : item->avatarFormat;
		return GAIR_SUCCESS;
	}
	if (( wParam & GAIF_FORCE ) != 0 && AI->hContact != NULL && proto->isOnline) {
		/* get avatar */
		return GAIR_WAITFOR;
	}
	return GAIR_NOAVATAR;
}

HANDLE __cdecl TlenProtocol::GetAwayMsg(HANDLE hContact)
{
	SENDACKTHREADDATA *tdata = new SENDACKTHREADDATA(this, hContact, 0);
	JabberForkThread((void (__cdecl *)(void*))TlenGetAwayMsgThread, 0, (void*)tdata);
	return (HANDLE)1;
}

int __cdecl TlenProtocol::RecvAwayMsg(HANDLE hContact, int mode, PROTORECVEVENT* evt)
{
	return 0;
}

int __cdecl TlenProtocol::SendAwayMsg(HANDLE hContact, HANDLE hProcess, const char* msg)
{
	return 0;
}

HANDLE __cdecl TlenProtocol::FileAllow(HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	TLEN_FILE_TRANSFER *ft;
	JABBER_LIST_ITEM *item;
	char *nick;

	if (!isOnline) return 0;

	ft = (TLEN_FILE_TRANSFER *) hTransfer;
	ft->szSavePath = mir_strdup(mir_t2a(szPath));	//TODO convert to PROTOCHAR*
	if ((item=JabberListAdd(this, LIST_FILE, ft->iqId)) != NULL) {
		item->ft = ft;
	}
	nick = JabberNickFromJID(ft->jid);
	if (ft->newP2P) {
		JabberSend(this, "<iq to='%s'><query xmlns='p2p'><fs t='%s' e='5' i='%s' v='1'/></query></iq>", ft->jid, ft->jid, ft->iqId);
	} else {
		JabberSend(this, "<f t='%s' i='%s' e='5' v='1'/>", nick, ft->iqId);
	}
	mir_free(nick);
	return (HANDLE)hTransfer;
}

int __cdecl TlenProtocol::FileDeny(HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason)
{
	TLEN_FILE_TRANSFER *ft;
	char *nick;

	if (!isOnline) return 1;

	ft = (TLEN_FILE_TRANSFER *) hTransfer;
	nick = JabberNickFromJID(ft->jid);
	if (ft->newP2P) {
		JabberSend(this, "<f i='%s' e='4' t='%s'/>", ft->iqId, nick);\
	} else {
		JabberSend(this, "<f i='%s' e='4' t='%s'/>", ft->iqId, nick);\
	}
	mir_free(nick);
	TlenP2PFreeFileTransfer(ft);
	return 0;
}

int __cdecl TlenProtocol::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename) {
	return 0;
}

int __cdecl TlenProtocol::FileCancel(HANDLE hContact, HANDLE hTransfer)
{
	TLEN_FILE_TRANSFER *ft = (TLEN_FILE_TRANSFER *) hTransfer;
	JabberLog(this, "Invoking FileCancel()");
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

HANDLE __cdecl TlenProtocol::SendFile(HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	TLEN_FILE_TRANSFER *ft;
	int i, j;
	struct _stat statbuf;
	DBVARIANT dbv;
	char *nick, *p, idStr[10];
	JABBER_LIST_ITEM *item;
	int id;

	if (!isOnline) return 0;
//	if (db_get_w(ccs->hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) return 0;
	if (db_get(hContact, m_szModuleName, "jid", &dbv)) return 0;
	ft = TlenFileCreateFT(this, dbv.pszVal);
	for (ft->fileCount=0; ppszFiles[ft->fileCount]; ft->fileCount++);
	ft->files = (char **) mir_alloc(sizeof(char *) * ft->fileCount);
	ft->filesSize = (long *) mir_alloc(sizeof(long) * ft->fileCount);
	ft->allFileTotalSize = 0;
	for (i=j=0; i<ft->fileCount; i++) {
		char* ppszFiles_i_A = mir_t2a(ppszFiles[i]);
		if (_stat(ppszFiles_i_A, &statbuf))
			JabberLog(this, "'%s' is an invalid filename", ppszFiles[i]);
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

	id = JabberSerialNext(this);
	_snprintf(idStr, sizeof(idStr), "%d", id);
	if ((item=JabberListAdd(this, LIST_FILE, idStr)) != NULL) {
		ft->iqId = mir_strdup(idStr);
		nick = JabberNickFromJID(ft->jid);
		item->ft = ft;
		if (tlenOptions.useNewP2P) {
			JabberSend(this, "<iq to='%s'><query xmlns='p2p'><fs t='%s' e='1' i='%s' c='%d' s='%d' v='%d'/></query></iq>",
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
				p = JabberTextEncode(p);
				JabberSend(this, "<f t='%s' n='%s' e='1' i='%s' c='1' s='%d' v='1'/>", nick, p, idStr, ft->allFileTotalSize);
				mir_free(ppszFiles[0]);
				mir_free(p);
			} else {
				JabberSend(this, "<f t='%s' e='1' i='%s' c='%d' s='%d' v='1'/>", nick, idStr, ft->fileCount, ft->allFileTotalSize);
			}
		}
		mir_free(nick);
	}

	return (HANDLE) ft;
}

int __cdecl TlenProtocol::SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList){
	return 0;
}

int __cdecl TlenProtocol::SendUrl(HANDLE hContact, int flags, const char* urlt){
	return 0;
}

int __cdecl TlenProtocol::RecvMsg(HANDLE hContact, PROTORECVEVENT* evt)
{
	return Proto_RecvMessage(hContact, evt);
}

int __cdecl TlenProtocol::RecvFile(HANDLE hContact, PROTOFILEEVENT* evt)
{
	return Proto_RecvFile(hContact, evt);
}

int __cdecl TlenProtocol::RecvUrl(HANDLE hContact, PROTORECVEVENT*){
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

int JabberDbSettingChanged(void *ptr, WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	TlenProtocol *proto = (TlenProtocol *)ptr;
	// no action for hContact == NULL or when offline
	if ((HANDLE) wParam == NULL) return 0;
	if (!proto->isConnected) return 0;

	if (!strcmp(cws->szModule, "CList")) {
		HANDLE hContact;
		DBVARIANT dbv;
		JABBER_LIST_ITEM *item;
		char *szProto, *nick, *jid, *group;

		hContact = (HANDLE) wParam;
		szProto = GetContactProto(hContact);
		if (szProto == NULL || strcmp(szProto, proto->m_szModuleName)) return 0;
		// A contact's group is changed
		if (!strcmp(cws->szSetting, "Group")) {
			if (!db_get(hContact, proto->m_szModuleName, "jid", &dbv)) {
				if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
					db_free(&dbv);
					if (!db_get(hContact, "CList", "MyHandle", &dbv)) {
						nick = JabberTextEncode(dbv.pszVal);
						db_free(&dbv);
					} else if (!db_get(hContact, proto->m_szModuleName, "Nick", &dbv)) {
						nick = JabberTextEncode(dbv.pszVal);
						db_free(&dbv);
					} else {
						nick = JabberNickFromJID(item->jid);
					}
					if (nick != NULL) {
						// Note: we need to compare with item->group to prevent infinite loop
						if (cws->value.type == DBVT_DELETED && item->group != NULL) {
							JabberLog(proto, "Group set to nothing");
							JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'></item></query></iq>", nick, item->jid);
						} else if (cws->value.pszVal != NULL) {
							char *newGroup = settingToChar(cws);
							if (item->group == NULL || strcmp(newGroup, item->group)) {
								JabberLog(proto, "Group set to %s", newGroup);
								if ((group=TlenGroupEncode(newGroup)) != NULL) {
									JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, item->jid, group);
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

//			hContact = (HANDLE) wParam;
//			szProto = GetContactProto(hContact);
//			if (szProto == NULL || strcmp(szProto, proto->m_szModuleName)) return 0;

			if (!db_get(hContact, proto->m_szModuleName, "jid", &dbv)) {
				jid = dbv.pszVal;
				if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
					if (cws->value.type == DBVT_DELETED) {
						newNick = mir_strdup((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_NOMYHANDLE));
					} else if (cws->value.pszVal != NULL) {
						newNick = settingToChar(cws);
					} else {
						newNick = NULL;
					}
					// Note: we need to compare with item->nick to prevent infinite loop
					if (newNick != NULL && (item->nick == NULL || (item->nick != NULL && strcmp(item->nick, newNick)))) {
						if ((nick=JabberTextEncode(newNick)) != NULL) {
							JabberLog(proto, "Nick set to %s", newNick);
							if (item->group != NULL && (group=TlenGroupEncode(item->group)) != NULL) {
								JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, jid, group);
								mir_free(group);
							} else {
								JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'></item></query></iq>", nick, jid);
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
				if (!db_get(hContact, proto->m_szModuleName, "jid", &dbv)) {
					jid = mir_strdup(dbv.pszVal);
					db_free(&dbv);
					JabberLog(proto, "Add %s permanently to list", jid);
					if (!db_get(hContact, "CList", "MyHandle", &dbv)) {
						nick = JabberTextEncode(dbv.pszVal); //Utf8Encode
						db_free(&dbv);
					}
					else {
						nick = JabberNickFromJID(jid);
					}
					if (nick != NULL) {
						JabberLog(proto, "jid=%s nick=%s", jid, nick);
						if (!db_get(hContact, "CList", "Group", &dbv)) {
							if ((pGroup=TlenGroupEncode(dbv.pszVal)) != NULL) {
								JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, jid, pGroup);
								JabberSend(proto, "<presence to='%s' type='subscribe'/>", jid);
								mir_free(pGroup);
							}
							db_free(&dbv);
						}
						else {
							JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'/></query></iq>", nick, jid);
							JabberSend(proto, "<presence to='%s' type='subscribe'/>", jid);
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

int JabberContactDeleted(void *ptr, WPARAM wParam, LPARAM lParam)
{
	char *szProto;
	DBVARIANT dbv;
	TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline)	// should never happen
		return 0;
	szProto = GetContactProto((HANDLE)wParam);
	if (szProto == NULL || strcmp(szProto, proto->m_szModuleName))
		return 0;
	if (!db_get((HANDLE) wParam, proto->m_szModuleName, "jid", &dbv)) {
		char *jid, *p, *q;

		jid = dbv.pszVal;
		if ((p=strchr(jid, '@')) != NULL) {
			if ((q=strchr(p, '/')) != NULL)
				*q = '\0';
		}
		if (JabberListExist(proto, LIST_ROSTER, jid)) {
			// Remove from roster, server also handles the presence unsubscription process.
			JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item jid='%s' subscription='remove'/></query></iq>", jid);
		}

		db_free(&dbv);
	}
	return 0;
}

int __cdecl TlenProtocol::UserIsTyping(HANDLE hContact, int type)
{
	DBVARIANT dbv;
	JABBER_LIST_ITEM *item;

	if (!isOnline) return 0;
	if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
		if ((item=JabberListGetItemPtr(this, LIST_ROSTER, dbv.pszVal)) != NULL /*&& item->wantComposingEvent == TRUE*/) {
			switch (type) {
			case PROTOTYPE_SELFTYPING_OFF:
				JabberSend(this, "<m tp='u' to='%s'/>", dbv.pszVal);
				break;
			case PROTOTYPE_SELFTYPING_ON:
				JabberSend(this, "<m tp='t' to='%s'/>", dbv.pszVal);
				break;
			}
		}
		db_free(&dbv);
	}
	return 0;
}

INT_PTR TlenGetMyAvatar(void *ptr, LPARAM wParam, LPARAM lParam)
{
	TCHAR* buf = (TCHAR*)wParam;
	int  size = ( int )lParam;
	TlenProtocol *proto = (TlenProtocol *)ptr;

	if ( buf == NULL || size <= 0 )
		return -1;

	TlenGetAvatarFileName( proto, NULL, buf, size );
	//wParam (buf) is output var
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
		CheckDlgButton(hwndDlg, IDC_PUBLICAVATAR, TRUE);
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

INT_PTR TlenSetMyAvatar(void *ptr, LPARAM wParam, LPARAM lParam)
{
	TCHAR* szFileName = ( TCHAR* )lParam;
	TCHAR tFileName[ MAX_PATH ];
	int fileIn;
	TlenProtocol *proto = (TlenProtocol *)ptr;
	if (!proto->isOnline) return 1;
	if (szFileName != NULL) {
		int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_USER_CHANGEAVATAR), NULL, TlenChangeAvatarDlgProc, (LPARAM) NULL);
		TlenGetAvatarFileName( proto, NULL, tFileName, MAX_PATH);
		if ( CopyFile( szFileName, tFileName, FALSE ) == FALSE ) {
			return 1;
		}
		char* tFileNameA = mir_t2a(tFileName); //TODO - drop io.h
		fileIn = open( tFileNameA, O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
		if ( fileIn != -1 ) {
			long  dwPngSize = filelength(fileIn);
			BYTE* pResult = (BYTE *)mir_alloc(dwPngSize);
			if (pResult != NULL) {
				read( fileIn, pResult, dwPngSize );
				close( fileIn );
				TlenUploadAvatar(proto, pResult, dwPngSize, (result & 0x10000) != 0);
				mir_free(pResult);
			}
		}
		mir_free(tFileName);
		mir_free(tFileNameA);
	} else {
		TlenRemoveAvatar(proto);
	}
	return 0;
}

INT_PTR TlenGetAvatarCaps(void *ptr, LPARAM wParam, LPARAM lParam)
{
	TlenProtocol *proto = (TlenProtocol *)ptr;
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
		return (proto->tlenOptions.enableAvatars && proto->isOnline) ? 1 : 0;
	case AF_DONTNEEDDELAYS:
		return 1;
	case AF_MAXFILESIZE:
		return 10 * 1024;
	case AF_DELAYAFTERFAIL:
		return 0;
	}
	return 0;
}

int __cdecl TlenProtocol::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	//TlenProtocol *proto = (TlenProtocol *)ptr;
	switch( iEventType ) {
	case EV_PROTO_ONLOAD:    return TlenOnModulesLoaded(this, 0, 0 );
	case EV_PROTO_ONOPTIONS: return TlenOptionsInit(this, wParam, lParam );
	case EV_PROTO_ONEXIT:    return TlenPreShutdown(this, 0, 0 );
	case EV_PROTO_ONRENAME:
		{
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_NAME | CMIF_TCHAR;
			mi.ptszName = m_tszUserName;
			Menu_ModifyItem(hMenuRoot, &mi);
			/* FIXME: Rename network user as well */
		}
	}
	return 1;
}

// PSS_ADDED
int __cdecl TlenProtocol::AuthRecv(HANDLE hContact, PROTORECVEVENT* evt)
{
	return 1;
}

// PSS_AUTHREQUEST
int __cdecl TlenProtocol::AuthRequest(HANDLE hContact, const PROTOCHAR* szMessage)
{
	return 1;
}

HANDLE __cdecl TlenProtocol::ChangeInfo(int iInfoType, void* pInfoData)
{
	return NULL;
}


int __cdecl TlenProtocol::RecvContacts(HANDLE hContact, PROTORECVEVENT* evt)
{
	return 1;
}


extern INT_PTR CALLBACK TlenAccMgrUIDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR TlenAccMgrUI(void *ptr, LPARAM wParam, LPARAM lParam)
{
	return (INT_PTR) CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, TlenAccMgrUIDlgProc, (LPARAM) ptr);
}

void TlenInitServicesVTbl(TlenProtocol *proto) {

	char s[128];

	sprintf(s, "%s%s", proto->m_szModuleName, PS_GETNAME);
	CreateServiceFunction_Ex(s, proto, TlenGetName);

	sprintf(s, "%s%s", proto->m_szModuleName, PS_GETAVATARINFO);
	CreateServiceFunction_Ex(s, proto, TlenGetAvatarInfo);

	sprintf(s, "%s%s", proto->m_szModuleName, "/SendNudge");
	CreateServiceFunction_Ex(s, proto, TlenSendAlert);

	sprintf(s, "%s%s", proto->m_szModuleName, PS_GETAVATARCAPS);
	CreateServiceFunction_Ex(s, proto, TlenGetAvatarCaps);

	sprintf(s, "%s%s", proto->m_szModuleName, PS_SETMYAVATART);
	CreateServiceFunction_Ex(s, proto, TlenSetMyAvatar);

	sprintf(s, "%s%s", proto->m_szModuleName, PS_GETMYAVATART);
	CreateServiceFunction_Ex(s, proto, TlenGetMyAvatar);

	sprintf(s, "%s%s", proto->m_szModuleName, PS_GETSTATUS);
	CreateServiceFunction_Ex(s, proto, TlenGetStatus);

	sprintf(s, "%s%s", proto->m_szModuleName, PS_CREATEACCMGRUI);
	CreateServiceFunction_Ex(s, proto, TlenAccMgrUI);

}

TlenProtocol::TlenProtocol( const char *aProtoName, const TCHAR *aUserName) :
	PROTO<TlenProtocol>(aProtoName, aUserName)
{
	TlenInitServicesVTbl(this);

	InitializeCriticalSection(&modeMsgMutex);
	InitializeCriticalSection(&csSend);

	char text[_MAX_PATH];
	sprintf(text, "%s/%s", m_szModuleName, "Nudge");
	hTlenNudge = CreateHookableEvent(text);

	HookEventObj_Ex(ME_SYSTEM_MODULESLOADED, this, TlenSystemModulesLoaded);
	HookEventObj_Ex(ME_OPT_INITIALISE, this, TlenOptionsInit);
	HookEventObj_Ex(ME_DB_CONTACT_SETTINGCHANGED, this, JabberDbSettingChanged);
	HookEventObj_Ex(ME_DB_CONTACT_DELETED, this, JabberContactDeleted);
	HookEventObj_Ex(ME_CLIST_PREBUILDCONTACTMENU, this, TlenPrebuildContactMenu);
//	HookEventObj_Ex(ME_SKIN2_ICONSCHANGED, this, TlenIconsChanged);
	HookEventObj_Ex(ME_SYSTEM_PRESHUTDOWN, this, TlenPreShutdown);


	DBVARIANT dbv;
	if (!db_get(NULL, m_szModuleName, "LoginServer", &dbv)) {
		db_free(&dbv);
	} else {
		db_set_s(NULL, m_szModuleName, "LoginServer", "tlen.pl");
	}
	if (!db_get(NULL, m_szModuleName, "ManualHost", &dbv)) {
		db_free(&dbv);
	} else {
		db_set_s(NULL, m_szModuleName, "ManualHost", "s1.tlen.pl");
	}

	TlenLoadOptions(this);

	JabberWsInit(this);
	JabberSerialInit(this);
	JabberIqInit(this);
	JabberListInit(this);
}

TlenProtocol::~TlenProtocol()
{

	uninitMenuItems(this);
	TlenVoiceCancelAll(this);
	TlenFileCancelAll(this);
	if (hTlenNudge)
		DestroyHookableEvent(hTlenNudge);
	UnhookEvents_Ex(this);
	JabberListUninit(this);
	JabberIqUninit(this);
	JabberSerialUninit(this);
	DeleteCriticalSection(&modeMsgMutex);
	DeleteCriticalSection(&csSend);
	//DestroyServices_Ex(this);
	JabberWsUninit(this);

	mir_free(modeMsgs.szOnline);
	mir_free(modeMsgs.szAway);
	mir_free(modeMsgs.szNa);
	mir_free(modeMsgs.szDnd);
	mir_free(modeMsgs.szFreechat);
	mir_free(modeMsgs.szInvisible);
}
