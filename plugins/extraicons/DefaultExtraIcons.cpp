/*
 Copyright (C) 2009 Ricardo Pescuma Domenecci

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

#include "commons.h"

/*
 0, // EXTRA_ICON_VISMODE
 1, // EXTRA_ICON_EMAIL
 2, // EXTRA_ICON_PROTO
 3, // EXTRA_ICON_SMS
 4, // EXTRA_ICON_ADV1
 5, // EXTRA_ICON_ADV2
 6, // EXTRA_ICON_WEB
 7, // EXTRA_ICON_CLIENT
 8, // EXTRA_ICON_ADV3
 9, // EXTRA_ICON_ADV4
 */

static void ProtocolInit();
static void DBExtraIconsInit();

void DefaultExtraIcons_Load()
{
	DBExtraIconsInit();
	ProtocolInit();
}

void DefaultExtraIcons_Unload()
{
}

// DB extra icons ///////////////////////////////////////////////////////////////////////

struct Info;

HANDLE hExtraVisibility = NULL;
HANDLE hExtraChat = NULL;
HANDLE hExtraGender = NULL;

static void SetVisibility(HANDLE hContact, int apparentMode, BOOL clear)
{
	if (hContact == NULL)
		return;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (IsEmpty(proto))
		return;

	if (apparentMode <= 0)
		apparentMode = DBGetContactSettingWord(hContact, proto, "ApparentMode", 0);

	const char *ico = NULL;

	if (DBGetContactSettingByte(hContact, proto, "ChatRoom", 0))
	{
		// Is chat
		if (apparentMode == ID_STATUS_OFFLINE)
			ico = "ChatActivity";

		if (ico == NULL && !clear)
			return;

		ExtraIcon_SetIcon(hExtraChat, hContact, ico);
	}
	else
	{
		// Not chat
		if (apparentMode == ID_STATUS_OFFLINE)
			ico = "NeverVis";

		else if (apparentMode == ID_STATUS_ONLINE)
			ico = "AlwaysVis";

		if (ico == NULL && !clear)
			return;

		ExtraIcon_SetIcon(hExtraVisibility, hContact, ico);
	}
}

static void SetGender(HANDLE hContact, int gender, BOOL clear)
{
	if (hContact == NULL)
		return;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (IsEmpty(proto))
		return;

	if (gender <= 0)
		gender = DBGetContactSettingByte(hContact, proto, "Gender", 0);
	if (gender <= 0)
		gender = DBGetContactSettingByte(hContact, "UserInfo", "Gender", 0);

	const char *ico = NULL;
	if (gender == 'M')
		ico = "gender_male";
	else if (gender == 'F')
		ico = "gender_female";
	else
		ico = NULL;

	if (ico == NULL && !clear)
		return;

	ExtraIcon_SetIcon(hExtraGender, hContact, ico);
}

static void EmailOnClick(Info *info, const char *text);
static void HomepageOnClick(Info *info, const char *text);
static void DefaultSetIcon(HANDLE hContact, Info *info, const char *text);

struct Info
{
	const char *name;
	const char *desc;
	const char *icon;
	const char *db[8];
	void (*SetIcon)(HANDLE hContact, Info *info, const char *text);
	void (*OnClick)(Info *info, const char *text);
	HANDLE hExtraIcon;
} infos[] = { 
	{ "homepage", "Homepage", "core_main_2", { NULL, "Homepage",
											"UserInfo", "Homepage" }, DefaultSetIcon, &HomepageOnClick, NULL }, 
	{ "sms", "Phone/SMS", "core_main_17", { NULL, "Cellular",
											"UserInfo", "Cellular",
											"UserInfo", "Phone",
											"UserInfo", "MyPhone0" }, DefaultSetIcon, NULL, NULL }, 
	{ "email", "E-mail", "core_main_14", { NULL, "e-mail", 
											"UserInfo", "e-mail", 
											"UserInfo", "Mye-mail0" }, DefaultSetIcon, &EmailOnClick, NULL }, 
};

static void EmailOnClick(Info *info, const char *text)
{
	char cmd[1024];
	mir_snprintf(cmd, MAX_REGS(cmd), "mailto:%s", text);
	ShellExecute(NULL, "open", cmd, NULL, NULL, SW_SHOW);
}

static void HomepageOnClick(Info *info, const char *text)
{
	ShellExecute(NULL, "open", text, NULL, NULL, SW_SHOW);
}

static void DefaultSetIcon(HANDLE hContact, Info *info, const char *text)
{
	ExtraIcon_SetIcon(info->hExtraIcon, hContact, text ? info->icon : NULL);
}

static void SetExtraIcons(HANDLE hContact)
{
	if (hContact == NULL)
		return;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (IsEmpty(proto))
		return;

	for (unsigned int i = 0; i < MAX_REGS(infos); ++i)
	{
		Info &info = infos[i];

		bool show = false;
		for (unsigned int j = 0; !show && j < MAX_REGS(info.db); j += 2)
		{
			if (info.db[j + 1] == NULL)
				break;

			DBVARIANT dbv = { 0 };
			if (!DBGetContactSettingString(hContact, info.db[j] == NULL ? proto : info.db[j], info.db[j+1], &dbv))
			{
				if (!IsEmpty(dbv.pszVal))
				{
					info.SetIcon(hContact, &info, dbv.pszVal);
					show = true;
				}
				DBFreeVariant(&dbv);
			}
		}
	}
}

