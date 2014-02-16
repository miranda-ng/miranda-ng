#include "skype.h"
#include "skype_chat.h"

CSkypeProto::CSkypeProto(const char* protoName, const TCHAR* userName) :
	PROTO<CSkypeProto>(protoName, userName),
	Skype(1), 
	skypeKitPort(8963),
	chatRooms(5)
{
	this->rememberPassword = false;

	::InitializeCriticalSection(&this->contact_search_lock);

	this->SetAllContactStatus(ID_STATUS_OFFLINE);

	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = this->m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_EMOTE;
	dbEventType.descr = "Skype emote";
	::CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_CONTACTS;
	dbEventType.descr = "Skype contacts";
	dbEventType.eventIcon = CSkypeProto::GetSkinIconHandle("sendContacts");
	::CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = SKYPE_DB_EVENT_TYPE_CALL;
	dbEventType.descr = "Skype call";
	dbEventType.eventIcon = CSkypeProto::GetSkinIconHandle("call");
	::CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	this->InitInstanceServiceList();
}

CSkypeProto::~CSkypeProto()
{
	::DeleteCriticalSection(&this->contact_search_lock);

	::mir_free(this->login);
	if (this->password)
	{
		::mir_free(this->password);
		this->password = NULL;
	}
}

MCONTACT __cdecl CSkypeProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	CContact::Ref contact;
	this->GetContact((char *)mir_ptr<char>(::mir_utf8encodeW(psr->id)), contact);
	return this->AddContact(contact);
}

MCONTACT __cdecl CSkypeProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent)
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);

	/*if ((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) != -1)
	{
	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei) == 0 &&
	!strcmp(dbei.szModule, m_szModuleName) &&
	(dbei.eventType == EVENTTYPE_AUTHREQUEST || dbei.eventType == EVENTTYPE_CONTACTS))
	{
	char *nick = (char*)(dbei.pBlob + sizeof(DWORD) * 2);
	char *firstName = nick + strlen(nick) + 1;
	char *lastName = firstName + strlen(firstName) + 1;
	char *skypeName = lastName + strlen(lastName) + 1;
	return AddContactBySkypeName(::mir_a2u(skypeName), ::mir_a2u(nick), 0);
	}
	}*/
	return 0;
}

int __cdecl CSkypeProto::Authorize(HANDLE hDbEvent)
{
	if (this->IsOnline() && hDbEvent)
	{
		MCONTACT hContact = this->GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		return CSkypeProto::GrantAuth(hContact, NULL);
	}

	return 1;
}

int __cdecl CSkypeProto::AuthDeny(HANDLE hDbEvent, const TCHAR* szReason)
{
	if (this->IsOnline())
	{
		MCONTACT hContact = this->GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_CONTACT_ID)
			return 1;

		return CSkypeProto::RevokeAuth(hContact, NULL);
	}

	return 1;
}

int __cdecl CSkypeProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT* pre)
{
	DWORD flags = 0;

	if (pre->flags & PREF_CREATEREAD)
		flags |= DBEF_READ;

	if (pre->flags & PREF_UTF)
		flags |= DBEF_UTF;

	this->AddDBEvent(
		hContact,
		EVENTTYPE_AUTHREQUEST,
		pre->timestamp,
		flags,
		pre->lParam,
		(PBYTE)pre->szMessage);

	return 0;
}

int __cdecl CSkypeProto::AuthRequest(MCONTACT hContact, const TCHAR* szMessage)
{
	if (this->IsOnline() && hContact)
	{
		CContact::Ref contact;
		SEString sid(_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));
		if (this->GetContact(sid, contact))
		{
			contact->SetBuddyStatus(Contact::AUTHORIZED_BY_ME);
			contact->SendAuthRequest(::mir_utf8encodeW(szMessage));
		}

		return 0;
	}

	return 1;
}

