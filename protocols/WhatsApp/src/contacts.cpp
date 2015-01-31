#include "common.h"

bool WhatsAppProto::IsMyContact(MCONTACT hContact, bool include_chat)
{
	const char *proto = GetContactProto(hContact);
	if (proto && strcmp(m_szModuleName, proto) == 0) {
		if (include_chat)
			return true;

		return !isChatRoom(hContact);
	}

	return false;
}

MCONTACT WhatsAppProto::AddToContactList(const std::string& jid, BYTE , bool dont_check, const char *new_name, bool isChatRoom, bool isHidden)
{
	if (jid == m_szJid)
		return NULL;

	if (!dont_check) {
		// First, check if this contact exists
		MCONTACT hContact = ContactIDToHContact(jid);
		if (hContact) {
			if (new_name != NULL) {
				DBVARIANT dbv;
				string oldName;
				if (db_get_utf(hContact, m_szModuleName, WHATSAPP_KEY_NICK, &dbv))
					oldName = jid.c_str();
				else {
					oldName = dbv.pszVal;
					db_free(&dbv);
				}

				if (oldName.compare(string(new_name)) != 0) {
					db_set_utf(hContact, m_szModuleName, WHATSAPP_KEY_NICK, new_name);

					CMString tmp(FORMAT, TranslateT("is now known as '%s'"), ptrT(mir_utf8decodeT(new_name)));
					this->NotifyEvent(_A2T(oldName.c_str()), tmp, hContact, WHATSAPP_EVENT_OTHER);
				}
			}

			if (db_get_b(hContact, "CList", "Hidden", 0) > 0)
				db_unset(hContact, "CList", "Hidden");

			return hContact;
		}
	}

	// If not, make a new contact!
	MCONTACT hContact = CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact == 0)
		return INVALID_CONTACT_ID;

	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName);
	setString(hContact, "ID", jid.c_str());
	debugLogA("Added contact %s", jid.c_str());
	setString(hContact, "MirVer", "WhatsApp");
	db_unset(hContact, "CList", "MyHandle");
	db_set_b(hContact, "CList", "NotOnList", 1);

	ptrT tszGroup(getTStringA(WHATSAPP_KEY_DEF_GROUP));
	if (tszGroup)
		db_set_ts(hContact, "CList", "Group", tszGroup);

	if (new_name != NULL)
		db_set_utf(hContact, m_szModuleName, WHATSAPP_KEY_NICK, new_name);

	if (isChatRoom)
		setByte(hContact, "SimpleChatRoom", 1);

	return hContact;
}

MCONTACT WhatsAppProto::ContactIDToHContact(const std::string& phoneNumber)
{
	// Cache
	std::map<string, MCONTACT>::iterator it = this->hContactByJid.find(phoneNumber);
	if (it != this->hContactByJid.end())
		return it->second;

	const char* idForContact = "ID";
	const char* idForChat = "ChatRoomID";

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		const char* id = isChatRoom(hContact) ? idForChat : idForContact;

		DBVARIANT dbv;
		if (!getString(hContact, id, &dbv)) {
			if (strcmp(phoneNumber.c_str(), dbv.pszVal) == 0) {
				db_free(&dbv);
				this->hContactByJid[phoneNumber] = hContact;
				return hContact;
			}

			db_free(&dbv);
		}
	}

	return 0;
}

void WhatsAppProto::SetAllContactStatuses(int status, bool reset_client)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		if (reset_client) {
			DBVARIANT dbv;
			if (!getTString(hContact, "MirVer", &dbv)) {
				if (_tcscmp(dbv.ptszVal, _T("WhatsApp")))
					setTString(hContact, "MirVer", _T("WhatsApp"));
				db_free(&dbv);
			}

			db_set_ws(hContact, "CList", "StatusMsg", _T(""));
		}

		if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != status)
			setWord(hContact, "Status", status);
	}
}

