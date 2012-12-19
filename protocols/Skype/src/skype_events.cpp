#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->InitChat();

	this->HookEvent(ME_OPT_INITIALISE, &CSkypeProto::OnOptionsInit);
	this->HookEvent(ME_USERINFO_INITIALISE, &CSkypeProto::OnUserInfoInit);
	
	this->login = ::DBGetString(NULL, this->m_szModuleName, "sid");
	this->rememberPassword = this->GetSettingByte("RememberPassword") > 0;

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	this->SetStatus(ID_STATUS_OFFLINE);

	return 0;
}

int CSkypeProto::OnContactDeleted(WPARAM wParam, LPARAM lParam)
{
	this->RevokeAuth(wParam, lParam);

	return 0;
}

void CSkypeProto::OnMessageSended(CConversation::Ref conversation, CMessage::Ref message)
{
	SEString data;	

	uint timestamp;
	message->GetPropTimestamp(timestamp);	

	message->GetPropAuthor(data);
	char *sid = ::mir_strdup((const char*)data);

	message->GetPropBodyXml(data);
	char *text = ::mir_strdup((const char*)data);

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		CParticipant::Refs participants;
		conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
		
		for (uint i = 0; i < participants.size(); i ++)
		{
			participants[i]->GetPropIdentity(data);
			char *contactSid = ::mir_strdup((const char *)data);
			//todo: get nickname
			this->RaiseMessageSendedEvent(
				timestamp,
				contactSid,
				contactSid,
				text);
		}
	}
	else
	{
		conversation->GetPropIdentity(data);
		char *chatID = ::mir_utf8encode((const char*)data);

		this->ChatEvent(chatID, sid, /*GC_EVENT_MESSAGE*/0x0040, text);
	}
}

void CSkypeProto::OnMessageReceived(CConversation::Ref conversation, CMessage::Ref message)
{
	SEString data;

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	message->GetPropAuthor(data);
	char *sid = ::mir_strdup((const char*)data);
		
	message->GetPropBodyXml(data);
	char *text = ::mir_utf8decodeA((const char*)data);

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		message->GetPropAuthorDisplayname(data);
		char *nick = ::mir_strdup((const char*)data);	

		this->RaiseMessageReceivedEvent(
			(DWORD)timestamp, 
			sid, 
			nick, 
			text);
	}
	else
	{
		conversation->GetPropIdentity(data);
		char *chatID = ::mir_strdup((const char*)data);

		this->ChatEvent(chatID, sid, /*GC_EVENT_MESSAGE*/ 0x0040, text);
	}

	/*const char *msg = (const char*)propValues[2];
	int len = ::strlen(msg) + 8;
	wchar_t *xml = new wchar_t[len];
	::mir_sntprintf(xml, len, L"<m>%s</m>", ::mir_utf8decodeW(msg));

	int bytesProcessed = 0;
	HXML hXml = xi.parseString(xml, &bytesProcessed, NULL);*/
}

void CSkypeProto::OnMessage(CConversation::Ref conversation, CMessage::Ref message)
{
	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	CMessage::SENDING_STATUS sendingStatus;
	message->GetPropSendingStatus(sendingStatus);

	if (messageType == CMessage::POSTED_TEXT)
	{
		if (sendingStatus == CMessage::SENT)
			this->OnMessageSended(conversation, message);
		else if (!sendingStatus)
			this->OnMessageReceived(conversation, message);
	}
}