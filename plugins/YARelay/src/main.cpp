/*
yaRelay.cpp

Yet Another Relay plugin. v.0.0.0.3
This plugin forwards all incoming messages to any contact.

Features:
 - Forwards all messages from any specified contact (or from all contacts)
 - Works only if your status is equals to specified (of set of statuses)
 - Could be specified any template for sent messages
 - Original message could be split up (by size)
 - Could be specified number of split parts to send
 - Incoming message could be marked as 'read' (optional)
 - Outgoing messages could be saved in history (optional)

(c)2005 Anar Ibragimoff (ai91@mail.ru)

*/

#include "stdafx.h"

#include "..\Utils\mir_buffer.h"

CLIST_INTERFACE *pcli;
HINSTANCE hInst;
int hLangpack;

MCONTACT hForwardFrom, hForwardTo;
TCHAR tszForwardTemplate[MAXTEMPLATESIZE]; 
int iSplit, iSplitMaxSize, iSendParts, iMarkRead, iSendAndHistory, iForwardOnStatus;

LIST<MESSAGE_PROC> arMessageProcs(10, HandleKeySortT);

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {01202E6A-C1B3-42E5-838A-3E497B31F38E}
	{0x1202e6a, 0xc1b3, 0x42e5, {0x83, 0x8a, 0x3e, 0x49, 0x7b, 0x31, 0xf3, 0x8e}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

/**
* Protocols àcknowledgement
*/
int ProtoAck(WPARAM wparam,LPARAM lparam)
{
	ACKDATA *pAck = (ACKDATA *)lparam;
	if (pAck->type != ACKTYPE_MESSAGE || pAck->result != ACKRESULT_SUCCESS)
		return 0;

	MESSAGE_PROC* p = arMessageProcs.find((MESSAGE_PROC*)&pAck->hProcess);
	if (p == NULL)
		return 0;

	if (iSendAndHistory > 0){
		time_t ltime;
		time(&ltime);

		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.szModule = "yaRelay";
		dbei.timestamp = ltime;
		dbei.flags = DBEF_SENT | DBEF_UTF;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.cbBlob = (DWORD)strlen(p->msgText) + 1;
		dbei.pBlob = (PBYTE)p->msgText;
		db_event_add(hForwardTo, &dbei);
	}

	mir_free(p->msgText);
	arMessageProcs.remove(p);
	mir_free(p);
	return 0;
}

/**
* New event was added into DB.
*/
static int MessageEventAdded(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	HANDLE hDBEvent = (HANDLE)lParam;

	// is the message sender accepted for forwarding
	if (hForwardFrom != 0 && hForwardFrom != hContact)
		return 0;

	// is receiver specified
	if (hForwardTo == 0)
		return 0;

	// don't reply to receiver
	if (hForwardTo == hContact)
		return 0;

	// is current status acceptable
	int statMask;
	switch(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)){
		case ID_STATUS_OFFLINE:  statMask = STATUS_OFFLINE  ;break;
		case ID_STATUS_ONLINE:   statMask = STATUS_ONLINE   ;break;
		case ID_STATUS_AWAY:     statMask = STATUS_AWAY     ;break;
		case ID_STATUS_NA:       statMask = STATUS_NA       ;break;
		case ID_STATUS_OCCUPIED: statMask = STATUS_OCCUPIED ;break;   
		case ID_STATUS_DND:      statMask = STATUS_DND      ;break;
		case ID_STATUS_FREECHAT: statMask = STATUS_FREECHAT ;break;
		case ID_STATUS_INVISIBLE:statMask = STATUS_INVISIBLE;break;
	}
	if ((iForwardOnStatus & statMask) == 0)
		return 0;

	// receive message from DB
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = db_event_getBlobSize(hDBEvent);
	if (dbei.cbBlob == -1)
		return 0;

	dbei.pBlob = (unsigned char*)alloca(dbei.cbBlob);
	db_event_get(hDBEvent, &dbei);
	if (dbei.flags & DBEF_SENT || dbei.flags & DBEF_READ || (dbei.eventType != EVENTTYPE_MESSAGE))
		return 0;

	// get time and date
	time_t tTime = dbei.timestamp;
	tm *tm_time = gmtime(&tTime);

	// build a message
	Buffer<char> szUtfMsg;
	ptrA szTemplate( mir_utf8encodeT(tszForwardTemplate));
	for (char *p = szTemplate; *p; p++) {
		if (*p != '%') {
			szUtfMsg.append(*p);
			continue;
		}

		TCHAR buf[100];
		switch(*++p) {
		case 'u':
		case 'U':
			szUtfMsg.append( ptrA(mir_utf8encodeT(pcli->pfnGetContactDisplayName(hContact, 0))));
			break;

		case 'i':
		case 'I':
			{
				// get sender's uin
				CONTACTINFO ci = { sizeof(ci) };
				ci.dwFlag = CNF_UNIQUEID;
				if (CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci) == 0){
					if (ci.type == CNFT_ASCIIZ)
						_tcsncpy_s(buf, ci.pszVal, _TRUNCATE);
					else if (ci.type == CNFT_BYTE)
						mir_sntprintf(buf, SIZEOF(buf), _T("%u"), ci.bVal);
					else if (ci.type == CNFT_WORD)
						mir_sntprintf(buf, SIZEOF(buf), _T("%u"), ci.wVal);
					else if (ci.type == CNFT_DWORD)
						mir_sntprintf(buf, SIZEOF(buf), _T("%u"), ci.dVal);
				}
				else mir_sntprintf(buf, SIZEOF(buf), _T("%p"), hContact);
			}
			szUtfMsg.append( ptrA(mir_utf8encodeT(buf)));
			break;

		case 't':
		case 'T':
			_tcsftime(buf, 10, _T("%H:%M"), tm_time);
			szUtfMsg.append( ptrA(mir_utf8encodeT(buf)));
			break;

		case 'd':
		case 'D':
			_tcsftime(buf, 12, _T("%d/%m/%Y"), tm_time);
			szUtfMsg.append( ptrA(mir_utf8encodeT(buf)));
			break;

		case 'm':
		case 'M':
			if (dbei.flags & DBEF_UTF)
				szUtfMsg.append((char*)dbei.pBlob, dbei.cbBlob);
			else
				szUtfMsg.append( ptrA(mir_utf8encode((char*)dbei.pBlob)));
			break;

		case '%':
			szUtfMsg.append('%');
			break;
		}
	}

	int iPartCount = 1;
	size_t cbMsgSize = szUtfMsg.len, cbPortion = cbMsgSize;
	if (iSplit > 0) {
		iPartCount = min(iSendParts, int((szUtfMsg.len + iSplitMaxSize) / iSplitMaxSize));
		cbPortion = iSplitMaxSize;
	}

	char *szBuf = szUtfMsg.str;
	for (int i=0; i < iPartCount; i++, szBuf += cbPortion) {
		char *szMsgPart = (char*)mir_alloc(cbPortion+1);
		strncpy(szMsgPart, szBuf, cbPortion);
		szMsgPart[cbPortion] = 0;

		HANDLE hMsgProc = (HANDLE)CallContactService(hForwardTo, PSS_MESSAGE, PREF_UTF, (LPARAM)szMsgPart);

		MESSAGE_PROC* msgProc = (MESSAGE_PROC*)mir_alloc(sizeof(MESSAGE_PROC));
		msgProc->hProcess = hMsgProc;
		msgProc->msgText = szMsgPart;
		msgProc->retryCount = 0;
		arMessageProcs.insert(msgProc);
	}

	// mark message as 'read'
	if (iMarkRead > 0)
		db_event_markRead(hContact, hDBEvent);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

	// Load plugin options from DB
	hForwardFrom = (MCONTACT)db_get_dw(NULL, "yaRelay", "ForwardFrom", 0);
	hForwardTo = (MCONTACT)db_get_dw(NULL, "yaRelay", "ForwardTo", 0);

	iForwardOnStatus = db_get_dw(NULL, "yaRelay", "ForwardOnStatus", STATUS_OFFLINE | STATUS_AWAY | STATUS_NA);

	DBVARIANT dbv;
	if (!db_get_ts(NULL, "yaRelay", "ForwardTemplate", &dbv)){
		_tcsncpy(tszForwardTemplate, dbv.ptszVal, SIZEOF(tszForwardTemplate));
		db_free(&dbv);
	}
	else _tcsncpy(tszForwardTemplate, _T("%u: %m"), MAXTEMPLATESIZE-1);

	iSplit          = db_get_dw(NULL, "yaRelay", "Split", 0);
	iSplitMaxSize   = db_get_dw(NULL, "yaRelay", "SplitMaxSize", 100);
	iSendParts      = db_get_dw(NULL, "yaRelay", "SendParts", 0);
	iMarkRead       = db_get_dw(NULL, "yaRelay", "MarkRead", 0);
	iSendAndHistory = db_get_dw(NULL, "yaRelay", "SendAndHistory", 1);

	// hook events
	HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_PROTO_ACK, ProtoAck);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
