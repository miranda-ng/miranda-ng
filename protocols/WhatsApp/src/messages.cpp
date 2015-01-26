#include "common.h"

int WhatsAppProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);

	return Proto_RecvMessage(hContact, pre);
}

void WhatsAppProto::onMessageForMe(FMessage* paramFMessage, bool paramBoolean)
{
	bool isChatRoom = !paramFMessage->remote_resource.empty();

	std::string* msg;
	switch (paramFMessage->media_wa_type) {
	case FMessage::WA_TYPE_IMAGE:
	case FMessage::WA_TYPE_AUDIO:
	case FMessage::WA_TYPE_VIDEO:
		msg = &paramFMessage->media_url;
		break;
	default:
		msg = &paramFMessage->data;
	}

	if (isChatRoom) {
		msg->insert(0, std::string("[").append(paramFMessage->notifyname).append("]: "));
	}

	MCONTACT hContact = this->AddToContactList(paramFMessage->key->remote_jid, 0, false,
		isChatRoom ? NULL : paramFMessage->notifyname.c_str(), isChatRoom);

	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.szMessage = const_cast<char*>(msg->c_str());
	recv.timestamp = paramFMessage->timestamp; //time(NULL);
	ProtoChainRecvMsg(hContact, &recv);

	m_pConnection->sendMessageReceived(paramFMessage);
}

int WhatsAppProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	ptrA jid(getStringA(hContact, "ID"));
	if (jid == NULL)
		return 0;

	if (m_pConnection != NULL) {
		debugLogA("No connection");
		return 0;
	}

	if (getByte(hContact, "SimpleChatRoom", 0) > 0 && getByte(hContact, "IsGroupMember", 0) == 0) {
		debugLogA("not a group member");
		return 0;
	}
	
	int msgId = this->m_pConnection->msg_id++;
	try {
		std::string id = "msg" + Utilities::intToStr(msgId);
		FMessage fmsg(new Key((const char*)jid, true, id));
		fmsg.data = msg;
		fmsg.timestamp = time(NULL);

		m_pConnection->sendMessage(&fmsg);
	}
	catch (exception &e) {
		debugLogA("exception: %s", e.what());
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)msgId, (LPARAM)e.what());
	}
	catch (...) {
		debugLogA("unknown exception");
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)msgId, (LPARAM)"Failed sending message");
	}

	return msgId;
}

void WhatsAppProto::onIsTyping(const std::string& paramString, bool paramBoolean)
{
	MCONTACT hContact = this->AddToContactList(paramString, 0, false);
	if (hContact != NULL) {
		CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)
			paramBoolean ? PROTOTYPE_CONTACTTYPING_INFINITE : PROTOTYPE_CONTACTTYPING_OFF);
	}
}

int WhatsAppProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && isOnline())
		ForkThread(&WhatsAppProto::SendTypingWorker, new send_typing(hContact, type));

	return 0;
}

void WhatsAppProto::SendTypingWorker(void* p)
{
	if (p == NULL)
		return;

	send_typing *typing = static_cast<send_typing*>(p);

	// Don't send typing notifications to contacts which are offline 
	if (getWord(typing->hContact, "Status", 0) == ID_STATUS_OFFLINE)
		return;

	ptrA jid(getStringA(typing->hContact, WHATSAPP_KEY_ID));
	if (jid && this->isOnline()) {
		if (typing->status == PROTOTYPE_SELFTYPING_ON)
			m_pConnection->sendComposing((char*)jid);
		else
			m_pConnection->sendPaused((char*)jid);
	}

	delete typing;
}

void WhatsAppProto::onMessageStatusUpdate(FMessage* fmsg)
{
	MCONTACT hContact = this->ContactIDToHContact(fmsg->key->remote_jid);
	if (hContact == 0)
		return;

	if (fmsg->status == FMessage::STATUS_RECEIVED_BY_SERVER) {
		int msgId = atoi(fmsg->key->id.substr(3).c_str());
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)msgId, 0);
	}
}
