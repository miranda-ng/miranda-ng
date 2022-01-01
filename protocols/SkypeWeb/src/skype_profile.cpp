/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

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

std::map<std::wstring, std::wstring> CSkypeProto::languages;

void CSkypeProto::InitLanguages()
{
	std::map<std::wstring, std::wstring> result;
	result[L"ab"] = L"Abkhazian";
	result[L"aa"] = L"Afar";
	result[L"af"] = L"Afrikaans";
	result[L"ak"] = L"Akan";
	result[L"sq"] = L"Albanian";
	result[L"am"] = L"Amharic";
	result[L"ar"] = L"Arabic";
	result[L"an"] = L"Aragonese";
	result[L"hy"] = L"Armenian";
	result[L"as"] = L"Assamese";
	result[L"av"] = L"Avaric";
	result[L"ae"] = L"Avestan";
	result[L"ay"] = L"Aymara";
	result[L"az"] = L"Azerbaijani";
	result[L"bm"] = L"Bambara";
	result[L"ba"] = L"Bashkir";
	result[L"eu"] = L"Basque";
	result[L"be"] = L"Belarusian";
	result[L"bn"] = L"Bengali";
	result[L"bh"] = L"Bihari languages";
	result[L"bi"] = L"Bislama";
	result[L"nb"] = L"Bokmal, Norwegian";
	result[L"bs"] = L"Bosnian";
	result[L"br"] = L"Breton";
	result[L"bg"] = L"Bulgarian";
	result[L"my"] = L"Burmese";
	result[L"ca"] = L"Catalan";
	result[L"km"] = L"Central Khmer";
	result[L"ch"] = L"Chamorro";
	result[L"ce"] = L"Chechen";
	result[L"ny"] = L"Chichewa";
	result[L"zh"] = L"Chinese";
	result[L"za"] = L"Chuang";
	result[L"cu"] = L"Church Slavic";
	result[L"cv"] = L"Chuvash";
	result[L"kw"] = L"Cornish";
	result[L"co"] = L"Corsican";
	result[L"cr"] = L"Cree";
	result[L"hr"] = L"Croatian";
	result[L"cs"] = L"Czech";
	result[L"da"] = L"Danish";
	result[L"dv"] = L"Divehi";
	result[L"nl"] = L"Dutch";
	result[L"dz"] = L"Dzongkha";
	result[L"en"] = L"English";
	result[L"eo"] = L"Esperanto";
	result[L"et"] = L"Estonian";
	result[L"ee"] = L"Ewe";
	result[L"fo"] = L"Faroese";
	result[L"fj"] = L"Fijian";
	result[L"fi"] = L"Finnish";
	result[L"fr"] = L"French";
	result[L"ff"] = L"Fulah";
	result[L"gd"] = L"Gaelic";
	result[L"gl"] = L"Galician";
	result[L"lg"] = L"Ganda";
	result[L"ka"] = L"Georgian";
	result[L"de"] = L"German";
	result[L"ki"] = L"Gikuyu";
	result[L"el"] = L"Greek, Modern (1453-)";
	result[L"kl"] = L"Greenlandic";
	result[L"gn"] = L"Guarani";
	result[L"gu"] = L"Gujarati";
	result[L"ht"] = L"Haitian Creole";
	result[L"ha"] = L"Hausa";
	result[L"he"] = L"Hebrew";
	result[L"hz"] = L"Herero";
	result[L"hi"] = L"Hindi";
	result[L"ho"] = L"Hiri Motu";
	result[L"hu"] = L"Hungarian";
	result[L"is"] = L"Icelandic";
	result[L"io"] = L"Ido";
	result[L"ig"] = L"Igbo";
	result[L"id"] = L"Indonesian";
	result[L"ia"] = L"Interlingua (International Auxiliary Language Association)";
	result[L"ie"] = L"Interlingue";
	result[L"iu"] = L"Inuktitut";
	result[L"ik"] = L"Inupiaq";
	result[L"ga"] = L"Irish";
	result[L"it"] = L"Italian";
	result[L"ja"] = L"Japanese";
	result[L"jv"] = L"Javanese";
	result[L"kn"] = L"Kannada";
	result[L"kr"] = L"Kanuri";
	result[L"ks"] = L"Kashmiri";
	result[L"kk"] = L"Kazakh";
	result[L"rw"] = L"Kinyarwanda";
	result[L"ky"] = L"Kirghiz";
	result[L"kv"] = L"Komi";
	result[L"kg"] = L"Kongo";
	result[L"ko"] = L"Korean";
	result[L"kj"] = L"Kuanyama";
	result[L"ku"] = L"Kurdish";
	result[L"lo"] = L"Lao";
	result[L"la"] = L"Latin";
	result[L"lv"] = L"Latvian";
	result[L"lb"] = L"Letzeburgesch";
	result[L"this"] = L"Limburgan";
	result[L"ln"] = L"Lingala";
	result[L"lt"] = L"Lithuanian";
	result[L"lu"] = L"Luba-Katanga";
	result[L"mk"] = L"Macedonian";
	result[L"mg"] = L"Malagasy";
	result[L"ms"] = L"Malay";
	result[L"ml"] = L"Malayalam";
	result[L"mt"] = L"Maltese";
	result[L"gv"] = L"Manx";
	result[L"mi"] = L"Maori";
	result[L"mr"] = L"Marathi";
	result[L"mh"] = L"Marshallese";
	result[L"mn"] = L"Mongolian";
	result[L"na"] = L"Nauru";
	result[L"nv"] = L"Navajo";
	result[L"nd"] = L"Ndebele, North";
	result[L"nr"] = L"Ndebele, South";
	result[L"ng"] = L"Ndonga";
	result[L"ne"] = L"Nepali";
	result[L"se"] = L"Northern Sami";
	result[L"no"] = L"Norwegian";
	result[L"nn"] = L"Norwegian Nynorsk";
	result[L"ii"] = L"Nuosu";
	result[L"oc"] = L"Occitan (post 1500)";
	result[L"oj"] = L"Ojibwa";
	result[L"or"] = L"Oriya";
	result[L"om"] = L"Oromo";
	result[L"os"] = L"Ossetic";
	result[L"pi"] = L"Pali";
	result[L"pa"] = L"Panjabi";
	result[L"ps"] = L"Pashto";
	result[L"fa"] = L"Persian";
	result[L"pl"] = L"Polish";
	result[L"pt"] = L"Portuguese";
	result[L"qu"] = L"Quechua";
	result[L"ro"] = L"Romanian";
	result[L"rm"] = L"Romansh";
	result[L"rn"] = L"Rundi";
	result[L"ru"] = L"Russian";
	result[L"sm"] = L"Samoan";
	result[L"sg"] = L"Sango";
	result[L"sa"] = L"Sanskrit";
	result[L"sc"] = L"Sardinian";
	result[L"sr"] = L"Serbian";
	result[L"sn"] = L"Shona";
	result[L"sd"] = L"Sindhi";
	result[L"si"] = L"Sinhalese";
	result[L"sk"] = L"Slovak";
	result[L"sl"] = L"Slovenian";
	result[L"so"] = L"Somali";
	result[L"st"] = L"Sotho, Southern";
	result[L"es"] = L"Spanish";
	result[L"su"] = L"Sundanese";
	result[L"sw"] = L"Swahili";
	result[L"ss"] = L"Swati";
	result[L"sv"] = L"Swedish";
	result[L"tl"] = L"Tagalog";
	result[L"ty"] = L"Tahitian";
	result[L"tg"] = L"Tajik";
	result[L"ta"] = L"Tamil";
	result[L"tt"] = L"Tatar";
	result[L"te"] = L"Telugu";
	result[L"th"] = L"Thai";
	result[L"bo"] = L"Tibetan";
	result[L"ti"] = L"Tigrinya";
	result[L"to"] = L"Tonga (Tonga Islands)";
	result[L"ts"] = L"Tsonga";
	result[L"tn"] = L"Tswana";
	result[L"tr"] = L"Turkish";
	result[L"tk"] = L"Turkmen";
	result[L"tw"] = L"Twi";
	result[L"ug"] = L"Uighur";
	result[L"uk"] = L"Ukrainian";
	result[L"ur"] = L"Urdu";
	result[L"uz"] = L"Uzbek";
	result[L"ve"] = L"Venda";
	result[L"vi"] = L"Vietnamese";
	result[L"vo"] = L"Volapuk";
	result[L"wa"] = L"Walloon";
	result[L"cy"] = L"Welsh";
	result[L"fy"] = L"Western Frisian";
	result[L"wo"] = L"Wolof";
	result[L"xh"] = L"Xhosa";
	result[L"yi"] = L"Yiddish";
	result[L"yo"] = L"Yoruba";
	result[L"zu"] = L"Zulu";
}

