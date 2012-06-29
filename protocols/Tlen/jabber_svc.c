/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda IM
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
DWORD_PTR TlenGetCaps(PROTO_INTERFACE *ptr, int type, HANDLE hContact)
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

INT_PTR TlenGetName(void *ptr, WPARAM wParam, LPARAM lParam)
{
    TlenProtocol *proto = (TlenProtocol *)ptr;
	strncpy((char *) lParam, proto->iface.m_szProtoName, wParam);
	return 0;
}

HICON TlenGetIcon(PROTO_INTERFACE *ptr, int iconIndex)
{
	if ((iconIndex&0xffff) == PLI_PROTOCOL) {
		HICON hIcon = GetIcolibIcon(IDI_TLEN);
		HICON hIconCopy = CopyIcon(hIcon);
		ReleaseIcolibIcon(hIcon);
		return hIconCopy;
	}
	return (HICON) NULL;
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

HANDLE TlenBasicSearch(PROTO_INTERFACE *ptr, const char *id)
{
	char *jid;
	int iqId = 0;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if (!proto->isOnline) return 0;
	if (id == NULL) return 0;
	if ((jid=JabberTextEncode(id)) != NULL) {
		proto->searchJID = mir_strdup(id);
		TlenResetSearchQuery(proto);
		JabberStringAppend(&proto->searchQuery, &proto->searchQueryLen, "<i>%s</i>", jid);
		iqId = TlenRunSearch(proto);
		mir_free(jid);
	}
	return (HANDLE)iqId;
}

HANDLE TlenSearchByEmail(PROTO_INTERFACE *ptr, const char* email)
{
	char *emailEnc;
	int iqId = 0;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 0;
	if (email == NULL) return 0;

	if ((emailEnc=JabberTextEncode(email)) != NULL) {
		TlenResetSearchQuery(proto);
		JabberStringAppend(&proto->searchQuery, &proto->searchQueryLen, "<email>%s</email>", emailEnc);
		iqId = TlenRunSearch(proto);
		mir_free(emailEnc);
	}
	return (HANDLE)iqId;
}

HANDLE TlenSearchByName(PROTO_INTERFACE *ptr, const char* nick, const char* firstName, const char* lastName)
{
	char *p;
	int iqId = 0;

    TlenProtocol *proto = (TlenProtocol *)ptr;
	if (!proto->isOnline) return 0;

	TlenResetSearchQuery(proto);

	if (nick != NULL && nick[0] != '\0') {
		if ((p=JabberTextEncode(nick)) != NULL) {
			JabberStringAppend(&proto->searchQuery, &proto->searchQueryLen, "<nick>%s</nick>", p);
			mir_free(p);
		}
	}
	if (firstName != NULL && firstName[0] != '\0') {
		if ((p=JabberTextEncode(firstName)) != NULL) {
			JabberStringAppend(&proto->searchQuery, &proto->searchQueryLen, "<first>%s</first>", p);
			mir_free(p);
		}
	}
	if (lastName != NULL && lastName[0] != '\0') {
		if ((p=JabberTextEncode(lastName)) != NULL) {
			JabberStringAppend(&proto->searchQuery, &proto->searchQueryLen, "<last>%s</last>", p);
			mir_free(p);
		}
	}

	iqId = TlenRunSearch(proto);
	return (HANDLE)iqId;
}

HWND TlenCreateAdvSearchUI(PROTO_INTERFACE *ptr, HWND owner)
{
	return (HWND) CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADVSEARCH), owner, TlenAdvSearchDlgProc);
}

HWND TlenSearchAdvanced(PROTO_INTERFACE *ptr, HWND owner)
{
	int iqId;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if (!proto->isOnline) return 0;

	TlenResetSearchQuery(proto);
	iqId = JabberSerialNext(proto);
	if ((proto->searchQuery = TlenAdvSearchCreateQuery(owner, iqId)) != NULL) {
		iqId = TlenRunSearch(proto);
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
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) proto->iface.m_szModuleName);
		DBWriteContactSettingString(hContact, proto->iface.m_szModuleName, "jid", jid);
		if ((nick=JabberNickFromJID(newJid)) == NULL)
			nick = mir_strdup(newJid);
		DBWriteContactSettingString(hContact, "CList", "MyHandle", nick);
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
		DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
		if (flags & PALF_TEMPORARY)
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
	}
	else {
		// already exist
		// Set up a dummy "NotOnList" when adding permanently only
		if (!(flags&PALF_TEMPORARY))
			DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
	}

	return hContact;
}

HANDLE TlenAddToList(PROTO_INTERFACE *ptr, int flags, PROTOSEARCHRESULT *psr)
{
	HANDLE hContact;
    TlenProtocol *proto = (TlenProtocol *)ptr;
    JABBER_SEARCH_RESULT *jsr = (JABBER_SEARCH_RESULT*)psr;
	if (jsr->hdr.cbSize != sizeof(JABBER_SEARCH_RESULT))
		return (int) NULL;
	hContact = AddToListByJID(proto, jsr->jid, flags);	// wParam is flag e.g. PALF_TEMPORARY
	return hContact;
}

