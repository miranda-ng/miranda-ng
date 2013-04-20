#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->InitSkype();

	this->InitChat();
	this->InitNetLib();
	this->InitCustomFolders();

	this->HookEvent(ME_OPT_INITIALISE,		&CSkypeProto::OnOptionsInit);
	this->HookEvent(ME_USERINFO_INITIALISE, &CSkypeProto::OnUserInfoInit);

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
			wchar_t *chatID = ::db_get_wsa(hContact, this->m_szModuleName, "ChatRoomID");
			this->LeaveChat(chatID);

			CConversation::Ref conversation;
			this->skype->GetConversationByIdentity(::mir_utf8encodeW(chatID), conversation);
			conversation->RetireFrom();
			conversation->Delete();
		}
		else
			this->RevokeAuth(wParam, lParam);
	}

	return 0;
}

int CSkypeProto::OnMessagePreCreate(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;

	MessageRef message(evt->seq);
	
	SEBinary guid;
	if (message->GetPropGuid(guid))
	{
		evt->dbei->pBlob = (PBYTE)::mir_realloc(evt->dbei->pBlob, (evt->dbei->cbBlob + 32));
		::memcpy(&evt->dbei->pBlob[evt->dbei->cbBlob], guid.data(), 32);
		evt->dbei->cbBlob += 32;
	}

	return 1;
}

void CSkypeProto::OnMessageSended(CConversation::Ref conversation, CMessage::Ref message, CContact::Ref receiver)
{
	SEString data;

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	CMessage::SENDING_STATUS sstatus;
	message->GetPropSendingStatus(sstatus);

	CMessage::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);

	message->GetPropBodyXml(data);
	wchar_t *text = ::mir_utf8decodeW(CSkypeProto::RemoveHtml(data));

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		HANDLE hContact = this->AddContact(receiver);
			
		//if (sstatus != CMessage::SENDING)
		{
			this->SendBroadcast(
				hContact,
				ACKTYPE_MESSAGE,
				sstatus == CMessage::FAILED_TO_SEND ? ACKRESULT_FAILED : ACKRESULT_SUCCESS,
				(HANDLE)message->getOID(), 0);

			SEBinary guid;
			message->GetPropGuid(guid);

			this->RaiseMessageSendedEvent(
				hContact,
				timestamp,
				guid.data(),
				text);
		}
	}
	else
	{
		receiver->GetIdentity(data);
		wchar_t *sid = ::mir_utf8decodeW(data);

		conversation->GetPropIdentity(data);
		wchar_t *cid = ::mir_utf8decodeW(data);

		//this->SendChatMessage(cid, sid, ::mir_utf8decodeA(text));

		wchar_t *nick = (wchar_t *)::db_get_wsa(NULL, this->m_szModuleName, "Nick");
		if (::wcsicmp(nick, L"") == 0)
		{
			nick = sid;
		}

		this->SendChatMessage(
			cid, 
			nick, 
			text);

		::mir_free(sid);
		::mir_free(cid);
	}

	::mir_free(text);
}

void CSkypeProto::OnMessageReceived(CConversation::Ref conversation, CMessage::Ref message, CContact::Ref author)
{
	SEString data;

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	CMessage::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);
		
	message->GetPropBodyXml(data);
	wchar_t *text = ::mir_utf8decodeW(CSkypeProto::RemoveHtml(data));

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		HANDLE hContact = this->AddContact(author);

		SEBinary guid;
		message->GetPropGuid(guid);

		this->RaiseMessageReceivedEvent(
			hContact,
			timestamp, 
			guid.data(),
			text,
			status != CMessage::UNCONSUMED_NORMAL);
	}
	else
	{
		message->GetPropAuthor(data);
		wchar_t *sid = ::mir_utf8decodeW(data);

		conversation->GetPropIdentity(data);
		wchar_t *cid = ::mir_utf8decodeW(data);

		this->SendChatMessage(cid, sid, text);

		::mir_free(sid);
		::mir_free(cid);
	}
	
	::mir_free(text);
}

