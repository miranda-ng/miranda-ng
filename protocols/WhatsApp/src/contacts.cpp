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
	std::map<string, MCONTACT>::iterator it = m_hContactByJid.find(phoneNumber);
	if (it != m_hContactByJid.end())
		return it->second;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		const char *id = isChatRoom(hContact) ? "ChatRoomID" : WHATSAPP_KEY_ID;

		ptrA szId(getStringA(hContact, id));
		if (!mir_strcmp(phoneNumber.c_str(), szId)) {
			m_hContactByJid[phoneNumber] = hContact;
			return hContact;
		}
	}

	return 0;
}

void WhatsAppProto::SetAllContactStatuses(int status, bool reset_client)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (reset_client) {
			ptrT tszMirVer(getTStringA(hContact, "MirVer"));
			if (mir_tstrcmp(tszMirVer, _T("WhatsApp")))
				setTString(hContact, "MirVer", _T("WhatsApp"));

			db_set_ws(hContact, "CList", "StatusMsg", _T(""));
		}

		if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != status)
			setWord(hContact, "Status", status);
	}
}

void WhatsAppProto::ProcessBuddyList(void*)
{
	// m_pConnection->setFlush(false);

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
		if (jid)
			m_pConnection->sendQueryLastOnline((char*)jid);
	}

	// m_pConnection->setFlush(true);

	try {
		m_pConnection->sendGetGroups();
	}
	CODE_BLOCK_CATCH_ALL
}

void WhatsAppProto::onAvailable(const std::string &paramString, bool paramBoolean)
{
	MCONTACT hContact = AddToContactList(paramString);
	if (hContact != NULL) {
		if (paramBoolean)
			setWord(hContact, "Status", ID_STATUS_ONLINE);
		else {
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			UpdateStatusMsg(hContact);
		}
	}

	setDword(hContact, WHATSAPP_KEY_LAST_SEEN, 0);
	UpdateStatusMsg(hContact);
}

void WhatsAppProto::onLastSeen(const std::string &paramString1, int paramInt, const string &paramString2)
{
	MCONTACT hContact = AddToContactList(paramString1);
	setDword(hContact, WHATSAPP_KEY_LAST_SEEN, paramInt);

	UpdateStatusMsg(hContact);
}

void WhatsAppProto::UpdateStatusMsg(MCONTACT hContact)
{
	std::wstringstream ss;

	int lastSeen = getDword(hContact, WHATSAPP_KEY_LAST_SEEN, -1);
	if (lastSeen != -1) {
		time_t ts = time(NULL) - lastSeen;
		TCHAR stzLastSeen[MAX_PATH];
		_tcsftime(stzLastSeen, SIZEOF(stzLastSeen), TranslateT("Last seen on %x at %X"), localtime(&ts));
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
	MCONTACT hContact = ContactIDToHContact(jid);
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
	MCONTACT hContact = ContactIDToHContact(jid);
	return (hContact) ? pcli->pfnGetContactDisplayName(hContact, 0) : _T("none");
}