HANDLE __cdecl CSkypeProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath ) 
{ 
	uint oid = (uint)hTransfer;

	this->debugLogW(L"Incoming file transfer is accepted");

	bool success;
	wchar_t fullPath[MAX_PATH] = {0};

	SEString data;
	MessageRef msgRef(oid);
	TransferRefs transfers;
	msgRef->GetTransfers(transfers);
	for (uint i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];
		transfer->GetPropFilename(data);
		ptrW name(::mir_utf8decodeW(data));
		::mir_sntprintf(fullPath, MAX_PATH, L"%s%s", szPath, name);

		auto fOid = transfer->getOID();
		FileTransferParam ftp = this->transferts[oid];

		//PROTOFILETRANSFERSTATUS pfts = { sizeof(pfts) };
		//pfts.hContact = hContact;
		//pfts.flags = PFTS_UNICODE | PFTS_RECEIVING;
		//pfts.ptszFiles = ftp.pfts.ptszFiles;
		//pfts.totalFiles = ftp.pfts.totalFiles;
		//pfts.currentFileNumber = i;
		//pfts.totalBytes = ftp.files[fOid].size;
		//pfts.totalProgress = ftp.files[fOid].transfered;
		//pfts.tszWorkingDir = mir_wstrdup(szPath);
		//pfts.currentFileNumber = 0;
		//pfts.tszCurrentFile = mir_wstrdup(fullPath);
		////pfts.tszCurrentFile = ::mir_utf8decodeW(data);
		//pfts.currentFileSize = ftp.files[fOid].size;
		//pfts.currentFileProgress = ftp.files[fOid].transfered;

		//if ( !ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)oid, (LPARAM)&pfts))
			if ( !transfer->Accept((char *)ptrA(::mir_utf8encodeW(fullPath)), success) || !success)
			{
				this->debugLogW(L"Cannot accept file transfer");
				this->transferList.remove_val(transfer);
			}
	}

	return hTransfer; 
}

int    __cdecl CSkypeProto::FileCancel(MCONTACT hContact, HANDLE hTransfer ) 
{
	uint oid = (uint)hTransfer;

	MessageRef msgRef(oid);
	TransferRefs transfers;
	Transfer::STATUS transferStatus;
	msgRef->GetTransfers(transfers);
	for (uint i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];
		transfer->GetPropStatus(transferStatus);
		if (transferStatus <= Transfer::CANCELLED && this->transferList.contains(transfer))
		{
			if ( !transfer->Cancel())
				this->debugLogW(L"Incoming file transfer is cancelled");
			this->transferList.remove_val(transfer);
		}
	}
	this->transferts.erase(this->transferts.find(oid));

	return 1; 
}

int    __cdecl CSkypeProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason )
{
	uint oid = (uint)hTransfer;

	MessageRef msgRef(oid);
	TransferRefs transfers;
	Transfer::STATUS transferStatus;
	msgRef->GetTransfers(transfers);
	for (uint i = 0; i < transfers.size(); i++)
	{
		auto transfer = transfers[i];
		transfer->GetPropStatus(transferStatus);
		if (transferStatus <= Transfer::CANCELLED && this->transferList.contains(transfer))
		{
			if ( !transfer->Cancel())
				this->debugLogW(L"Incoming file transfer is denied");
			this->transferList.remove_val(transfer);
		}
	}
	this->transferts.erase(this->transferts.find(oid));

	return 1;
}

int    __cdecl CSkypeProto::FileResume( HANDLE hTransfer, int* action, const TCHAR** szFilename )
{
	if ( !this->IsOnline())
		return 1;

	uint oid = (uint)hTransfer;

	//auto fOid = transfers[i]->getOID();
	FileTransferParam ftp = this->transferts[oid];

	MessageRef msgRef(oid);
	TransferRefs transfers;
	msgRef->GetTransfers(transfers);
	for (uint i = 0; i < transfers.size(); i++){}

	switch (*action)
	{
	case FILERESUME_SKIP:
		/*if (ft->p2p_appID != 0)
		p2p_sendStatus(ft, 603);
		else
		msnftp_sendAcceptReject (ft, false);*/
		break;

	case FILERESUME_RESUME:
		//replaceStrT(ft->std.tszCurrentFile, *szFilename);
		break;

	case FILERESUME_RENAME:
		//replaceStrT(ft->std.tszCurrentFile, *szFilename);
		break;

	default:
		/*bool fcrt = ft->create() != -1;
		if (ft->p2p_appID != 0)
		{
		if (fcrt)
		p2p_sendFeedStart(ft);

		p2p_sendStatus(ft, fcrt ? 200 : 603);
		}
		else
		msnftp_sendAcceptReject (ft, fcrt);*/

		//ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, ft, 0);
		break;
	}

	return 0;
}