void CSkypeProto::OnTransferChanged(int prop, CTransfer::Ref transfer)
{
	if (prop == Transfer::P_STATUS)
	{
		Transfer::STATUS status;
		transfer->GetPropStatus(status);

		auto ft = this->FindFileTransfer(transfer);

		SEString data;
		transfer->GetPropPartnerHandle(data);
		mir_ptr<wchar_t> sid(::mir_utf8decodeW(data));

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

			SEString data;
			transfer->GetPropPartnerHandle(data);
			mir_ptr<wchar_t> sid(::mir_utf8decodeW(data));

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

	//CMessage::SENDING_STATUS sendingStatus;
	//message->GetPropSendingStatus(sendingStatus);

	//CMessage::CONSUMPTION_STATUS status;
	//message->GetPropConsumptionStatus(status);

	switch (messageType)
	{
	case CMessage::POSTED_EMOTE:
		//int i = 0;
		break;

	case CMessage::POSTED_TEXT:
		{
			SEString identity;
			message->GetPropAuthor(identity);			
			wchar_t *sid = ::mir_utf8decodeW(identity);
			if (::wcsicmp(sid, this->login) == 0)
			{
				CParticipant::Refs participants;
				conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
				participants[0]->GetPropIdentity(identity);
				CContact::Ref receiver;
				this->skype->GetContact(identity, receiver);
				this->OnMessageSended(conversation, message, receiver);
			}
			else
			{
				CContact::Ref author;
				this->skype->GetContact(identity, author);
				this->OnMessageReceived(conversation, message, author);
			}	
			::mir_free(sid);
		}
		break;

	case CMessage::POSTED_FILES:
		this->OnFileReceived(conversation, message);
		break;

	case CMessage::ADDED_CONSUMERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			wchar_t *cid = ::mir_utf8decodeW(data);

			HANDLE hContact = this->GetChatRoomByID(cid);
			if ( !hContact || ::db_get_w(hContact, this->m_szModuleName, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
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
						::mir_utf8decodeW(data), 
						::mir_utf8decodeW(CParticipant::GetRankName(rank)),
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
					if (::wcsicmp(::mir_a2u(sid), this->login) != 0 && !alreadyInChat.contains(sid))
					{
						CContact::Ref contact;
						CContact::AVAILABILITY status;
						this->skype->GetContact(sid, contact);
						contact->GetPropAvailability(status);

						//todo: fix rank
						this->AddChatContact(
							cid, 
							::mir_a2u(sid), 
							::mir_utf8decodeW(CParticipant::GetRankName(CParticipant::WRITER)),
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
			wchar_t *cid = ::mir_utf8decodeW(data);

			StringList alreadyInChat(this->GetChatUsers(cid), " ");
			
			message->GetPropAuthor(data);	
			wchar_t *sid = ::mir_utf8decodeW(data);
			if (::wcsicmp(sid, this->login) != 0)
				if (alreadyInChat.contains(::mir_u2a(sid)))
					this->RemoveChatContact(cid, sid);
		}
		break;
	case CMessage::RETIRED_OTHERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			wchar_t *cid = ::mir_utf8decodeW(data);

			message->GetPropIdentities(data);

			StringList alreadyInChat(this->GetChatUsers(cid), " ");
			StringList needToKick(data, " ");
				
			for (int i = 0; i < needToKick.getCount(); i++)
			{
				wchar_t *sid = ::mir_utf8decodeW(needToKick[i]);
				if (::wcsicmp(sid, this->login) != 0 && !alreadyInChat.contains(::mir_u2a(sid)))
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

	case CMessage::STARTED_LIVESESSION:
		//conversation->LeaveLiveSession();

		uint timestamp;
		message->GetPropTimestamp(timestamp);

		SEString identity;
		message->GetPropAuthor(identity);

		CContact::Ref author;
		this->skype->GetContact(identity, author);

		HANDLE hContact = this->AddContact(author);
		
		wchar_t *message = new wchar_t[14];
		::wcscpy(message, L"Incoming call");
		
		this->AddDataBaseEvent(
			hContact,
			SKYPE_DB_EVENT_TYPE_CALL,
			timestamp,
			DBEF_UTF,
			::wcslen(message) + 1,
			(PBYTE)message);
		break;

	//case CMessage::REQUESTED_AUTH:
	//	break;

	//case CMessage::GRANTED_AUTH:
	//	break;

	//case CMessage::BLOCKED:
	//	break;
	}
}