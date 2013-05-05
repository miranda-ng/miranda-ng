#include "skype_proto.h"

int CSkypeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	this->InitChat();
	this->InitNetLib();
	this->InitCustomFolders();
	this->InitInstanceHookList();

	if (::ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = {0};
		bbd.cbSize = sizeof(BBButton);
		bbd.pszModuleName = MODULE;

		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.ptszTooltip = ::TranslateT("Spawn conference");
		bbd.hIcon = CSkypeProto::GetIconHandle("confSpawn");
		bbd.dwButtonID = BBB_ID_CONF_SPAWN;
		bbd.dwDefPos = 100 + bbd.dwButtonID;
		
		::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);
	}

	g_skype->SetOnMessageCallback(
		(CSkype::OnMessaged)&CSkypeProto::OnMessage, 
		this);

	return 0;
}

int CSkypeProto::OnPreShutdown(WPARAM, LPARAM)
{
	this->SetStatus(ID_STATUS_OFFLINE);

	this->UninitInstanceHookList();
	this->UninitNetLib();

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
			g_skype->GetConversationByIdentity(::mir_utf8encodeW(chatID), conversation);
			conversation->RetireFrom();
			conversation->Delete();
		}
		else
			this->RevokeAuth(wParam, lParam);
	}

	return 0;
}

INT_PTR __cdecl CSkypeProto::OnAccountManagerInit(WPARAM wParam, LPARAM lParam)
{
	return (int)::CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_ACCMGR),
		(HWND)lParam,
		&CSkypeProto::SkypeMainOptionsProc,
		(LPARAM)this);
}

int __cdecl CSkypeProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInstance;
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position = 271828;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = SkypeMainOptionsProc;
	::Options_AddPage(wParam, &odp);

	return 0;
}

int __cdecl CSkypeProto::OnTabSRMMButtonPressed(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	
	if (cbcd->dwButtonId == BBB_ID_CONF_SPAWN)
	{
		if (this->IsOnline() && !this->IsChatRoom(hContact))
		{
			StringList targets;
			targets.insert(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN));

			this->StartChat(targets);
		}
	}

	return 1;
}

int CSkypeProto::OnMessagePreCreate(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;

	MessageRef message(evt->seq);
	SEBinary guid;
	if (message->GetPropGuid(guid))
	{
		char *cguid = ::mir_strdup(guid.data());
		cguid[guid.size()] = 0;

		evt->dbei->pBlob = (PBYTE)::mir_realloc(evt->dbei->pBlob, evt->dbei->cbBlob + guid.size());
		::memcpy((char *)&evt->dbei->pBlob[evt->dbei->cbBlob], cguid, guid.size());
		evt->dbei->cbBlob += (DWORD)guid.size();
	}

	return 1;
}

void CSkypeProto::OnMessageReceived(CConversation::Ref &conversation, CMessage::Ref &message)
{
	SEString data;

	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	CMessage::CONSUMPTION_STATUS status;
	message->GetPropConsumptionStatus(status);
		
	message->GetPropBodyXml(data);
	char *text = CSkypeProto::RemoveHtml(data);

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		message->GetPropAuthor(data);			
		
		CContact::Ref author;
		g_skype->GetContact(data, author);

		HANDLE hContact = this->AddContact(author);

		SEBinary guid;
		message->GetPropGuid(guid);

		char *cguid = ::mir_strdup(guid.data());
		cguid[guid.size()] = 0;

		this->RaiseMessageReceivedEvent(
			hContact,
			timestamp, 
			cguid,
			text,
			status == CMessage::UNCONSUMED_NORMAL);
	}
	else
	{
		message->GetPropAuthor(data);
		wchar_t *sid = ::mir_utf8decodeW(data);

		conversation->GetPropIdentity(data);
		wchar_t *cid = ::mir_utf8decodeW(data);

		this->SendChatMessage(cid, sid, ::mir_utf8decodeW(text));

		::mir_free(sid);
		::mir_free(cid);
	}
}

