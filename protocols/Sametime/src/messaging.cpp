#include "StdAfx.h"
#include "sametime.h"

CSametimeProto* getProtoFromMwConversation(mwConversation* conv)
{
	mwServiceIm* serviceIM = mwConversation_getService(conv);
	mwService* service = mwServiceIm_getService(serviceIM);
	mwSession* session = mwService_getSession(service);
	return (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
}

void mwIm_conversation_opened(mwConversation* conv)
{
	CSametimeProto* proto = getProtoFromMwConversation(conv);
	proto->debugLog(_T("mwIm_conversation_opened() start"));

	mwIdBlock* idb = mwConversation_getTarget(conv);
	MCONTACT hContact = proto->FindContactByUserId(idb->user);

	if (!hContact) {
		proto->debugLog(_T("mwIm_conversation_opened() !hContact"));
		mwSametimeList* user_list = mwSametimeList_new();
		mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, Translate("None"));
		mwSametimeUser* stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, idb);

		proto->AddContact(stuser, (proto->options.add_contacts ? false : true));
		proto->GetMoreDetails(idb->user);
	}

	ContactMessageQueue::iterator i;
	mir_cslock lck(proto->q_cs);
	if ((i = proto->contact_message_queue.find(hContact)) != proto->contact_message_queue.end()) {
		while (i->second.size()) {
			mwConversation_send(conv, mwImSend_PLAIN, (gconstpointer)i->second.front().c_str());
			i->second.pop();
		}
		proto->contact_message_queue.erase(i);
	}

	// gives linker error 'unresolved external symbol' :( So instead we will either add ciphers to the session or not (see session.cpp)
	//mwConversation_setEncrypted(conv, options.encrypt_session);
}

/** A conversation has been closed */
void mwIm_conversation_closed(mwConversation* conv, guint32 err)
{
	CSametimeProto* proto = getProtoFromMwConversation(conv);
	proto->debugLog(_T("mwIm_conversation_closed() start err=[%d]"), err);

	if (err & ERR_FAILURE && err != CONNECTION_RESET) {
		proto->showPopup(err);
		if (err == ERR_NO_COMMON_ENCRYPT && !(proto->options.encrypt_session))
			proto->showPopup(TranslateT("No common encryption method. Try to enable encryption in protocol options."), SAMETIME_POPUP_INFO);
	}

	mwIdBlock* idb = mwConversation_getTarget(conv);
	MCONTACT hContact = proto->FindContactByUserId(idb->user);
	if (hContact) {
		mir_cslock lck(proto->q_cs);
		ContactMessageQueue::iterator i = proto->contact_message_queue.find(hContact);
		if (i != proto->contact_message_queue.end())
			proto->contact_message_queue.erase(i);
	}
}

/** A message has been received on a conversation */
void mwIm_conversation_recv(mwConversation* conv, mwImSendType type, gconstpointer msg)
{
	CSametimeProto* proto = getProtoFromMwConversation(conv);
	proto->debugLog(_T("mwIm_conversation_recv() start"));

	mwIdBlock* idb = mwConversation_getTarget(conv);
	MCONTACT hContact = proto->FindContactByUserId(idb->user);
	proto->debugLog(_T("mwIm_conversation_recv() type=[%d] hContact=[%x]"), type, hContact);

	if (type == mwImSend_TYPING) {
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (GPOINTER_TO_UINT(msg) == 0 ? 0 : 2));
		return;
	}

	if (type != mwImSend_PLAIN)
		return;

	PROTORECVEVENT pre = { 0 };
	time_t t = time(NULL);
	pre.timestamp = t;
	pre.szMessage = (char*)msg;
	ProtoChainRecvMsg(hContact, &pre);
}

void mwIm_place_invite(struct mwConversation* conv, const char* message, const char* title, const char* name)
{
	CSametimeProto* proto = getProtoFromMwConversation(conv);
	proto->debugLog(_T("mwIm_place_invite() start"));

	///TODO unimplemented

	TCHAR* tszMessage = mir_utf8decodeT(message);

	TCHAR msg[512];
	mir_sntprintf(msg, SIZEOF(msg), TranslateT("SERVICE NOT IMPLEMENTED. %s"), tszMessage);
	proto->showPopup(msg, SAMETIME_POPUP_INFO);

	mir_free(tszMessage);
}

mwImHandler mwIm_handler = {
	mwIm_conversation_opened,
	mwIm_conversation_closed,
	mwIm_conversation_recv,
	mwIm_place_invite,
	NULL
};

HANDLE CSametimeProto::SendMessageToUser(MCONTACT hContact, const char *szMsg)
{
	debugLog(_T("CSametimeProto::SendMessageToUser()  hContact=[%x]"), hContact);

	mwAwareIdBlock id_block;
	if (GetAwareIdFromContact(hContact, &id_block)) {
		mwIdBlock idb;
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation* conv = mwServiceIm_getConversation(service_im, &idb);
		if (conv) {
			if (!mwConversation_isOpen(conv)) {
				debugLog(_T("CSametimeProto::SendMessageToUser()  mwConversation_isOpen"));
				mir_cslock lck(q_cs);
				contact_message_queue[hContact].push(szMsg);
				mwConversation_open(conv);
			}
			else {
				debugLog(_T("CSametimeProto::SendMessageToUser()  !mwConversation_isOpen"));
				mwConversation_send(conv, mwImSend_PLAIN, szMsg);
			}

			free(id_block.user);
			return (HANDLE)conv;
		}

		free(id_block.user);
	}

	return 0;
}

void CSametimeProto::SendTyping(MCONTACT hContact, bool typing)
{
	debugLog(_T("CSametimeProto::SendTyping() hContact=[%x] type=[%d]"), hContact, typing);

	mwAwareIdBlock id_block;
	if (GetAwareIdFromContact(hContact, &id_block)) {
		mwIdBlock idb;
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation* conv = mwServiceIm_getConversation(service_im, &idb);
		if (conv) {
			if (mwConversation_isOpen(conv)) {
				debugLog(_T("CSametimeProto::SendTyping() send"));
				mwConversation_send(conv, mwImSend_TYPING, (gconstpointer)GUINT_TO_POINTER(typing ? 1 : 0));
			}
		}

		free(id_block.user);
	}
}

void CSametimeProto::CloseIm(MCONTACT hContact)
{
	debugLog(_T("CSametimeProto::CloseIm() hContact=[%x]"), hContact);

	mwAwareIdBlock id_block;
	if (GetAwareIdFromContact(hContact, &id_block)) {
		mwIdBlock idb;
		idb.user = id_block.user;
		idb.community = id_block.community;

		mwConversation* conv = mwServiceIm_getConversation(service_im, &idb);
		if (conv) {
			if (mwConversation_isOpen(conv)) {
				debugLog(_T("CSametimeProto::CloseIm() mwConversation_close"));
				mwConversation_close(conv, 0);
			}
		}
		free(id_block.user);
	}
}

void CSametimeProto::InitMessaging()
{
	debugLog(_T("CSametimeProto::InitMessaging()"));
	mwSession_addService(session, (mwService*)(service_im = mwServiceIm_new(session, &mwIm_handler)));
	mwServiceIm_setClientType(service_im, mwImClient_PLAIN);
}

void CSametimeProto::DeinitMessaging()
{
	debugLog(_T("CSametimeProto::DeinitMessaging()"));
	mwSession_removeService(session, mwService_IM);
	mwService_free((mwService*)service_im);
	service_im = 0;
}
