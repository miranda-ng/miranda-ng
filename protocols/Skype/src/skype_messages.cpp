#include "skype.h"

int CSkypeProto::OnMessagePreCreate(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;
	if ( strcmp(GetContactProto(evt->hContact), this->m_szModuleName))
		return 0;

	MessageRef message(evt->seq);
	SEBinary guid;
	if (message->GetPropGuid(guid))
	{
		CMessage::TYPE messageType;
		message->GetPropType(messageType);

		if (messageType == CMessage::POSTED_TEXT)
		{
			evt->dbei->pBlob = (PBYTE)::mir_realloc(evt->dbei->pBlob, evt->dbei->cbBlob + guid.size());
			::memcpy((char *)&evt->dbei->pBlob[evt->dbei->cbBlob], guid.data(), guid.size());
			evt->dbei->cbBlob += (DWORD)guid.size();
		}
		else if (messageType == CMessage::POSTED_EMOTE)
		{
			evt->dbei->pBlob = (PBYTE)::mir_realloc(evt->dbei->pBlob, evt->dbei->cbBlob + guid.size() - 4);
			::memcpy((char *)&evt->dbei->pBlob[0], (char *)&evt->dbei->pBlob[4], evt->dbei->cbBlob - 4);
			::memcpy((char *)&evt->dbei->pBlob[evt->dbei->cbBlob - 4], guid.data(), guid.size());
			evt->dbei->cbBlob += (DWORD)guid.size() - 4;

			evt->dbei->eventType = SKYPE_DB_EVENT_TYPE_EMOTE;
		}
	}

	return 1;
}

void CSkypeProto::OnMessageReceived(const ConversationRef &conversation, const MessageRef &message)
{
	SEString data;

	Message::TYPE messageType;
	message->GetPropType(messageType);

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	Message::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);
		
	message->GetPropBodyXml(data);
	ptrA text( CSkypeProto::RemoveHtml(data));

	ContactRef author;
	message->GetPropAuthor(data);
	this->GetContact(data, author);

	MCONTACT hContact = this->AddContact(author, true);
	this->UserIsTyping(hContact, PROTOTYPE_SELFTYPING_OFF);

	SEBinary guid;
	message->GetPropGuid(guid);
	ReadMessageParam param = { guid, messageType };

	//if (status != CMessage::UNCONSUMED_NORMAL)
		if (this->IsMessageInDB(hContact, timestamp, guid))
			return;

	DWORD flags = PREF_UTF;
	if (status != CMessage::UNCONSUMED_NORMAL) 
		flags |= PREF_CREATEREAD;

	PROTORECVEVENT recv = { 0 };
	recv.flags = flags;	
	recv.lParam = (LPARAM)&param;
	recv.timestamp = timestamp;
	recv.szMessage = ::mir_strdup(text);
	::ProtoChainRecvMsg(hContact, &recv);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSkypeProto::OnMessageSent(const ConversationRef &conversation, const MessageRef &message)
{
	SEString data;

	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	CMessage::SENDING_STATUS sstatus;
	message->GetPropSendingStatus(sstatus);

	CMessage::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);

	message->GetPropBodyXml(data);
	ptrA text( CSkypeProto::RemoveHtml(data));

	CParticipant::Refs participants;
	conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
	participants[0]->GetPropIdentity(data);
		
	ptrW sid(::mir_utf8decodeW(data));

	MCONTACT hContact = this->GetContactBySid(sid);
	this->SendBroadcast(
		hContact,
		ACKTYPE_MESSAGE,
		sstatus == CMessage::FAILED_TO_SEND ? ACKRESULT_FAILED : ACKRESULT_SUCCESS,
		(HANDLE)message->getOID(), 0);
			
	SEBinary guid;
	message->GetPropGuid(guid);

	this->RaiseMessageSentEvent(
		hContact,
		timestamp,
		guid,
		text,
		status == CMessage::UNCONSUMED_NORMAL);
}

