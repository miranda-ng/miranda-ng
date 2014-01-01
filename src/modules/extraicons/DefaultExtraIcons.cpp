/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-14 Miranda NG project

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

// DB extra icons ///////////////////////////////////////////////////////////////////////

struct Info;

HANDLE hExtraVisibility = NULL;
HANDLE hExtraChat = NULL;
HANDLE hExtraGender = NULL;

static void SetVisibility(HANDLE hContact, int apparentMode, bool clear)
{
	if (hContact == NULL)
		return;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
		return;

	if (apparentMode <= 0)
		apparentMode = db_get_w(hContact, proto, "ApparentMode", 0);

	HANDLE hExtraIcon, hIcolib = NULL;

	if ( db_get_b(hContact, proto, "ChatRoom", 0)) {
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

static void SetGender(HANDLE hContact, int gender, bool clear)
{
	if (hContact == NULL)
		return;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
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

static void EmailOnClick(Info *info, const char *text);
static void HomepageOnClick(Info *info, const char *text);
static void DefaultSetIcon(HANDLE hContact, Info *info, const char *text);

struct Info
{
	const char *name;
	const char *desc;
	int         iSkinIcon;
	const char *db[8];
	void (*SetIcon)(HANDLE hContact, Info *info, const char *text);
	void (*OnClick)(Info *info, const char *text);

	HANDLE hIcolib, hExtraIcon;
}
static infos[] =
{
	{ "homepage", "Homepage", SKINICON_EVENT_URL,
		{ NULL, "Homepage", "UserInfo", "Homepage" },
		DefaultSetIcon, &HomepageOnClick },
	{ "sms", "Phone/SMS", SKINICON_OTHER_SMS,
		{ NULL, "Cellular", "UserInfo", "Cellular", "UserInfo", "Phone", "UserInfo", "MyPhone0" },
		DefaultSetIcon, NULL },
	{ "email", "E-mail", SKINICON_OTHER_SENDEMAIL,
		{ NULL, "e-mail", "UserInfo", "e-mail", "UserInfo", "Mye-mail0" },
		DefaultSetIcon, &EmailOnClick },
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

static void DefaultSetIcon(HANDLE hContact, Info *info, const char *text)
{
	ExtraIcon_SetIcon(info->hExtraIcon, hContact, text ? info->hIcolib : NULL);
}

static void SetExtraIcons(HANDLE hContact)
{
	if (hContact == NULL)
		return;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
		return;

	for (unsigned int i = 0; i < SIZEOF(infos); i++) {
		Info &p = infos[i];

		bool show = false;
		for (unsigned int j = 0; !show && j < SIZEOF(p.db); j += 2) {
			if (p.db[j + 1] == NULL)
				break;

			DBVARIANT dbv;
			if (!db_get_s(hContact, p.db[j] == NULL ? proto : p.db[j], p.db[j+1], &dbv)) {
				if (!IsEmpty(dbv.pszVal)) {
					p.SetIcon(hContact, &p, dbv.pszVal);
					show = true;
				}
				db_free(&dbv);
			}
		}
	}
}

static int SettingChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*) lParam;

	if (hContact == NULL)
		return 0;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
		return 0;

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
			p.SetIcon(hContact, &p, show ? cws->value.pszVal : NULL);

			break;
		}
	}

	return 0;
}

static int DefaultOnClick(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	Info *p = (Info*)param;
	if (p == NULL)
		return 0;

	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL)
		return 0;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
		return 0;

	bool found = false;
	for (int j = 0; !found && j < SIZEOF(p->db); j += 2) {
		if (p->db[j + 1] == NULL)
			break;

		DBVARIANT dbv;
		if ( !db_get_s(hContact, p->db[j] == NULL ? proto : p->db[j], p->db[j+1], &dbv)) {
			if (!IsEmpty(dbv.pszVal)) {
				p->OnClick(p, dbv.pszVal);
				found = true;
			}

			db_free(&dbv);
		}
	}

	return 0;
}

// Protocol /////////////////////////////////////////////////////////////////////////////

struct ProtoInfo
{
	string proto;
	HANDLE hImage;
};

vector<ProtoInfo> protos;

static HANDLE hExtraProto;

static int ProtocolRebuildIcons(WPARAM wParam, LPARAM lParam)
{
	protos.clear();
	return 0;
}

static ProtoInfo *FindProto(const char * proto)
{
	for (unsigned int i = 0; i < protos.size(); i++) {
		ProtoInfo *pi = &protos[i];
		if (strcmp(pi->proto.c_str(), proto) == 0)
			return pi;
	}

	HICON hIcon = LoadSkinnedProtoIcon(proto, ID_STATUS_ONLINE);
	if (hIcon == NULL)
		return NULL;

	HANDLE hImage = ExtraIcon_Add(hIcon);
	if (hImage == INVALID_HANDLE_VALUE)
		return NULL;

	ProtoInfo tmp;
	tmp.proto = proto;
	tmp.hImage = hImage;
	protos.push_back(tmp);

	return &protos[protos.size() - 1];
}

static int ProtocolApplyIcon(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;

	char *proto = GetContactProto(hContact);
	if ( IsEmpty(proto))
		return 0;

	ProtoInfo *pi = FindProto(proto);

	HANDLE hImage = INVALID_HANDLE_VALUE;
	if (pi != NULL)
		hImage = pi->hImage;

	ExtraIcon_SetIcon(hExtraProto, hContact, hImage);

	return 0;
}

static int ProtocolOnClick(WPARAM wParam, LPARAM lParam, LPARAM param)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL)
		return 0;

	CallService(MS_USERINFO_SHOWDIALOG, (WPARAM) hContact, 0);
	return 0;
}

void DefaultExtraIcons_Load()
{
	hExtraChat = ExtraIcon_Register("chat_activity", LPGEN("Chat activity"), "ChatActivity");
	hExtraVisibility = ExtraIcon_Register("visibility", "Visibility", LoadSkinnedIconName(SKINICON_OTHER_VISIBLE_ALL));
	hExtraGender = ExtraIcon_Register("gender", "Gender", "gender_male");
	hExtraProto = ExtraIcon_Register("protocol", "Account", LoadSkinnedIconName(SKINICON_OTHER_ACCMGR),
		&ProtocolRebuildIcons, &ProtocolApplyIcon, &ProtocolOnClick);

	for (unsigned int i = 0; i < SIZEOF(infos); i++) {
		Info &p = infos[i];
		p.hIcolib = LoadSkinnedIconHandle(p.iSkinIcon);
		if (p.OnClick)
			p.hExtraIcon = ExtraIcon_Register(p.name, p.desc, LoadSkinnedIconName(p.iSkinIcon), DefaultOnClick, (LPARAM) &p);
		else
			p.hExtraIcon = ExtraIcon_Register(p.name, p.desc, LoadSkinnedIconName(p.iSkinIcon));
	}

	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		SetExtraIcons(hContact);
		SetVisibility(hContact, -1, false);
		SetGender(hContact, -1, false);
	}

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);
}
