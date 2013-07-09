#include "common.h"

int WhatsAppProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);

	return Proto_RecvMessage(hContact, pre);
}

void WhatsAppProto::onMessageForMe(FMessage* paramFMessage, bool paramBoolean)
{
	bool isChatRoom = !paramFMessage->remote_resource.empty();

	std::string* msg;
	switch (paramFMessage->media_wa_type)
	{
	case FMessage::WA_TYPE_IMAGE:
	case FMessage::WA_TYPE_AUDIO:
	case FMessage::WA_TYPE_VIDEO:
		msg = &paramFMessage->media_url;
		break;
	default:
		msg = &paramFMessage->data;
	}

	if (isChatRoom)
	{
		msg->insert(0, std::string("[").append(paramFMessage->notifyname).append("]: "));
	}

	HANDLE hContact = this->AddToContactList(paramFMessage->key->remote_jid, 0, false, 
		isChatRoom ? NULL : paramFMessage->notifyname.c_str(), isChatRoom);

	PROTORECVEVENT recv = {0};
	recv.flags = PREF_UTF;
	recv.szMessage = const_cast<char*>(msg->c_str());
	recv.timestamp = paramFMessage->timestamp; //time(NULL);
	ProtoChainRecvMsg(hContact, &recv);

	this->connection->sendMessageReceived(paramFMessage);
}

int WhatsAppProto::SendMsg(HANDLE hContact, int flags, const char *msg)
{
	LOG("");
	int msgId = ++(this->msgId);

	ForkThread( &WhatsAppProto::SendMsgWorker, new send_direct(hContact, msg, (HANDLE) msgId, flags & IS_CHAT));
	return this->msgIdHeader + msgId;
}

void WhatsAppProto::SendMsgWorker(void* p)
{
	LOG("");
	if (p == NULL)
		return;

	DBVARIANT dbv;
	send_direct *data = static_cast<send_direct*>(p);

	if (db_get_b(data->hContact, m_szModuleName, "SimpleChatRoom", 0) > 0 &&
		db_get_b(data->hContact, m_szModuleName, "IsGroupMember", 0) == 0)
	{
		LOG("not a group member");
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED,
			(HANDLE) (this->msgIdHeader + this->msgId), (LPARAM) "You cannot send messages to groups if you are not a member.");
	}
	else if (!db_get_s(data->hContact, m_szModuleName,"ID", &dbv, DBVT_ASCIIZ) &&
		this->connection != NULL)
	{
		try
		{
			db_set_dw(data->hContact, m_szModuleName, WHATSAPP_KEY_LAST_MSG_STATE, 2);
			db_set_dw(data->hContact, m_szModuleName, WHATSAPP_KEY_LAST_MSG_ID_HEADER, this->msgIdHeader);
			db_set_dw(data->hContact, m_szModuleName, WHATSAPP_KEY_LAST_MSG_ID, this->msgId);

			std::stringstream ss;
			ss << this->msgIdHeader << "-" << this->msgId;
			Key* key = new Key(Key(dbv.pszVal, true, ss.str())); // deleted by FMessage
			FMessage fmsg(key);
			fmsg.data = data->msg;
			fmsg.timestamp = time(NULL);

			db_free(&dbv);

			this->connection->sendMessage(&fmsg);
		}
		catch (exception &e)
		{
			LOG("exception: %s", e.what());
			ProtoBroadcastAck(data->hContact,ACKTYPE_MESSAGE,ACKRESULT_FAILED,
				(HANDLE) (this->msgIdHeader + this->msgId), (LPARAM) e.what());
		}
		catch (...)
		{
			LOG("unknown exception");
			ProtoBroadcastAck(data->hContact,ACKTYPE_MESSAGE,ACKRESULT_FAILED,
			  (HANDLE) (this->msgIdHeader + this->msgId), (LPARAM) "Failed sending message");
		}
	}
	else
	{
		LOG("No connection");
		ProtoBroadcastAck(data->hContact,ACKTYPE_MESSAGE,ACKRESULT_FAILED,
			(HANDLE) (this->msgIdHeader + this->msgId), (LPARAM) "You cannot send messages when you are offline.");
	}

	delete data;
}

void WhatsAppProto::RecvMsgWorker(void *p)
{
	if (p == NULL)
		return;

	//WAConnection.cpp l1225 - message will be deleted. We cannot send the ack inside a thread!
	//FMessage *fmsg = static_cast<FMessage*>(p);
	//this->connection->sendMessageReceived(fmsg);

	//delete fmsg;
}

void WhatsAppProto::onIsTyping(const std::string& paramString, bool paramBoolean)
{
	HANDLE hContact = this->AddToContactList(paramString, 0, false);
	if (hContact != NULL)
	{
		CallService(MS_PROTO_CONTACTISTYPING, (WPARAM) hContact, (LPARAM)
			paramBoolean ? PROTOTYPE_CONTACTTYPING_INFINITE : PROTOTYPE_CONTACTTYPING_OFF);
	}
}


int WhatsAppProto::UserIsTyping(HANDLE hContact,int type)
{ 
	if (hContact && isOnline())
		ForkThread(&WhatsAppProto::SendTypingWorker, new send_typing(hContact, type));

	return 0;
}

void WhatsAppProto::SendTypingWorker(void* p)
{
	if(p == NULL)
		return;

	send_typing *typing = static_cast<send_typing*>(p);

	// Don't send typing notifications to contacts which are offline 
	if ( db_get_w(typing->hContact,m_szModuleName,"Status", 0) == ID_STATUS_OFFLINE)
		return;

	DBVARIANT dbv;
	if ( !db_get_s(typing->hContact,m_szModuleName,WHATSAPP_KEY_ID,&dbv, DBVT_ASCIIZ) &&
		this->isOnline())
	{
		if (typing->status == PROTOTYPE_SELFTYPING_ON) {
			this->connection->sendComposing(dbv.pszVal);
		} else {
			this->connection->sendPaused(dbv.pszVal);
		}
	}		

	delete typing;
}

void WhatsAppProto::onMessageStatusUpdate(FMessage* fmsg)
{
	LOG("");

	HANDLE hContact = this->ContactIDToHContact(fmsg->key->remote_jid);
	if (hContact == 0)
		return;

	int state = 5 - fmsg->status;
	if (state != 0 && state != 1)
		return;

	int header;
	int id;
	size_t delimPos = fmsg->key->id.find("-");
	
	std::stringstream ss;
	ss << fmsg->key->id.substr(0, delimPos);
	ss >> header;
	
	ss.clear();
	ss << fmsg->key->id.substr(delimPos + 1);
	ss >> id;

	if (state == 1)
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) (header + id),0);

	if (db_get_dw(hContact, m_szModuleName, WHATSAPP_KEY_LAST_MSG_ID_HEADER, 0) == header &&
		db_get_dw(hContact, m_szModuleName, WHATSAPP_KEY_LAST_MSG_ID, -1) == id)
	{
		db_set_dw(hContact, m_szModuleName, WHATSAPP_KEY_LAST_MSG_STATE, state);
		this->UpdateStatusMsg(hContact);
	}
}