static int SettingChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*) lParam;

	if (hContact == NULL)
		return 0;

	char *proto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (IsEmpty(proto))
		return 0;

	bool isProto = (strcmp(cws->szModule, proto) == 0);

	if (isProto && strcmp(cws->szSetting, "ApparentMode") == 0)
	{
		SetVisibility(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.wVal, TRUE);
		return 0;
	}

	if (strcmp(cws->szSetting, "Gender") == 0 && (isProto || strcmp(cws->szModule, "UserInfo") == 0))
	{
		SetGender(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.bVal, TRUE);
		return 0;
	}

	for (unsigned int i = 0; i < MAX_REGS(infos); ++i)
	{
		Info &info = infos[i];

		for (unsigned int j = 0; j < MAX_REGS(info.db); j += 2)
		{
			if (info.db[j + 1] == NULL)
				break;
			if (info.db[j] == NULL && !isProto)
				continue;
			if (info.db[j] != NULL && strcmp(cws->szModule, info.db[j]))
				continue;
			if (strcmp(cws->szSetting, info.db[j + 1]))
				continue;

			bool show = (cws->value.type != DBVT_DELETED && !IsEmpty(cws->value.pszVal));
			info.SetIcon(hContact, &info, show ? cws->value.pszVal : NULL);

			break;
		}
	}

	return 0;
}

static int DefaultOnClick(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	Info *info = (Info *) param;
	if (info == NULL)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (IsEmpty(proto))
		return 0;

	bool found = false;
	for (unsigned int j = 0; !found && j < MAX_REGS(info->db); j += 2)
	{
		if (info->db[j + 1] == NULL)
			break;

		DBVARIANT dbv = { 0 };
		if (!DBGetContactSettingString(hContact, info->db[j] == NULL ? proto : info->db[j], info->db[j+1], &dbv))
		{
			if (!IsEmpty(dbv.ptszVal))
			{
				info->OnClick(info, dbv.ptszVal);
				found = true;
			}

			DBFreeVariant(&dbv);
		}
	}

	return 0;
}

static void DBExtraIconsInit()
{
	hExtraChat = ExtraIcon_Register("chat_activity", "Chat activity", "ChatActivity");
	hExtraVisibility = ExtraIcon_Register("visibility", "Visibility", "AlwaysVis");
	hExtraGender = ExtraIcon_Register("gender", "Gender", "gender_male");
	for (unsigned int i = 0; i < MAX_REGS(infos); ++i)
	{
		Info &info = infos[i];
		if (info.OnClick)
			info.hExtraIcon = ExtraIcon_Register(info.name, info.desc, info.icon, DefaultOnClick, (LPARAM) &info);
		else
			info.hExtraIcon = ExtraIcon_Register(info.name, info.desc, info.icon);
	}

	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL)
	{
		SetExtraIcons(hContact);
		SetVisibility(hContact, -1, FALSE);
		SetGender(hContact, -1, FALSE);

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	hHooks.push_back(HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged));
}

// Protocol /////////////////////////////////////////////////////////////////////////////

struct ProtoInfo
{
	string proto;
	HANDLE hImage;
};

vector<ProtoInfo> protos;

HANDLE hExtraProto = NULL;

static int ProtocolRebuildIcons(WPARAM wParam, LPARAM lParam)
{
	protos.clear();
	return 0;
}

static ProtoInfo *FindProto(const char * proto)
{
	for (unsigned int i = 0; i < protos.size(); ++i)
	{
		ProtoInfo *pi = &protos[i];
		if (strcmp(pi->proto.c_str(), proto) == 0)
			return pi;
	}

	HICON hIcon = LoadSkinnedProtoIcon(proto, ID_STATUS_ONLINE);
	if (hIcon == NULL)
		return NULL;

	HANDLE hImage = (HANDLE) CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM) hIcon, 0);
	if (hImage == NULL)
		return NULL;

	ProtoInfo tmp;
	tmp.proto = proto;
	tmp.hImage = hImage;
	protos.push_back(tmp);

	return &protos[protos.size() - 1];
}

static int ProtocolApplyIcon(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;

	char *proto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (IsEmpty(proto))
		return 0;

	ProtoInfo *pi = FindProto(proto);

	HANDLE hImage = NULL;
	if (pi != NULL)
		hImage = pi->hImage;

	ExtraIcon_SetIcon(hExtraProto, hContact, hImage);

	return 0;
}

static int ProtocolOnClick(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	CallService(MS_USERINFO_SHOWDIALOG, (WPARAM) hContact, 0);
	return 0;
}

static void ProtocolInit()
{
	hExtraProto = ExtraIcon_Register("protocol", "Account", "core_main_34", &ProtocolRebuildIcons, &ProtocolApplyIcon,
			&ProtocolOnClick);
}
