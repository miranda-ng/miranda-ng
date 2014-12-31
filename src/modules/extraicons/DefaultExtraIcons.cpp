/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-15 Miranda NG project

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"

#include "m_cluiframes.h"

#include "ExtraIcon.h"
#include "extraicons.h"

ExtraIcon* GetExtraIcon(HANDLE id);

////////////////////////////////////////////////////////////////////////////////////////
// DB extra icons

HANDLE hExtraVisibility, hExtraChat, hExtraGender, hExtraProto;

static void SetVisibility(MCONTACT hContact, int apparentMode, bool clear)
{
	if (hContact == NULL)
		return;

	char *proto = GetContactProto(hContact);
	if (IsEmpty(proto))
		return;

	if (apparentMode <= 0)
		apparentMode = db_get_w(hContact, proto, "ApparentMode", 0);

	HANDLE hExtraIcon, hIcolib = NULL;

	if (db_get_b(hContact, proto, "ChatRoom", 0)) {
		// Is chat
		hExtraIcon = hExtraChat;
		if (apparentMode == ID_STATUS_OFFLINE)
			hIcolib = Skin_GetIconHandle("ChatActivity");
	}
	else {
		// Not chat
		hExtraIcon = hExtraVisibility;
		if (apparentMode == ID_STATUS_OFFLINE)
			hIcolib = LoadSkinnedIconHandle(SKINICON_OTHER_INVISIBLE_ALL);
		else if (apparentMode == ID_STATUS_ONLINE)
			hIcolib = LoadSkinnedIconHandle(SKINICON_OTHER_VISIBLE_ALL);
	}

	if (hIcolib != NULL || clear) {
		ExtraIcon *extra = GetExtraIcon(hExtraIcon);
		if (extra)
			extra->setIcon((int)hExtraIcon, hContact, hIcolib);
	}
}

static void SetGender(MCONTACT hContact, int gender, bool clear)
{
	if (hContact == NULL)
		return;

	char *proto = GetContactProto(hContact);
	if (IsEmpty(proto))
		return;

	if (gender <= 0)
		gender = db_get_b(hContact, proto, "Gender", 0);
	if (gender <= 0)
		gender = db_get_b(hContact, "UserInfo", "Gender", 0);

	const char *ico;
	if (gender == 'M')
		ico = "gender_male";
	else if (gender == 'F')
		ico = "gender_female";
	else
		ico = NULL;

	if (ico != NULL || clear) {
		ExtraIcon *extra = GetExtraIcon(hExtraGender);
		if (extra)
			extra->setIconByName((int)hExtraGender, hContact, ico);
	}
}

struct Info
{
	const char *name;
	const char *desc;
	int         iSkinIcon;
	const char *db[8];
	void(*OnClick)(Info *info, const char *text);

	HANDLE hIcolib, hExtraIcon;
};

static void EmailOnClick(Info *info, const char *text)
{
	char cmd[1024];
	mir_snprintf(cmd, SIZEOF(cmd), "mailto:%s", text);
	ShellExecuteA(NULL, "open", cmd, NULL, NULL, SW_SHOW);
}

static void HomepageOnClick(Info *info, const char *text)
{
	ShellExecuteA(NULL, "open", text, NULL, NULL, SW_SHOW);
}

static Info infos[] =
{
	{ "homepage", "Homepage", SKINICON_OTHER_MIRANDAWEB,
		{ NULL, "Homepage", "UserInfo", "Homepage" },
		&HomepageOnClick },
	{ "sms", "Phone/SMS", SKINICON_OTHER_SMS,
		{ NULL, "Cellular", "UserInfo", "Cellular", "UserInfo", "Phone", "UserInfo", "MyPhone0" },
		NULL },
	{ "email", "E-mail", SKINICON_OTHER_SENDEMAIL,
		{ NULL, "e-mail", "UserInfo", "e-mail", "UserInfo", "Mye-mail0" },
		&EmailOnClick },
};

static void SetExtraIcons(MCONTACT hContact)
{
	if (hContact == NULL)
		return;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
		return;

	for (unsigned int i = 0; i < SIZEOF(infos); i++) {
		Info &p = infos[i];

		for (unsigned int j = 0; j < SIZEOF(p.db); j += 2) {
			if (p.db[j + 1] == NULL)
				break;

			ptrA szValue(db_get_sa(hContact, p.db[j] == NULL ? proto : p.db[j], p.db[j + 1]));
			if (!IsEmpty(szValue)) {
				ExtraIcon_SetIcon(p.hExtraIcon, hContact, p.hIcolib);
				break;
			}
		}
	}
}