void WhatsAppProto::ProcessBuddyList(void*)
{
	std::vector<std::string> jids;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
		if (jid) {
			try {
				if (getByte(hContact, "SimpleChatRoom", 0) == 0) {
					m_pConnection->sendQueryLastOnline((char*)jid);
					m_pConnection->sendPresenceSubscriptionRequest((char*)jid);
				}
			}
			CODE_BLOCK_CATCH_ALL
		}
	}

	try {
		m_pConnection->sendGetGroups();
		m_pConnection->sendGetOwningGroups();
	}
	CODE_BLOCK_CATCH_ALL
}

void WhatsAppProto::onAvailable(const std::string& paramString, bool paramBoolean)
{
	MCONTACT hContact = this->AddToContactList(paramString, 0, false);
	if (hContact != NULL) {
		if (paramBoolean)
			setWord(hContact, "Status", ID_STATUS_ONLINE);
		else {
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			this->UpdateStatusMsg(hContact);
		}
	}

	setDword(hContact, WHATSAPP_KEY_LAST_SEEN, 0);
	this->UpdateStatusMsg(hContact);
}

void WhatsAppProto::onLastSeen(const std::string& paramString1, int paramInt, const string &paramString2)
{
	MCONTACT hContact = this->AddToContactList(paramString1, 0, false);
	setDword(hContact, WHATSAPP_KEY_LAST_SEEN, paramInt);

	this->UpdateStatusMsg(hContact);
}

void WhatsAppProto::UpdateStatusMsg(MCONTACT hContact)
{
	std::wstringstream ss;

	int lastSeen = getDword(hContact, WHATSAPP_KEY_LAST_SEEN, -1);
	if (lastSeen != -1) {
		time_t timestamp = time(NULL) - lastSeen;
		TCHAR stzLastSeen[MAX_PATH];
		_tcsftime(stzLastSeen, SIZEOF(stzLastSeen), TranslateT("Last seen on %x at %X"), localtime(&timestamp));
		ss << stzLastSeen;
	}

	db_set_ws(hContact, "CList", "StatusMsg", ss.str().c_str());
}

void WhatsAppProto::onPictureChanged(const std::string& from, const std::string& author, bool set)
{
	if (this->isOnline()) {
		vector<string> ids;
		ids.push_back(from);
		m_pConnection->sendGetPicture(from, "image");
	}
}

void WhatsAppProto::onSendGetPicture(const std::string& jid, const std::vector<unsigned char>& data, const std::string& id)
{
	MCONTACT hContact = this->ContactIDToHContact(jid);
	if (hContact) {
		debugLogA("Updating avatar for jid %s", jid.c_str());

		// Save avatar
		std::tstring filename = GetAvatarFileName(hContact);
		FILE *f = _tfopen(filename.c_str(), _T("wb"));
		size_t r = fwrite(std::string(data.begin(), data.end()).c_str(), 1, data.size(), f);
		fclose(f);

		PROTO_AVATAR_INFORMATIONT ai = { sizeof(ai) };
		ai.hContact = hContact;
		ai.format = PA_FORMAT_JPEG;
		_tcsncpy_s(ai.filename, filename.c_str(), _TRUNCATE);

		int ackResult;
		if (r > 0) {
			setString(hContact, WHATSAPP_KEY_AVATAR_ID, id.c_str());
			ackResult = ACKRESULT_SUCCESS;
		}
		else {
			ackResult = ACKRESULT_FAILED;
		}
		ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ackResult, (HANDLE)&ai, 0);
	}
}

TCHAR* WhatsAppProto::GetContactDisplayName(const string& jid)
{
	MCONTACT hContact = this->ContactIDToHContact(jid);
	return (hContact) ? pcli->pfnGetContactDisplayName(hContact, 0) : _T("none");
}

// Group contacts --------------------------

void WhatsAppProto::SendGetGroupInfoWorker(void* data)
{
	if (this->isOnline())
		m_pConnection->sendGetGroupInfo(*((std::string*) data));
}

