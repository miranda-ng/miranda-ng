#include "common.h"

MCONTACT WhatsAppProto::AddToContactList(const std::string &jid, const char *new_name)
{
	if (jid == m_szJid)
		return NULL;

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

	// If not, make a new contact!
	if ((hContact = CallService(MS_DB_CONTACT_ADD, 0, 0)) == 0)
		return INVALID_CONTACT_ID;

	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName);
	setString(hContact, "ID", jid.c_str());
	debugLogA("Added contact %s", jid.c_str());
	setString(hContact, "MirVer", "WhatsApp");
	db_unset(hContact, "CList", "MyHandle");
	db_set_b(hContact, "CList", "NotOnList", 1);
	db_set_ts(hContact, "CList", "Group", m_tszDefaultGroup);

	if (new_name != NULL)
		db_set_utf(hContact, m_szModuleName, WHATSAPP_KEY_NICK, new_name);

	return hContact;
}

MCONTACT WhatsAppProto::ContactIDToHContact(const std::string &phoneNumber)
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
				m_pConnection->sendQueryLastOnline(std::string(jid));
			}
			CODE_BLOCK_CATCH_ALL
		}
	}

	try {
		m_pConnection->sendGetGroups();
	}
	CODE_BLOCK_CATCH_ALL
}

void WhatsAppProto::onAvailable(const std::string &paramString, bool paramBoolean)
{
	MCONTACT hContact = this->AddToContactList(paramString);
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

void WhatsAppProto::onLastSeen(const std::string &paramString1, int paramInt, const string &paramString2)
{
	MCONTACT hContact = this->AddToContactList(paramString1);
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

void WhatsAppProto::onContactChanged(const std::string &jid, bool added)
{
}

void WhatsAppProto::onPictureChanged(const std::string &jid, const std::string &id, bool set)
{
	if (isOnline())
		m_pConnection->sendGetPicture(jid.c_str(), "preview");
}

void WhatsAppProto::onSendGetPicture(const std::string &jid, const std::vector<unsigned char>& data, const std::string &id)
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
