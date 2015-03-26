#include "stdafx.h"

static HANDLE hDBEventPreAdd, hDBEventAdded, hContactSettingChanged;
static CRITICAL_SECTION RemoveChainCS={0}, *lpRemoveChainCS = &RemoveChainCS;
static UINT_PTR timerId = 0;

struct DeleteEventNode {
	DeleteEventNode *next;
	time_t timestamp;
	MCONTACT hContact;
	MEVENT hDbEvent;
};
struct DeleteEventHead {
	DeleteEventNode *first;
	DeleteEventNode *last;
};
static DeleteEventHead DeleteEvents = {0,0};

VOID CALLBACK DeleteTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	if (!DeleteEvents.first) return;
	EnterCriticalSection(lpRemoveChainCS);
	DeleteEventNode *prev =0, *current, *next;
	DBEVENTINFO info = { sizeof(info) };
	next = DeleteEvents.first;
	while (current = next) {
		if (difftime(time(0), current->timestamp) < 1) break;
		if (!db_event_get(current->hDbEvent, &info)) // && info.flags&DBEF_READ)
		{
			db_event_delete(current->hContact, current->hDbEvent);
			next = current->next;
			if (prev) prev->next = next;
			else if (DeleteEvents.first == current) DeleteEvents.first = next;
			delete current;
		}
		else {
			prev = current;
			next = current->next;
		}
	}
	if (!DeleteEvents.first) DeleteEvents.last = 0;
	LeaveCriticalSection(lpRemoveChainCS);
}


