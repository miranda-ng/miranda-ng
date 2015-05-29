/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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

std::map<std::tstring, std::tstring> CSkypeProto::languages;

void CSkypeProto::InitLanguages()
{
	std::map<std::tstring, std::tstring> result;
	result[_T("ab")] = _T("Abkhazian");
	result[_T("aa")] = _T("Afar");
	result[_T("af")] = _T("Afrikaans");
	result[_T("ak")] = _T("Akan");
	result[_T("sq")] = _T("Albanian");
	result[_T("am")] = _T("Amharic");
	result[_T("ar")] = _T("Arabic");
	result[_T("an")] = _T("Aragonese");
	result[_T("hy")] = _T("Armenian");
	result[_T("as")] = _T("Assamese");
	result[_T("av")] = _T("Avaric");
	result[_T("ae")] = _T("Avestan");
	result[_T("ay")] = _T("Aymara");
	result[_T("az")] = _T("Azerbaijani");
	result[_T("bm")] = _T("Bambara");
	result[_T("ba")] = _T("Bashkir");
	result[_T("eu")] = _T("Basque");
	result[_T("be")] = _T("Belarusian");
	result[_T("bn")] = _T("Bengali");
	result[_T("bh")] = _T("Bihari languages");
	result[_T("bi")] = _T("Bislama");
	result[_T("nb")] = _T("Bokmal, Norwegian");
	result[_T("bs")] = _T("Bosnian");
	result[_T("br")] = _T("Breton");
	result[_T("bg")] = _T("Bulgarian");
	result[_T("my")] = _T("Burmese");
	result[_T("es")] = _T("Castilian");
	result[_T("ca")] = _T("Catalan");
	result[_T("km")] = _T("Central Khmer");
	result[_T("ch")] = _T("Chamorro");
	result[_T("ce")] = _T("Chechen");
	result[_T("ny")] = _T("Chewa");
	result[_T("ny")] = _T("Chichewa");
	result[_T("zh")] = _T("Chinese");
	result[_T("za")] = _T("Chuang");
	result[_T("cu")] = _T("Church Slavic");
	result[_T("cu")] = _T("Church Slavonic");
	result[_T("cv")] = _T("Chuvash");
	result[_T("kw")] = _T("Cornish");
	result[_T("co")] = _T("Corsican");
	result[_T("cr")] = _T("Cree");
	result[_T("hr")] = _T("Croatian");
	result[_T("cs")] = _T("Czech");
	result[_T("da")] = _T("Danish");
	result[_T("dv")] = _T("Dhivehi");
	result[_T("dv")] = _T("Divehi");
	result[_T("nl")] = _T("Dutch");
	result[_T("dz")] = _T("Dzongkha");
	result[_T("en")] = _T("English");
	result[_T("eo")] = _T("Esperanto");
	result[_T("et")] = _T("Estonian");
	result[_T("ee")] = _T("Ewe");
	result[_T("fo")] = _T("Faroese");
	result[_T("fj")] = _T("Fijian");
	result[_T("fi")] = _T("Finnish");
	result[_T("nl")] = _T("Flemish");
	result[_T("fr")] = _T("French");
	result[_T("ff")] = _T("Fulah");
	result[_T("gd")] = _T("Gaelic");
	result[_T("gl")] = _T("Galician");
	result[_T("lg")] = _T("Ganda");
	result[_T("ka")] = _T("Georgian");
	result[_T("de")] = _T("German");
	result[_T("ki")] = _T("Gikuyu");
	result[_T("el")] = _T("Greek, Modern (1453-)");
	result[_T("kl")] = _T("Greenlandic");
	result[_T("gn")] = _T("Guarani");
	result[_T("gu")] = _T("Gujarati");
	result[_T("ht")] = _T("Haitian");
	result[_T("ht")] = _T("Haitian Creole");
	result[_T("ha")] = _T("Hausa");
	result[_T("he")] = _T("Hebrew");
	result[_T("hz")] = _T("Herero");
	result[_T("hi")] = _T("Hindi");
	result[_T("ho")] = _T("Hiri Motu");
	result[_T("hu")] = _T("Hungarian");
	result[_T("is")] = _T("Icelandic");
	result[_T("io")] = _T("Ido");
	result[_T("ig")] = _T("Igbo");
	result[_T("id")] = _T("Indonesian");
	result[_T("ia")] = _T("Interlingua (International Auxiliary Language Association)");
	result[_T("ie")] = _T("Interlingue");
	result[_T("iu")] = _T("Inuktitut");
	result[_T("ik")] = _T("Inupiaq");
	result[_T("ga")] = _T("Irish");
	result[_T("it")] = _T("Italian");
	result[_T("ja")] = _T("Japanese");
	result[_T("jv")] = _T("Javanese");
	result[_T("kl")] = _T("Kalaallisut");
	result[_T("kn")] = _T("Kannada");
	result[_T("kr")] = _T("Kanuri");
	result[_T("ks")] = _T("Kashmiri");
	result[_T("kk")] = _T("Kazakh");
	result[_T("ki")] = _T("Kikuyu");
	result[_T("rw")] = _T("Kinyarwanda");
	result[_T("ky")] = _T("Kirghiz");
	result[_T("kv")] = _T("Komi");
	result[_T("kg")] = _T("Kongo");
	result[_T("ko")] = _T("Korean");
	result[_T("kj")] = _T("Kuanyama");
	result[_T("ku")] = _T("Kurdish");
	result[_T("kj")] = _T("Kwanyama");
	result[_T("ky")] = _T("Kyrgyz");
	result[_T("lo")] = _T("Lao");
	result[_T("la")] = _T("Latin");
	result[_T("lv")] = _T("Latvian");
	result[_T("lb")] = _T("Letzeburgesch");
	result[_T("li")] = _T("Limburgan");
	result[_T("li")] = _T("Limburger");
	result[_T("li")] = _T("Limburgish");
	result[_T("ln")] = _T("Lingala");
	result[_T("lt")] = _T("Lithuanian");
	result[_T("lu")] = _T("Luba-Katanga");
	result[_T("lb")] = _T("Luxembourgish");
	result[_T("mk")] = _T("Macedonian");
	result[_T("mg")] = _T("Malagasy");
	result[_T("ms")] = _T("Malay");
	result[_T("ml")] = _T("Malayalam");
	result[_T("dv")] = _T("Maldivian");
	result[_T("mt")] = _T("Maltese");
	result[_T("gv")] = _T("Manx");
	result[_T("mi")] = _T("Maori");
	result[_T("mr")] = _T("Marathi");
	result[_T("mh")] = _T("Marshallese");
	result[_T("ro")] = _T("Moldavian");
	result[_T("ro")] = _T("Moldovan");
	result[_T("mn")] = _T("Mongolian");
	result[_T("na")] = _T("Nauru");
	result[_T("nv")] = _T("Navaho");
	result[_T("nv")] = _T("Navajo");
	result[_T("nd")] = _T("Ndebele, North");
	result[_T("nr")] = _T("Ndebele, South");
	result[_T("ng")] = _T("Ndonga");
	result[_T("ne")] = _T("Nepali");
	result[_T("nd")] = _T("North Ndebele");
	result[_T("se")] = _T("Northern Sami");
	result[_T("no")] = _T("Norwegian");
	result[_T("nb")] = _T("Norwegian Bokmal");
	result[_T("nn")] = _T("Norwegian Nynorsk");
	result[_T("ii")] = _T("Nuosu");
	result[_T("ny")] = _T("Nyanja");
	result[_T("nn")] = _T("Nynorsk, Norwegian");
	result[_T("ie")] = _T("Occidental");
	result[_T("oc")] = _T("Occitan (post 1500)");
	result[_T("oj")] = _T("Ojibwa");
	result[_T("cu")] = _T("Old Bulgarian");
	result[_T("cu")] = _T("Old Church Slavonic");
	result[_T("cu")] = _T("Old Slavonic");
	result[_T("or")] = _T("Oriya");
	result[_T("om")] = _T("Oromo");
	result[_T("os")] = _T("Ossetian");
	result[_T("os")] = _T("Ossetic");
	result[_T("pi")] = _T("Pali");
	result[_T("pa")] = _T("Panjabi");
	result[_T("ps")] = _T("Pashto");
	result[_T("fa")] = _T("Persian");
	result[_T("pl")] = _T("Polish");
	result[_T("pt")] = _T("Portuguese");
	result[_T("pa")] = _T("Punjabi");
	result[_T("ps")] = _T("Pushto");
	result[_T("qu")] = _T("Quechua");
	result[_T("ro")] = _T("Romanian");
	result[_T("rm")] = _T("Romansh");
	result[_T("rn")] = _T("Rundi");
	result[_T("ru")] = _T("Russian");
	result[_T("sm")] = _T("Samoan");
	result[_T("sg")] = _T("Sango");
	result[_T("sa")] = _T("Sanskrit");
	result[_T("sc")] = _T("Sardinian");
	result[_T("gd")] = _T("Scottish Gaelic");
	result[_T("sr")] = _T("Serbian");
	result[_T("sn")] = _T("Shona");
	result[_T("ii")] = _T("Sichuan Yi");
	result[_T("sd")] = _T("Sindhi");
	result[_T("si")] = _T("Sinhala");
	result[_T("si")] = _T("Sinhalese");
	result[_T("sk")] = _T("Slovak");
	result[_T("sl")] = _T("Slovenian");
	result[_T("so")] = _T("Somali");
	result[_T("st")] = _T("Sotho, Southern");
	result[_T("nr")] = _T("South Ndebele");
	result[_T("es")] = _T("Spanish");
	result[_T("su")] = _T("Sundanese");
	result[_T("sw")] = _T("Swahili");
	result[_T("ss")] = _T("Swati");
	result[_T("sv")] = _T("Swedish");
	result[_T("tl")] = _T("Tagalog");
	result[_T("ty")] = _T("Tahitian");
	result[_T("tg")] = _T("Tajik");
	result[_T("ta")] = _T("Tamil");
	result[_T("tt")] = _T("Tatar");
	result[_T("te")] = _T("Telugu");
	result[_T("th")] = _T("Thai");
	result[_T("bo")] = _T("Tibetan");
	result[_T("ti")] = _T("Tigrinya");
	result[_T("to")] = _T("Tonga (Tonga Islands)");
	result[_T("ts")] = _T("Tsonga");
	result[_T("tn")] = _T("Tswana");
	result[_T("tr")] = _T("Turkish");
	result[_T("tk")] = _T("Turkmen");
	result[_T("tw")] = _T("Twi");
	result[_T("ug")] = _T("Uighur");
	result[_T("uk")] = _T("Ukrainian");
	result[_T("ur")] = _T("Urdu");
	result[_T("ug")] = _T("Uyghur");
	result[_T("uz")] = _T("Uzbek");
	result[_T("ca")] = _T("Valencian");
	result[_T("ve")] = _T("Venda");
	result[_T("vi")] = _T("Vietnamese");
	result[_T("vo")] = _T("Volapuk");
	result[_T("wa")] = _T("Walloon");
	result[_T("cy")] = _T("Welsh");
	result[_T("fy")] = _T("Western Frisian");
	result[_T("wo")] = _T("Wolof");
	result[_T("xh")] = _T("Xhosa");
	result[_T("yi")] = _T("Yiddish");
	result[_T("yo")] = _T("Yoruba");
	result[_T("za")] = _T("Zhuang");
	result[_T("zu")] = _T("Zulu");
}

