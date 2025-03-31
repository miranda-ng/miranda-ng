/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

uint16_t CTeamsProto::GetContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

void CTeamsProto::SetContactStatus(MCONTACT hContact, uint16_t status)
{
	uint16_t oldStatus = GetContactStatus(hContact);
	if (oldStatus != status) {
		setWord(hContact, "Status", status);
		if (status == ID_STATUS_OFFLINE)
			delSetting(hContact, "MirVer");
	}
}

void CTeamsProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	ptrW tszChatID(getWStringA(hContact, DBKEY_ID));
	if (tszChatID != NULL)
		Chat_Control(Chat_Find(tszChatID, m_szModuleName), (iStatus == ID_STATUS_OFFLINE) ? SESSION_OFFLINE : SESSION_ONLINE);
}

MCONTACT CTeamsProto::GetContactFromAuthEvent(MEVENT hEvent)
{
	uint32_t body[3];
	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(uint32_t) * 2;
	dbei.pBlob = (char *)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (mir_strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;
	return DbGetAuthEventContact(&dbei);
}

MCONTACT CTeamsProto::FindContact(const char *skypeId)
{
	for (auto &hContact : AccContacts())
		if (!mir_strcmpi(skypeId, ptrA(getUStringA(hContact, DBKEY_ID))))
			return hContact;

	return 0;
}

MCONTACT CTeamsProto::FindContact(const wchar_t *skypeId)
{
	for (auto &hContact : AccContacts())
		if (!mir_wstrcmpi(skypeId, getMStringW(hContact, DBKEY_ID)))
			return hContact;

	return 0;
}

MCONTACT CTeamsProto::AddContact(const char *skypeId, const char *nick, bool isTemporary)
{
	MCONTACT hContact = FindContact(skypeId);
	if (hContact)
		return hContact;

	hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	setString(hContact, DBKEY_ID, skypeId);
	setUString(hContact, "Nick", (nick) ? nick : GetSkypeNick(skypeId));

	if (m_wstrCListGroup) {
		Clist_GroupCreate(0, m_wstrCListGroup);
		Clist_SetGroup(hContact, m_wstrCListGroup);
	}

	setByte(hContact, "Auth", 1);
	setByte(hContact, "Grant", 1);

	if (isTemporary)
		Contact::RemoveFromList(hContact);
	return hContact;
}

void CTeamsProto::LoadContactsAuth(MHttpResponse *response, AsyncHttpRequest*)
{
	SkypeReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	for (auto &item : root["invite_list"]) {
		std::string skypeId = item["mri"].as_string();
		std::string reason = item["greeting"].as_string();

		time_t eventTime = 0;
		for (auto &it : item["invites"])
			eventTime = IsoToUnixTime(it["time"].as_string());

		std::string displayName = item["displayname"].as_string();
		const char *szNick = (displayName.empty()) ? nullptr : displayName.c_str();

		MCONTACT hContact = AddContact(skypeId.c_str(), szNick);
		time_t lastEventTime = getDword(hContact, "LastAuthRequestTime");
		if (lastEventTime && lastEventTime >= eventTime)
			continue;

		setUString(hContact, "Nick", displayName.c_str());

		setDword(hContact, "LastAuthRequestTime", eventTime);
		delSetting(hContact, "Auth");

		DB::AUTH_BLOB blob(hContact, displayName.c_str(), nullptr, nullptr, skypeId.c_str(), reason.c_str());

		DB::EventInfo dbei;
		dbei.iTimestamp = time(0);
		dbei.cbBlob = blob.size();
		dbei.pBlob = blob;
		ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&dbei);
	}
}

