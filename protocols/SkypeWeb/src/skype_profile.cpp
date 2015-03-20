#include "common.h"

std::map<std::tstring, std::tstring> CSkypeProto::languages;

void CSkypeProto::InitLanguages()
{
	std::map<std::tstring, std::tstring> result;
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
	result[L"es"] = L"Castilian";
	result[L"ca"] = L"Catalan";
	result[L"km"] = L"Central Khmer";
	result[L"ch"] = L"Chamorro";
	result[L"ce"] = L"Chechen";
	result[L"ny"] = L"Chewa";
	result[L"ny"] = L"Chichewa";
	result[L"zh"] = L"Chinese";
	result[L"za"] = L"Chuang";
	result[L"cu"] = L"Church Slavic";
	result[L"cu"] = L"Church Slavonic";
	result[L"cv"] = L"Chuvash";
	result[L"kw"] = L"Cornish";
	result[L"co"] = L"Corsican";
	result[L"cr"] = L"Cree";
	result[L"hr"] = L"Croatian";
	result[L"cs"] = L"Czech";
	result[L"da"] = L"Danish";
	result[L"dv"] = L"Dhivehi";
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
	result[L"nl"] = L"Flemish";
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
	result[L"ht"] = L"Haitian";
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
	result[L"kl"] = L"Kalaallisut";
	result[L"kn"] = L"Kannada";
	result[L"kr"] = L"Kanuri";
	result[L"ks"] = L"Kashmiri";
	result[L"kk"] = L"Kazakh";
	result[L"ki"] = L"Kikuyu";
	result[L"rw"] = L"Kinyarwanda";
	result[L"ky"] = L"Kirghiz";
	result[L"kv"] = L"Komi";
	result[L"kg"] = L"Kongo";
	result[L"ko"] = L"Korean";
	result[L"kj"] = L"Kuanyama";
	result[L"ku"] = L"Kurdish";
	result[L"kj"] = L"Kwanyama";
	result[L"ky"] = L"Kyrgyz";
	result[L"lo"] = L"Lao";
	result[L"la"] = L"Latin";
	result[L"lv"] = L"Latvian";
	result[L"lb"] = L"Letzeburgesch";
	result[L"li"] = L"Limburgan";
	result[L"li"] = L"Limburger";
	result[L"li"] = L"Limburgish";
	result[L"ln"] = L"Lingala";
	result[L"lt"] = L"Lithuanian";
	result[L"lu"] = L"Luba-Katanga";
	result[L"lb"] = L"Luxembourgish";
	result[L"mk"] = L"Macedonian";
	result[L"mg"] = L"Malagasy";
	result[L"ms"] = L"Malay";
	result[L"ml"] = L"Malayalam";
	result[L"dv"] = L"Maldivian";
	result[L"mt"] = L"Maltese";
	result[L"gv"] = L"Manx";
	result[L"mi"] = L"Maori";
	result[L"mr"] = L"Marathi";
	result[L"mh"] = L"Marshallese";
	result[L"ro"] = L"Moldavian";
	result[L"ro"] = L"Moldovan";
	result[L"mn"] = L"Mongolian";
	result[L"na"] = L"Nauru";
	result[L"nv"] = L"Navaho";
	result[L"nv"] = L"Navajo";
	result[L"nd"] = L"Ndebele, North";
	result[L"nr"] = L"Ndebele, South";
	result[L"ng"] = L"Ndonga";
	result[L"ne"] = L"Nepali";
	result[L"nd"] = L"North Ndebele";
	result[L"se"] = L"Northern Sami";
	result[L"no"] = L"Norwegian";
	result[L"nb"] = L"Norwegian Bokmal";
	result[L"nn"] = L"Norwegian Nynorsk";
	result[L"ii"] = L"Nuosu";
	result[L"ny"] = L"Nyanja";
	result[L"nn"] = L"Nynorsk, Norwegian";
	result[L"ie"] = L"Occidental";
	result[L"oc"] = L"Occitan (post 1500)";
	result[L"oj"] = L"Ojibwa";
	result[L"cu"] = L"Old Bulgarian";
	result[L"cu"] = L"Old Church Slavonic";
	result[L"cu"] = L"Old Slavonic";
	result[L"or"] = L"Oriya";
	result[L"om"] = L"Oromo";
	result[L"os"] = L"Ossetian";
	result[L"os"] = L"Ossetic";
	result[L"pi"] = L"Pali";
	result[L"pa"] = L"Panjabi";
	result[L"ps"] = L"Pashto";
	result[L"fa"] = L"Persian";
	result[L"pl"] = L"Polish";
	result[L"pt"] = L"Portuguese";
	result[L"pa"] = L"Punjabi";
	result[L"ps"] = L"Pushto";
	result[L"qu"] = L"Quechua";
	result[L"ro"] = L"Romanian";
	result[L"rm"] = L"Romansh";
	result[L"rn"] = L"Rundi";
	result[L"ru"] = L"Russian";
	result[L"sm"] = L"Samoan";
	result[L"sg"] = L"Sango";
	result[L"sa"] = L"Sanskrit";
	result[L"sc"] = L"Sardinian";
	result[L"gd"] = L"Scottish Gaelic";
	result[L"sr"] = L"Serbian";
	result[L"sn"] = L"Shona";
	result[L"ii"] = L"Sichuan Yi";
	result[L"sd"] = L"Sindhi";
	result[L"si"] = L"Sinhala";
	result[L"si"] = L"Sinhalese";
	result[L"sk"] = L"Slovak";
	result[L"sl"] = L"Slovenian";
	result[L"so"] = L"Somali";
	result[L"st"] = L"Sotho, Southern";
	result[L"nr"] = L"South Ndebele";
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
	result[L"ug"] = L"Uyghur";
	result[L"uz"] = L"Uzbek";
	result[L"ca"] = L"Valencian";
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
	result[L"za"] = L"Zhuang";
	result[L"zu"] = L"Zulu";
}

