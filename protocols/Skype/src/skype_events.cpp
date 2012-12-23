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
	HANDLE hContact = (HANDLE)wParam;
	if (hContact && this->IsOnline())
	{
		if (this->IsChatRoom(hContact))
		{
			char *chatID = ::DBGetString(hContact, this->m_szModuleName, "ChatRoomID");
			this->LeaveChat(chatID);

			CConversation::Ref conversation;
			g_skype->GetConversationByIdentity(chatID, conversation);
			conversation->RetireFrom();
			conversation->Delete();
		}
		else
			this->RevokeAuth(wParam, lParam);
	}

	return 0;
}

void CSkypeProto::OnMessageSended(CConversation::Ref conversation, CMessage::Ref message)
{
	SEString data;

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	message->GetPropAuthor(data);
	char *sid = ::mir_strdup(data);

	message->GetPropBodyXml(data);
	char *text = ::mir_strdup(data);

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		CParticipant::Refs participants;
		conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
		
		for (uint i = 0; i < participants.size(); i ++)
		{
			participants[i]->GetPropIdentity(data);
			char *contactSid = ::mir_strdup(data);
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
		char *cid = ::mir_strdup(data);

		this->SendChatMessage(cid, sid, ::mir_utf8decodeA(text));

		::mir_free(cid);
	}
}

void CSkypeProto::OnMessageReceived(CConversation::Ref conversation, CMessage::Ref message)
{
	SEString data;

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	message->GetPropAuthor(data);
	char *sid = ::mir_strdup(data);
		
	message->GetPropBodyXml(data);
	char *text = ::mir_strdup(data);

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		message->GetPropAuthorDisplayname(data);
		char *nick = ::mir_utf8decodeA(data);

		this->RaiseMessageReceivedEvent(
			(DWORD)timestamp, 
			sid, 
			nick, 
			CSkypeProto::RemoveHtml(text));
	}
	else
	{
		conversation->GetPropIdentity(data);
		char *cid = ::mir_strdup(data);

		this->SendChatMessage(cid, sid, ::mir_utf8decodeA(text));

		::mir_free(cid);
	}

	/*int len = ::strlen(text) + 8;
	wchar_t *xml = new wchar_t[len];
	::mir_sntprintf(xml, len, L"<m>%s</m>", ::mir_utf8decodeW(text));

	int bytesProcessed = 0;
	HXML hXml = xi.parseString(xml, &bytesProcessed, NULL);*/
}

void CSkypeProto::OnMessage(CConversation::Ref conversation, CMessage::Ref message)
{
	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	CMessage::SENDING_STATUS sendingStatus;
	message->GetPropSendingStatus(sendingStatus);

	CMessage::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);

	// it's old message (hystory sync)
	if (status == CMessage::CONSUMED) return;

	switch (messageType)
	{
	case CMessage::POSTED_EMOTE:
	case CMessage::POSTED_TEXT:
		{
			SEString data;

			message->GetPropAuthor(data);
			char *sid = ::mir_strdup(data);

			if (::stricmp(sid, this->login) == 0)
				this->OnMessageSended(conversation, message);
			else
				this->OnMessageReceived(conversation, message);
		}
		break;

	case CMessage::ADDED_CONSUMERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			char *cid = ::mir_strdup(data);

			HANDLE hContact = this->GetChatRoomByID(cid);
			if ( !hContact || ::DBGetContactSettingWord(hContact, this->m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			{
				SEStringList empty;
				this->StartChat(cid, empty);
				
				CParticipant::Refs participants;
				conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
				for (uint i = 0; i < participants.size(); i++)
				{
					participants[i]->GetPropIdentity(data);
					this->AddChatContact(cid, data);
				}
			}

			{
				message->GetPropIdentities(data);

				StringList alreadyInChat(this->GetChatUsers(cid), " ");
				StringList needToAdd(data, " ");
				
				for (int i = 0; i < needToAdd.getCount(); i++)
				{
					char *sid = needToAdd[i];
					if (::stricmp(sid, this->login) != 0 && !alreadyInChat.contains(sid))
						this->AddChatContact(cid, sid);
				}
			}
		}
		break;

	case CMessage::RETIRED:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			char *cid = ::mir_strdup(data);

			StringList alreadyInChat(this->GetChatUsers(cid), " ");
			
			message->GetPropAuthor(data);	
			char *sid = ::mir_strdup(data);
			if (::stricmp(sid, this->login) != 0)
				if (alreadyInChat.contains(sid))
					this->RemoveChatContact(cid, sid);
		}
		break;
	case CMessage::RETIRED_OTHERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			char *cid = ::mir_strdup(data);

			message->GetPropIdentities(data);

			StringList alreadyInChat(this->GetChatUsers(cid), " ");
			StringList needToKick(data, " ");
				
			for (int i = 0; i < needToKick.getCount(); i++)
			{
				char *sid = needToKick[i];
				if (::stricmp(sid, this->login) != 0 && !alreadyInChat.contains(sid))
					this->KickChatContact(cid, sid);
			}
		}
		break;

	case CMessage::SPAWNED_CONFERENCE:
		{
			SEString data;
			conversation->GetPropIdentity(data);
			char *cid = ::mir_strdup(data);

			/*HANDLE hContact = this->GetChatRoomByID(cid);
			if ( !hContact || ::DBGetContactSettingWord(hContact, this->m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			{
				this->JoinChat(cid);
			}*/
		}
		break;

	//case CMessage::REQUESTED_AUTH:
	//	break;

	//case CMessage::GRANTED_AUTH:
	//	break;

	//case CMessage::BLOCKED:
	//	break;
	}
}