HANDLE TlenAddToListByEvent(PROTO_INTERFACE *ptr, int flags, int iContact, HANDLE hDbEvent)
{
	DBEVENTINFO dbei;
	HANDLE hContact;
	char *nick, *firstName, *lastName, *jid;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) == (DWORD)(-1))
		return (int)(HANDLE) NULL;
	if ((dbei.pBlob=(PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return (int)(HANDLE) NULL;
	if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM) &dbei)) {
		mir_free(dbei.pBlob);
		return (int)(HANDLE) NULL;
	}
	if (strcmp(dbei.szModule, proto->iface.m_szModuleName)) {
		mir_free(dbei.pBlob);
		return (int)(HANDLE) NULL;
	}

/*
	// EVENTTYPE_CONTACTS is when adding from when we receive contact list (not used in Jabber)
	// EVENTTYPE_ADDED is when adding from when we receive "You are added" (also not used in Jabber)
	// Jabber will only handle the case of EVENTTYPE_AUTHREQUEST
	// EVENTTYPE_AUTHREQUEST is when adding from the authorization request dialog
*/

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return (int)(HANDLE) NULL;
	}

	nick = (char *) (dbei.pBlob + sizeof(DWORD) + sizeof(HANDLE));
	firstName = nick + strlen(nick) + 1;
	lastName = firstName + strlen(firstName) + 1;
	jid = lastName + strlen(lastName) + 1;

	hContact = (HANDLE) AddToListByJID(proto, jid, flags);
	mir_free(dbei.pBlob);

	return hContact;
}

int TlenAuthAllow(PROTO_INTERFACE *ptr, HANDLE hContact)
{
	DBEVENTINFO dbei;
	char *nick, *firstName, *lastName, *jid;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline)
		return 1;

	memset(&dbei, sizeof(dbei), 0);
	dbei.cbSize = sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hContact, 0)) == (DWORD)(-1))
		return 1;
	if ((dbei.pBlob=(PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return 1;
	if (CallService(MS_DB_EVENT_GET, (WPARAM)hContact, (LPARAM) &dbei)) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (strcmp(dbei.szModule, proto->iface.m_szModuleName)) {
		mir_free(dbei.pBlob);
		return 1;
	}

	nick = (char *) (dbei.pBlob + sizeof(DWORD) + sizeof(HANDLE));
	firstName = nick + strlen(nick) + 1;
	lastName = firstName + strlen(firstName) + 1;
	jid = lastName + strlen(lastName) + 1;

	JabberSend(proto, "<presence to='%s' type='subscribed'/>", jid);

	// Automatically add this user to my roster if option is enabled
	if (DBGetContactSettingByte(NULL, proto->iface.m_szModuleName, "AutoAdd", TRUE) == TRUE) {
		HANDLE hContact;
		JABBER_LIST_ITEM *item;

		if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, jid))==NULL || (item->subscription!=SUB_BOTH && item->subscription!=SUB_TO)) {
			JabberLog(proto, "Try adding contact automatically jid=%s", jid);
			if ((hContact=AddToListByJID(proto, jid, 0)) != NULL) {
				// Trigger actual add by removing the "NotOnList" added by AddToListByJID()
				// See AddToListByJID() and JabberDbSettingChanged().
				DBDeleteContactSetting(hContact, "CList", "NotOnList");
			}
		}
	}

	mir_free(dbei.pBlob);
	return 0;
}

int TlenAuthDeny(PROTO_INTERFACE *ptr, HANDLE hContact, const TCHAR* szReason)
{
	DBEVENTINFO dbei;
	char *nick, *firstName, *lastName, *jid;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline)
		return 1;

	memset(&dbei, sizeof(dbei), 0);
	dbei.cbSize = sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hContact, 0)) == (DWORD)(-1))
		return 1;
	if ((dbei.pBlob=(PBYTE) mir_alloc(dbei.cbBlob)) == NULL)
		return 1;
	if (CallService(MS_DB_EVENT_GET, (WPARAM)hContact, (LPARAM) &dbei)) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST) {
		mir_free(dbei.pBlob);
		return 1;
	}
	if (strcmp(dbei.szModule, proto->iface.m_szModuleName)) {
		mir_free(dbei.pBlob);
		return 1;
	}

	nick = (char *) (dbei.pBlob + sizeof(DWORD) + sizeof(HANDLE));
	firstName = nick + strlen(nick) + 1;
	lastName = firstName + strlen(firstName) + 1;
	jid = lastName + strlen(lastName) + 1;

	JabberSend(proto, "<presence to='%s' type='unsubscribed'/>", jid);
	JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item jid='%s' subscription='remove'/></query></iq>", jid);
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
		proto->iface.m_iDesiredStatus = initialStatus;

		oldStatus = proto->iface.m_iStatus;
		proto->iface.m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(proto->iface.m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, proto->iface.m_iStatus);
		thread->hThread = (HANDLE) JabberForkThread((void (__cdecl *)(void*))JabberServerThread, 0, thread);
	}
}

