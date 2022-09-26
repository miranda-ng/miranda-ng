/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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

static char *StatusModeToDbSetting(int status, const char *suffix);

ProtocolArray protocols;

void InitProtocolData()
{
	for (auto &pa : Accounts()) {
		if (pa->szModuleName == nullptr || pa->szModuleName[0] == '\0' || pa->bIsVirtual)
			continue;

		// Found a protocol
		Protocol *p = new Protocol(pa->szModuleName, pa->tszAccountName);
		if (p->IsValid())
			protocols.insert(p);
		else
			delete p;
	}
}

void DeInitProtocolData()
{
	for (auto &it : protocols)
		delete it;
	protocols.destroy();
}

// Protocol Class ///////////////////////////////////////////////////////////////////////////////////////////

Protocol::Protocol(const char *aName, const wchar_t *descr)
{
	mir_strncpy(name, aName, _countof(name));
	mir_wstrncpy(description, descr, _countof(description));

	data_changed = true;

	// Load services
	int caps = CallProtoService(name, PS_GETCAPS, PFLAGNUM_1, 0);
	if (caps & PF1_IM)
		valid = true;
	else
		return;

	can_have_listening_to = (ProtoServiceExists(name, PS_SET_LISTENINGTO) != 0);

	PF3 = CallProtoService(name, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0);
	caps = CallProtoService(name, PS_GETCAPS, PFLAGNUM_4, 0);
	can_have_avatar = (caps & PF4_AVATARS) != 0;
	can_set_nick = ProtoServiceExists(name, PS_SETMYNICKNAME) != FALSE;

	// Initial value
	GetStatus();
	GetStatusMsg();
	GetNick();
	GetAvatar();
}

Protocol::~Protocol()
{}

void Protocol::lcopystr(wchar_t *dest, wchar_t *src, size_t maxlen)
{
	if (mir_wstrcmp(dest, src) != 0) {
		data_changed = true;
		mir_wstrncpy(dest, src, (uint32_t)maxlen);
	}
}

bool Protocol::IsValid()
{
	return valid;
}

int Protocol::GetStatus()
{
	int iStatus = Proto_GetStatus(name);
	if (iStatus != status)
		data_changed = true;

	status = iStatus;

	// check if protocol supports custom status
	CUSTOM_STATUS css = { sizeof(css) };
	wchar_t tszXStatusName[256], tszXStatusMessage[1024];
	if (ProtoServiceExists(name, PS_GETCUSTOMSTATUSEX)) {
		// check if custom status is set
		css.flags = CSSF_UNICODE | CSSF_MASK_STATUS | CSSF_MASK_NAME | CSSF_MASK_MESSAGE | CSSF_DEFAULT_NAME;
		css.status = &custom_status;
		css.ptszName = tszXStatusName;
		css.ptszMessage = tszXStatusMessage;
		if (CallProtoService(name, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&css) != 0)
			tszXStatusMessage[0] = tszXStatusName[0] = 0, custom_status = 0;
	}
	else custom_status = 0;

	// if protocol supports custom status, but it is not set (custom_status will be -1), show normal status
	if (custom_status < 0)
		custom_status = 0;

	if (custom_status == 0) {
		wchar_t *tmp = Clist_GetStatusModeDescription(status, 0);
		lcopystr(status_name, tmp, _countof(status_name));
	}
	else {
		wchar_t *p = (tszXStatusName[0] != 0) ? TranslateW(tszXStatusName) : TranslateT("<no status name>");
		if (tszXStatusMessage[0])
			mir_snwprintf(status_name, L"%s: %s", p, tszXStatusMessage);
		else
			lcopystr(status_name, p, _countof(status_name));
	}

	return status;
}

void Protocol::SetStatus(int aStatus)
{
	if (ServiceExists(MS_CS_SETSTATUSEX)) {
		int pCount = 0;
		for (auto &pa : Accounts())
			if (CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) != 0)
				pCount++;
	
		PROTOCOLSETTINGEX **pse = (PROTOCOLSETTINGEX **)mir_calloc(pCount * sizeof(PROTOCOLSETTINGEX *));
		for (int i = 0; i < pCount; i++) {
			pse[i] = (PROTOCOLSETTINGEX *)mir_calloc(sizeof(PROTOCOLSETTINGEX));
			pse[i]->m_szName = "";
		}

		pse[0]->m_status = aStatus;
		pse[0]->m_szName = name;

		wchar_t status_msg[256];
		GetStatusMsg(aStatus, status_msg, _countof(status_msg));
		pse[0]->m_szMsg = status_msg;

		CallService(MS_CS_SETSTATUSEX, (WPARAM)&pse, pCount);

		for (int i = 0; i < pCount; i++)
			mir_free(pse[i]);
		mir_free(pse);
	}
	else {
		CallProtoService(name, PS_SETSTATUS, aStatus, 0);

		if (CanSetStatusMsg(aStatus)) {
			wchar_t status_msg[MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE];
			GetStatusMsg(aStatus, status_msg, _countof(status_msg));
			SetStatusMsg(aStatus, status_msg);
		}
	}
}

