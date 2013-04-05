#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->InitSkype();

	//this->InitChat();
	this->InitNetLib();
	this->InitCustomFolders();

	this->HookEvent(ME_OPT_INITIALISE,		&CSkypeProto::OnOptionsInit);
	this->HookEvent(ME_USERINFO_INITIALISE, &CSkypeProto::OnUserInfoInit);
	
	//this->login = ::DBGetString(NULL, this->m_szModuleName, "sid");
	//this->rememberPassword = this->GetSettingByte("RememberPassword") > 0;

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	this->SetStatus(ID_STATUS_OFFLINE);

	this->UninitNetLib();

	this->UninitSkype();

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
			this->skype->GetConversationByIdentity(chatID, conversation);
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
		
		//for (uint i = 0; i < participants.size(); i ++)
		{
			participants[0]->GetPropIdentity(data);
			char *contactSid = ::mir_strdup(data);
			//todo: get nickname
			this->RaiseMessageSendedEvent(
				timestamp,
				contactSid,
				contactSid,
				CSkypeProto::RemoveHtml(text));
		}
	}
	else
	{
		conversation->GetPropIdentity(data);
		char *cid = ::mir_strdup(data);

		//this->SendChatMessage(cid, sid, ::mir_utf8decodeA(text));

		char *nick = (char *)::DBGetString(NULL, this->m_szModuleName, "Nick");
		if (::stricmp(nick, "") == 0)
		{
			nick = sid;
		}

		this->SendChatMessage(
			cid, 
			nick, 
			CSkypeProto::RemoveHtml(::mir_utf8decodeA(text)));

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

		this->SendChatMessage(cid, sid, CSkypeProto::RemoveHtml(::mir_utf8decodeA(text)));

		::mir_free(cid);
	}

	/*int len = ::strlen(text) + 8;
	wchar_t *xml = new wchar_t[len];
	::mir_sntprintf(xml, len, L"<m>%s</m>", ::mir_utf8decodeW(text));

	int bytesProcessed = 0;
	HXML hXml = xi.parseString(xml, &bytesProcessed, NULL);*/
}

void CSkypeProto::OnTransferChanged(int prop, CTransfer::Ref transfer)
{
	if (prop == Transfer::P_STATUS)
	{
		Transfer::STATUS status;
		transfer->GetPropStatus(status);

		/*CConversation::Ref conversation;
		transfer->GetPropConvoId(conversation);

		SEBinary guid;
		transfer->GetPropChatmsgGuid(guid);
		auto ft = this->FindTransfer(guid);*/
		auto ft = this->FindFileTransfer(transfer);

		SEString sid;
		transfer->GetPropPartnerHandle(sid);
		//CParticipant::Refs participants;
		//conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
		////for (uint i = 0; i < participants.size(); i++)
		//participants[0]->GetPropIdentity(sid);

		HANDLE hContact = this->GetContactBySid(sid);

		switch(status)
		{
		/*case CTransfer::NEW:
			break;*/
		/*case CTransfer::WAITING_FOR_ACCEPT:
			break;*/
		case CTransfer::CONNECTING:
			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)ft, 0);
			break;
		case CTransfer::TRANSFERRING:
		case CTransfer::TRANSFERRING_OVER_RELAY:
			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)ft, 0);
			break;
		case CTransfer::FAILED:
			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ft, 0);
			break;
		case CTransfer::COMPLETED:
			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)ft, 0);
			break;
		case CTransfer::CANCELLED:
		case CTransfer::CANCELLED_BY_REMOTE:
			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)ft, 0);
			break;
		}
	}
	if (prop == Transfer::P_BYTESTRANSFERRED)
	{
		//PROTOFILETRANSFERSTATUS
		//this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)ccid, 0);
		//SEString transferProgressStr;
		//transfer->GetPropBytestransferred(transferProgressStr);
		//uint transferProgress = transferProgressStr.toUInt();

		//SEString fileSizeStr;
		//transfer->GetPropFilesize(fileSizeStr);

		//// fileSize is float here, to avoid trouble with 
		//// files lessthan 100 bytes in size.
		//float fileSize = (float)fileSizeStr.toUInt();
		//float progress = (100 * transferProgress) / fileSize;

		//uint transferRate;
		//transfer->GetPropBytespersecond(transferRate);
		//float transferRateInKb = (float)transferRate / 1024;

		//PROTOFILETRANSFERSTATUS pfts = {0};
		//pfts.cbSize = sizeof(pfts);
		////pfts.szCurrentFile
		//pfts.currentFileProgress = progress;
		//pfts.

		//this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)ccid, 0);
  //  printf("Progress: %3.0f%% (%1.0f KB/s)\n", progress, transferRateInKb);
	}
}

