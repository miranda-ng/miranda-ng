#include "common.h"

int WhatsAppProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);

	return Proto_RecvMessage(hContact, pre);
}

void WhatsAppProto::onMessageForMe(const FMessage &pMsg)
{
	// someone sent us a contact. launch contact addition dialog
	if (pMsg.media_wa_type == FMessage::WA_TYPE_CONTACT) {
		MCONTACT hContact = AddToContactList(pMsg.media_url, pMsg.media_name.c_str());

		ADDCONTACTSTRUCT acs = { 0 };
		acs.handleType = HANDLE_CONTACT;
		acs.hContact = hContact;
		acs.szProto = m_szModuleName;
		CallServiceSync(MS_ADDCONTACT_SHOW, 0, (LPARAM)&acs);
	}
	else {
		std::string msg(pMsg.data);
		if (!pMsg.media_url.empty()) {
			if (!msg.empty())
				msg.append("\n");
			msg += pMsg.media_url;
		}

		MCONTACT hContact = this->AddToContactList(pMsg.key.remote_jid, pMsg.notifyname.c_str());

		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.szMessage = const_cast<char*>(msg.c_str());
		recv.timestamp = pMsg.timestamp;
		ProtoChainRecvMsg(hContact, &recv);
	}

	if (isOnline())
		m_pConnection->sendMessageReceived(pMsg);
}

int WhatsAppProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	ptrA jid(getStringA(hContact, "ID"));
	if (jid == NULL)
		return 0;

	if (m_pConnection == NULL) {
		debugLogA("No connection");
		return 0;
	}

	int msgId = GetSerial();
	try {
		time_t now = time(NULL);
		std::string id = Utilities::intToStr(now) + "-" + Utilities::intToStr(msgId);
		FMessage fmsg(std::string(jid), true, id);
		fmsg.timestamp = now;
		fmsg.data = msg;

		m_pConnection->sendMessage(&fmsg);
		utils::setStatusMessage(hContact, NULL);
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

void WhatsAppProto::onIsTyping(const std::string &paramString, bool paramBoolean)
{
	MCONTACT hContact = this->AddToContactList(paramString);
	if (hContact != NULL) {
		CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)
			paramBoolean ? PROTOTYPE_CONTACTTYPING_INFINITE : PROTOTYPE_CONTACTTYPING_OFF);
	}
}

int WhatsAppProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && isOnline()) {
		ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
		if (jid && isOnline()) {
			if (type == PROTOTYPE_SELFTYPING_ON)
				m_pConnection->sendComposing((char*)jid);
			else
				m_pConnection->sendPaused((char*)jid);
		}
	}

	return 0;
}

void WhatsAppProto::onMessageStatusUpdate(const FMessage &fmsg)
{
	MCONTACT hContact = this->ContactIDToHContact(fmsg.key.remote_jid);
	if (hContact == 0)
		return;

	if (isChatRoom(hContact)) {
		onGroupMessageReceived(fmsg);
		return;
	}

	const TCHAR *ptszBy;
	switch (fmsg.status) {
	case FMessage::STATUS_RECEIVED_BY_SERVER: ptszBy = TranslateT("server"); break;
	case FMessage::STATUS_RECEIVED_BY_TARGET: ptszBy = pcli->pfnGetContactDisplayName(hContact, 0);  break;
	default:
		return;
	}

	size_t delim = fmsg.key.id.find('-');
	if (delim == string::npos)
		return;

	int msgId = atoi(fmsg.key.id.substr(delim+1).c_str());
	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)msgId, 0);

	time_t timestamp = atol(fmsg.key.id.substr(0, delim).c_str());

	TCHAR ttime[64];
	_tcsftime(ttime, SIZEOF(ttime), _T("%X"), localtime(&timestamp));
	utils::setStatusMessage(hContact, CMString(FORMAT, TranslateT("Message received: %s by %s"), ttime, ptszBy));
}