void CSkypeProto::UpdateProfileFirstName(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "firstname");
	CMString firstname = ptrT(json_as_string(node));
	if (!firstname.IsEmpty() && firstname != "null")
		setTString(hContact, "FirstName", firstname);
	else
		delSetting(hContact, "FirstName");
}

void CSkypeProto::UpdateProfileLastName(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "lastname");
	CMString lastname = ptrT(json_as_string(node));
	if (!lastname.IsEmpty() && lastname != "null")
		setTString(hContact, "LastName", lastname);
	else
		delSetting(hContact, "LastName");
}

void CSkypeProto::UpdateProfileDisplayName(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "displayname");
	CMString displayname = ptrT(json_as_string(node));
	if (!displayname.IsEmpty() && displayname != "null")
		setTString(hContact, "Nick", displayname);
	else
		delSetting(hContact, "Nick");
}

void CSkypeProto::UpdateProfileGender(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "gender");
	CMString gender = ptrT(json_as_string(node));
	if (!gender.IsEmpty() && gender != "null")
		setByte(hContact, "Gender", (BYTE)(_ttoi(gender) == 1 ? 'M' : 'F'));
	else
		delSetting(hContact, "Gender");
}

void CSkypeProto::UpdateProfileBirthday(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "birthday");
	CMString birthday = ptrT(json_as_string(node));
	if (!birthday.IsEmpty() && birthday != "null") {
		int d, m, y;
		_stscanf(birthday.GetBuffer(), _T("%d-%d-%d"), &y, &m, &d);
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

void CSkypeProto::UpdateProfileCountry(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "country");
	CMStringA isocode = mir_t2a(ptrT(json_as_string(node)));
	if (!isocode.IsEmpty() && isocode != "null")
	{
		char *country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)(char*)isocode.GetBuffer(), 0);
		setTString(hContact, "Country", _A2T(country));
	}
	else
		delSetting(hContact, "Country");
}

void CSkypeProto::UpdateProfileState(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "province");
	CMString province = mir_t2a(ptrT(json_as_string(node)));
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "State", province);
	else
		delSetting(hContact, "State");
}

void CSkypeProto::UpdateProfileCity(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "city");
	CMString city = ptrT(json_as_string(node));
	if (!city.IsEmpty() && city != "null")
		setTString(hContact, "City", city);
	else
		delSetting(hContact, "City");
}