int TlenSetStatus(PROTO_INTERFACE *ptr, int iNewStatus)
{
	int oldStatus;
	HANDLE s;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	proto->iface.m_iDesiredStatus = iNewStatus;

 	if (iNewStatus == ID_STATUS_OFFLINE) {
		if (proto->threadData) {
			if (proto->isConnected) {
				JabberSendPresence(proto, ID_STATUS_OFFLINE);
			}
			s = proto;
			proto->threadData = NULL;
			if (proto->isConnected) {
				Sleep(200);
//				JabberSend(s, "</s>");
				// Force closing connection
				proto->isConnected = FALSE;
				proto->isOnline = FALSE;
				Netlib_CloseHandle(s);
			}
		}
		else {
			if (proto->iface.m_iStatus != ID_STATUS_OFFLINE) {
				oldStatus = proto->iface.m_iStatus;
				proto->iface.m_iStatus = ID_STATUS_OFFLINE;
				ProtoBroadcastAck(proto->iface.m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, proto->iface.m_iStatus);
			}
		}
	}
	else if (iNewStatus != proto->iface.m_iStatus) {
		if (!proto->isConnected)
			TlenConnect(proto, iNewStatus);
		else {
			// change status
			oldStatus = proto->iface.m_iStatus;
			// send presence update
			JabberSendPresence(proto, iNewStatus);
			ProtoBroadcastAck(proto->iface.m_szModuleName, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, proto->iface.m_iStatus);
		}
	}
	return 0;
}

INT_PTR TlenGetStatus(void *ptr, WPARAM wParam, LPARAM lParam)
{
    TlenProtocol *proto = (TlenProtocol *)ptr;
	return proto->iface.m_iStatus;
}


int TlenSetAwayMsg(PROTO_INTERFACE *ptr, int iStatus, const char* msg )
{
	char **szMsg;
	char *newModeMsg;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	JabberLog(proto, "SetAwayMsg called, wParam=%d lParam=%s", iStatus, msg);

	newModeMsg = JabberTextEncode(msg);

	EnterCriticalSection(&proto->modeMsgMutex);

	switch (iStatus) {
	case ID_STATUS_ONLINE:
		szMsg = &proto->modeMsgs.szOnline;
		break;
	case ID_STATUS_AWAY:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		szMsg = &proto->modeMsgs.szAway;
		break;
	case ID_STATUS_NA:
		szMsg = &proto->modeMsgs.szNa;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		szMsg = &proto->modeMsgs.szDnd;
		break;
	case ID_STATUS_FREECHAT:
		szMsg = &proto->modeMsgs.szFreechat;
		break;
	case ID_STATUS_INVISIBLE:
		szMsg = &proto->modeMsgs.szInvisible;
		break;
	default:
		LeaveCriticalSection(&proto->modeMsgMutex);
		return 1;
	}

	if ((*szMsg==NULL && newModeMsg==NULL) ||
		(*szMsg!=NULL && newModeMsg!=NULL && !strcmp(*szMsg, newModeMsg))) {
		// Message is the same, no update needed
		if (newModeMsg != NULL) mir_free(newModeMsg);
	}
	else {
		// Update with the new mode message
		if (*szMsg != NULL) mir_free(*szMsg);
		*szMsg = newModeMsg;
		// Send a presence update if needed
		if (iStatus == proto->iface.m_iStatus) {
			JabberSendPresence(proto, proto->iface.m_iStatus);
		}
	}

	LeaveCriticalSection(&proto->modeMsgMutex);
	return 0;
}