DWORD_PTR __cdecl CSkypeProto:: GetCaps(int type, MCONTACT hContact)
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME |
			PF1_AUTHREQ | PF1_CHAT | PF1_SERVERCLIST | PF1_CONTACT/* | PF1_ADDSEARCHRES*/;
	case PFLAGNUM_2:
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_INVISIBLE | PF2_ONTHEPHONE;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_AVATARS | 
			/*PF4_OFFLINEFILES | */PF4_IMSENDUTF | PF4_IMSENDOFFLINE | PF4_NOAUTHDENYREASON;
	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)::Translate("Skype name");
	case PFLAG_MAXCONTACTSPERPACKET:
		return 1024;
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)SKYPE_SETTINGS_SID;
	default:
		return 0;
	}
}

int    __cdecl CSkypeProto::GetInfo(MCONTACT hContact, int infoType ) { return 0; }

HANDLE __cdecl CSkypeProto::SearchBasic(const TCHAR* id)
{
	if ( !this->IsOnline())
		return 0;

	this->ForkThread(&CSkypeProto::SearchBySidAsync, ::mir_tstrdup(id));

	return (HANDLE)SKYPE_SEARCH_BYSID;
}

HANDLE __cdecl CSkypeProto::SearchByEmail(const TCHAR* email)
{
	if ( !this->IsOnline())
		return 0;

	this->ForkThread(&CSkypeProto::SearchByEmailAsync, ::mir_tstrdup(email));

	return (HANDLE)SKYPE_SEARCH_BYEMAIL;
}

HANDLE __cdecl CSkypeProto::SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName)
{
	PROTOSEARCHRESULT *psr = new PROTOSEARCHRESULT();
	psr->cbSize = sizeof(psr);
	psr->flags = PSR_TCHAR;
	psr->nick = ::mir_wstrdup(nick);
	psr->firstName = ::mir_wstrdup(firstName);
	psr->lastName = ::mir_wstrdup(lastName);

	this->ForkThread(&CSkypeProto::SearchByNamesAsync, psr);

	return (HANDLE)SKYPE_SEARCH_BYNAMES;
}

HWND   __cdecl CSkypeProto::SearchAdvanced( HWND owner ) { return 0; }

HWND   __cdecl CSkypeProto::CreateExtendedSearchUI( HWND owner ){ return 0; }