void CSkypeProto::OnMessageSended(CConversation::Ref &conversation, CMessage::Ref &message)
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
	char *text = CSkypeProto::RemoveHtml(data);

	CConversation::TYPE type;
	conversation->GetPropType(type);
	if (type == CConversation::DIALOG)
	{
		CParticipant::Refs participants;
		conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
		participants[0]->GetPropIdentity(data);
		
		CContact::Ref receiver;
		g_skype->GetContact(data, receiver);

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

			char *cguid = ::mir_strdup(guid.data());
			cguid[guid.size()] = 0;

			this->RaiseMessageSendedEvent(
				hContact,
				timestamp,
				cguid,
				text,
				status == CMessage::UNCONSUMED_NORMAL);
		}
	}
	else
	{
		conversation->GetPropIdentity(data);
		wchar_t *cid = ::mir_utf8decodeW(data);

		wchar_t *nick = ::db_get_wsa(NULL, this->m_szModuleName, "Nick");
		if (::wcsicmp(nick, L"") == 0)
		{
			nick = ::db_get_wsa(NULL, this->m_szModuleName, SKYPE_SETTINGS_LOGIN);
		}

		this->SendChatMessage(cid, nick, ::mir_utf8decodeW(text));

		::mir_free(nick);
		::mir_free(cid);
	}
}


void CSkypeProto::OnTransferChanged(CTransfer::Ref transfer, int prop)
{
	switch (prop)
	{
	case Transfer::P_STATUS:
		{
			SEBinary guid;
			transfer->GetPropChatmsgGuid(guid);

			CMessage::Ref message;
			g_skype->GetMessageByGuid(guid, message);

			uint oid = message->getOID();

			SEString data;
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(mir_ptr<wchar_t>(::mir_utf8decodeW(data)));

			Transfer::STATUS status;
			transfer->GetPropStatus(status);
			switch(status)
			{
			/*case CTransfer::NEW:
				break;*/
			/*case CTransfer::WAITING_FOR_ACCEPT:
				break;*/
			case CTransfer::CONNECTING:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, (HANDLE)oid, 0);
				break;
			case CTransfer::TRANSFERRING:
			case CTransfer::TRANSFERRING_OVER_RELAY:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)oid, 0);
				break;
			case CTransfer::FAILED:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)oid, 0);
				this->transferList.remove_val(transfer);
				break;
			case CTransfer::COMPLETED:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)oid, 0);
				this->transferList.remove_val(transfer);
				break;
			case CTransfer::CANCELLED:
			case CTransfer::CANCELLED_BY_REMOTE:
				this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)oid, 0);
				this->transferList.remove_val(transfer);
				break;
			}
		}
		break;

	case Transfer::P_BYTESTRANSFERRED:
		{
			SEString data;

			SEBinary guid;			
			transfer->GetPropChatmsgGuid(guid);

			CMessage::Ref message;
			g_skype->GetMessageByGuid(guid, message);

			uint oid = message->getOID();				

			PROTOFILETRANSFERSTATUS pfts = {0};
			pfts.cbSize = sizeof(pfts);
			pfts.flags = PFTS_UTF | PFTS_RECEIVING;
			pfts.totalFiles = 1;
			pfts.currentFileNumber = 0;
			
			transfer->GetPropPartnerHandle(data);
			HANDLE hContact = this->GetContactBySid(mir_ptr<wchar_t>(::mir_utf8decodeW(data)));
			pfts.hContact = hContact;
		
			transfer->GetPropFilename(data);
			pfts.szCurrentFile = ::mir_strdup(data);

			pfts.pszFiles = &pfts.szCurrentFile;

			transfer->GetPropFilesize(data);
			pfts.totalBytes = pfts.currentFileSize = data.toUInt64();

			transfer->GetPropBytestransferred(data);
			pfts.totalProgress = pfts.currentFileProgress = data.toUInt64();

			this->SendBroadcast(hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)oid, (LPARAM)&pfts);
		}
		break;
	}
}

void CSkypeProto::OnFile(CConversation::Ref &conversation, CMessage::Ref &message)
{
	CTransfer::Refs transfers;
	message->GetTransfers(transfers);
	//Sid::fetch(transferList);

	Transfer::TYPE transferType;
	Transfer::STATUS transferStatus;

	for (uint i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];

		// For incomings, we need to check for transfer status, just to be sure.
		// In some cases, a transfer can appear with STATUS == PLACEHOLDER
		// As such transfers cannot be accepted, we will need to just store
		// the reference to Transfer Object and then check for further
		// status changes in Transfer::OnChange
		transfer->GetPropStatus(transferStatus);
		if (transferStatus == Transfer::NEW)
		{
			transfer->GetPropType(transferType);
			if (transferType == Transfer::INCOMING)
			{
				transfer.fetch();
				transfer->SetOnTransferCallback(
					(CTransfer::OnTransfer)&CSkypeProto::OnTransferChanged,
					this);
				this->transferList.append(transfer);

				uint timestamp;
				message->GetPropTimestamp(timestamp);

				SEString data;
				transfer->GetPropPartnerHandle(data);
				HANDLE hContact = this->GetContactBySid(mir_ptr<wchar_t>(::mir_utf8decodeW(data)));

				transfer->GetPropFilename(data);
				wchar_t *path = ::mir_utf8decodeW(data);

				PROTORECVFILET pre = {0};
				pre.flags = PREF_TCHAR;
				pre.fileCount = 1;
				pre.timestamp = timestamp;
				pre.tszDescription = L" ";
				pre.ptszFiles =  &path;
				pre.lParam = (LPARAM)message->getOID();
				::ProtoChainRecvFile(hContact, &pre);
			}
		}
	}
}

