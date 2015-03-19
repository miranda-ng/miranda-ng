#include "common.h"

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
	int value = json_as_int(node);
	if (value)
		setByte(hContact, "Gender", (BYTE)(value == 1 ? 'M' : 'F'));
	else
		delSetting(hContact, "Gender");
}

void CSkypeProto::UpdateProfileBirthday(JSONNODE *root, MCONTACT hContact)
{
	JSONNODE *node = json_get(root, "birthday");
	// parse "YYYY-MM-DD"
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
	CMStringA isocode = mir_t2a(ptrT(json_as_string(node)));
	if (!isocode.IsEmpty() && isocode != "null")
	{
		// convert language code to language name
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