int    __cdecl CSkypeProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre) 
{
	this->debugLogW(L"Incoming contacts");
	::db_unset(hContact, "CList", "Hidden");

	return (INT_PTR)this->AddDBEvent(
		hContact,
		EVENTTYPE_CONTACTS,
		pre->timestamp,
		DBEF_UTF | ((pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0),
		pre->lParam,
		(PBYTE)pre->szMessage);
}

int    __cdecl CSkypeProto::RecvFile(MCONTACT hContact, PROTORECVFILET* pre) 
{ 
	this->debugLogW(L"Incoming file transfer");
	::db_unset(hContact, "CList", "Hidden");
	return ::Proto_RecvFile(hContact, pre);
}

int __cdecl CSkypeProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
	this->debugLogW(L"Incoming message");
	::db_unset(hContact, "CList", "Hidden");

	ReadMessageParam *param = (ReadMessageParam*)pre->lParam;

	char *message = (char *)pre->szMessage;
	size_t msgLen = ::strlen(message) + 1;

	message = (char *)::mir_realloc(message, msgLen + param->guid.size());
	::memcpy((char *)&message[msgLen], param->guid.data(), param->guid.size());

	return (INT_PTR)this->AddDBEvent(
		hContact,
		param->msgType == CMessage::POSTED_TEXT ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_EMOTE,
		pre->timestamp,
		DBEF_UTF | ((pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0),
		DWORD(msgLen + param->guid.size()),
		(PBYTE)message);
}

int __cdecl CSkypeProto::RecvUrl(MCONTACT hContact, PROTORECVEVENT *) { return 0; }

int __cdecl CSkypeProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	if (this->IsOnline() && hContact && hContactsList)
	{
		this->debugLogW(L"Outcoming contacts");

		ConversationRef conversation;
		if ( !this->isChatRoom(hContact))
		{
			SEStringList targets;
			targets.append((char *)_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));

			this->GetConversationByParticipants(targets, conversation);
		}
		else
		{
			this->GetConversationByIdentity((char *)_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)), conversation);
		}
		if ( !conversation)
			return 0; 

		ContactRefs contacts;
		for (int i = 0; i < nContacts; i++)
		{
			CContact::Ref contact;

			ptrW csid(::db_get_wsa(hContactsList[i], this->m_szModuleName, SKYPE_SETTINGS_SID));
			this->GetContact((char *)ptrA(::mir_utf8encodeW(csid)), contact);
			contacts.append(contact);
		}

		time_t timestamp = time(NULL);

		if ( !conversation->PostContacts(contacts))
			return 0;

		// todo: bad hack
		CMessage::Refs msgs;
		this->GetMessageListByType(Message::POSTED_CONTACTS, true, msgs, timestamp);

		if (msgs.size() == 0)
			return 0;

		CMessage::Ref lastMsg = msgs[msgs.size() - 1];

		return lastMsg->getOID();
	}

	return 0; 
}

HANDLE __cdecl CSkypeProto::SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles)
{
	if (this->IsOnline() && hContact && ppszFiles)
	{
		this->debugLogW(L"Outcoming file transfer");

		ConversationRef conversation;
		if ( !this->isChatRoom(hContact))
		{
			SEStringList targets;
			targets.append((char *)_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));

			this->GetConversationByParticipants(targets, conversation);
		}
		else
		{
			this->GetConversationByIdentity((char *)_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)), conversation);
		}
		if ( !conversation)
			return 0; 

		FileTransferParam ftp;
		ftp.pfts.flags = PFTS_SENDING | PFTS_UNICODE;
		ftp.pfts.hContact = hContact;
		for (ftp.pfts.totalFiles = 0; ppszFiles[ftp.pfts.totalFiles]; ftp.pfts.totalFiles++);
		ftp.pfts.ptszFiles = new wchar_t*[ftp.pfts.totalFiles + 1];

		wchar_t *wd = new wchar_t[wcslen(ppszFiles[0]) + 1];
		wcscpy(wd, ppszFiles[0]);
		PathRemoveFileSpec(wd);
		ftp.pfts.tszWorkingDir = wd;

		SEFilenameList fileList;
		for (int i = 0; ppszFiles[i]; i++)
		{
			ftp.pfts.ptszFiles[i] = ::mir_wstrdup(ppszFiles[i]);
			fileList.append((char *)ptrA(::mir_utf8encodeW(ppszFiles[i])));
		}

		auto error = TRANSFER_OPEN_SUCCESS;
		SEFilename errFile;
		MessageRef msgRef;
		if ( !conversation->PostFiles(fileList, " ", error, errFile, msgRef) || error)
			return 0;		

		SEString data;
		TransferRefs transfers;
		if (msgRef->GetTransfers(transfers))
		{
			for (uint i = 0; i < transfers.size(); i++)
			{
				transfers[i].fetch();
				this->transferList.append(transfers[i]);				

				transfers[i]->GetPropFilesize(data);
				Sid::uint64 size = data.toUInt64();

				ftp.files.insert(std::make_pair(transfers[i]->getOID(), FileParam(size)));
				ftp.pfts.totalBytes += size;
			}
		}

		auto oid = msgRef->getOID();
		this->transferts.insert(std::make_pair(oid, ftp));

		return (HANDLE)oid;
	}

	return 0; 
}