void CSkypeProto::UpdateProfileFirstName(const JSONNode &root, MCONTACT hContact)
{
	CMStringW firstname = root["firstname"].as_mstring();
	if (!firstname.IsEmpty() && firstname != "null")
		setWString(hContact, "FirstName", firstname);
	else
		delSetting(hContact, "FirstName");
}

void CSkypeProto::UpdateProfileLastName(const JSONNode &root, MCONTACT hContact)
{
	CMStringW lastname = root["lastname"].as_mstring();
	if (!lastname.IsEmpty() && lastname != "null")
		setWString(hContact, "LastName", lastname);
	else
		delSetting(hContact, "LastName");
}

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
		CMStringW displayname((!node) ? root["username"].as_mstring() : node.as_mstring());
		if (!displayname.IsEmpty() && displayname != "null")
			setWString(hContact, "Nick", displayname);
		else
			delSetting(hContact, "Nick");
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
		swscanf(birthday.GetBuffer(), L"%d-%d-%d", &y, &m, &d);
		setWord(hContact, "BirthYear", y);
		setByte(hContact, "BirthDay", d);
		setByte(hContact, "BirthMonth", m);
	}
	else {
		delSetting(hContact, "BirthYear");
		delSetting(hContact, "BirthDay");
		delSetting(hContact, "BirthMonth");
	}
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