// add prefix to sent messages
int OnDatabaseEventPreAdd(WPARAM hContact, LPARAM lParam)
{
	if (!options.prefix_messages || !lParam)
		return 0;

	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if ((dbei->eventType != EVENTTYPE_MESSAGE) || !(dbei->flags & DBEF_SENT) || (dbei->flags & DBEF_OTR_PREFIXED))
		return 0;

	if(dbei->cbBlob == 0 || dbei->pBlob == 0)
		return 0; // just to be safe

	const char *proto = GetContactProto(hContact);
	if (!proto)
		return 0;
	if (db_get_b(hContact, proto, "ChatRoom", 0) == 1)
		return 0;
	
	if(strcmp(proto, META_PROTO) == 0) {
		hContact = db_mc_getMostOnline(hContact);
		if (!hContact) return 0;
		proto = GetContactProto(hContact);
		if (!proto )	return 0;
	}
	

	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);	
	bool encrypted = otr_context_get_trust(context) != TRUST_NOT_PRIVATE;
	if (!encrypted) return 0;
	
	DBEVENTINFO my_dbei = *dbei; // copy the other event

	char *msg = (char *)dbei->pBlob;
	char *newmsg = 0;
	DWORD alloclen = 0;
	DWORD msglen = (DWORD)strlen(msg);
	if (dbei->flags & DBEF_UTF) {
		int prefixlen = (int)strnlen(options.prefix, 64);
		if (strncmp(msg, options.prefix, prefixlen) == 0) return 0;
		alloclen = (msglen+prefixlen+1)*sizeof(char);

		// get additional data
		DWORD len2 = alloclen- prefixlen*sizeof(char);
		int datalen = dbei->cbBlob - len2;
		if (datalen < 0) datalen = 0;

		newmsg = (char*)mir_alloc(alloclen);
		memset(newmsg, 0, alloclen+datalen);
		strncpy(newmsg, options.prefix, prefixlen);
		strncat(newmsg, msg, msglen);
		// append additional data
		if (datalen) {
				memcpy(newmsg+alloclen, msg+len2, datalen);
				alloclen += datalen;
		}
	} else {
		bool dz = false;
		if(msglen < dbei->cbBlob) {
			for(unsigned int i = msglen; i < dbei->cbBlob; i++) {
				if(msg[i] == 0 && msg[i - 1] == 0) {
					dz = true;
					break;
				}
			}
		}
		if(dz) {
			// yes, unicode
			wchar_t* msgw = (wchar_t *)&msg[msglen];
			wchar_t *prefixw = mir_utf8decodeW(options.prefix);
			int prefixlenw = (int)wcslen(prefixw);
			if (wcsncmp(msgw, prefixw, prefixlenw) == 0) {
				mir_free(prefixw);
				return 0;
			}
			int msglenw = (int)wcslen(msgw);

			char *prefix = mir_utf8decodeA(options.prefix);
			int prefixlen = (int)strlen(prefix);

			alloclen = (msglen+prefixlen+1)* sizeof(char) + (msglenw + prefixlenw +1) * sizeof(wchar_t);
			// get additional data
			DWORD len2 = (msglen+1)* sizeof(char) + (msglenw+1) * sizeof(wchar_t);
			int datalen = dbei->cbBlob - len2;
			if (datalen < 0) datalen = 0;
			
			newmsg = (char*)mir_alloc(alloclen+datalen);
			memset(newmsg, 0, alloclen+datalen);
			strncpy(newmsg, prefix, prefixlen);
			strncat(newmsg, msg, msglen);
			wchar_t *p = (wchar_t*) newmsg + (msglen + prefixlen + 1);
			wcsncpy(p, prefixw, prefixlenw);
			wcsncat(p, msgw, msglenw);
			mir_free(prefix);
			mir_free(prefixw);
			// append additional data
			if (datalen) {
				memcpy(newmsg+alloclen, msg+len2, datalen);
				alloclen += datalen;
			}
		}
		else {
			char *prefix = mir_utf8decodeA(options.prefix);
			int prefixlen = (int)strlen(prefix);
			if (strncmp(msg, prefix, prefixlen) == 0) {
				mir_free(prefix);
				return 0;
			}
			alloclen = msglen+prefixlen+sizeof(char);
			// get additional data
			DWORD len2 = alloclen-prefixlen;
			int datalen = dbei->cbBlob - len2;
			if (datalen < 0) datalen = 0;

			newmsg = (char*)mir_alloc(alloclen+datalen);
			memset(newmsg, 0, alloclen+datalen);
			strncpy(newmsg, prefix, prefixlen);
			strncat(newmsg, msg, msglen);
			mir_free(prefix);
			// append additional data
			if (datalen) {
				memcpy(newmsg+alloclen, msg+len2, datalen);
				alloclen += datalen;
			}
		}

	}
	
	my_dbei.pBlob = (BYTE *)newmsg;
	my_dbei.cbBlob = alloclen;
	my_dbei.flags |= DBEF_OTR_PREFIXED;

	db_event_add(hContact, &my_dbei);
	if (newmsg) mir_free(newmsg);
	
	// stop original event from being added
	return 1;
}

 int OnDatabaseEventAdded(WPARAM hContact, LPARAM lParam)
 {
	if (!options.delete_history) return 0;

	static char* prefixutf = mir_utf8encodeT(TranslateT(LANG_INLINE_PREFIX));
	static char* prefix = Translate(LANG_INLINE_PREFIX);
	static DWORD lenutf = (DWORD)strlen(prefixutf);
	static DWORD len = (DWORD)strlen(prefix);

	DBEVENTINFO info = { sizeof(info) };
	info.cbBlob = lenutf*2;
	info.pBlob = (PBYTE)mir_alloc(info.cbBlob);
	if (!db_event_get(lParam, &info)) {
		if(info.eventType == EVENTTYPE_MESSAGE) {
			MCONTACT hSub;
			if((hSub = db_mc_getMostOnline(hContact)) != 0)
				hContact = hSub;

			ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
			if (context && otr_context_get_trust(context) != TRUST_NOT_PRIVATE ) {
				// only delete encrypted messages that are no OTR system messages
				if ( options.delete_systeminfo || 
					((info.flags&DBEF_UTF && !(info.cbBlob >lenutf && 0==strncmp((char*)info.pBlob, prefixutf, lenutf)))
					|| (!(info.flags&DBEF_UTF) && !(info.cbBlob >len && 0==strncmp((char*)info.pBlob, prefix, len)))))
				{
					DeleteEventNode *node = new DeleteEventNode();
					node->hContact = hContact;
					node->hDbEvent = lParam;
					node->timestamp = time(0);
					node->next = 0;
					EnterCriticalSection(lpRemoveChainCS);
					if (DeleteEvents.last)
						DeleteEvents.last->next = node;
					else 
						DeleteEvents.first = node;
					DeleteEvents.last = node;
					LeaveCriticalSection(lpRemoveChainCS);
				}
			}
		}
	}
	mir_free(prefixutf);
	mir_free(info.pBlob);
	return 0;
}