void CSkypeProto::OnMessage(CConversation::Ref conversation, CMessage::Ref message)
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
			
			if (::wcsicmp(mir_ptr<wchar_t>(::mir_utf8decodeW(author)), this->login) == 0)
				this->OnMessageSended(conversation, message);
			else
				this->OnMessageReceived(conversation, message);
		}
		break;

	case CMessage::POSTED_FILES:
		this->OnFile(conversation, message);
		break;

	case CMessage::ADDED_CONSUMERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			wchar_t *cid = ::mir_utf8decodeW(data);

			HANDLE hContact = this->AddChatRoom(conversation);
			if ( !this->IsContactOnline(hContact))
			{
				this->JoinToChat(conversation);
			}
			else
			{
				StringList alreadyInChat(this->GetChatUsers(cid));

				message->GetPropIdentities(data);
				StringList needToAdd(::mir_utf8decodeW(data));

				CParticipant::Refs participants;
				conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
				for (uint i = 0; i < participants.size(); i++)
				{
					participants[i]->GetPropIdentity(data);
					mir_ptr<wchar_t> sid = ::mir_utf8decodeW(data);
					
					if (needToAdd.contains(sid) && !alreadyInChat.contains(sid))
					{
						CContact::Ref contact;
						g_skype->GetContact((char *)mir_ptr<char>(::mir_utf8encodeW(sid)), contact);

						CContact::AVAILABILITY status;
						contact->GetPropAvailability(status);

						CParticipant::RANK rank;
						participants[i]->GetPropRank(rank);

						this->AddChatContact(
							cid, 
							sid, 
							CSkypeProto::Roles[rank],
							CSkypeProto::MirandaToSkypeStatus(status));
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

			StringList alreadyInChat(this->GetChatUsers(cid));
			
			message->GetPropAuthor(data);	
			wchar_t *sid = ::mir_utf8decodeW(data);
			if (::wcsicmp(sid, this->login) != 0)
				if (alreadyInChat.contains(sid))
					this->RemoveChatContact(cid, sid);
		}
		break;
	case CMessage::RETIRED_OTHERS:
		{
			SEString data;

			conversation->GetPropIdentity(data);
			wchar_t *cid = ::mir_utf8decodeW(data);

			message->GetPropIdentities(data);

			StringList alreadyInChat(this->GetChatUsers(cid));
			StringList needToKick(::mir_utf8decodeW(data));
				
			for (size_t i = 0; i < needToKick.size(); i++)
			{
				const wchar_t *sid = needToKick[i];
				if (::wcsicmp(sid, this->login) != 0 && !alreadyInChat.contains(sid))
					this->KickChatContact(cid, sid);
			}
		}
		break;

	case CMessage::SPAWNED_CONFERENCE:
		{
			SEString data;
			conversation->GetPropIdentity(data);
			char *cid = ::mir_strdup(data);

			HANDLE hContact = this->AddChatRoom(conversation);
			if ( !this->IsContactOnline(hContact))
			{
				this->JoinToChat(conversation);
			}
		}
		break;

	case CMessage::STARTED_LIVESESSION:
		conversation->LeaveLiveSession();

		uint timestamp;
		message->GetPropTimestamp(timestamp);

		SEString identity;
		message->GetPropAuthor(identity);

		CContact::Ref author;
		g_skype->GetContact(identity, author);

		HANDLE hContact = this->AddContact(author);
		
		char *message = ::mir_utf8encode(Translate("Incoming call"));
		
		this->AddDBEvent(
			hContact,
			SKYPE_DB_EVENT_TYPE_CALL,
			timestamp,
			DBEF_UTF,
			(DWORD)::strlen(message) + 1,
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