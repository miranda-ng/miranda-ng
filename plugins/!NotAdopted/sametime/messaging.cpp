#include "messaging.h"

#include <map>
#include <queue>

typedef std::queue<std::string> MessageQueue;
typedef std::map<HANDLE, MessageQueue> ContactMessageQueue;

ContactMessageQueue contact_message_queue;
CRITICAL_SECTION q_cs;

mwServiceIm *service_im = 0;

void mwIm_conversation_opened(mwConversation *conv) {
	mwIdBlock *idb = mwConversation_getTarget(conv);
	HANDLE hContact = FindContactByUserId(idb->user);

	if(!hContact) {
		mwSametimeList *user_list = mwSametimeList_new();
		mwSametimeGroup *stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, Translate("None"));
		mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, idb);

		AddContact(stuser, (options.add_contacts ? false : true));
		GetMoreDetails(idb->user);
	}
	
	ContactMessageQueue::iterator i;
	EnterCriticalSection(&q_cs);
	if((i = contact_message_queue.find(hContact)) != contact_message_queue.end()) {
		while(i->second.size()) {
			mwConversation_send(conv, mwImSend_PLAIN, (gconstpointer)i->second.front().c_str());
			i->second.pop();
		}
		contact_message_queue.erase(i);
	}
	LeaveCriticalSection(&q_cs);

	// gives linker error 'unresolved external symbol' :( So instead we will either add ciphers to the session or not (see session.cpp)
	//mwConversation_setEncrypted(conv, options.encrypt_session);
}

/** A conversation has been closed */
void mwIm_conversation_closed(mwConversation *conv, guint32 err) {
	if(err & ERR_FAILURE && err != CONNECTION_RESET) {
		char *msg = mwError(err);
		TCHAR *ts = u2t(msg);
		//MessageBox(0, ts, TranslateT("Sametime Error"), MB_OK | MB_ICONWARNING);
		ShowError(TranslateTS(ts));
		g_free(msg);
		free(ts);
	}

	mwIdBlock *idb = mwConversation_getTarget(conv);
	HANDLE hContact = FindContactByUserId(idb->user);
	if(hContact) {
		ContactMessageQueue::iterator i;
		EnterCriticalSection(&q_cs);
		if((i = contact_message_queue.find(hContact)) != contact_message_queue.end()) {
			contact_message_queue.erase(i);
		}
		LeaveCriticalSection(&q_cs);
	}
}

/** A message has been received on a conversation */
void mwIm_conversation_recv(mwConversation *conv, mwImSendType type, gconstpointer msg) {
	mwIdBlock *idb = mwConversation_getTarget(conv);
	HANDLE hContact = FindContactByUserId(idb->user);

	if(type == mwImSend_TYPING) {
		CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)(GPOINTER_TO_UINT(msg) == 0 ? 0 : 2));
		return;
	}

	if(type != mwImSend_PLAIN) return;

	wchar_t temp[MAX_MESSAGE_SIZE];
	char text[MAX_MESSAGE_SIZE];

	MultiByteToWideChar(CP_UTF8, 0, (const char *)msg, -1, temp, MAX_MESSAGE_SIZE);
	WideCharToMultiByte(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), WC_COMPOSITECHECK, temp, -1, text, MAX_MESSAGE_SIZE * sizeof(char), 0, 0);

	CCSDATA ccs = {0};
	PROTORECVEVENT pre = {0};

	ccs.hContact = hContact;
	ccs.szProtoService = PSR_MESSAGE;
	ccs.wParam = 0;
	ccs.lParam = (LPARAM)&pre;

	pre.timestamp = (DWORD)time(0);

	int tMsgBodyLen = strlen(text);
	int tRealBodyLen = wcslen(temp);

	int tMsgBufLen = (tMsgBodyLen+1) * sizeof(char) + (tRealBodyLen+1)*sizeof( wchar_t );
	char* tMsgBuf = ( char* )malloc( tMsgBufLen );

	char* p = tMsgBuf;

	strcpy( p, text );
	p += (tMsgBodyLen+1);

	if ( tRealBodyLen != 0 ) {
		wcscpy((wchar_t *)p, temp);
		pre.flags = PREF_UNICODE;
	}

	pre.szMessage = tMsgBuf;

	DBDeleteContactSetting(hContact, "CList", "Hidden");

	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);

	free(tMsgBuf);
}