bool Protocol::CanGetStatusMsg()
{
	return CanGetStatusMsg(status);
}

bool Protocol::CanGetStatusMsg(int aStatus)
{
	return (CallProtoService(name, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) != 0 && (PF3 & Proto_Status2Flag(aStatus));
}

bool Protocol::CanSetStatusMsg()
{
	return CanSetStatusMsg(GetStatus()); // <- Simple away handled by this one
}

bool Protocol::CanSetStatusMsg(int aStatus)
{
	return CanGetStatusMsg(aStatus);
}

void Protocol::GetStatusMsg(int aStatus, wchar_t *msg, size_t msg_size)
{
	if (!CanGetStatusMsg())
		lcopystr(msg, L"", msg_size);
	else if (aStatus == status && ProtoServiceExists(name, PS_GETMYAWAYMSG)) {
		ptrW tmp((wchar_t *)CallProtoService(name, PS_GETMYAWAYMSG, 0, SGMA_UNICODE));
		lcopystr(msg, tmp == NULL ? L"" : tmp, msg_size);
	}
	else if (ServiceExists(MS_AWAYMSG_GETSTATUSMSGW)) {
		ptrW tmp((wchar_t *)CallService(MS_AWAYMSG_GETSTATUSMSGW, (WPARAM)aStatus, (LPARAM)name));
		lcopystr(msg, tmp == NULL ? L"" : tmp, msg_size);
	}
}

wchar_t* Protocol::GetStatusMsg()
{
	GetStatusMsg(status, status_message, _countof(status_message));
	return status_message;
}

void Protocol::SetStatusMsg(const wchar_t *message)
{
	SetStatusMsg(GetStatus(), message);
}

void Protocol::SetStatusMsg(int aStatus, const wchar_t *message)
{
	if (!CanSetStatusMsg(aStatus))
		return;

	CallProtoService(name, PS_SETAWAYMSG, (WPARAM)aStatus, (LPARAM)message);
}

bool Protocol::HasAvatar()
{
	GetAvatar();

	return avatar_bmp != nullptr;
}

bool Protocol::CanGetAvatar()
{
	return (can_have_avatar && g_bAvsExist);
}

void Protocol::GetAvatar()
{
	// See if can get one
	if (!CanGetAvatar())
		return;

	avatar_file[0] = '\0';
	avatar_bmp = nullptr;
	ace = nullptr;

	// Get HBITMAP from cache
	ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)name);
	if (ace != nullptr)
		avatar_bmp = ace->hbmPic;

	data_changed = true;
}


int Protocol::GetNickMaxLength()
{
	if (!ProtoServiceExists(name, PS_GETMYNICKNAMEMAXLENGTH))
		return MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE;

	int ret = CallProtoService(name, PS_GETMYNICKNAMEMAXLENGTH, 0, 0);
	if (ret <= 0)
		ret = MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE;
	return ret;
}

wchar_t* Protocol::GetNick()
{
	ptrW nick(Contact::GetInfo(CNF_DISPLAY, NULL, name));
	lcopystr(nickname, (nick != NULL) ? nick : L"", _countof(nickname));
	return nickname;
}

bool Protocol::CanSetNick()
{
	return can_set_nick;
}

void Protocol::SetNick(const wchar_t *nick)
{
	// See if can get one
	if (!CanSetNick())
		return;

	if (nick == nullptr)
		return;

	// Get it
	CallProtoService(name, PS_SETMYNICKNAME, SMNN_UNICODE, (LPARAM)nick);
}

bool Protocol::CanSetAvatar()
{
	return g_bAvsExist && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)name, 0);
}

void Protocol::SetAvatar(const wchar_t *file_name)
{
	if (CanSetAvatar())
		CallService(MS_AV_SETMYAVATARW, (WPARAM)name, (LPARAM)file_name);
}

bool Protocol::CanGetListeningTo()
{
	return can_have_listening_to;
}

bool Protocol::CanSetListeningTo()
{
	return CanGetListeningTo() && ServiceExists(MS_LISTENINGTO_ENABLE);
}

bool Protocol::ListeningToEnabled()
{
	return CanSetListeningTo() && CallService(MS_LISTENINGTO_ENABLED, (WPARAM)name, 0) != 0;
}

wchar_t *Protocol::GetListeningTo()
{
	if (!CanGetListeningTo()) {
		lcopystr(listening_to, L"", _countof(listening_to));
		return listening_to;
	}

	DBVARIANT dbv = { 0 };
	if (db_get_ws(0, name, "ListeningTo", &dbv)) {
		lcopystr(listening_to, L"", _countof(listening_to));
		return listening_to;
	}

	lcopystr(listening_to, dbv.pwszVal, _countof(listening_to));
	db_free(&dbv);
	return listening_to;
}

// ProtocolDataArray Class /////////////////////////////////////////////////////////////////////////////