void CSkypeProto::UpdateProfileFirstName(const JSONNode &root, MCONTACT hContact)
{
	CMString firstname = root["firstname"].as_mstring();
	if (!firstname.IsEmpty() && firstname != "null")
		setTString(hContact, "FirstName", firstname);
	else
		delSetting(hContact, "FirstName");
}

void CSkypeProto::UpdateProfileLastName(const JSONNode &root, MCONTACT hContact)
{
	CMString lastname = root["lastname"].as_mstring();
	if (!lastname.IsEmpty() && lastname != "null")
		setTString(hContact, "LastName", lastname);
	else
		delSetting(hContact, "LastName");
}

void CSkypeProto::UpdateProfileDisplayName(const JSONNode &root, MCONTACT hContact)
{
	ptrT firstname(getTStringA(hContact, "FirstName"));
	ptrT lastname(getTStringA(hContact, "LastName"));
	if (firstname) {
		CMString nick = firstname;
		if (lastname)
			nick.AppendFormat(_T(" %s"), lastname);
		setTString(hContact, "Nick", nick);
	}
	else if (lastname)
		setTString(hContact, "Nick", lastname);
	else {
		
		const JSONNode &node = root["displayname"];
		CMString displayname((!node) ? root["username"].as_mstring() : node.as_mstring());
		if (!displayname.IsEmpty() && displayname != "null")
			setTString(hContact, "Nick", displayname);
		else
			delSetting(hContact, "Nick");
	}
}