void WhatsAppProto::onGroupInfo(const std::string& gjid, const std::string& ownerJid, const std::string& subject, const std::string& createrJid, int paramInt1, int paramInt2)
{
	debugLogA("'%s', '%s', '%s', '%s'", gjid.c_str(), ownerJid.c_str(), subject.c_str(), createrJid.c_str());
	MCONTACT hContact = ContactIDToHContact(gjid);
	if (!hContact) {
		debugLogA("Group info requested for non existing contact '%s'", gjid.c_str());
		return;
	}
	setByte(hContact, "SimpleChatRoom", ownerJid.compare(m_szJid) == 0 ? 2 : 1);
	if (this->isOnline())
		m_pConnection->sendGetParticipants(gjid);
}

void WhatsAppProto::onGroupInfoFromList(const std::string& paramString1, const std::string& paramString2, const std::string& paramString3, const std::string& paramString4, int paramInt1, int paramInt2)
{
	// Called before onOwningGroups() or onParticipatingGroups() is called!
}

void WhatsAppProto::onGroupNewSubject(const std::string& from, const std::string& author, const std::string& newSubject, int paramInt)
{
	debugLogA("'%s', '%s', '%s'", from.c_str(), author.c_str(), newSubject.c_str());
	MCONTACT hContact = this->AddToContactList(from, 0, false, newSubject.c_str(), true);
}

void WhatsAppProto::onGroupAddUser(const std::string& paramString1, const std::string& paramString2)
{
	debugLogA("%s - user: %s", paramString1.c_str(), paramString2.c_str());
	MCONTACT hContact = this->AddToContactList(paramString1);
	TCHAR *ptszGroupName = pcli->pfnGetContactDisplayName(hContact, 0);

	if (paramString2.compare(m_szJid) == 0) {
		this->NotifyEvent(ptszGroupName, TranslateT("You have been added to the group"), hContact, WHATSAPP_EVENT_OTHER);
		setByte(hContact, "IsGroupMember", 1);
	}
	else {
		CMString tmp(FORMAT, TranslateT("User '%s' has been added to the group"), this->GetContactDisplayName(paramString2));
		this->NotifyEvent(ptszGroupName, tmp, hContact, WHATSAPP_EVENT_OTHER);
	}

	if (this->isOnline())
		m_pConnection->sendGetGroupInfo(paramString1);
}

void WhatsAppProto::onGroupRemoveUser(const std::string &paramString1, const std::string &paramString2)
{
	debugLogA("%s - user: %s", paramString1.c_str(), paramString2.c_str());
	MCONTACT hContact = this->ContactIDToHContact(paramString1);
	if (!hContact)
		return;

	TCHAR *ptszGroupName = pcli->pfnGetContactDisplayName(hContact, 0);

	if (paramString2.compare(m_szJid) == 0) {
		//db_set_b(hContact, "CList", "Hidden", 1);
		setByte(hContact, "IsGroupMember", 0);

		this->NotifyEvent(ptszGroupName, TranslateT("You have been removed from the group"), hContact, WHATSAPP_EVENT_OTHER);
	}
	else if (this->isOnline()) {
		CMString tmp(FORMAT, TranslateT("User '%s' has been removed from the group"), this->GetContactDisplayName(paramString2));
		this->NotifyEvent(ptszGroupName, tmp, hContact, WHATSAPP_EVENT_OTHER);

		m_pConnection->sendGetGroupInfo(paramString1);
	}
}

void WhatsAppProto::onLeaveGroup(const std::string &paramString)
{
	// Won't be called for unknown reasons!
	debugLogA("%s", this->GetContactDisplayName(paramString));
	MCONTACT hContact = this->ContactIDToHContact(paramString);
	if (hContact)
		setByte(hContact, "IsGroupMember", 0);
}

