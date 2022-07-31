#include "stdafx.h"

uint16_t CToxProto::GetContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

void CToxProto::SetContactStatus(MCONTACT hContact, uint16_t status)
{
	uint16_t oldStatus = GetContactStatus(hContact);
	if (oldStatus != status)
		setWord(hContact, "Status", status);
}

MCONTACT CToxProto::GetContactFromAuthEvent(MEVENT hEvent)
{
	uint32_t body[3];
	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(uint32_t) * 2;
	dbei.pBlob = (uint8_t*)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (mir_strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

MCONTACT CToxProto::GetContact(const Tox *tox, const int friendNumber)
{
	if (!tox)
		return NULL;

	uint8_t data[TOX_PUBLIC_KEY_SIZE];
	TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
	if (!tox_friend_get_public_key(tox, friendNumber, data, &error)) {
		debugLogA(__FUNCTION__": failed to get friend (%d) public key (%d)", friendNumber, error);
		return NULL;
	}
	ToxHexAddress pubKey(data, TOX_PUBLIC_KEY_SIZE);
	return GetContact(pubKey);
}

MCONTACT CToxProto::GetContact(const char *pubKey)
{
	for (auto &hContact : AccContacts()) {
		ptrA contactPubKey(getStringA(hContact, TOX_SETTINGS_ID));
		// check only public key part of address
		if (mir_strncmpi(pubKey, contactPubKey, TOX_PUBLIC_KEY_SIZE) == 0)
			return hContact;
	}
	return 0;
}

ToxHexAddress CToxProto::GetContactPublicKey(const Tox *tox, const int friendNumber)
{
	if (!tox)
		return ToxHexAddress::Empty();

	uint8_t data[TOX_PUBLIC_KEY_SIZE];
	TOX_ERR_FRIEND_GET_PUBLIC_KEY error;
	if (!tox_friend_get_public_key(tox, friendNumber, data, &error)) {
		debugLogA(__FUNCTION__": failed to get friend (%d) public key (%d)", friendNumber, error);
		return ToxHexAddress::Empty();
	}
	ToxHexAddress pubKey(data, TOX_PUBLIC_KEY_SIZE);
	return pubKey;
}

MCONTACT CToxProto::AddContact(const char *address, const wchar_t *nick, const wchar_t *dnsId, bool isTemporary)
{
	MCONTACT hContact = GetContact(address);
	if (hContact)
		return hContact;

	hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	setString(hContact, TOX_SETTINGS_ID, address);

	if (mir_wstrlen(nick))
		setWString(hContact, "Nick", nick);
	else
		setWString(hContact, "Nick", _A2T(address));

	if (mir_wstrlen(dnsId))
		setWString(hContact, TOX_SETTINGS_DNS, dnsId);

	if (!isTemporary) {
		Clist_SetGroup(hContact, m_defaultGroup);

		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 1);
	}
	else
		Contact::RemoveFromList(hContact);

	return hContact;
}

uint32_t CToxProto::GetToxFriendNumber(MCONTACT hContact)
{
	ToxBinAddress pubKey(ptrA(getStringA(hContact, TOX_SETTINGS_ID)));
	TOX_ERR_FRIEND_BY_PUBLIC_KEY error;
	uint32_t friendNumber = tox_friend_by_public_key(m_tox, pubKey.GetPubKey(), &error);
	if (error != TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK)
		debugLogA(__FUNCTION__": failed to get friend number (%d)", error);
	return friendNumber;
}

void CToxProto::LoadFriendList(Tox *tox)
{
	size_t count = tox_self_get_friend_list_size(tox);
	if (count > 0) {
		uint32_t *friends = (uint32_t*)mir_alloc(count * sizeof(uint32_t));
		tox_self_get_friend_list(tox, friends);

		for (size_t i = 0; i < count; i++) {
			uint32_t friendNumber = friends[i];

			ToxHexAddress pubKey = GetContactPublicKey(tox, friendNumber);
			if (pubKey == ToxHexAddress::Empty())
				continue;

			MCONTACT hContact = AddContact(pubKey);
			if (hContact) {
				delSetting(hContact, "Auth");
				delSetting(hContact, "Grant");

				TOX_ERR_FRIEND_QUERY getNameResult;
				uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
				if (tox_friend_get_name(tox, friendNumber, nick, &getNameResult))
					setWString(hContact, "Nick", ptrW(mir_utf8decodeW((char*)nick)));
				else
					debugLogA(__FUNCTION__": failed to get friend name (%d)", getNameResult);

				TOX_ERR_FRIEND_GET_LAST_ONLINE getLastOnlineResult;
				uint64_t timestamp = tox_friend_get_last_online(tox, friendNumber, &getLastOnlineResult);
				if (getLastOnlineResult == TOX_ERR_FRIEND_GET_LAST_ONLINE_OK)
					setDword(hContact, "LastEventDateTS", timestamp);
				else
					debugLogA(__FUNCTION__": failed to get friend last online (%d)", getLastOnlineResult);
			}
		}
		mir_free(friends);
	}
}

INT_PTR CToxProto::OnRequestAuth(WPARAM hContact, LPARAM lParam)
{
	if (!IsOnline())
		return 0;

	char *reason = lParam ? (char*)lParam : " ";
	size_t length = mir_strlen(reason);
	ToxBinAddress address(ptrA(getStringA(hContact, TOX_SETTINGS_ID)));

	TOX_ERR_FRIEND_ADD addFriendResult;
	/*int32_t friendNumber = */tox_friend_add(m_tox, address, (uint8_t*)reason, length, &addFriendResult);
	if (addFriendResult != TOX_ERR_FRIEND_ADD_OK) {
		debugLogA(__FUNCTION__": failed to request auth(%d)", addFriendResult);
		return addFriendResult;
	}

	Contact::PutOnList(hContact);
	delSetting(hContact, "Grant");

	/*uint8_t nick[TOX_MAX_NAME_LENGTH] = { 0 };
	TOX_ERR_FRIEND_QUERY errorFriendQuery;
	if (tox_friend_get_name(toxThread->Tox(), friendNumber, nick, &errorFriendQuery))
		setWString(hContact, "Nick", ptrW(mir_utf8decodeW((char*)nick)));
	else
		debugLogA(__FUNCTION__": failed to get friend name (%d)", errorFriendQuery);*/

	return 0;
}

INT_PTR CToxProto::OnGrantAuth(WPARAM hContact, LPARAM)
{
	if (!IsOnline())
		return 0;

	ToxBinAddress pubKey(ptrA(getStringA(hContact, TOX_SETTINGS_ID)));
	TOX_ERR_FRIEND_ADD error;
	tox_friend_add_norequest(m_tox, pubKey, &error);
	if (error != TOX_ERR_FRIEND_ADD_OK) {
		debugLogA(__FUNCTION__": failed to grant auth (%d)", error);
		return error;
	}

	Contact::PutOnList(hContact);
	delSetting(hContact, "Grant");

	SaveToxProfile(m_tox);

	return 0;
}

void CToxProto::OnContactDeleted(MCONTACT hContact)
{
	if (!IsOnline())
		return;

	if (!isChatRoom(hContact)) {
		int32_t friendNumber = GetToxFriendNumber(hContact);
		TOX_ERR_FRIEND_DELETE error;
		if (!tox_friend_delete(m_tox, friendNumber, &error)) {
			debugLogA(__FUNCTION__": failed to delete friend (%d)", error);
			return;
		}
		SaveToxProfile(m_tox);
	}
}

void CToxProto::OnFriendRequest(Tox*, const uint8_t *pubKey, const uint8_t *message, size_t /* length */, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxHexAddress address(pubKey);
	MCONTACT hContact = proto->AddContact(address);
	if (!hContact) {
		Netlib_Logf(proto->m_hNetlibUser, __FUNCTION__": failed to create contact");
		return;
	}

	proto->delSetting(hContact, "Auth");

	DB::AUTH_BLOB blob(hContact, nullptr, nullptr, nullptr, (LPCSTR)address, (LPCSTR)message);

	PROTORECVEVENT pre = { 0 };
	pre.timestamp = now();
	pre.lParam = blob.size();
	pre.szMessage = blob;
	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
}

void CToxProto::OnFriendNameChange(Tox *tox, uint32_t friendNumber, const uint8_t *name, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	if (MCONTACT hContact = proto->GetContact(tox, friendNumber)) {
		ptrA rawName((char*)mir_alloc(length + 1));
		memcpy(rawName, name, length);
		rawName[length] = 0;

		ptrW nickname(mir_utf8decodeW(rawName));
		proto->setWString(hContact, "Nick", nickname);
	}
}

void CToxProto::OnStatusMessageChanged(Tox *tox, uint32_t friendNumber, const uint8_t *message, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	if (MCONTACT hContact = proto->GetContact(tox, friendNumber)) {
		ptrA rawMessage((char*)mir_alloc(length + 1));
		memcpy(rawMessage, message, length);
		rawMessage[length] = 0;

		ptrW statusMessage(mir_utf8decodeW(rawMessage));
		db_set_ws(hContact, "CList", "StatusMsg", statusMessage);
	}
}

void CToxProto::OnUserStatusChanged(Tox *tox, uint32_t friendNumber, TOX_USER_STATUS userstatus, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (hContact) {
		int status = proto->ToxToMirandaStatus(userstatus);
		proto->SetContactStatus(hContact, status);
	}
}

void CToxProto::OnConnectionStatusChanged(Tox *tox, uint32_t friendNumber, TOX_CONNECTION status, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (!hContact)
		return;

	if (status == TOX_CONNECTION_NONE) {
		proto->SetContactStatus(hContact, ID_STATUS_OFFLINE);
		return;
	}

	if (proto->GetContactStatus(hContact) != ID_STATUS_OFFLINE)
		return;

	proto->delSetting(hContact, "Auth");
	proto->delSetting(hContact, "Grant");

	// resume incoming transfers
	// proto->ResumeIncomingTransfers(friendNumber);

	// update avatar
	ptrW avatarPath(proto->GetAvatarFilePath());
	if (IsFileExists(avatarPath)) {
		FILE *hFile = _wfopen(avatarPath, L"rb");
		if (!hFile) {
			proto->debugLogA(__FUNCTION__": failed to open avatar file");
			return;
		}

		fseek(hFile, 0, SEEK_END);
		size_t length = ftell(hFile);
		rewind(hFile);

		uint8_t hash[TOX_HASH_LENGTH];
		DBVARIANT dbv;
		if (!db_get(NULL, proto->m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv)) {
			memcpy(hash, dbv.pbVal, TOX_HASH_LENGTH);
			db_free(&dbv);
		}

		proto->debugLogA(__FUNCTION__": send avatar to friend (%d)", friendNumber);

		TOX_ERR_FILE_SEND error;
		uint32_t fileNumber = tox_file_send(proto->m_tox, friendNumber, TOX_FILE_KIND_AVATAR, length, hash, nullptr, 0, &error);
		if (error != TOX_ERR_FILE_SEND_OK) {
			Netlib_Logf(proto->m_hNetlibUser, __FUNCTION__": failed to set new avatar");
			fclose(hFile);
			return;
		}

		AvatarTransferParam *transfer = new AvatarTransferParam(friendNumber, fileNumber, nullptr, length);
		transfer->pfts.flags |= PFTS_SENDING;
		memcpy(transfer->hash, hash, TOX_HASH_LENGTH);
		transfer->pfts.hContact = hContact;
		transfer->hFile = hFile;
		proto->transfers.Add(transfer);
	}
	else {
		proto->debugLogA(__FUNCTION__": unset avatar for friend (%d)", friendNumber);
		tox_file_send(proto->m_tox, friendNumber, TOX_FILE_KIND_AVATAR, 0, nullptr, nullptr, 0, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// User info dialog

class ToxUserInfoDlg : public CUserInfoPageDlg
{
	CToxProto *ppro;

public:
	ToxUserInfoDlg(CToxProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_USER_INFO),
		ppro(_ppro)
	{
	}

	bool OnRefresh() override
	{
		char *szProto = (m_hContact == NULL) ? ppro->m_szModuleName : Proto_GetBaseAccountName(m_hContact);
		if (szProto != nullptr)
			SetDlgItemText(m_hwnd, IDC_DNS_ID, ptrW(ppro->getWStringA(m_hContact, TOX_SETTINGS_DNS)));
		return false;
	}

	bool OnApply() override
	{
		char *szProto = (m_hContact == NULL) ? ppro->m_szModuleName : Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		wchar_t dnsId[MAX_PATH];
		GetDlgItemText(m_hwnd, IDC_DNS_ID, dnsId, MAX_PATH);
		ppro->setWString(m_hContact, TOX_SETTINGS_DNS, dnsId);
		return true;
	}
};

int CToxProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (!Proto_IsProtocolLoaded(m_szModuleName))
		return 0;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto != nullptr && !mir_strcmp(szProto, m_szModuleName)) {
		USERINFOPAGE uip = {};
		uip.flags = ODPF_UNICODE | ODPF_DONTTRANSLATE;
		uip.szTitle.w = m_tszUserName;
		uip.pDialog = new ToxUserInfoDlg(this);
		g_plugin.addUserInfo(wParam, &uip);
	}

	return 0;
}
