/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-22 Miranda NG team

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

#include "stdafx.h"

#include "m_cluiframes.h"

#include "extraicons.h"
#include "chat.h"

////////////////////////////////////////////////////////////////////////////////////////
// DB extra icons

HANDLE hExtraVisibility, hExtraChat, hExtraChatMute, hExtraGender, hExtraProto;

static void SetVisibility(MCONTACT hContact, int apparentMode, bool clear)
{
	if (hContact == 0)
		return;

	char *proto = Proto_GetBaseAccountName(hContact);
	if (IsEmpty(proto))
		return;

	if (apparentMode <= 0)
		apparentMode = db_get_w(hContact, proto, "ApparentMode", 0);

	HANDLE hExtraIcon, hIcolib = nullptr;

	// Is chat
	if (Contact::IsGroupChat(hContact, proto)) {
		hExtraIcon = hExtraChat;
		if (apparentMode == ID_STATUS_OFFLINE)
			hIcolib = IcoLib_GetIconHandle("ChatActivity");
	}
	else { // Not chat
		hExtraIcon = hExtraVisibility;
		if (apparentMode == ID_STATUS_OFFLINE)
			hIcolib = Skin_GetIconHandle(SKINICON_OTHER_INVISIBLE_ALL);
		else if (apparentMode == ID_STATUS_ONLINE)
			hIcolib = Skin_GetIconHandle(SKINICON_OTHER_VISIBLE_ALL);
	}

	if (hIcolib != nullptr || clear)
		ExtraIcon_SetIcon(hExtraIcon, hContact, hIcolib);
}

static void SetGender(MCONTACT hContact, int gender, bool clear)
{
	if (hContact == 0)
		return;

	char *proto = Proto_GetBaseAccountName(hContact);
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
		ico = nullptr;

	if (ico != nullptr || clear)
		ExtraIcon_SetIconByName(hExtraGender, hContact, ico);
}

static void SetChatMute(MCONTACT hContact, int mode)
{
	if (hContact == 0)
		return;

	if (mode == -1)
		mode = db_get_b(hContact, "SRMM", "MuteMode", CHATMODE_NORMAL);

	HANDLE hIcon;
	switch (mode) {
	case CHATMODE_MUTE: hIcon = Skin_GetIconHandle(SKINICON_OTHER_OFF); break;
	case CHATMODE_UNMUTE: hIcon = Skin_GetIconHandle(SKINICON_OTHER_ON); break;
	default:
		hIcon = nullptr; break;
	}

	ExtraIcon_SetIcon(hExtraChatMute, hContact, hIcon);
}

struct Info
{
	const char *name;
	const char *desc;
	int         iSkinIcon;
	const char *db[8];
	void(*OnClick)(Info *info, const char *text);
	int        flags;

	HANDLE hIcolib, hExtraIcon;
};

static void EmailOnClick(Info*, const char *text)
{
	char cmd[1024];
	mir_snprintf(cmd, "mailto:%s", text);
	ShellExecuteA(nullptr, "open", cmd, nullptr, nullptr, SW_SHOW);
}

static void HomepageOnClick(Info*, const char *text)
{
	ShellExecuteA(nullptr, "open", text, nullptr, nullptr, SW_SHOW);
}

static Info infos[] =
{
	{ "homepage", LPGEN("Homepage"), SKINICON_OTHER_MIRANDAWEB,
		{ nullptr, "Homepage", "UserInfo", "Homepage" },
		&HomepageOnClick, EIF_DISABLED_BY_DEFAULT },
	{ "sms", LPGEN("Phone/SMS"), SKINICON_OTHER_SMS,
		{ nullptr, "Cellular", "UserInfo", "Cellular", "UserInfo", "Phone", "UserInfo", "MyPhone0" },
		nullptr, EIF_DISABLED_BY_DEFAULT },
	{ "email", LPGEN("E-mail"), SKINICON_OTHER_SENDEMAIL,
		{ nullptr, "e-mail", "UserInfo", "e-mail", "UserInfo", "Mye-mail0" },
		&EmailOnClick, EIF_DISABLED_BY_DEFAULT },
};

static void SetExtraIcons(MCONTACT hContact)
{
	if (hContact == 0)
		return;

	char *proto = Proto_GetBaseAccountName(hContact);
	if ( IsEmpty(proto))
		return;

	for (auto &p : infos) {
		for (unsigned int j = 0; j < _countof(p.db); j += 2) {
			if (p.db[j + 1] == nullptr)
				break;

			ptrA szValue(db_get_sa(hContact, p.db[j] == nullptr ? proto : p.db[j], p.db[j + 1]));
			if (!IsEmpty(szValue)) {
				ExtraIcon_SetIcon(p.hExtraIcon, hContact, p.hIcolib);
				break;
			}
		}
	}
}