int __cdecl CSkypeProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	this->debugLogW(L"Outcoming message");
	SEStringList targets;
	targets.append((char *)_T2A(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID)));

	CConversation::Ref conversation;
	this->GetConversationByParticipants(targets, conversation);

	if (conversation)
	{
		CMessage::Ref message;
		if (!conversation->PostText(msg, message))
			return 0;

		return message->getOID();
	}

	return 0;
}

int __cdecl CSkypeProto::SendUrl(MCONTACT hContact, int flags, const char *url) { return 0; }

int __cdecl CSkypeProto::SetApparentMode(MCONTACT hContact, int mode) { return 0; }

int CSkypeProto::SetStatus(int new_status)
{
	switch (new_status)
	{
	case ID_STATUS_OCCUPIED:
		new_status = ID_STATUS_DND;
		break;
	case ID_STATUS_FREECHAT:
		new_status = ID_STATUS_ONLINE;
		break;
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
	case ID_STATUS_NA:
		new_status = ID_STATUS_AWAY;
		break;
	}

	if (new_status == this->m_iDesiredStatus)
		return 0;

	int old_status = this->m_iStatus;
	this->m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE)
	{
		this->LogOut();
		this->m_iStatus = this->m_iDesiredStatus = ID_STATUS_OFFLINE;

		this->SendBroadcast(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		if ( !::Miranda_Terminated())
		{
			this->SetAllContactStatus(ID_STATUS_OFFLINE);
			this->CloseAllChatSessions();
		}

		return 0;
	}
	else
	{
		if (old_status == ID_STATUS_OFFLINE && !this->IsOnline())
		{
			this->m_iStatus = ID_STATUS_CONNECTING;
			if ( !this->LogIn())
				return 0;
		}
		else
		{
			if ( this->account->IsOnline())
			{
				SetServerStatus(new_status);
				return 0;
			}

			this->SendBroadcast(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
			return 0;
		}
	}

	this->SendBroadcast(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return 0;
}

HANDLE __cdecl CSkypeProto::GetAwayMsg(MCONTACT hContact) { return 0; }
int    __cdecl CSkypeProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt) { return 0; }
int    __cdecl CSkypeProto::SetAwayMsg(int m_iStatus, const TCHAR *msg) { return 0; }

int __cdecl CSkypeProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && this->IsOnline() && this->m_iStatus != ID_STATUS_INVISIBLE)
	{
		ptrW sid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
		if (sid != NULL && ::wcsicmp(sid, this->login) != 0)
		{
			SEStringList targets;
			targets.append((char *)ptrA(::mir_utf8encodeW(sid)));

			CConversation::Ref conversation;
			this->GetConversationByParticipants(targets, conversation);

			if (conversation)
			{
				switch (type)
				{
				case PROTOTYPE_SELFTYPING_ON:
					conversation->SetMyTextStatusTo(Participant::WRITING);
					return 0;

				case PROTOTYPE_SELFTYPING_OFF:
					conversation->SetMyTextStatusTo(Participant::READING); //todo: mb TEXT_UNKNOWN?
					return 0;
				}
			}
		}
	}

	return 1;
}

int __cdecl CSkypeProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType)
	{
	case EV_PROTO_ONLOAD:
		return this->OnProtoModulesLoaded(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return this->OnPreShutdown(wParam, lParam);

	case EV_PROTO_ONOPTIONS:
		return this->OnOptionsInit(wParam,lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return this->OnContactDeleted(wParam, lParam);

	case EV_PROTO_ONMENU:
		this->OnInitStatusMenu();
		break;
	}

	return 1;
}