void FinishSession(MCONTACT hContact) {
	if (!hContact) return;
	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	TrustLevel level = otr_context_get_trust(context);
	if (level == TRUST_UNVERIFIED || level == TRUST_PRIVATE) {
		otrl_context_force_finished(context);
		//SetEncryptionStatus(hContact, TRUST_FINISHED);
		otr_gui_gone_insecure(context->app_data, context);
		//otrl_message_disconnect(otr_user_state, &ops, hContact, context->accountname, context->protocol, context->username);
		//SetEncryptionStatus(hContact, TRUST_NOT_PRIVATE);
	}
	return;
}

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;

	if(mwd->uType == MSG_WINDOW_EVT_CLOSE && options.end_window_close) {
		FinishSession(mwd->hContact);
		return 0;
	}

	if(mwd->uType != MSG_WINDOW_EVT_OPEN) return 0;

	MCONTACT hContact = mwd->hContact, hTemp;
	if((hTemp = db_mc_getMostOnline(hContact)) != 0)
		hContact = hTemp;

	if (!CallService(MS_PROTO_ISPROTOONCONTACT, hContact, (LPARAM)MODULENAME))
		return 0;

	lib_cs_lock();
	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	lib_cs_unlock();

	SetEncryptionStatus(hContact, otr_context_get_trust(context));

	return 0;
}

// if it's a protocol going offline, attempt to send terminate session to all contacts of that protocol
// (this would be hooked as the ME_CLIST_STATUSMODECHANGE handler except that event is sent *after* the proto goes offline)
int StatusModeChange(WPARAM wParam, LPARAM lParam) {
	int status = (int)wParam;

	if(status != ID_STATUS_OFFLINE ) return 0;
	
	const char *proto = (char *)lParam;
	MCONTACT hContact;

	lib_cs_lock();
	
		ConnContext *context = otr_user_state->context_root;
		while(context) {
			if(context->msgstate == OTRL_MSGSTATE_ENCRYPTED && (proto == 0 || strcmp(proto, context->protocol) == 0)) {
				hContact = (MCONTACT)context->app_data;
				
				if(hContact) {
					otrl_message_disconnect(otr_user_state, &ops, (void*)hContact, context->accountname, context->protocol, context->username, OTRL_INSTAG_BEST);
					SetEncryptionStatus(hContact, TRUST_NOT_PRIVATE);
				}
				
			}
			context = context->next;
		}
	lib_cs_unlock();
	
	return 0;
}

int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (!options.end_offline)
		return 0;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (!lParam || strcmp(cws->szSetting, "Status") != 0) return 0;
	int status=0;
	switch (cws->value.type){
		case DBVT_WORD:
			status = cws->value.wVal;
			break;
		case DBVT_BYTE:
			status = cws->value.bVal;
			break;
		case DBVT_DWORD:
			status = cws->value.dVal;
			break;
	}
	if (status == ID_STATUS_OFFLINE) {
		if (!hContact) {
			// Protocol is going offline
			// Terminate sessions with all contacts of that proto
			StatusModeChange((WPARAM) ID_STATUS_OFFLINE, (LPARAM)cws->szModule);
			return 0;
		}else if(CallService(MS_PROTO_ISPROTOONCONTACT, hContact, (LPARAM)MODULENAME)) {
			// only care about contacts to which this filter is attached
			FinishSession(hContact);
		}
	}


	return 0;
}

void InitDBFilter() {
	InitializeCriticalSectionAndSpinCount(lpRemoveChainCS, 500);
	hDBEventPreAdd = HookEvent(ME_DB_EVENT_FILTER_ADD, OnDatabaseEventPreAdd);
	hDBEventAdded = HookEvent(ME_DB_EVENT_ADDED, OnDatabaseEventAdded);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	timerId = SetTimer(0, 0, 1000, DeleteTimerProc);
}
void DeinitDBFilter() {
	UnhookEvent(hDBEventPreAdd);
	hDBEventPreAdd = 0;
	UnhookEvent(hDBEventAdded);
	hDBEventAdded = 0;
	UnhookEvent(hContactSettingChanged);
	hContactSettingChanged=0;
	if (timerId) KillTimer(0, timerId);
	DeleteTimerProc(0,0,0,0);
	DeleteCriticalSection(lpRemoveChainCS);
}