void CSkypeProto::UpdateProfileState(const JSONNode &root, MCONTACT hContact)
{
	CMStringW province = root["province"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setWString(hContact, "State", province);
	else
		delSetting(hContact, "State");
}

void CSkypeProto::UpdateProfileCity(const JSONNode &root, MCONTACT hContact)
{
	CMStringW city = root["city"].as_mstring();
	if (!city.IsEmpty() && city != "null")
		setWString(hContact, "City", city);
	else
		delSetting(hContact, "City");
}

void CSkypeProto::UpdateProfileLanguage(const JSONNode &root, MCONTACT hContact)
{
	CMStringW isocode = root["language"].as_mstring();
	if (!isocode.IsEmpty() && isocode != "null")
		setWString(hContact, "Language0", languages[isocode.GetBuffer()].c_str());
	else
		delSetting(hContact, "Language0");
}

void CSkypeProto::UpdateProfileHomepage(const JSONNode &root, MCONTACT hContact)
{
	CMStringW homepage = root["homepage"].as_mstring();
	if (!homepage.IsEmpty() && homepage != "null")
		setWString(hContact, "Homepage", homepage);
	else
		delSetting(hContact, "Homepage");
}

void CSkypeProto::UpdateProfileAbout(const JSONNode &root, MCONTACT hContact)
{
	CMStringW about = root["about"].as_mstring();
	if (!about.IsEmpty() && about != "null")
		setWString(hContact, "About", about);
	else
		delSetting(hContact, "About");
}

void CSkypeProto::UpdateProfileEmails(const JSONNode &root, MCONTACT hContact)
{
	const JSONNode &node = root["emails"];
	if (node) {
		const JSONNode &items = node.as_array();
		for (size_t i = 0; i < min(items.size(), 3); i++) {
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

void CSkypeProto::UpdateProfilePhoneMobile(const JSONNode &root, MCONTACT hContact)
{
	CMStringW province = root["phoneMobile"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setWString(hContact, "Cellular", province);
	else
		delSetting(hContact, "Cellular");
}

void CSkypeProto::UpdateProfilePhoneHome(const JSONNode &root, MCONTACT hContact)
{
	CMStringW province = root["phone"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setWString(hContact, "Phone", province);
	else
		delSetting(hContact, "Phone");
}

void CSkypeProto::UpdateProfilePhoneOffice(const JSONNode &root, MCONTACT hContact)
{
	CMStringW province = root["phoneOffice"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setWString(hContact, "CompanyPhone", province);
	else
		delSetting(hContact, "CompanyPhone");
}

void CSkypeProto::UpdateProfileXStatusMessage(const JSONNode &root, MCONTACT hContact)
{
	CMStringW province = root["richMood"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setWString(hContact, "XStatusMsg", province);
	else
		delSetting(hContact, "XStatusMsg");
}

void CSkypeProto::UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact)
{
	CMStringW province = root["avatarUrl"].as_mstring();
	if (!province.IsEmpty() && province != "null") {
		SetAvatarUrl(hContact, province);
		ReloadAvatarInfo(hContact);
	}
}

//{"firstname":"Echo \/ Sound Test Service", "lastname" : null, "birthday" : null, "gender" : null, "country" : null, "city" : null, "language" : null, "homepage" : null, "about" : null, "province" : null, "jobtitle" : null, "emails" : [], "phoneMobile" : null, "phoneHome" : null, "phoneOffice" : null, "mood" : null, "richMood" : null, "avatarUrl" : null, "username" : "echo123"}
void CSkypeProto::LoadProfile(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest)
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

	UpdateProfileFirstName(root, hContact);
	UpdateProfileLastName(root, hContact);
	UpdateProfileDisplayName(root, hContact);
	UpdateProfileGender(root, hContact);
	UpdateProfileBirthday(root, hContact);
	UpdateProfileCountry(root, hContact);
	UpdateProfileState(root, hContact);
	UpdateProfileCity(root, hContact);
	UpdateProfileLanguage(root, hContact);
	UpdateProfileHomepage(root, hContact);
	UpdateProfileAbout(root, hContact);
	//jobtitle
	UpdateProfileEmails(root, hContact);
	UpdateProfilePhoneMobile(root, hContact);
	UpdateProfilePhoneHome(root, hContact);
	UpdateProfilePhoneOffice(root, hContact);
	//richMood
	UpdateProfileAvatar(root, hContact);

	ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, 0);
}