void CSkypeProto::OnMessageEvent(const ConversationRef &conversation, const MessageRef &message)
{
	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	switch (messageType)
	{
	case CMessage::POSTED_EMOTE:
	case CMessage::POSTED_TEXT:
		{
			SEString author;
			message->GetPropAuthor(author);
			
			if (::wcsicmp(ptrW(::mir_utf8decodeW(author)), this->login) == 0)
				this->OnMessageSent(conversation, message);
			else
				this->OnMessageReceived(conversation, message);
		}
		break;

	case CMessage::STARTED_LIVESESSION:
		{
			Message::CONSUMPTION_STATUS status;
			message->GetPropConsumptionStatus(status);
			if (status != Message::UNCONSUMED_NORMAL)
				break;

			uint timestamp;
			message->GetPropTimestamp(timestamp);

			SEString identity;
			message->GetPropAuthor(identity);

			CContact::Ref author;
			this->GetContact(identity, author);

			MCONTACT hContact = this->AddContact(author);
		
			char *message = ::mir_utf8encode(::Translate("Incoming call started"));
		
			this->AddDBEvent(
				hContact,
				SKYPE_DB_EVENT_TYPE_CALL,
				timestamp,
				DBEF_UTF,
				(DWORD)::strlen(message) + 1,
				(PBYTE)message);
			//temp popup
			/*TCHAR popuptext[MAX_PATH];
			mir_sntprintf(popuptext, SIZEOF(popuptext), TranslateT("Incoming call from %s. Use offical skype for calling."), ptrW(::mir_utf8decodeW(identity)));
			this->ShowNotification(popuptext);*/
		}
		break;

	case CMessage::ENDED_LIVESESSION:
		{
			Message::CONSUMPTION_STATUS status;
			message->GetPropConsumptionStatus(status);
			if (status != Message::UNCONSUMED_NORMAL)
				break;

			uint timestamp;
			message->GetPropTimestamp(timestamp);

			SEString identity;
			message->GetPropAuthor(identity);

			CContact::Ref author;
			this->GetContact(identity, author);

			MCONTACT hContact = this->AddContact(author);
		
			char *message = ::mir_utf8encode(::Translate("Incoming call finished"));
		
			this->AddDBEvent(
				hContact,
				SKYPE_DB_EVENT_TYPE_CALL,
				timestamp,
				DBEF_UTF,
				(DWORD)::strlen(message) + 1,
				(PBYTE)message);
		}
		break;
	}
}

void CSkypeProto::SyncMessageHystory(const ConversationRef &conversation, const time_t timestamp)
{
	if (conversation)
	{
		conversation->SetConsumedHorizon(timestamp);
		MessageRefs oldMessages, newMessages;
		conversation->GetLastMessages(oldMessages, newMessages, timestamp);
		for (size_t i = 0; i < oldMessages.size(); i++)
			this->OnMessageEvent(conversation, oldMessages[i]);
		for (size_t i = 0; i < newMessages.size(); i++)
			this->OnMessageEvent(conversation, newMessages[i]);
		conversation->SetConsumedHorizon(time(NULL));
	}
}

void CSkypeProto::SyncHistoryCommand(MCONTACT hContact, time_t timestamp)
{
	if (hContact)
	{
		ptrW sid( ::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));

		ConversationRef conversation;
		if ( !this->isChatRoom(hContact))
		{
			SEStringList target;
			target.append((char *)ptrA(::mir_utf8encodeW(sid)));
			this->GetConversationByParticipants(target, conversation);
		}
		else
			this->GetConversationByIdentity((char *)ptrA(::mir_utf8encodeW(sid)), conversation);

		if (conversation)
		{
			this->SyncMessageHystory(conversation, timestamp);
			CSkypeProto::ShowNotification(TranslateT("history synchronization"), TranslateT("Done!"), MB_ICONINFORMATION, hContact);
		}
	}
}

int CSkypeProto::SyncLastDayHistoryCommand(WPARAM wParam, LPARAM lParam)
{
	time_t timestamp = time(NULL);
	timestamp -= 60*60*24;
	this->SyncHistoryCommand((MCONTACT)wParam, timestamp);
	return 0;
}

int CSkypeProto::SyncLastWeekHistoryCommand(WPARAM wParam, LPARAM lParam)
{
	time_t timestamp = time(NULL);
	timestamp -= 60*60*24*7;
	this->SyncHistoryCommand((MCONTACT)wParam, timestamp);
	return 0;
}

int CSkypeProto::SyncLastMonthHistoryCommand(WPARAM wParam, LPARAM lParam)
{
	time_t timestamp = time(NULL);
	timestamp -= 60*60*24*30;
	this->SyncHistoryCommand((MCONTACT)wParam, timestamp);
	return 0;
}

int CSkypeProto::SyncLast3MonthHistoryCommand(WPARAM wParam, LPARAM lParam)
{
	time_t timestamp = time(NULL);
	timestamp -= 60*60*24*90;
	this->SyncHistoryCommand((MCONTACT)wParam, timestamp);
	return 0;
}

int CSkypeProto::SyncLastYearHistoryCommand(WPARAM wParam, LPARAM lParam)
{
	time_t timestamp = time(NULL);
	timestamp -= 60*60*24*365;
	this->SyncHistoryCommand((MCONTACT)wParam, timestamp);
	return 0;
}

int CSkypeProto::SyncAllTimeHistoryCommand(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact)
	{
		ptrW sid( ::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));

		ConversationRef conversation;
		if ( !this->isChatRoom(hContact))
		{
			SEStringList target;
			target.append((char *)ptrA(::mir_utf8encodeW(sid)));
			this->GetConversationByParticipants(target, conversation);
		}
		else
			this->GetConversationByIdentity((char *)ptrA(::mir_utf8encodeW(sid)), conversation);

		if (conversation)
		{
			uint timestamp;
			conversation->GetPropCreationTimestamp(timestamp);
			this->SyncMessageHystory(conversation, timestamp);
			CSkypeProto::ShowNotification(TranslateT("history synchronization"), TranslateT("Done!"), MB_ICONINFORMATION, hContact);
		}
	}
	return 0;
}