void CSkypeProto::UpdateProfileGender(const JSONNode &root, MCONTACT hContact)
{
	CMString gender = root["gender"].as_mstring();
	if (!gender.IsEmpty() && gender != "null")
		setByte(hContact, "Gender", (BYTE)(_ttoi(gender) == 1 ? 'M' : 'F'));
	else
		delSetting(hContact, "Gender");
}

void CSkypeProto::UpdateProfileBirthday(const JSONNode &root, MCONTACT hContact)
{
	CMString birthday = root["birthday"].as_mstring();
	if (!birthday.IsEmpty() && birthday != "null")
	{
		int d, m, y;
		_stscanf(birthday.GetBuffer(), _T("%d-%d-%d"), &y, &m, &d);
		setWord(hContact, "BirthYear", y);
		setByte(hContact, "BirthDay", d);
		setByte(hContact, "BirthMonth", m);
	}
	else
	{
		delSetting(hContact, "BirthYear");
		delSetting(hContact, "BirthDay");
		delSetting(hContact, "BirthMonth");
	}
}

void CSkypeProto::UpdateProfileCountry(const JSONNode &root, MCONTACT hContact)
{
	std::string isocode = root["country"].as_string();
	if (!isocode.empty() && isocode != "null")
	{
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode.c_str(), 0);
		setString(hContact, "Country", country);
	}
	else delSetting(hContact, "Country");
}