void WhatsAppProto::onGetParticipants(const std::string& gjid, const std::vector<string>& participants)
{
	debugLogA("%s", this->GetContactDisplayName(gjid));

	MCONTACT hUserContact, hContact = this->ContactIDToHContact(gjid);
	if (!hContact)
		return;

	if (db_get_b(hContact, "CList", "Hidden", 0) == 1)
		return;

	bool isHidden = true;
	bool isOwningGroup = getByte(hContact, "SimpleChatRoom", 0) == 2;

	if (isOwningGroup)
		this->isMemberByGroupContact[hContact].clear();

	for (std::vector<string>::const_iterator it = participants.begin(); it != participants.end(); ++it) {
		// Hide, if we are not member of the group
		// Sometimes the group is shown shortly after hiding it again, due to other threads which stored the contact
		//	 in a cache before it has been removed (E.g. picture-id list in processBuddyList)
		if (isHidden && m_szJid.compare(*it) == 0) {
			isHidden = false;
			if (!isOwningGroup) {
				// Break, as we don't need to collect group-members
				break;
			}
		}

		// #TODO Slow for big count of participants
		// #TODO If a group is hidden it has been deleted from the local contact list
		//			 => don't allow to add users anymore
		if (isOwningGroup) {
			hUserContact = this->ContactIDToHContact(*it);
			if (hUserContact) {
				this->isMemberByGroupContact[hContact][hUserContact] = true;
			}
		}
	}
	if (isHidden) {
		//db_set_b(hContact, "CList", "Hidden", 1);
		// #TODO Check if it's possible to reach this point at all
		setByte(hContact, "IsGroupMember", 0);
	}
}

// Menu handler
INT_PTR __cdecl WhatsAppProto::OnAddContactToGroup(WPARAM wParam, LPARAM, LPARAM lParam)
{
	TCHAR *a = pcli->pfnGetContactDisplayName((MCONTACT)wParam, 0);
	TCHAR *b = pcli->pfnGetContactDisplayName((MCONTACT)lParam, 0);
	debugLogA("Request add user %S to group %S", a, b);

	if (!this->isOnline())
		return NULL;

	DBVARIANT dbv;
	if (getString((MCONTACT)wParam, "ID", &dbv))
		return NULL;

	std::vector<string> participants;
	participants.push_back(string(dbv.pszVal));
	db_free(&dbv);

	if (getString((MCONTACT)lParam, "ID", &dbv))
		return NULL;

	m_pConnection->sendAddParticipants(string(dbv.pszVal), participants);

	db_free(&dbv);
	return NULL;
}

// Menu handler
INT_PTR __cdecl WhatsAppProto::OnRemoveContactFromGroup(WPARAM wParam, LPARAM, LPARAM lParam)
{
	TCHAR *a = pcli->pfnGetContactDisplayName((MCONTACT)wParam, 0);
	TCHAR *b = pcli->pfnGetContactDisplayName((MCONTACT)lParam, 0);
	debugLogA("Request remove user %S from group %S", a, b);

	if (!this->isOnline())
		return NULL;

	DBVARIANT dbv;
	if (getString((MCONTACT)lParam, "ID", &dbv))
		return NULL;

	std::vector<string> participants;
	participants.push_back(string(dbv.pszVal));
	db_free(&dbv);

	if (getString((MCONTACT)wParam, "ID", &dbv))
		return NULL;

	m_pConnection->sendRemoveParticipants(string(dbv.pszVal), participants);

	db_free(&dbv);
	return NULL;
}

void WhatsAppProto::onOwningGroups(const std::vector<string>& paramVector)
{
	this->HandleReceiveGroups(paramVector, true);
}

void WhatsAppProto::onParticipatingGroups(const std::vector<string>& paramVector)
{
	this->HandleReceiveGroups(paramVector, false);
}