void CSkypeProto::OnFileReceived(CConversation::Ref conversation, CMessage::Ref message)
{
	CTransfer::Refs transferList;
	message->GetTransfers(transferList);
	Sid::fetch(transferList);

	Transfer::TYPE transferType;
	Transfer::STATUS transferStatus;

	for (uint i = 0; i < transferList.size(); i++)
	{
		auto transfer = transferList[i];
		transfer.fetch();
		transfer->SetOnTransferCallback(
			(CTransfer::OnTransfer)&CSkypeProto::OnTransferChanged,
			this);
		//this->transferList.append(transfer);

		// For incomings, we need to check for transfer status, just to be sure.
		// In some cases, a transfer can appear with STATUS == PLACEHOLDER
		// As such transfers cannot be accepted, we will need to just store
		// the reference to Transfer Object and then check for further
		// status changes in Transfer::OnChange
		transfer->GetPropType(transferType);
		transfer->GetPropStatus(transferStatus);
		if ((transferType == Transfer::INCOMING) && (transferStatus == Transfer::NEW))
		{
			//transferList[i]->AutoAccept();
			SEString name;
			transfer->GetPropFilename(name);

			wchar_t *path = ::mir_utf8decodeW(name);

			SEString sid;
			transfer->GetPropPartnerHandle(sid);
			HANDLE hContact = this->GetContactBySid(sid);

			auto ft = new FileTransfer(this);
			ft->transfers.append(transfer);
			transfer->GetPropChatmsgGuid(ft->guid);
			this->fileTransferList.insert(ft);

			PROTORECVFILET pre = {0};
			pre.flags = PREF_TCHAR;
			pre.fileCount = 1;
			pre.timestamp = time(NULL);
			pre.tszDescription = L" ";
			pre.ptszFiles = &path;
			pre.lParam = (LPARAM)ft;
			::ProtoChainRecvFile(hContact, &pre);

			::mir_free(path);
		}
	}

	//SEString bodyXml;
	//message->GetPropBodyXml(bodyXml);
	//printf("File transfer msg BodyXML:\n%s\n", (const char*)bodyXml);
}

void CSkypeProto::OnMessage(CConversation::Ref conversation, CMessage::Ref message)
{
	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	CMessage::SENDING_STATUS sendingStatus;
	message->GetPropSendingStatus(sendingStatus);

	if (messageType == CMessage::POSTED_FILES)
	{
		this->OnFileReceived(conversation, message);
		return;
	}

	CMessage::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);

	// it's old message (hystory sync)
	if (status == CMessage::CONSUMED && sendingStatus != CMessage::SENT) return;

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

	case CMessage::POSTED_FILES:
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

					CContact::Ref contact;
					CContact::AVAILABILITY status;
					this->skype->GetContact(data, contact);
					contact->GetPropAvailability(status);

					CParticipant::RANK rank;
					participants[i]->GetPropRank(rank);

					this->AddChatContact(
						cid, 
						data, 
						CParticipant::GetRankName(rank),
						status);
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
					{
						CContact::Ref contact;
						CContact::AVAILABILITY status;
						this->skype->GetContact(sid, contact);
						contact->GetPropAvailability(status);

						//todo: fix rank
						this->AddChatContact(
							cid, 
							sid, 
							CParticipant::GetRankName(CParticipant::WRITER),
							status);
					}
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