int JabberGetInfo(PROTO_INTERFACE *ptr, HANDLE hContact, int infoType)
{
	DBVARIANT dbv;
	int iqId;
	char *nick, *pNick;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 1;
	if (hContact==NULL) {
		iqId = JabberSerialNext(proto);
		JabberIqAdd(proto, iqId, IQ_PROC_NONE, TlenIqResultVcard);
		JabberSend(proto, "<iq type='get' id='"JABBER_IQID"%d' to='tuba'><query xmlns='jabber:iq:register'></query></iq>", iqId);
	} else {
		if (DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) return 1;
		if ((nick=JabberNickFromJID(dbv.pszVal)) != NULL) {
			if ((pNick=JabberTextEncode(nick)) != NULL) {
				iqId = JabberSerialNext(proto);
				JabberIqAdd(proto, iqId, IQ_PROC_NONE, TlenIqResultVcard);
				JabberSend(proto, "<iq type='get' id='"JABBER_IQID"%d' to='tuba'><query xmlns='jabber:iq:search'><i>%s</i></query></iq>", iqId, pNick);
				mir_free(pNick);
			}
			mir_free(nick);
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

int TlenSetApparentMode(PROTO_INTERFACE *ptr, HANDLE hContact, int mode)
{
	DBVARIANT dbv;
	int oldMode;
	char *jid;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 0;
	if (!DBGetContactSettingByte(NULL, proto->iface.m_szModuleName, "VisibilitySupport", FALSE)) return 0;
	if (mode!=0 && mode!=ID_STATUS_ONLINE && mode!=ID_STATUS_OFFLINE) return 1;
	oldMode = DBGetContactSettingWord(hContact, proto->iface.m_szModuleName, "ApparentMode", 0);
	if ((int) mode == oldMode) return 1;
	DBWriteContactSettingWord(hContact, proto->iface.m_szModuleName, "ApparentMode", (WORD) mode);
	if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
		jid = dbv.pszVal;
		switch (mode) {
		case ID_STATUS_ONLINE:
			if (proto->iface.m_iStatus==ID_STATUS_INVISIBLE || oldMode==ID_STATUS_OFFLINE)
				JabberSend(proto, "<presence to='%s'><show>available</show></presence>", jid);
			break;
		case ID_STATUS_OFFLINE:
			if (proto->iface.m_iStatus!=ID_STATUS_INVISIBLE || oldMode==ID_STATUS_ONLINE)
				JabberSend(proto, "<presence to='%s' type='invisible'/>", jid);
			break;
		case 0:
			if (oldMode==ID_STATUS_ONLINE && proto->iface.m_iStatus==ID_STATUS_INVISIBLE)
				JabberSend(proto, "<presence to='%s' type='invisible'/>", jid);
			else if (oldMode==ID_STATUS_OFFLINE && proto->iface.m_iStatus!=ID_STATUS_INVISIBLE)
				JabberSend(proto, "<presence to='%s'><show>available</show></presence>", jid);
			break;
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

typedef struct{
    TlenProtocol *proto;
    HANDLE hContact;
} SENDACKTHREADDATA;

static void __cdecl JabberSendMessageAckThread(void *ptr)
{
    SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;
	SleepEx(10, TRUE);
	ProtoBroadcastAck(data->proto->iface.m_szModuleName, data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
    mir_free(data);
}

static void __cdecl TlenSendMessageFailedThread(void *ptr)
{
    SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;
	SleepEx(10, TRUE);
	ProtoBroadcastAck(data->proto->iface.m_szModuleName, data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE) 2, 0);
    mir_free(data);
}

static void __cdecl TlenGetAwayMsgThread(void *ptr)
{
	DBVARIANT dbv;
	JABBER_LIST_ITEM *item;
    SENDACKTHREADDATA *data = (SENDACKTHREADDATA *)ptr;
	if (!DBGetContactSetting(data->hContact, data->proto->iface.m_szModuleName, "jid", &dbv)) {
		if ((item=JabberListGetItemPtr(data->proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
            DBFreeVariant(&dbv);
            if (item->statusMessage != NULL) {
				ProtoBroadcastAck(data->proto->iface.m_szModuleName, data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE) 1, (LPARAM) item->statusMessage);
				return;
			}
		}
		else {
			DBFreeVariant(&dbv);
		}
	}
	ProtoBroadcastAck(data->proto->iface.m_szModuleName, data->hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE) 1, (LPARAM) "");
    mir_free(data);
}

INT_PTR TlenSendAlert(void *ptr, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = ( HANDLE )wParam;
	DBVARIANT dbv;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if (proto->isOnline && !DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
		JabberSend(proto, "<m tp='a' to='%s'/>", dbv.pszVal);

		DBFreeVariant(&dbv);
	}
	return 0;
}


int TlenSendMessage(PROTO_INTERFACE *ptr, HANDLE hContact, int flags, const char* msg)
{
	DBVARIANT dbv;
	char *msgEnc;
	JABBER_LIST_ITEM *item;
	int id;
	char msgType[16];
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline || DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
        SENDACKTHREADDATA *tdata = (SENDACKTHREADDATA*) mir_alloc(sizeof(SENDACKTHREADDATA));
        tdata->proto = proto;
        tdata->hContact = hContact;
		JabberForkThread(TlenSendMessageFailedThread, 0, (void *) tdata);
		return 2;
	}
	if (!strcmp(msg, "<alert>")) {
        SENDACKTHREADDATA *tdata = (SENDACKTHREADDATA*) mir_alloc(sizeof(SENDACKTHREADDATA));
        tdata->proto = proto;
        tdata->hContact = hContact;
		JabberSend(proto, "<m tp='a' to='%s'/>", dbv.pszVal);
		JabberForkThread(JabberSendMessageAckThread, 0, (void *) tdata);
	}  else if (!strcmp(msg, "<image>")) {
        SENDACKTHREADDATA *tdata = (SENDACKTHREADDATA*) mir_alloc(sizeof(SENDACKTHREADDATA));
        tdata->proto = proto;
        tdata->hContact = hContact;
		id = JabberSerialNext(proto);
		JabberSend(proto, "<message to='%s' type='%s' crc='%x' idt='%d'/>", dbv.pszVal, "pic", 0x757f044, id);
		JabberForkThread(JabberSendMessageAckThread, 0, (void *) tdata);
	} else {
		if ((msgEnc=JabberTextEncode(msg)) != NULL) {
			if (JabberListExist(proto, LIST_CHATROOM, dbv.pszVal) && strchr(dbv.pszVal, '/')==NULL) {
				strcpy(msgType, "groupchat");
			} else if (DBGetContactSettingByte(hContact, proto->iface.m_szModuleName, "bChat", FALSE)) {
				strcpy(msgType, "privchat");
			} else {
				strcpy(msgType, "chat");
			}
			if (!strcmp(msgType, "groupchat") || DBGetContactSettingByte(NULL, proto->iface.m_szModuleName, "MsgAck", FALSE) == FALSE) {
                SENDACKTHREADDATA *tdata = (SENDACKTHREADDATA*) mir_alloc(sizeof(SENDACKTHREADDATA));
                tdata->proto = proto;
                tdata->hContact = hContact;
				if (!strcmp(msgType, "groupchat")) {
					JabberSend(proto, "<message to='%s' type='%s'><body>%s</body></message>", dbv.pszVal, msgType, msgEnc);
				} else if (!strcmp(msgType, "privchat")) {
					JabberSend(proto, "<m to='%s'><b n='6' s='10' f='0' c='000000'>%s</b></m>", dbv.pszVal, msgEnc);
				} else {
					id = JabberSerialNext(proto);
					JabberSend(proto, "<message to='%s' type='%s' id='"JABBER_IQID"%d'><body>%s</body><x xmlns='jabber:x:event'><composing/></x></message>", dbv.pszVal, msgType, id, msgEnc);
				}
				JabberForkThread(JabberSendMessageAckThread, 0, (void *) tdata);
			}
			else {
				id = JabberSerialNext(proto);
				if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal)) != NULL)
					item->idMsgAckPending = id;
				JabberSend(proto, "<message to='%s' type='%s' id='"JABBER_IQID"%d'><body>%s</body><x xmlns='jabber:x:event'><offline/><delivered/><composing/></x></message>", dbv.pszVal, msgType, id, msgEnc);
			}
		}
		mir_free(msgEnc);
	}
	DBFreeVariant(&dbv);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// JabberGetAvatarInfo - retrieves the avatar info

static INT_PTR TlenGetAvatarInfo(void *ptr, WPARAM wParam, LPARAM lParam)
{
	BOOL downloadingAvatar = FALSE;
	char *avatarHash = NULL;
	JABBER_LIST_ITEM *item = NULL;
	DBVARIANT dbv;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	PROTO_AVATAR_INFORMATION* AI = ( PROTO_AVATAR_INFORMATION* )lParam;
	if (!proto->tlenOptions.enableAvatars) return GAIR_NOAVATAR;

	if (AI->hContact != NULL) {
		if (!DBGetContactSetting(AI->hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
			item = JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal);
			DBFreeVariant(&dbv);
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

HANDLE TlenGetAwayMsg(PROTO_INTERFACE *ptr, HANDLE hContact)
{
    TlenProtocol *proto = (TlenProtocol *)ptr;
    SENDACKTHREADDATA *tdata = (SENDACKTHREADDATA*) mir_alloc(sizeof(SENDACKTHREADDATA));
    tdata->proto = proto;
    tdata->hContact = hContact;
	JabberForkThread((void (__cdecl *)(void*))TlenGetAwayMsgThread, 0, (void *) tdata);
	return (HANDLE)1;
}

HANDLE TlenFileAllow(PROTO_INTERFACE *ptr, HANDLE hContact, HANDLE hTransfer, const char* szPath)
{
	TLEN_FILE_TRANSFER *ft;
	JABBER_LIST_ITEM *item;
	char *nick;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 0;

	ft = (TLEN_FILE_TRANSFER *) hTransfer;
	ft->szSavePath = mir_strdup(szPath);
	if ((item=JabberListAdd(proto, LIST_FILE, ft->iqId)) != NULL) {
		item->ft = ft;
	}
	nick = JabberNickFromJID(ft->jid);
	if (ft->newP2P) {
		JabberSend(proto, "<iq to='%s'><query xmlns='p2p'><fs t='%s' e='5' i='%s' v='1'/></query></iq>", ft->jid, ft->jid, ft->iqId);
	} else {
		JabberSend(proto, "<f t='%s' i='%s' e='5' v='1'/>", nick, ft->iqId);
	}
	mir_free(nick);
	return (HANDLE)hTransfer;
}

int TlenFileDeny(PROTO_INTERFACE *ptr, HANDLE hContact, HANDLE hTransfer, const char* szReason)
{
	TLEN_FILE_TRANSFER *ft;
	char *nick;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 1;

	ft = (TLEN_FILE_TRANSFER *) hTransfer;
	nick = JabberNickFromJID(ft->jid);
	if (ft->newP2P) {
		JabberSend(proto, "<f i='%s' e='4' t='%s'/>", ft->iqId, nick);\
	} else {
		JabberSend(proto, "<f i='%s' e='4' t='%s'/>", ft->iqId, nick);\
	}
	mir_free(nick);
	TlenP2PFreeFileTransfer(ft);
	return 0;
}

int TlenFileCancel(PROTO_INTERFACE *ptr, HANDLE hContact, HANDLE hTransfer)
{
	TLEN_FILE_TRANSFER *ft = (TLEN_FILE_TRANSFER *) hTransfer;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	JabberLog(proto, "Invoking FileCancel()");
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

HANDLE TlenSendFile(PROTO_INTERFACE *ptr, HANDLE hContact, const char* szDescription, char** ppszFiles)
{
	TLEN_FILE_TRANSFER *ft;
	int i, j;
	struct _stat statbuf;
	DBVARIANT dbv;
	char *nick, *p, idStr[10];
	JABBER_LIST_ITEM *item;
	int id;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 0;
//	if (DBGetContactSettingWord(ccs->hContact, iface.m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) return 0;
	if (DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) return 0;
    ft = TlenFileCreateFT(proto, dbv.pszVal);
	for(ft->fileCount=0; ppszFiles[ft->fileCount]; ft->fileCount++);
	ft->files = (char **) mir_alloc(sizeof(char *) * ft->fileCount);
	ft->filesSize = (long *) mir_alloc(sizeof(long) * ft->fileCount);
	ft->allFileTotalSize = 0;
	for(i=j=0; i<ft->fileCount; i++) {
		if (_stat(ppszFiles[i], &statbuf))
			JabberLog(proto, "'%s' is an invalid filename", ppszFiles[i]);
		else {
			ft->filesSize[j] = statbuf.st_size;
			ft->files[j++] = mir_strdup(ppszFiles[i]);
			ft->allFileTotalSize += statbuf.st_size;
		}
	}
	ft->fileCount = j;
	ft->szDescription = mir_strdup(szDescription);
	ft->hContact = hContact;
	ft->currentFile = 0;
	DBFreeVariant(&dbv);

	id = JabberSerialNext(proto);
	_snprintf(idStr, sizeof(idStr), "%d", id);
	if ((item=JabberListAdd(proto, LIST_FILE, idStr)) != NULL) {
		ft->iqId = mir_strdup(idStr);
		nick = JabberNickFromJID(ft->jid);
		item->ft = ft;
		if (proto->tlenOptions.useNewP2P) {
			JabberSend(proto, "<iq to='%s'><query xmlns='p2p'><fs t='%s' e='1' i='%s' c='%d' s='%d' v='%d'/></query></iq>",
				ft->jid, ft->jid, idStr, ft->fileCount, ft->allFileTotalSize, ft->fileCount);

			ft->newP2P = TRUE;
		} else {
			if (ft->fileCount == 1) {
				if ((p=strrchr(ppszFiles[0], '\\')) != NULL)
					p++;
				else
					p = ppszFiles[0];
				p = JabberTextEncode(p);
				JabberSend(proto, "<f t='%s' n='%s' e='1' i='%s' c='1' s='%d' v='1'/>", nick, p, idStr, ft->allFileTotalSize);
				mir_free(p);
			}
			else
				JabberSend(proto, "<f t='%s' e='1' i='%s' c='%d' s='%d' v='1'/>", nick, idStr, ft->fileCount, ft->allFileTotalSize);
		}
		mir_free(nick);
	}

	return (HANDLE) ft;
}

int TlenRecvMessage(PROTO_INTERFACE *ptr, HANDLE hContact, PROTORECVEVENT* evt)
{
	CCSDATA ccs = { hContact, PSR_MESSAGE, 0, ( LPARAM )evt };
	return CallService( MS_PROTO_RECVMSG, 0, ( LPARAM )&ccs );
}

int TlenRecvFile(PROTO_INTERFACE *ptr, HANDLE hContact, PROTORECVFILE* evt)
{
	CCSDATA ccs = { hContact, PSR_FILE, 0, ( LPARAM )evt };
	return CallService( MS_PROTO_RECVFILE, 0, ( LPARAM )&ccs );
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
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto==NULL || strcmp(szProto, proto->iface.m_szModuleName)) return 0;
//		if (DBGetContactSettingByte(hContact, iface.m_szModuleName, "ChatRoom", 0) != 0) return 0;
		// A contact's group is changed
		if (!strcmp(cws->szSetting, "Group")) {
			if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
				if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
					DBFreeVariant(&dbv);
					if (!DBGetContactSetting(hContact, "CList", "MyHandle", &dbv)) {
						nick = JabberTextEncode(dbv.pszVal);
						DBFreeVariant(&dbv);
					} else if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "Nick", &dbv)) {
						nick = JabberTextEncode(dbv.pszVal);
						DBFreeVariant(&dbv);
					} else {
						nick = JabberNickFromJID(item->jid);
					}
					if (nick != NULL) {
						// Note: we need to compare with item->group to prevent infinite loop
						if (cws->value.type==DBVT_DELETED && item->group!=NULL) {
							JabberLog(proto, "Group set to nothing");
							JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'></item></query></iq>", nick, item->jid);
						} else if (cws->value.pszVal != NULL) {
							char *newGroup = settingToChar(cws);
							if (item->group==NULL || strcmp(newGroup, item->group)) {
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
					DBFreeVariant(&dbv);
				}
			}
		}
		// A contact is renamed
		else if (!strcmp(cws->szSetting, "MyHandle")) {
			char *newNick;

//			hContact = (HANDLE) wParam;
//			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
//			if (szProto==NULL || strcmp(szProto, iface.m_szModuleName)) return 0;

			if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
				jid = dbv.pszVal;
				if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal)) != NULL) {
					if (cws->value.type == DBVT_DELETED) {
						newNick = mir_strdup((char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_NOMYHANDLE));
					} else if (cws->value.pszVal!=NULL) {
                        newNick = settingToChar(cws);
					} else {
						newNick = NULL;
					}
					// Note: we need to compare with item->nick to prevent infinite loop
					if (newNick!=NULL && (item->nick==NULL || (item->nick!=NULL && strcmp(item->nick, newNick)))) {
						if ((nick=JabberTextEncode(newNick)) != NULL) {
							JabberLog(proto, "Nick set to %s", newNick);
							if (item->group!=NULL && (group=TlenGroupEncode(item->group))!=NULL) {
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
				DBFreeVariant(&dbv);
			}
		}
		// A temporary contact has been added permanently
		else if (!strcmp(cws->szSetting, "NotOnList")) {
			char *jid, *nick, *pGroup;

			if (cws->value.type==DBVT_DELETED || (cws->value.type==DBVT_BYTE && cws->value.bVal==0)) {
				if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
					jid = mir_strdup(dbv.pszVal);
					DBFreeVariant(&dbv);
					JabberLog(proto, "Add %s permanently to list", jid);
					if (!DBGetContactSetting(hContact, "CList", "MyHandle", &dbv)) {
						nick = JabberTextEncode(dbv.pszVal); //Utf8Encode
						DBFreeVariant(&dbv);
					}
					else {
						nick = JabberNickFromJID(jid);
					}
					if (nick != NULL) {
						JabberLog(proto, "jid=%s nick=%s", jid, nick);
						if (!DBGetContactSetting(hContact, "CList", "Group", &dbv)) {
							if ((pGroup=TlenGroupEncode(dbv.pszVal)) != NULL) {
								JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'><group>%s</group></item></query></iq>", nick, jid, pGroup);
								JabberSend(proto, "<presence to='%s' type='subscribe'/>", jid);
								mir_free(pGroup);
							}
							DBFreeVariant(&dbv);
						}
						else {
							JabberSend(proto, "<iq type='set'><query xmlns='jabber:iq:roster'><item name='%s' jid='%s'/></query></iq>", nick, jid);
							JabberSend(proto, "<presence to='%s' type='subscribe'/>", jid);
						}
						mir_free(nick);
						DBDeleteContactSetting(hContact, "CList", "Hidden");
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

	if(!proto->isOnline)	// should never happen
		return 0;
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (szProto==NULL || strcmp(szProto, proto->iface.m_szModuleName))
		return 0;
	if (!DBGetContactSetting((HANDLE) wParam, proto->iface.m_szModuleName, "jid", &dbv)) {
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

		DBFreeVariant(&dbv);
	}
	return 0;
}

int TlenUserIsTyping(PROTO_INTERFACE *ptr, HANDLE hContact, int type)
{
	DBVARIANT dbv;
	JABBER_LIST_ITEM *item;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if (!proto->isOnline) return 0;
	if (!DBGetContactSetting(hContact, proto->iface.m_szModuleName, "jid", &dbv)) {
		if ((item=JabberListGetItemPtr(proto, LIST_ROSTER, dbv.pszVal))!=NULL /*&& item->wantComposingEvent==TRUE*/) {
			switch (type) {
			case PROTOTYPE_SELFTYPING_OFF:
				JabberSend(proto, "<m tp='u' to='%s'/>", dbv.pszVal);
				break;
			case PROTOTYPE_SELFTYPING_ON:
				JabberSend(proto, "<m tp='t' to='%s'/>", dbv.pszVal);
				break;
			}
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR TlenGetMyAvatar(void *ptr, WPARAM wParam, LPARAM lParam)
{
	char* buf = ( char* )wParam;
	int  size = ( int )lParam;
    TlenProtocol *proto = (TlenProtocol *)ptr;

	if ( buf == NULL || size <= 0 )
		return -1;

	TlenGetAvatarFileName( proto, NULL, buf, size );
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

INT_PTR TlenSetMyAvatar(void *ptr, WPARAM wParam, LPARAM lParam)
{
	char* szFileName = ( char* )lParam;
   	char tFileName[ MAX_PATH ];
	int fileIn;
    TlenProtocol *proto = (TlenProtocol *)ptr;
	if(!proto->isOnline) return 1;
	if (szFileName != NULL) {
		int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_USER_CHANGEAVATAR), NULL, TlenChangeAvatarDlgProc, (LPARAM) NULL);
		TlenGetAvatarFileName( proto, NULL, tFileName, MAX_PATH);
		if ( CopyFileA( szFileName, tFileName, FALSE ) == FALSE ) {
			return 1;
		}
		fileIn = open( tFileName, O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
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
	} else {
		TlenRemoveAvatar(proto);
	}
	return 0;
}

INT_PTR TlenGetAvatarCaps(void *ptr, WPARAM wParam, LPARAM lParam)
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


int TlenOnEvent( PROTO_INTERFACE *ptr, PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam )
{
    TlenProtocol *proto = (TlenProtocol *)ptr;
	switch( eventType ) {
	case EV_PROTO_ONLOAD:    return TlenOnModulesLoaded(proto, 0, 0 );
	case EV_PROTO_ONOPTIONS: return TlenOptionsInit(proto, wParam, lParam );
	case EV_PROTO_ONEXIT:    return TlenPreShutdown(proto, 0, 0 );
	case EV_PROTO_ONRENAME:
		{
			CLISTMENUITEM clmi = { 0 };
			clmi.cbSize = sizeof( CLISTMENUITEM );
			clmi.flags = CMIM_NAME | CMIF_TCHAR;
			clmi.ptszName = proto->iface.m_tszUserName;
			CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )proto->hMenuRoot, ( LPARAM )&clmi );
			/* FIXME: Rename network user as well */
    	}
    }
	return 1;
}

// PSS_ADDED
int TlenAuthRecv(PROTO_INTERFACE *ptr,  HANDLE hContact, PROTORECVEVENT* evt )
{
	return 1;
}

// PSS_AUTHREQUEST
int TlenAuthRequest(PROTO_INTERFACE *ptr,  HANDLE hContact, const TCHAR* szMessage )
{
	return 1;
}

HANDLE TlenChangeInfo(PROTO_INTERFACE *ptr,   int iInfoType, void* pInfoData )
{
	return NULL;
}

int TlenRecvContacts(PROTO_INTERFACE *ptr,   HANDLE hContact, PROTORECVEVENT* evt)
{
	return 1;
}


extern INT_PTR CALLBACK TlenAccMgrUIDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR TlenAccMgrUI(void *ptr, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR) CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, TlenAccMgrUIDlgProc, (LPARAM) ptr);
}

void TlenInitServicesVTbl(TlenProtocol *proto) {
	char s[128];
    proto->iface.vtbl = (PROTO_INTERFACE_VTBL*) mir_alloc(sizeof(PROTO_INTERFACE_VTBL));
    proto->iface.vtbl->AddToList = TlenAddToList;
    proto->iface.vtbl->AddToListByEvent = TlenAddToListByEvent;
    proto->iface.vtbl->AuthDeny = TlenAuthDeny;
    proto->iface.vtbl->AuthRecv = TlenAuthRecv;
    proto->iface.vtbl->AuthRequest = TlenAuthRequest;
    proto->iface.vtbl->Authorize = TlenAuthAllow;
    proto->iface.vtbl->ChangeInfo = TlenChangeInfo;
    proto->iface.vtbl->FileAllow = TlenFileAllow;
    proto->iface.vtbl->FileCancel = TlenFileCancel;
    proto->iface.vtbl->FileDeny = TlenFileDeny;
    proto->iface.vtbl->FileResume = NULL;
    proto->iface.vtbl->SearchBasic = TlenBasicSearch;
    proto->iface.vtbl->SearchByEmail = TlenSearchByEmail;
    proto->iface.vtbl->SearchByName = TlenSearchByName;
    proto->iface.vtbl->SearchAdvanced = TlenSearchAdvanced;
    proto->iface.vtbl->CreateExtendedSearchUI = TlenCreateAdvSearchUI;

    proto->iface.vtbl->RecvContacts = TlenRecvContacts;
    proto->iface.vtbl->RecvFile = TlenRecvFile;
    proto->iface.vtbl->RecvMsg = TlenRecvMessage;
    proto->iface.vtbl->RecvUrl = NULL;

    proto->iface.vtbl->SendContacts = NULL;
    proto->iface.vtbl->SendFile = TlenSendFile;
    proto->iface.vtbl->SendMsg = TlenSendMessage;
    proto->iface.vtbl->SendUrl = NULL;

    proto->iface.vtbl->GetCaps = TlenGetCaps;
    proto->iface.vtbl->GetIcon = TlenGetIcon;
    proto->iface.vtbl->GetInfo = JabberGetInfo;
    proto->iface.vtbl->SetApparentMode = TlenSetApparentMode;
    proto->iface.vtbl->SetStatus = TlenSetStatus;


    proto->iface.vtbl->GetAwayMsg = TlenGetAwayMsg;
    proto->iface.vtbl->RecvAwayMsg = NULL;
    proto->iface.vtbl->SendAwayMsg = NULL;
    proto->iface.vtbl->SetAwayMsg = TlenSetAwayMsg;

    proto->iface.vtbl->UserIsTyping = TlenUserIsTyping;

    proto->iface.vtbl->OnEvent = TlenOnEvent;

	sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_GETNAME);
	CreateServiceFunction_Ex(s, proto, TlenGetName);

	sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_GETAVATARINFO);
	CreateServiceFunction_Ex(s, proto, TlenGetAvatarInfo);

	sprintf(s, "%s%s", proto->iface.m_szModuleName, "/SendNudge");
	CreateServiceFunction_Ex(s, proto, TlenSendAlert);

	sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_GETAVATARCAPS);
	CreateServiceFunction_Ex(s, proto, TlenGetAvatarCaps);

	sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_SETMYAVATAR);
	CreateServiceFunction_Ex(s, proto, TlenSetMyAvatar);

	sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_GETMYAVATAR);
	CreateServiceFunction_Ex(s, proto, TlenGetMyAvatar);

    sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_GETSTATUS);
	CreateServiceFunction_Ex(s, proto, TlenGetStatus);

    sprintf(s, "%s%s", proto->iface.m_szModuleName, PS_CREATEACCMGRUI);
	CreateServiceFunction_Ex(s, proto, TlenAccMgrUI);

}