mwImHandler mwIm_handler = {
	mwIm_conversation_opened,
	mwIm_conversation_closed,
	mwIm_conversation_recv
};

HANDLE SendMessageToUser(HANDLE hContact, char *msg) {
	mwIdBlock idb;
	mwAwareIdBlock id_block;

	wchar_t temp[MAX_MESSAGE_SIZE];
	char text[MAX_MESSAGE_SIZE];

	MultiByteToWideChar(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), MB_PRECOMPOSED, msg, -1, temp, MAX_MESSAGE_SIZE);
	WideCharToMultiByte(CP_UTF8, 0, temp, -1, text, MAX_MESSAGE_SIZE * sizeof(char), 0, 0);

	if(GetAwareIdFromContact(hContact, &id_block)) {
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation *conv = mwServiceIm_getConversation(service_im, &idb);
		if(conv) {
			if(!mwConversation_isOpen(conv)) {
				EnterCriticalSection(&q_cs);
				contact_message_queue[hContact].push(text);
				LeaveCriticalSection(&q_cs);
				mwConversation_open(conv);
			} else
				mwConversation_send(conv, mwImSend_PLAIN, (gconstpointer)msg);

			free(id_block.user);
			return (HANDLE)conv;
		}

		free(id_block.user);
	}

	return 0;
}

HANDLE SendMessageToUserW(HANDLE hContact, wchar_t *msg) {
	mwIdBlock idb;
	mwAwareIdBlock id_block;

	char text[MAX_MESSAGE_SIZE];

	WideCharToMultiByte(CP_UTF8, 0, msg, -1, text, MAX_MESSAGE_SIZE * sizeof(char), 0, 0);

	if(GetAwareIdFromContact(hContact, &id_block)) {
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation *conv = mwServiceIm_getConversation(service_im, &idb);
		if(conv) {
			if(!mwConversation_isOpen(conv)) {
				EnterCriticalSection(&q_cs);
				contact_message_queue[hContact].push(text);
				LeaveCriticalSection(&q_cs);

				mwConversation_open(conv);
			} else
				mwConversation_send(conv, mwImSend_PLAIN, (gconstpointer)text);

			free(id_block.user);
			return (HANDLE)conv;
		}

		free(id_block.user);
	}

	return 0;
}

void SendTyping(HANDLE hContact, bool typing) {
	mwIdBlock idb;
	mwAwareIdBlock id_block;

	if(GetAwareIdFromContact(hContact, &id_block)) {
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation *conv = mwServiceIm_getConversation(service_im, &idb);
		if(conv) {
			if(mwConversation_isOpen(conv))
				mwConversation_send(conv, mwImSend_TYPING, (gconstpointer)GUINT_TO_POINTER(typing ? 1 : 0));
		}

		free(id_block.user);
	}
}

void CloseIm(HANDLE hContact) {
	mwIdBlock idb;
	mwAwareIdBlock id_block;

	if(GetAwareIdFromContact(hContact, &id_block)) {
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation *conv = mwServiceIm_getConversation(service_im, &idb);
		if(conv) {
			if(mwConversation_isOpen(conv))
				mwConversation_close(conv, 0);
		}
		free(id_block.user);
	}
}

void InitMessaging(mwSession *session) {
	InitializeCriticalSection(&q_cs);

	mwSession_addService(session, (mwService *)(service_im = mwServiceIm_new(session, &mwIm_handler)));
	mwServiceIm_setClientType(service_im, mwImClient_PLAIN);
}

void DeinitMessaging(mwSession *session) {
	mwSession_removeService(session, mwService_IM);
	mwService_free((mwService *)service_im);
	service_im = 0;

	DeleteCriticalSection(&q_cs);
}