ProtocolArray::ProtocolArray() :
	LIST<Protocol>(10)
{
	GetDefaultNick();
	GetDefaultAvatar();
}

Protocol* ProtocolArray::GetByName(const char *name)
{
	if (name == nullptr)
		return nullptr;

	for (auto &it: *this)
		if (mir_strcmp(name, it->name) == 0)
			return it;

	return nullptr;
}


bool ProtocolArray::CanSetStatusMsgPerProtocol()
{
	return ServiceExists(MS_SIMPLESTATUSMSG_CHANGESTATUSMSG) != 0;
}

void ProtocolArray::GetAvatars()
{
	for (auto &it: *this)
		it->GetAvatar();
}

void ProtocolArray::GetStatusMsgs()
{
	for (auto &it: *this)
		it->GetStatusMsg();
}

void ProtocolArray::GetStatuses()
{
	for (auto &it: *this)
		it->GetStatus();
}

int ProtocolArray::GetGlobalStatus()
{
	int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	if (status == ID_STATUS_CONNECTING)
		status = ID_STATUS_OFFLINE;

	return status;
}

bool ProtocolArray::CanSetAvatars()
{
	return g_bAvsExist;
}

void ProtocolArray::SetAvatars(const wchar_t *file_name)
{
	if (!CanSetAvatars())
		return;

	CallService(MS_AV_SETMYAVATARW, NULL, (WPARAM)file_name);
}

void ProtocolArray::SetNicks(const wchar_t *nick)
{
	if (nick == nullptr || nick[0] == '\0')
		return;

	mir_wstrncpy(default_nick, nick, _countof(default_nick));

	g_plugin.setWString(SETTING_DEFAULT_NICK, nick);

	for (auto &it: *this)
		it->SetNick(default_nick);
}

void ProtocolArray::SetStatus(int aStatus)
{
	Clist_SetStatusMode(aStatus);
}

void ProtocolArray::SetStatusMsgs(const wchar_t *message)
{
	for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_IDLE; i++)
		SetStatusMsgs(i, message);
}

void ProtocolArray::SetStatusMsgs(int status, const wchar_t *message)
{
	db_set_ws(0, "SRAway", StatusModeToDbSetting(status, "Msg"), message);

	// Save default also
	if (!db_get_b(0, "SRAway", StatusModeToDbSetting(status, "UsePrev"), 0))
		db_set_ws(0, "SRAway", StatusModeToDbSetting(status, "Default"), message);

	for (auto &it: *this)
		if (it->status == status)
			it->SetStatusMsg(status, message);
}

void ProtocolArray::GetDefaultNick()
{
	ptrW tszNick(g_plugin.getWStringA(SETTING_DEFAULT_NICK));
	if (tszNick)
		wcsncpy_s(default_nick, tszNick, _TRUNCATE);
	else
		default_nick[0] = '\0';
}

void ProtocolArray::GetDefaultAvatar()
{
	wcsncpy_s(default_avatar_file, ptrW(db_get_wsa(0, "ContactPhoto", "File", L"")), _TRUNCATE);
}

wchar_t* ProtocolArray::GetDefaultStatusMsg()
{
	return GetDefaultStatusMsg(CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
}

wchar_t* ProtocolArray::GetDefaultStatusMsg(int status)
{
	default_status_message[0] = '\0';

	if (status == ID_STATUS_CONNECTING)
		status = ID_STATUS_OFFLINE;

	wchar_t *tmp = (wchar_t *)CallService(MS_AWAYMSG_GETSTATUSMSGW, (WPARAM)status, 0);
	if (tmp != nullptr) {
		mir_wstrncpy(default_status_message, tmp, _countof(default_status_message));
		mir_free(tmp);
	}

	return default_status_message;
}

bool ProtocolArray::CanSetListeningTo()
{
	return ServiceExists(MS_LISTENINGTO_ENABLE) != 0;
}

bool ProtocolArray::ListeningToEnabled()
{
	return CanSetListeningTo() && CallService(MS_LISTENINGTO_ENABLED, 0, 0) != 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions

static char *StatusModeToDbSetting(int status, const char *suffix)
{
	char *prefix;
	static char str[64];

	switch (status) {
	case ID_STATUS_AWAY: prefix = "Away"; break;
	case ID_STATUS_NA: prefix = "Na"; break;
	case ID_STATUS_DND: prefix = "Dnd"; break;
	case ID_STATUS_OCCUPIED: prefix = "Occupied"; break;
	case ID_STATUS_FREECHAT: prefix = "FreeChat"; break;
	case ID_STATUS_ONLINE: prefix = "On"; break;
	case ID_STATUS_OFFLINE: prefix = "Off"; break;
	case ID_STATUS_INVISIBLE: prefix = "Inv"; break;
	case ID_STATUS_IDLE: prefix = "Idl"; break;
	default: return nullptr;
	}
	mir_strcpy(str, prefix);
	mir_strcat(str, suffix);
	return str;
}