void CSkypeProto::UpdateProfileLanguage(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "language");
	CMString isocode = ptrT(json_as_string(node));
	if (!isocode.IsEmpty() && isocode != "null")
		setTString(hContact, "Language0", languages[isocode.GetBuffer()].c_str());
	else
		delSetting(hContact, "Language0");
}

void CSkypeProto::UpdateProfileHomepage(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "homepage");
	CMString homepage = ptrT(json_as_string(node));
	if (!homepage.IsEmpty() && homepage != "null")
		setTString(hContact, "Homepage", homepage);
	else
		delSetting(hContact, "Homepage");
}

void CSkypeProto::UpdateProfileAbout(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "about");
	CMString about = ptrT(json_as_string(node));
	if (!about.IsEmpty() && about != "null")
		setTString(hContact, "About", about);
	else
		delSetting(hContact, "About");
}

void CSkypeProto::UpdateProfileEmails(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "emails");
	if (!json_empty(node))
	{
		JSONNODE *items = json_as_array(node), *item;
		for (size_t i = 0; i < min(json_size(items), 3); i++)
		{
			item = json_at(items, i);
			if (item == NULL)
				break;

			CMStringA name(FORMAT, "e-mail%d", i);
			CMString value = ptrT(json_as_string(item));
			setTString(hContact, name, value);
		}
		json_delete(items);
	}
	else
	{
		delSetting(hContact, "e-mail0");
		delSetting(hContact, "e-mail1");
		delSetting(hContact, "e-mail2");
	}
}

void CSkypeProto::UpdateProfilePhoneMobile(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "phoneMobile");
	CMString province = mir_t2a(ptrT(json_as_string(node)));
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "Cellular", province);
	else
		delSetting(hContact, "Cellular");
}

void CSkypeProto::UpdateProfilePhoneHome(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "phone");
	CMString province = mir_t2a(ptrT(json_as_string(node)));
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "Phone", province);
	else
		delSetting(hContact, "Phone");
}

void CSkypeProto::UpdateProfilePhoneOffice(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "phoneOffice");
	CMString province = mir_t2a(ptrT(json_as_string(node)));
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "CompanyPhone", province);
	else
		delSetting(hContact, "CompanyPhone");
}

void CSkypeProto::UpdateProfileStatusMessage(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "mood");
	CMString province = mir_t2a(ptrT(json_as_string(node)));
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "StatusMsg", province);
	else
		delSetting(hContact, "StatusMsg");
}

void CSkypeProto::UpdateProfileXStatusMessage(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "richMood");
	CMString province = mir_t2a(ptrT(json_as_string(node)));
	if (!province.IsEmpty() && province != "null")
		setTString(hContact, "XStatusMsg", province);
	else
		delSetting(hContact, "XStatusMsg");
}

void CSkypeProto::UpdateProfileAvatar(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "avatarUrl");
	// add avatar support
}

//{"firstname":"Echo \/ Sound Test Service", "lastname" : null, "birthday" : null, "gender" : null, "country" : null, "city" : null, "language" : null, "homepage" : null, "about" : null, "province" : null, "jobtitle" : null, "emails" : [], "phoneMobile" : null, "phoneHome" : null, "phoneOffice" : null, "mood" : null, "richMood" : null, "avatarUrl" : null, "username" : "echo123"}
void CSkypeProto::LoadProfile(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	UpdateProfileFirstName(root);
	UpdateProfileLastName(root);
	UpdateProfileDisplayName(root);
	UpdateProfileGender(root);
	UpdateProfileBirthday(root);
	UpdateProfileCountry(root);
	UpdateProfileState(root);
	UpdateProfileCity(root);
	UpdateProfileLanguage(root);
	UpdateProfileHomepage(root);
	UpdateProfileAbout(root);
	//jobtitle
	UpdateProfileEmails(root);
	UpdateProfilePhoneMobile(root);
	UpdateProfilePhoneHome(root);
	UpdateProfilePhoneOffice(root);
	UpdateProfileStatusMessage(root);
	//richMood
	UpdateProfileAvatar(root);
}