void WhatsAppProto::HandleReceiveGroups(const std::vector<string>& groups, bool isOwned)
{
	MCONTACT hContact;
	map<MCONTACT, bool> isMember; // at the moment, only members of owning groups are stored

	// This could take long time if there are many new groups which aren't
	//	 yet stored to the database. But that should be a rare case
	for (std::vector<string>::const_iterator it = groups.begin(); it != groups.end(); ++it) {
		hContact = this->AddToContactList(*it, 0, false, NULL, true);
		setByte(hContact, "IsGroupMember", 1);
		if (isOwned) {
			this->isMemberByGroupContact[hContact]; // []-operator creates entry, if it doesn't exist
			setByte(hContact, "SimpleChatRoom", 2);
			m_pConnection->sendGetParticipants(*it);
		}
		else isMember[hContact] = true;
	}

	// Mark as non-meber if group only exists locally
	if (!isOwned)
		for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
			if (!isChatRoom(hContact) && getByte(hContact, "SimpleChatRoom", 0) > 0)
				setByte(hContact, "IsGroupMember", isMember.find(hContact) == isMember.end() ? 0 : 1);
}

void WhatsAppProto::onGroupCreated(const std::string& paramString1, const std::string& paramString2)
{
	// Must be received after onOwningGroups() :/
	debugLogA("%s / %s", paramString1.c_str(), paramString2.c_str());
	string jid = paramString2 + string("@") + paramString1;
	MCONTACT hContact = this->AddToContactList(jid, 0, false, NULL, true);
	setByte(hContact, "SimpleChatRoom", 2);
}

// Menu-handler
INT_PTR __cdecl WhatsAppProto::OnCreateGroup(WPARAM wParam, LPARAM lParam)
{
	input_box* ib = new input_box;
	ib->defaultValue = _T("");
	ib->limit = WHATSAPP_GROUP_NAME_LIMIT;
	ib->proto = this;
	ib->text = _T("Enter group subject");
	ib->title = _T("WhatsApp - Create Group");
	ib->thread = &WhatsAppProto::SendCreateGroupWorker;
	HWND hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_INPUTBOX), 0, WhatsAppInputBoxProc, LPARAM(ib));
	ShowWindow(hDlg, SW_SHOW);
	return FALSE;
}

void __cdecl WhatsAppProto::SendSetGroupNameWorker(void* data)
{
	input_box_ret* ibr(static_cast<input_box_ret*>(data));
	string groupName(ibr->value);
	mir_free(ibr->value);

	ptrA jid(getStringA(*((MCONTACT*)ibr->userData), WHATSAPP_KEY_ID));
	if (jid && this->isOnline())
		m_pConnection->sendSetNewSubject((char*)jid, groupName);

	delete ibr->userData;
	delete ibr;
}

void __cdecl WhatsAppProto::SendCreateGroupWorker(void* data)
{
	input_box_ret* ibr(static_cast<input_box_ret*>(data));
	string groupName(ibr->value);
	mir_free(ibr->value);
	if (this->isOnline())
		m_pConnection->sendCreateGroupChat(groupName);
}

INT_PTR __cdecl WhatsAppProto::OnChangeGroupSubject(WPARAM hContact, LPARAM lParam)
{
	input_box* ib = new input_box;

	ptrT szNick(getTStringA(hContact, WHATSAPP_KEY_NICK));
	if (szNick != NULL)
		ib->defaultValue = szNick;

	ib->limit = WHATSAPP_GROUP_NAME_LIMIT;
	ib->text = _T("Enter new group subject");
	ib->title = _T("WhatsApp - Change Group Subject");
	ib->thread = &WhatsAppProto::SendSetGroupNameWorker;
	ib->proto = this;
	MCONTACT *hContactPtr = new MCONTACT(hContact);
	ib->userData = (void*)hContactPtr;

	HWND hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_INPUTBOX), 0, WhatsAppInputBoxProc, LPARAM(ib));
	ShowWindow(hDlg, SW_SHOW);
	return 0;
}

INT_PTR __cdecl WhatsAppProto::OnLeaveGroup(WPARAM hContact, LPARAM)
{
	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid && this->isOnline()) {
		setByte(hContact, "IsGroupMember", 0);
		m_pConnection->sendLeaveGroup((char*)jid);
	}
	return 0;
}