static int SettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		return 0;

	char *proto = Proto_GetBaseAccountName(hContact);
	if (IsEmpty(proto))
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	bool isProto = (strcmp(cws->szModule, proto) == 0);
	if (isProto && !strcmp(cws->szSetting, "ApparentMode")) {
		SetVisibility(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.wVal, true);
		return 0;
	}

	if (!strcmp(cws->szModule, "SRMM") && !strcmp(cws->szSetting, "MuteMode")) {
		SetChatMute(hContact, cws->value.type == DBVT_DELETED ? CHATMODE_NORMAL : cws->value.bVal);
		return 0;
	}

	if (!strcmp(cws->szSetting, "Gender") && (isProto || !strcmp(cws->szModule, "UserInfo"))) {
		SetGender(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.bVal, true);
		return 0;
	}

	for (auto &p : infos) {
		for (int j = 0; j < _countof(p.db); j += 2) {
			if (p.db[j + 1] == nullptr)
				break;
			if (p.db[j] == nullptr && !isProto)
				continue;
			if (p.db[j] != nullptr && strcmp(cws->szModule, p.db[j]))
				continue;
			if (strcmp(cws->szSetting, p.db[j + 1]))
				continue;

			bool show = (cws->value.type != DBVT_DELETED && !IsEmpty(cws->value.pszVal));
			ExtraIcon_SetIcon(p.hExtraIcon, hContact, show ? p.hIcolib : nullptr);
			break;
		}
	}

	return 0;
}

static int DefaultOnClick(WPARAM hContact, LPARAM, LPARAM param)
{
	Info *p = (Info*)param;
	if (p == nullptr)
		return 0;

	if (hContact == 0)
		return 0;

	char *proto = Proto_GetBaseAccountName(hContact);
	if (IsEmpty(proto))
		return 0;

	bool found = false;
	for (int j = 0; !found && j < _countof(p->db); j += 2) {
		if (p->db[j + 1] == nullptr)
			break;

		ptrA szValue(db_get_sa(hContact, p->db[j] == nullptr ? proto : p->db[j], p->db[j + 1]));
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
{	return mir_strcmp(p1->proto, p2->proto);
}

OBJLIST<ProtoInfo> arProtos(10, CompareProtos);

static int ProtocolRebuildIcons(WPARAM, LPARAM)
{
	arProtos.destroy();
	return 0;
}

static ProtoInfo* FindProto(const char *proto)
{
	ProtoInfo *p = arProtos.find((ProtoInfo*)&proto);
	if (p)
		return p;

	HICON hIcon = Skin_LoadProtoIcon(proto, ID_STATUS_ONLINE);
	if (hIcon == nullptr)
		return nullptr;

	HANDLE hImage = ExtraIcon_AddIcon(hIcon);
	if (hImage == INVALID_HANDLE_VALUE)
		return nullptr;

	p = new ProtoInfo(proto, hImage);
	arProtos.insert(p);
	return p;
}

static int ProtocolApplyIcon(WPARAM hContact, LPARAM)
{
	char *proto = Proto_GetBaseAccountName(hContact);
	if (IsEmpty(proto))
		return 0;

	HANDLE hImage = INVALID_HANDLE_VALUE;
	ProtoInfo *pi = FindProto(proto);
	if (pi != nullptr)
		hImage = pi->hImage;

	ExtraIcon_SetIcon(hExtraProto, hContact, hImage);
	return 0;
}

static int ProtocolOnClick(WPARAM wParam, LPARAM, LPARAM)
{
	if (wParam)
		CallService(MS_USERINFO_SHOWDIALOG, wParam, 0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////

void DefaultExtraIcons_Load()
{
	hExtraChat = ExtraIcon_RegisterIcolib("chat_activity", LPGEN("Chat activity"), "ChatActivity");
	hExtraChatMute = ExtraIcon_RegisterIcolib("chat_mute", LPGEN("Chat mute mode"), "ChatMute");
	hExtraVisibility = ExtraIcon_RegisterIcolib("visibility", "Visibility", Skin_GetIconHandle(SKINICON_OTHER_VISIBLE_ALL));
	hExtraGender = ExtraIcon_RegisterIcolib("gender", "Gender", "gender_male", nullptr, 0, EIF_DISABLED_BY_DEFAULT);
	hExtraProto = ExtraIcon_RegisterCallback("protocol", "Account", Skin_GetIconHandle(SKINICON_OTHER_ACCMGR),
		&ProtocolRebuildIcons, &ProtocolApplyIcon, &ProtocolOnClick, 0, EIF_DISABLED_BY_DEFAULT);

	for (auto &p : infos) {
		p.hIcolib = Skin_GetIconHandle(p.iSkinIcon);
		if (p.OnClick)
			p.hExtraIcon = ExtraIcon_RegisterIcolib(p.name, p.desc, Skin_GetIconHandle(p.iSkinIcon), DefaultOnClick, (LPARAM)&p, p.flags);
		else
			p.hExtraIcon = ExtraIcon_RegisterIcolib(p.name, p.desc, Skin_GetIconHandle(p.iSkinIcon), nullptr, 0, p.flags);
	}

	for (auto &hContact : Contacts()) {
		SetChatMute(hContact, -1);
		SetExtraIcons(hContact);
		SetVisibility(hContact, -1, false);
		SetGender(hContact, -1, false);
	}

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
}