void CSkypeProto::UpdateProfileState(const JSONNode &root, MCONTACT hContact)
{
	CMString province = root["province"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "State", province);
	else
		delSetting(hContact, "State");
}

void CSkypeProto::UpdateProfileCity(const JSONNode &root, MCONTACT hContact)
{
	CMString city = root["city"].as_mstring();
	if (!city.IsEmpty() && city != "null")
		setTString(hContact, "City", city);
	else
		delSetting(hContact, "City");
}

void CSkypeProto::UpdateProfileLanguage(const JSONNode &root, MCONTACT hContact)
{
	CMString isocode = root["language"].as_mstring();
	if (!isocode.IsEmpty() && isocode != "null")
		setTString(hContact, "Language0", languages[isocode.GetBuffer()].c_str());
	else
		delSetting(hContact, "Language0");
}

void CSkypeProto::UpdateProfileHomepage(const JSONNode &root, MCONTACT hContact)
{
	CMString homepage = root["homepage"].as_mstring();
	if (!homepage.IsEmpty() && homepage != "null")
		setTString(hContact, "Homepage", homepage);
	else
		delSetting(hContact, "Homepage");
}

void CSkypeProto::UpdateProfileAbout(const JSONNode &root, MCONTACT hContact)
{
	CMString about = root["about"].as_mstring();
	if (!about.IsEmpty() && about != "null")
		setTString(hContact, "About", about);
	else
		delSetting(hContact, "About");
}

void CSkypeProto::UpdateProfileEmails(const JSONNode &root, MCONTACT hContact)
{
	const JSONNode &node = root["emails"];
	if (node)
	{
		const JSONNode &items = node.as_array();
		for (size_t i = 0; i < min(items.size(), 3); i++)
		{
			const JSONNode &item = items.at(i);
			if (!item)
				break;

			CMStringA name(FORMAT, "e-mail%d", i);
			setTString(hContact, name, item.as_mstring());
		}
	}
	else
	{
		delSetting(hContact, "e-mail0");
		delSetting(hContact, "e-mail1");
		delSetting(hContact, "e-mail2");
	}
}

void CSkypeProto::UpdateProfilePhoneMobile(const JSONNode &root, MCONTACT hContact)
{
	CMString province = root["phoneMobile"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "Cellular", province);
	else
		delSetting(hContact, "Cellular");
}

void CSkypeProto::UpdateProfilePhoneHome(const JSONNode &root, MCONTACT hContact)
{
	CMString province = root["phone"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "Phone", province);
	else
		delSetting(hContact, "Phone");
}

void CSkypeProto::UpdateProfilePhoneOffice(const JSONNode &root, MCONTACT hContact)
{
	CMString province = root["phoneOffice"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "CompanyPhone", province);
	else
		delSetting(hContact, "CompanyPhone");
}

void CSkypeProto::UpdateProfileStatusMessage(const JSONNode &root, MCONTACT hContact)
{
	if (hContact == NULL)
		return;

	CMString province = root["mood"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		db_set_ts(hContact, "CList", "StatusMsg", province);
	else
		db_unset(hContact, "CList", "StatusMsg");
}

void CSkypeProto::UpdateProfileXStatusMessage(const JSONNode &root, MCONTACT hContact)
{
	CMString province = root["richMood"].as_mstring();
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "XStatusMsg", province);
	else
		delSetting(hContact, "XStatusMsg");
}

void CSkypeProto::UpdateProfileAvatar(const JSONNode &root, MCONTACT hContact)
{
	CMString province = root["avatarUrl"].as_mstring();
	if (!province.IsEmpty() && province != "null")
	{
		SetAvatarUrl(hContact, province);
		ReloadAvatarInfo(hContact);
	}
}

//{"firstname":"Echo \/ Sound Test Service", "lastname" : null, "birthday" : null, "gender" : null, "country" : null, "city" : null, "language" : null, "homepage" : null, "about" : null, "province" : null, "jobtitle" : null, "emails" : [], "phoneMobile" : null, "phoneHome" : null, "phoneOffice" : null, "mood" : null, "richMood" : null, "avatarUrl" : null, "username" : "echo123"}
void CSkypeProto::LoadProfile(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	std::string username = root["username"].as_string();
	MCONTACT hContact = NULL;
	if (!IsMe(username.c_str()))
		hContact = FindContact(username.c_str());

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
	UpdateProfileStatusMessage(root, hContact);
	//richMood
	UpdateProfileAvatar(root, hContact);
}
