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

CMPlugin g_plugin;

MCONTACT hForwardFrom, hForwardTo;
wchar_t tszForwardTemplate[MAXTEMPLATESIZE]; 
int iSplit, iSplitMaxSize, iSendParts, iMarkRead, iSendAndHistory, iForwardOnStatus;

LIST<MESSAGE_PROC> arMessageProcs(10, HandleKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {01202E6A-C1B3-42E5-838A-3E497B31F38E}
	{0x1202e6a, 0xc1b3, 0x42e5, {0x83, 0x8a, 0x3e, 0x49, 0x7b, 0x31, 0xf3, 0x8e}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// Protocols acknowledgement

int ProtoAck(WPARAM,LPARAM lparam)
{
	ACKDATA *pAck = (ACKDATA *)lparam;
	if (pAck->type != ACKTYPE_MESSAGE || pAck->result != ACKRESULT_SUCCESS)
		return 0;

	MESSAGE_PROC* p = arMessageProcs.find((MESSAGE_PROC*)&pAck->hProcess);
	if (p == nullptr)
		return 0;

	if (iSendAndHistory > 0){
		time_t ltime;
		time(&ltime);

		DBEVENTINFO dbei = {};
		dbei.szModule = MODULENAME;
		dbei.timestamp = ltime;
		dbei.flags = DBEF_SENT | DBEF_UTF;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.cbBlob = (uint32_t)mir_strlen(p->msgText) + 1;
		dbei.pBlob = (uint8_t*)p->msgText;
		db_event_add(hForwardTo, &dbei);
	}

	mir_free(p->msgText);
	arMessageProcs.remove(p);
	mir_free(p);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// New event was added into DB.

static int MessageEventAdded(WPARAM hContact, LPARAM hDBEvent)
{
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
		default: return 0;
	}
	if ((iForwardOnStatus & statMask) == 0)
		return 0;

	// receive message from DB
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	db_event_get(hDBEvent, &dbei);
	if (dbei.flags & DBEF_SENT || dbei.flags & DBEF_READ || (dbei.eventType != EVENTTYPE_MESSAGE))
		return 0;

	// get time and date
	time_t tTime = dbei.timestamp;
	tm *tm_time = gmtime(&tTime);

	// build a message
	CMStringA szUtfMsg;
	T2Utf szTemplate(tszForwardTemplate);
	for (char *p = szTemplate; *p; p++) {
		if (*p != '%') {
			szUtfMsg.AppendChar(*p);
			continue;
		}

		wchar_t buf[100];
		switch(*++p) {
		case 'u':
		case 'U':
			szUtfMsg.Append(T2Utf(Clist_GetContactDisplayName(hContact)));
			break;

		case 'i':
		case 'I':
			{
				ptrW id(Contact::GetInfo(CNF_UNIQUEID, NULL));
				if (id != NULL)
					wcsncpy_s(buf, id, _TRUNCATE);
				else
					_itow_s(hContact, buf, 10);
			}
			szUtfMsg.Append(T2Utf(buf));
			break;

		case 't':
		case 'T':
			wcsftime(buf, 10, L"%H:%M", tm_time);
			szUtfMsg.Append(T2Utf(buf));
			break;

		case 'd':
		case 'D':
			wcsftime(buf, 12, L"%d/%m/%Y", tm_time);
			szUtfMsg.Append(T2Utf(buf));
			break;	

		case 'm':
		case 'M':
			if (dbei.flags & DBEF_UTF)
				szUtfMsg.Append((char*)dbei.pBlob, dbei.cbBlob);
			else
				szUtfMsg.Append(ptrA(mir_utf8encode((char*)dbei.pBlob)));
			break;

		case '%':
			szUtfMsg.AppendChar('%');
			break;
		}
	}

	int iPartCount = 1;
	size_t cbMsgSize = szUtfMsg.GetLength(), cbPortion = cbMsgSize;
	if (iSplit > 0) {
		iPartCount = min(iSendParts, int((szUtfMsg.GetLength() + iSplitMaxSize) / iSplitMaxSize));
		cbPortion = iSplitMaxSize;
	}

	const char *szBuf = szUtfMsg;
	for (int i=0; i < iPartCount; i++, szBuf += cbPortion) {
		char *szMsgPart = (char*)mir_alloc(cbPortion+1);
		strncpy(szMsgPart, szBuf, cbPortion);
		szMsgPart[cbPortion] = 0;

		HANDLE hMsgProc = (HANDLE)ProtoChainSend(hForwardTo, PSS_MESSAGE, 0, (LPARAM)szMsgPart);

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

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	// Load plugin options from DB
	hForwardFrom = (MCONTACT)g_plugin.getDword("ForwardFrom", 0);
	hForwardTo = (MCONTACT)g_plugin.getDword("ForwardTo", 0);

	iForwardOnStatus = g_plugin.getDword("ForwardOnStatus", STATUS_OFFLINE | STATUS_AWAY | STATUS_NA);

	wchar_t *szForwardTemplate = g_plugin.getWStringA("ForwardTemplate");
	if (szForwardTemplate){
		wcsncpy(tszForwardTemplate, szForwardTemplate, _countof(tszForwardTemplate));
		mir_free(szForwardTemplate);
	}
	else wcsncpy(tszForwardTemplate, L"%u: %m", MAXTEMPLATESIZE-1);

	iSplit          = g_plugin.getDword("Split", 0);
	iSplitMaxSize   = g_plugin.getDword("SplitMaxSize", 100);
	iSendParts      = g_plugin.getDword("SendParts", 0);
	iMarkRead       = g_plugin.getDword("MarkRead", 0);
	iSendAndHistory = g_plugin.getDword("SendAndHistory", 1);

	// hook events
	HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_PROTO_ACK, ProtoAck);
	return 0;
}