static int SettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	char *proto = GetContactProto(hContact);
	if (IsEmpty(proto))
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	bool isProto = (strcmp(cws->szModule, proto) == 0);
	if (isProto && strcmp(cws->szSetting, "ApparentMode") == 0) {
		SetVisibility(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.wVal, true);
		return 0;
	}

	if (strcmp(cws->szSetting, "Gender") == 0 && (isProto || strcmp(cws->szModule, "UserInfo") == 0)) {
		SetGender(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.bVal, true);
		return 0;
	}

	for (int i = 0; i < SIZEOF(infos); i++) {
		Info &p = infos[i];

		for (int j = 0; j < SIZEOF(p.db); j += 2) {
			if (p.db[j + 1] == NULL)
				break;
			if (p.db[j] == NULL && !isProto)
				continue;
			if (p.db[j] != NULL && strcmp(cws->szModule, p.db[j]))
				continue;
			if (strcmp(cws->szSetting, p.db[j + 1]))
				continue;

			bool show = (cws->value.type != DBVT_DELETED && !IsEmpty(cws->value.pszVal));
			ExtraIcon_SetIcon(p.hExtraIcon, hContact, show ? p.hIcolib : NULL);
			break;
		}
	}

	return 0;
}

static int DefaultOnClick(WPARAM hContact, LPARAM lParam, LPARAM param)
{
	Info *p = (Info*)param;
	if (p == NULL)
		return 0;

	if (hContact == NULL)
		return 0;

	char *proto = GetContactProto(hContact);
	if (IsEmpty(proto))
		return 0;

	bool found = false;
	for (int j = 0; !found && j < SIZEOF(p->db); j += 2) {
		if (p->db[j + 1] == NULL)
			break;

		ptrA szValue(db_get_sa(hContact, p->db[j] == NULL ? proto : p->db[j], p->db[j + 1]));
		if (!IsEmpty(szValue)) {
			p->OnClick(p, szValue);
			found = true;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// Protocol icon

struct ProtoInfo
{
	ProtoInfo(LPCSTR _proto, HANDLE _image) :
		proto(mir_strdup(_proto)),
		hImage(_image)
		{}

	ptrA   proto;
	HANDLE hImage;
};

static int CompareProtos(const ProtoInfo *p1, const ProtoInfo *p2)
{	return strcmp(p1->proto, p2->proto);
}

OBJLIST<ProtoInfo> arProtos(10, CompareProtos);

static int ProtocolRebuildIcons(WPARAM wParam, LPARAM lParam)
{
	arProtos.destroy();
	return 0;
}

static ProtoInfo* FindProto(const char *proto)
{
	ProtoInfo *p = arProtos.find((ProtoInfo*)&proto);
	if (p)
		return p;

	HICON hIcon = LoadSkinnedProtoIcon(proto, ID_STATUS_ONLINE);
	if (hIcon == NULL)
		return NULL;

	HANDLE hImage = ExtraIcon_Add(hIcon);
	if (hImage == INVALID_HANDLE_VALUE)
		return NULL;

	p = new ProtoInfo(proto, hImage);
	arProtos.insert(p);
	return p;
}

static int ProtocolApplyIcon(WPARAM hContact, LPARAM lParam)
{
	char *proto = GetContactProto(hContact);
	if (IsEmpty(proto))
		return 0;

	HANDLE hImage = INVALID_HANDLE_VALUE;
	ProtoInfo *pi = FindProto(proto);
	if (pi != NULL)
		hImage = pi->hImage;

	ExtraIcon_SetIcon(hExtraProto, hContact, hImage);
	return 0;
}

static int ProtocolOnClick(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	if (wParam)
		CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

void DefaultExtraIcons_Load()
{
	hExtraChat = ExtraIcon_Register("chat_activity", LPGEN("Chat activity"), "ChatActivity");
	hExtraVisibility = ExtraIcon_Register("visibility", "Visibility", LoadSkinnedIconName(SKINICON_OTHER_VISIBLE_ALL));
	hExtraGender = ExtraIcon_Register("gender", "Gender", "gender_male");
	hExtraProto = ExtraIcon_Register("protocol", "Account", LoadSkinnedIconName(SKINICON_OTHER_ACCMGR),
		&ProtocolRebuildIcons, &ProtocolApplyIcon, &ProtocolOnClick);

	for (int i = 0; i < SIZEOF(infos); i++) {
		Info &p = infos[i];
		p.hIcolib = LoadSkinnedIconHandle(p.iSkinIcon);
		if (p.OnClick)
			p.hExtraIcon = ExtraIcon_Register(p.name, p.desc, LoadSkinnedIconName(p.iSkinIcon), DefaultOnClick, (LPARAM)&p);
		else
			p.hExtraIcon = ExtraIcon_Register(p.name, p.desc, LoadSkinnedIconName(p.iSkinIcon));
	}

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		SetExtraIcons(hContact);
		SetVisibility(hContact, -1, false);
		SetGender(hContact, -1, false);
	}

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
}
