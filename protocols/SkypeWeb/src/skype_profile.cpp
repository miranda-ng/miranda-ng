/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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

void CSkypeProto::UpdateProfileDisplayName(const JSONNode &root, MCONTACT hContact)
{
	ptrW firstname(getWStringA(hContact, "FirstName"));
	ptrW lastname(getWStringA(hContact, "LastName"));
	if (firstname) {
		CMStringW nick(firstname);
		if (lastname)
			nick.AppendFormat(L" %s", lastname.get());
		setWString(hContact, "Nick", nick);
	}
	else if (lastname)
		setWString(hContact, "Nick", lastname);
	else {
		const JSONNode &node = root["displayname"];
		SetString(hContact, "Nick", (node) ? node : root["username"]);
	}
}

void CSkypeProto::UpdateProfileGender(const JSONNode &root, MCONTACT hContact)
{
	CMStringW gender = root["gender"].as_mstring();
	if (!gender.IsEmpty() && gender != "null")
		setByte(hContact, "Gender", (uint8_t)(_wtoi(gender) == 1 ? 'M' : 'F'));
	else
		delSetting(hContact, "Gender");
}

void CSkypeProto::UpdateProfileBirthday(const JSONNode &root, MCONTACT hContact)
{
	CMStringW birthday = root["birthday"].as_mstring();
	if (!birthday.IsEmpty() && birthday != "null") {
		int d, m, y;
		if (3 == swscanf(birthday.GetBuffer(), L"%d-%d-%d", &y, &m, &d)) {
			setWord(hContact, "BirthYear", y);
			setByte(hContact, "BirthDay", d);
			setByte(hContact, "BirthMonth", m);
		}
	}

	delSetting(hContact, "BirthYear");
	delSetting(hContact, "BirthDay");
	delSetting(hContact, "BirthMonth");
}

void CSkypeProto::UpdateProfileCountry(const JSONNode &root, MCONTACT hContact)
{
	std::string isocode = root["country"].as_string();
	if (!isocode.empty() && isocode != "null") {
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode.c_str(), 0);
		setString(hContact, "Country", country);
	}
	else delSetting(hContact, "Country");
}

void CSkypeProto::UpdateProfileEmails(const JSONNode &root, MCONTACT hContact)
{
	const JSONNode &node = root["emails"];
	if (node) {
		const JSONNode &items = node.as_array();
		for (int i = 0; i < min(items.size(), 3); i++) {
			const JSONNode &item = items.at(i);
			if (!item)
				break;

			CMStringA name(FORMAT, "e-mail%d", i);
			setWString(hContact, name, item.as_mstring());
		}
	}
	else {
		delSetting(hContact, "e-mail0");
		delSetting(hContact, "e-mail1");
		delSetting(hContact, "e-mail2");
	}
}

void CSkypeProto::UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact)
{
	CMStringW szUrl = root["avatarUrl"].as_mstring();
	if (!szUrl.IsEmpty() && szUrl != "null") {
		SetAvatarUrl(hContact, szUrl);
		ReloadAvatarInfo(hContact);
	}
}

//{"firstname":"Echo \/ Sound Test Service", "lastname" : null, "birthday" : null, "gender" : null, "country" : null, "city" : null, "language" : null, "homepage" : null, "about" : null, "province" : null, "jobtitle" : null, "emails" : [], "phoneMobile" : null, "phoneHome" : null, "phoneOffice" : null, "mood" : null, "richMood" : null, "avatarUrl" : null, "username" : "echo123"}
void CSkypeProto::LoadProfile(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	MCONTACT hContact = (DWORD_PTR)pRequest->pUserInfo;

	JsonReply reply(response);
	if (reply.error()) {
		ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, 0);
		return;
	}

	auto &root = reply.data();
	std::string username = root["username"].as_string();
	if (username.empty()) {
		ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, 0);
		return;
	}

	if (m_szSkypename != username.c_str())
		m_szMyname = username.c_str();
	else
		m_szMyname = m_szSkypename;

	SetString(hContact, "City", root["city"]);
	SetString(hContact, "About", root["about"]);
	SetString(hContact, "Phone", root["phone"]);
	SetString(hContact, "State", root["province"]);
	SetString(hContact, "Cellular", root["phoneMobile"]);
	SetString(hContact, "Homepage", root["homepage"]);
	SetString(hContact, "LastName", root["lastname"]);
	SetString(hContact, "FirstName", root["firstname"]);
	SetString(hContact, "CompanyPhone", root["phoneOffice"]);

	if (auto &pMood = root["richMood"])
		RemoveHtml(pMood.as_mstring(), true); // this call extracts only emoji / mood id

	UpdateProfileDisplayName(root, hContact);
	UpdateProfileGender(root, hContact);
	UpdateProfileBirthday(root, hContact);
	UpdateProfileCountry(root, hContact);
	UpdateProfileEmails(root, hContact);
	UpdateProfileAvatar(root, hContact);

	ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0);
}
