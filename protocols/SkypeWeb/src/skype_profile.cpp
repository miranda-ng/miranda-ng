#include "common.h"

struct { TCHAR *szCode; TCHAR *szDescription; } g_LanguageCodes[] = {
	{ _T("aa"), LPGENT("Afar") },
	{ _T("ab"), LPGENT("Abkhazian") },
	{ _T("af"), LPGENT("Afrikaans") },
	{ _T("ak"), LPGENT("Akan") },
	{ _T("sq"), LPGENT("Albanian") },
	{ _T("am"), LPGENT("Amharic") },
	{ _T("ar"), LPGENT("Arabic") },
	{ _T("an"), LPGENT("Aragonese") },
	{ _T("hy"), LPGENT("Armenian") },
	{ _T("as"), LPGENT("Assamese") },
	{ _T("av"), LPGENT("Avaric") },
	{ _T("ae"), LPGENT("Avestan") },
	{ _T("ay"), LPGENT("Aymara") },
	{ _T("az"), LPGENT("Azerbaijani") },
	{ _T("ba"), LPGENT("Bashkir") },
	{ _T("bm"), LPGENT("Bambara") },
	{ _T("eu"), LPGENT("Basque") },
	{ _T("be"), LPGENT("Belarusian") },
	{ _T("bn"), LPGENT("Bengali") },
	{ _T("bh"), LPGENT("Bihari") },
	{ _T("bi"), LPGENT("Bislama") },
	{ _T("bs"), LPGENT("Bosnian") },
	{ _T("br"), LPGENT("Breton") },
	{ _T("bg"), LPGENT("Bulgarian") },
	{ _T("my"), LPGENT("Burmese") },
	{ _T("ca"), LPGENT("Catalan; Valencian") },
	{ _T("ch"), LPGENT("Chamorro") },
	{ _T("ce"), LPGENT("Chechen") },
	{ _T("zh"), LPGENT("Chinese") },
	{ _T("cu"), LPGENT("Church Slavic; Old Slavonic") },
	{ _T("cv"), LPGENT("Chuvash") },
	{ _T("kw"), LPGENT("Cornish") },
	{ _T("co"), LPGENT("Corsican") },
	{ _T("cr"), LPGENT("Cree") },
	{ _T("cs"), LPGENT("Czech") },
	{ _T("da"), LPGENT("Danish") },
	{ _T("dv"), LPGENT("Divehi; Dhivehi; Maldivian") },
	{ _T("nl"), LPGENT("Dutch; Flemish") },
	{ _T("dz"), LPGENT("Dzongkha") },
	{ _T("en"), LPGENT("English") },
	{ _T("eo"), LPGENT("Esperanto") },
	{ _T("et"), LPGENT("Estonian") },
	{ _T("ee"), LPGENT("Ewe") },
	{ _T("fo"), LPGENT("Faroese") },
	{ _T("fj"), LPGENT("Fijian") },
	{ _T("fi"), LPGENT("Finnish") },
	{ _T("fr"), LPGENT("French") },
	{ _T("fy"), LPGENT("Western Frisian") },
	{ _T("ff"), LPGENT("Fulah") },
	{ _T("ka"), LPGENT("Georgian") },
	{ _T("de"), LPGENT("German") },
	{ _T("gd"), LPGENT("Gaelic; Scottish Gaelic") },
	{ _T("ga"), LPGENT("Irish") },
	{ _T("gl"), LPGENT("Galician") },
	{ _T("gv"), LPGENT("Manx") },
	{ _T("el"), LPGENT("Greek, Modern (1453-)") },
	{ _T("gn"), LPGENT("Guarani") },
	{ _T("gu"), LPGENT("Gujarati") },
	{ _T("ht"), LPGENT("Haitian; Haitian Creole") },
	{ _T("ha"), LPGENT("Hausa") },
	{ _T("he"), LPGENT("Hebrew") },
	{ _T("hz"), LPGENT("Herero") },
	{ _T("hi"), LPGENT("Hindi") },
	{ _T("ho"), LPGENT("Hiri Motu") },
	{ _T("hu"), LPGENT("Hungarian") },
	{ _T("ig"), LPGENT("Igbo") },
	{ _T("is"), LPGENT("Icelandic") },
	{ _T("io"), LPGENT("Ido") },
	{ _T("ii"), LPGENT("Sichuan Yi") },
	{ _T("iu"), LPGENT("Inuktitut") },
	{ _T("ie"), LPGENT("Interlingue") },
	{ _T("ia"), LPGENT("Interlingua (International Auxiliary Language Association)") },
	{ _T("id"), LPGENT("Indonesian") },
	{ _T("ik"), LPGENT("Inupiaq") },
	{ _T("it"), LPGENT("Italian") },
	{ _T("jv"), LPGENT("Javanese") },
	{ _T("ja"), LPGENT("Japanese") },
	{ _T("kl"), LPGENT("Kalaallisut; Greenlandic") },
	{ _T("kn"), LPGENT("Kannada") },
	{ _T("ks"), LPGENT("Kashmiri") },
	{ _T("kr"), LPGENT("Kanuri") },
	{ _T("kk"), LPGENT("Kazakh") },
	{ _T("km"), LPGENT("Central Khmer") },
	{ _T("ki"), LPGENT("Kikuyu; Gikuyu") },
	{ _T("rw"), LPGENT("Kinyarwanda") },
	{ _T("ky"), LPGENT("Kirghiz; Kyrgyz") },
	{ _T("kv"), LPGENT("Komi") },
	{ _T("kg"), LPGENT("Kongo") },
	{ _T("ko"), LPGENT("Korean") },
	{ _T("kj"), LPGENT("Kuanyama; Kwanyama") },
	{ _T("ku"), LPGENT("Kurdish") },
	{ _T("lo"), LPGENT("Lao") },
	{ _T("la"), LPGENT("Latin") },
	{ _T("lv"), LPGENT("Latvian") },
	{ _T("li"), LPGENT("Limburgan; Limburger; Limburgish") },
	{ _T("ln"), LPGENT("Lingala") },
	{ _T("lt"), LPGENT("Lithuanian") },
	{ _T("lb"), LPGENT("Luxembourgish; Letzeburgesch") },
	{ _T("lu"), LPGENT("Luba-Katanga") },
	{ _T("lg"), LPGENT("Ganda") },
	{ _T("mk"), LPGENT("Macedonian") },
	{ _T("mh"), LPGENT("Marshallese") },
	{ _T("ml"), LPGENT("Malayalam") },
	{ _T("mi"), LPGENT("Maori") },
	{ _T("mr"), LPGENT("Marathi") },
	{ _T("ms"), LPGENT("Malay") },
	{ _T("mg"), LPGENT("Malagasy") },
	{ _T("mt"), LPGENT("Maltese") },
	{ _T("mo"), LPGENT("Moldavian") },
	{ _T("mn"), LPGENT("Mongolian") },
	{ _T("na"), LPGENT("Nauru") },
	{ _T("nv"), LPGENT("Navajo; Navaho") },
	{ _T("nr"), LPGENT("Ndebele, South; South Ndebele") },
	{ _T("nd"), LPGENT("Ndebele, North; North Ndebele") },
	{ _T("ng"), LPGENT("Ndonga") },
	{ _T("ne"), LPGENT("Nepali") },
	{ _T("nn"), LPGENT("Norwegian Nynorsk; Nynorsk, Norwegian") },
	{ _T("nb"), LPGENT("Bokmaal, Norwegian; Norwegian Bokmaal") },
	{ _T("no"), LPGENT("Norwegian") },
	{ _T("ny"), LPGENT("Chichewa; Chewa; Nyanja") },
	{ _T("oc"), LPGENT("Occitan (post 1500); Provencal") },
	{ _T("oj"), LPGENT("Ojibwa") },
	{ _T("or"), LPGENT("Oriya") },
	{ _T("om"), LPGENT("Oromo") },
	{ _T("os"), LPGENT("Ossetian; Ossetic") },
	{ _T("pa"), LPGENT("Panjabi; Punjabi") },
	{ _T("fa"), LPGENT("Persian") },
	{ _T("pi"), LPGENT("Pali") },
	{ _T("pl"), LPGENT("Polish") },
	{ _T("pt"), LPGENT("Portuguese") },
	{ _T("ps"), LPGENT("Pushto") },
	{ _T("qu"), LPGENT("Quechua") },
	{ _T("rm"), LPGENT("Romansh") },
	{ _T("ro"), LPGENT("Romanian") },
	{ _T("rn"), LPGENT("Rundi") },
	{ _T("ru"), LPGENT("Russian") },
	{ _T("sg"), LPGENT("Sango") },
	{ _T("sa"), LPGENT("Sanskrit") },
	{ _T("sr"), LPGENT("Serbian") },
	{ _T("hr"), LPGENT("Croatian") },
	{ _T("si"), LPGENT("Sinhala; Sinhalese") },
	{ _T("sk"), LPGENT("Slovak") },
	{ _T("sl"), LPGENT("Slovenian") },
	{ _T("se"), LPGENT("Northern Sami") },
	{ _T("sm"), LPGENT("Samoan") },
	{ _T("sn"), LPGENT("Shona") },
	{ _T("sd"), LPGENT("Sindhi") },
	{ _T("so"), LPGENT("Somali") },
	{ _T("st"), LPGENT("Sotho, Southern") },
	{ _T("es"), LPGENT("Spanish; Castilian") },
	{ _T("sc"), LPGENT("Sardinian") },
	{ _T("ss"), LPGENT("Swati") },
	{ _T("su"), LPGENT("Sundanese") },
	{ _T("sw"), LPGENT("Swahili") },
	{ _T("sv"), LPGENT("Swedish") },
	{ _T("ty"), LPGENT("Tahitian") },
	{ _T("ta"), LPGENT("Tamil") },
	{ _T("tt"), LPGENT("Tatar") },
	{ _T("te"), LPGENT("Telugu") },
	{ _T("tg"), LPGENT("Tajik") },
	{ _T("tl"), LPGENT("Tagalog") },
	{ _T("th"), LPGENT("Thai") },
	{ _T("bo"), LPGENT("Tibetan") },
	{ _T("ti"), LPGENT("Tigrinya") },
	{ _T("to"), LPGENT("Tonga (Tonga Islands)") },
	{ _T("tn"), LPGENT("Tswana") },
	{ _T("ts"), LPGENT("Tsonga") },
	{ _T("tk"), LPGENT("Turkmen") },
	{ _T("tr"), LPGENT("Turkish") },
	{ _T("tw"), LPGENT("Twi") },
	{ _T("ug"), LPGENT("Uighur; Uyghur") },
	{ _T("uk"), LPGENT("Ukrainian") },
	{ _T("ur"), LPGENT("Urdu") },
	{ _T("uz"), LPGENT("Uzbek") },
	{ _T("ve"), LPGENT("Venda") },
	{ _T("vi"), LPGENT("Vietnamese") },
	{ _T("vo"), LPGENT("Volapuk") },
	{ _T("cy"), LPGENT("Welsh") },
	{ _T("wa"), LPGENT("Walloon") },
	{ _T("wo"), LPGENT("Wolof") },
	{ _T("xh"), LPGENT("Xhosa") },
	{ _T("yi"), LPGENT("Yiddish") },
	{ _T("yo"), LPGENT("Yoruba") },
	{ _T("za"), LPGENT("Zhuang; Chuang") },
	{ _T("zu"), LPGENT("Zulu") },
	{ NULL, NULL }
};

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
	{
		for (int i = 0; g_LanguageCodes[i].szCode; i++) {
			if (!_tcscmp(isocode, g_LanguageCodes[i].szCode)) {
				setTString(hContact, "Language0", g_LanguageCodes[i].szDescription);
				break;
			}
		}
	}
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
		JSONNODE *items = json_as_array(root), *item;
		for (size_t i = 0; i < min(json_size(items), 3); i++)
		{
			item = json_at(items, i);
			if (item == NULL)
				break;

			// how to read array of string?
			//CMStringA name(FORMAT, "e-mail%d", i);
			//CMString value = ptrT(json_as_string(item));
			//setTString(hContact, name, value);
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