void CTeamsProto::LoadContactList(MHttpResponse *response, AsyncHttpRequest*)
{
	SkypeReply reply(response);
	if (reply.error())
		return;

	auto &root = reply.data();
	for (auto &item : root["contacts"]) {
		CMStringA szSkypeId = item["person_id"].as_mstring();
		if (!IsPossibleUserType(szSkypeId))
			continue;

		MCONTACT hContact = AddContact(szSkypeId, nullptr);
		if (szSkypeId == "28:e7a9407c-2467-4a04-9546-70081f4ea80d")
			m_hMyContact = hContact;

		std::string displayName = item["display_name"].as_string();
		if (!displayName.empty()) {
			if (m_hMyContact == hContact) {
				displayName = getMStringU("Nick");
				displayName += " ";
				displayName += TranslateU("(You)");

				setWord(hContact, "Status", ID_STATUS_ONLINE);
			}
			setUString(hContact, "Nick", displayName.c_str());
		}

		if (item["authorized"].as_bool()) {
			delSetting(hContact, "Auth");
			delSetting(hContact, "Grant");
		}
		else setByte(hContact, "Grant", 1);

		if (item["blocked"].as_bool())
			setByte(hContact, "IsBlocked", 1);
		else
			delSetting(hContact, "IsBlocked");

		ptrW wszGroup(Clist_GetGroup(hContact));
		if (wszGroup == nullptr && m_wstrCListGroup) {
			Clist_GroupCreate(0, m_wstrCListGroup);
			Clist_SetGroup(hContact, m_wstrCListGroup);
		}

		auto &profile = item["profile"];
		SetString(hContact, "Homepage", profile["website"]);

		auto wstr = profile["birthday"].as_mstring();
		if (!wstr.IsEmpty() ) {
			int nYear, nMonth, nDay;
			if (swscanf(wstr, L"%d-%d-%d", &nYear, &nMonth, &nDay) == 3)
				Contact::SetBirthday(hContact, nDay, nMonth, nYear);
		}

		wstr = profile["gender"].as_mstring();
		if (wstr == "male")
			setByte(hContact, "Gender", 'M');
		else if (wstr == "female")
			setByte(hContact, "Gender", 'F');

		auto &name = profile["name"];
		SetString(hContact, "FirstName", name["first"]);
		SetString(hContact, "LastName", name["surname"]);

		if (auto &pMood = profile["mood"])
			db_set_ws(hContact, "CList", "StatusMsg", RemoveHtml(pMood.as_mstring()));

		SetAvatarUrl(hContact, profile["avatar_url"].as_mstring());
		ReloadAvatarInfo(hContact);

		for (auto &phone : profile["phones"]) {
			CMStringW number = phone["number"].as_mstring();

			auto wszType = phone["type"].as_mstring();
			if (wszType == L"mobile")
				setWString(hContact, "Cellular", number);
			else if (wszType == L"phone")
				setWString(hContact, "Phone", number);
		}
	}

	PushRequest(new GetContactsAuthRequest());
}

INT_PTR CTeamsProto::OnRequestAuth(WPARAM hContact, LPARAM)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AddContactRequest(getId(hContact)));
	return 0;
}

INT_PTR CTeamsProto::OnGrantAuth(WPARAM hContact, LPARAM)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	PushRequest(new AuthAcceptRequest(getId(hContact)));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CTeamsProto::OnContactDeleted(MCONTACT hContact, uint32_t flags)
{
	if (IsOnline() && hContact && (flags & CDF_DEL_CONTACT)) {
		CMStringA szId(getId(hContact));
		if (isChatRoom(hContact))
			KickChatUser(szId, m_szOwnSkypeId);
		else
			PushRequest(new AsyncHttpRequest(REQUEST_DELETE, HOST_CONTACTS, "/users/SELF/contacts/" + mir_urlEncode(szId)));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CTeamsProto::BlockContact(WPARAM hContact, LPARAM)
{
	if (!IsOnline()) return 1;

	if (IDYES == MessageBox(NULL, TranslateT("Are you sure?"), TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION))
		PushRequest(new BlockContactRequest(this, hContact));
	return 0;
}

void CTeamsProto::OnBlockContact(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	MCONTACT hContact = (DWORD_PTR)pRequest->pUserInfo;
	if (response != nullptr)
		Contact::Hide(hContact);
}

INT_PTR CTeamsProto::UnblockContact(WPARAM hContact, LPARAM)
{
	PushRequest(new UnblockContactRequest(this, hContact));
	return 0;
}

void CTeamsProto::OnUnblockContact(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	if (response == nullptr)
		return;

	MCONTACT hContact = (DWORD_PTR)pRequest->pUserInfo;
	Contact::Hide(hContact, false);
	delSetting(hContact, "IsBlocked");
}
