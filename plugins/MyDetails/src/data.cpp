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


#include "commons.h"
#include "data.h"

static char *StatusModeToDbSetting(int status,const char *suffix);

ProtocolArray *protocols = NULL;

void InitProtocolData()
{
	PROTOACCOUNT **protos;
	int count;
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&protos);

	protocols = new ProtocolArray(count);

	for (int i = 0; i < count; i++) {
		PROTOACCOUNT* acc = protos[i];
		if (acc->szModuleName == NULL || acc->szModuleName[0] == '\0')
			continue;

		// Found a protocol
		Protocol *p = new Protocol(acc->szModuleName, acc->tszAccountName);
		if ( p->IsValid())
			protocols->Add(p);
		else
			delete p;
	}
}

void DeInitProtocolData()
{
	delete protocols;
}


// Protocol Class ///////////////////////////////////////////////////////////////////////////////////////////

Protocol::Protocol(const char *aName, const TCHAR* descr)
{
	lstrcpynA(name, aName, SIZEOF(name));
	lstrcpyn(description, descr, SIZEOF(description));

	nickname[0] = _T('\0');
	status_message[0] = _T('\0');
	listening_to[0] = _T('\0');
	ace = NULL;
	avatar_file[0] = _T('\0');
	avatar_bmp = NULL;
	custom_status = 0;
	data_changed = true;

	// Load services
	int caps = CallProtoService(name, PS_GETCAPS, PFLAGNUM_1, 0);
	if ((caps & PF1_IM) && !(!strcmp(aName, "MetaContacts") || !strcmp(aName, "NewsAggr")))
		valid = true;
	else
		valid = false;
	if (!valid)
		return;

	can_have_listening_to = (ProtoServiceExists(name, PS_SET_LISTENINGTO) != 0);

	caps = CallProtoService(name, PS_GETCAPS, PFLAGNUM_4, 0);
	can_have_avatar = (caps & PF4_AVATARS) != 0;

	PF3 = CallProtoService(name, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0);

	avatar_max_width = 0;
	avatar_max_height = 0;
	if (ProtoServiceExists(name, PS_GETMYAVATARMAXSIZE))
		CallProtoService(name, PS_GETMYAVATARMAXSIZE, (WPARAM) &avatar_max_width, (LPARAM) &avatar_max_height);

	can_set_nick = ProtoServiceExists(name, PS_SETMYNICKNAME) != FALSE;

	// Initial value
	GetStatus();
	GetStatusMsg();
	GetNick();
	GetAvatar();
}

Protocol::~Protocol()
{
}

void Protocol::lcopystr(TCHAR *dest, TCHAR *src, size_t maxlen)
{
	if (lstrcmp(dest, src) != 0)
	{
		data_changed = true;
		lstrcpyn(dest, src, maxlen);
	}
}

bool Protocol::IsValid()
{
	return valid;
}

int Protocol::GetStatus()
{
	int old_status = status;
	INT_PTR iStatus = CallProtoService(name, PS_GETSTATUS, 0, 0);
	if (iStatus == CALLSERVICE_NOTFOUND)
		return status = ID_STATUS_OFFLINE;

	status = (int)iStatus;

	if (old_status != status)
		data_changed = true;

	// check if protocol supports custom status
	if (ProtoServiceExists(name, PS_ICQ_GETCUSTOMSTATUS))
		// check if custom status is set
		custom_status = CallProtoService(name, PS_ICQ_GETCUSTOMSTATUS, (WPARAM) &custom_status_name, (LPARAM) &custom_status_message);
	else
		custom_status = 0;

	// if protocol supports custom status, but it is not set (custom_status will be -1), show normal status
	if (custom_status < 0) custom_status = 0;

	if (custom_status == 0) {
		TCHAR *tmp = (TCHAR*) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, GSMDF_TCHAR);
		lcopystr(status_name, tmp, SIZEOF(status_name));
	} else {
		DBVARIANT dbv;
		TCHAR tmp[256]; tmp[0] = 0;

		if (custom_status_name != NULL && custom_status_name[0] != '\0' && !DBGetContactSettingTString(0, name, custom_status_name, &dbv)) {
			if (dbv.ptszVal != NULL && dbv.ptszVal[0] != _T('\0'))
				lstrcpyn(tmp, dbv.ptszVal, SIZEOF(tmp));
			else
				lstrcpyn(tmp, TranslateT("<no status name>"), SIZEOF(tmp));

			DBFreeVariant(&dbv);
		} else {
			lstrcpyn(tmp, TranslateT("<no status name>"), SIZEOF(tmp));
		}

		if (custom_status_message != NULL && custom_status_message[0] != '\0' && !DBGetContactSettingTString(0, name, custom_status_message, &dbv)) {
			if (dbv.ptszVal != NULL && dbv.ptszVal[0] != '\0') {
				int len = lstrlen(tmp);

				if (len < SIZEOF(tmp))
					lstrcpyn(&tmp[len], _T(": "), SIZEOF(tmp) - len);

				len += 2;

				if (len < SIZEOF(tmp))
					lstrcpyn(&tmp[len], dbv.ptszVal, SIZEOF(tmp) - len);
			}

			DBFreeVariant(&dbv);
		}

		lcopystr(status_name, tmp, SIZEOF(status_name));
	}

	return status;
}

void Protocol::SetStatus(int aStatus)
{
	TCHAR status_msg[256];

	if (ServiceExists(MS_CS_SETSTATUSEX))
	{
		// :'(

		// BEGIN From commomstatus.cpp (KeepStatus)
		int i, count, pCount;
		PROTOACCOUNT **accs;

		pCount = 0;
		ProtoEnumAccounts(&count, &accs);
		for (i=0; i < count; i++) {
			if ( CallProtoService(accs[i]->szModuleName,PS_GETCAPS,PFLAGNUM_2,0)==0)
				continue;
			pCount++;
		}
		// END From commomstatus.cpp (KeepStatus)


		PROTOCOLSETTINGEX **pse = (PROTOCOLSETTINGEX **) mir_calloc(pCount * sizeof(PROTOCOLSETTINGEX *));

		for (i = 0; i < pCount; i++) {
			pse[i] = (PROTOCOLSETTINGEX *) mir_calloc(sizeof(PROTOCOLSETTINGEX));
			pse[i]->szName = "";
		}

		pse[0]->cbSize = sizeof(PROTOCOLSETTINGEX);
		pse[0]->status = aStatus;
		pse[0]->szName = name;

		GetStatusMsg(aStatus, status_msg, SIZEOF(status_msg));
		pse[0]->szMsg = status_msg;

		CallService(MS_CS_SETSTATUSEX, (WPARAM) &pse, 0);

		for (i = 0; i < pCount; i++)
			mir_free(pse[i]);
		mir_free(pse);
	}
	else
	{
		CallProtoService(name, PS_SETSTATUS, aStatus, 0);

		if (CanSetStatusMsg(aStatus)) {
			TCHAR status_msg[MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE];
			GetStatusMsg(aStatus, status_msg, SIZEOF(status_msg));
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
	return (CallProtoService(name, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) != 0
			&& (PF3 & Proto_Status2Flag(aStatus));
}


bool Protocol::CanSetStatusMsg()
{
	return CanSetStatusMsg(GetStatus()); // <- Simple away handled by this one
}

bool Protocol::CanSetStatusMsg(int aStatus)
{
	return CanGetStatusMsg(aStatus);
}

void Protocol::GetStatusMsg(int aStatus, TCHAR *msg, size_t msg_size)
{
	if ( !CanGetStatusMsg())
	{
		lcopystr(msg, _T(""), msg_size);
		return;
	}

	if (aStatus == status && ProtoServiceExists(name, PS_GETMYAWAYMSG) )
	{
		TCHAR *tmp = (TCHAR*) CallProtoService(name, PS_GETMYAWAYMSG, 0, SGMA_TCHAR);
		lcopystr(msg, tmp == NULL ? _T("") : tmp, msg_size);
	}

	else if (ServiceExists(MS_AWAYMSG_GETSTATUSMSG))
	{
		TCHAR *tmp = (TCHAR*) CallService(MS_AWAYMSG_GETSTATUSMSGT, (WPARAM)aStatus, 0);
		if (tmp != NULL)
		{
			lcopystr(msg, tmp, msg_size);
			mir_free(tmp);
		}
		else lcopystr(msg, _T(""), msg_size);
	}
}

TCHAR* Protocol::GetStatusMsg()
{
	GetStatusMsg(status, status_message, SIZEOF(status_message));
	return status_message;
}

void Protocol::SetStatusMsg(const TCHAR *message)
{
	SetStatusMsg(GetStatus(), message);
}

void Protocol::SetStatusMsg(int aStatus, const TCHAR *message)
{
	if ( !CanSetStatusMsg(aStatus))
		return;

	CallProtoService(name, PS_SETAWAYMSGT, (WPARAM)aStatus, (LPARAM)message);
}

bool Protocol::HasAvatar()
{
	GetAvatar();

	return avatar_bmp != NULL;
}

bool Protocol::CanGetAvatar()
{
	if ( !can_have_avatar)
		return false;

	if ( !ServiceExists(MS_AV_GETMYAVATAR))
		return false;

	return true;
}

void Protocol::GetAvatar()
{
	// See if can get one
	if ( !CanGetAvatar())
		return;

	avatar_file[0] = '\0';
	avatar_bmp = NULL;
	ace = NULL;

	// Get HBITMAP from cache
	ace = (avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM) name);
	if (ace != NULL)
		avatar_bmp = ace->hbmPic;

	data_changed = true;
}


bool Protocol::CanGetNick()
{
	return ServiceExists(MS_CONTACT_GETCONTACTINFO) != FALSE;
}

int Protocol::GetNickMaxLength()
{
	if (ProtoServiceExists(name, PS_GETMYNICKNAMEMAXLENGTH))
	{
		int ret = CallProtoService(name, PS_GETMYNICKNAMEMAXLENGTH, 0, 0);
		if (ret <= 0)
			ret = MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE;
		return ret;
	}
	else
		return MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE;
}

TCHAR* Protocol::GetNick()
{
	// See if can get one
	if ( !CanGetNick())
		return NULL;

	// Get it
	CONTACTINFO ci = { 0 };
	ci.cbSize = sizeof(ci);
	ci.hContact = NULL;
	ci.szProto = name;
	ci.dwFlag = CNF_DISPLAY;

#ifdef UNICODE
	ci.dwFlag |= CNF_UNICODE;
#endif

	if ( !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci))
	{
		// CNF_DISPLAY always returns a string type
		lcopystr(nickname, ci.pszVal, SIZEOF(nickname));
		mir_free(ci.pszVal);
	}
	else lcopystr(nickname, _T(""), SIZEOF(nickname));

	return nickname;
}


bool Protocol::CanSetNick()
{
	return can_set_nick;
}


void Protocol::SetNick(const TCHAR *nick)
{
	// See if can get one
	if ( !CanSetNick())
		return;

	if (nick == NULL)
		return;

	// Get it
	CallProtoService(name, PS_SETMYNICKNAME, SMNN_TCHAR, (LPARAM)nick);
}


bool Protocol::CanSetAvatar()
{
	return ServiceExists(MS_AV_SETMYAVATAR) != FALSE && ServiceExists(MS_AV_CANSETMYAVATAR) != FALSE &&
			CallService(MS_AV_CANSETMYAVATAR, (WPARAM) name, 0);
}

void Protocol::SetAvatar(const TCHAR *file_name)
{
	if ( !CanSetAvatar())
		return;

	CallService(MS_AV_SETMYAVATART, (WPARAM) name, (LPARAM) file_name);
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
	return CanSetListeningTo() && CallService(MS_LISTENINGTO_ENABLED, (WPARAM) name, 0) != 0;
}

TCHAR * Protocol::GetListeningTo()
{
	if ( !CanGetListeningTo())
	{
		lcopystr(listening_to, _T(""), SIZEOF(listening_to));
		return listening_to;
	}

	DBVARIANT dbv = {0};
	if ( DBGetContactSettingTString(NULL, name, "ListeningTo", &dbv))
	{
		lcopystr(listening_to, _T(""), SIZEOF(listening_to));
		return listening_to;
	}

	lcopystr(listening_to, dbv.ptszVal, SIZEOF(listening_to));
	DBFreeVariant(&dbv);
	return listening_to;
}

// ProtocolDataArray Class /////////////////////////////////////////////////////////////////////////////

ProtocolArray::ProtocolArray(int max_size)
{
	buffer = (Protocol **) malloc(max_size * sizeof(Protocol*));
	buffer_len = 0;

	GetDefaultNick();
	GetDefaultAvatar();
}


ProtocolArray::~ProtocolArray()
{
	if (buffer != NULL) {
		for ( int i = 0 ; i < buffer_len ; i++ )
			delete buffer[i];
		free(buffer);
	}
}


int ProtocolArray::GetSize()
{
	return buffer_len;
}


void ProtocolArray::Add(Protocol *p)
{
	buffer[buffer_len] = p;
	buffer_len++;
}


Protocol * ProtocolArray::Get(int i)
{
	if (i >= buffer_len)
		return NULL;
	else
		return buffer[i];
}


Protocol * ProtocolArray::Get(const char *name)
{
	if (name == NULL)
		return NULL;

	for ( int i = 0 ; i < buffer_len ; i++ )
	{
		if (strcmp(name, buffer[i]->name) == 0)
			return buffer[i];
	}

	return NULL;
}


bool ProtocolArray::CanSetStatusMsgPerProtocol()
{
	return ServiceExists(MS_SA_CHANGESTATUSMSG) != 0;
}


void ProtocolArray::GetAvatars()
{
	for ( int i = 0 ; i < buffer_len ; i++ )
	{
		buffer[i]->GetAvatar();
	}
}

void ProtocolArray::GetStatusMsgs()
{
	for ( int i = 0 ; i < buffer_len ; i++ )
		buffer[i]->GetStatusMsg();
}

void ProtocolArray::GetStatuses()
{
	for ( int i = 0 ; i < buffer_len ; i++ )
		buffer[i]->GetStatus();
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
	return ServiceExists(MS_AV_SETMYAVATART) != FALSE;
}

void ProtocolArray::SetAvatars(const TCHAR *file_name)
{
	if ( !CanSetAvatars())
		return;

	CallService(MS_AV_SETMYAVATART, NULL, (WPARAM) file_name);
}

void ProtocolArray::SetNicks(const TCHAR *nick)
{
	if (nick == NULL || nick[0] == '\0')
		return;

	lstrcpyn(default_nick, nick, SIZEOF(default_nick));

	DBWriteContactSettingTString(0, MODULE_NAME, SETTING_DEFAULT_NICK, nick);

	for ( int i = 0 ; i < buffer_len ; i++ )
		buffer[i]->SetNick(default_nick);
}


void ProtocolArray::SetStatus(int aStatus)
{
	CallService(MS_CLIST_SETSTATUSMODE, aStatus, 0);
}

void ProtocolArray::SetStatusMsgs(const TCHAR *message)
{
	for (int i = ID_STATUS_OFFLINE ; i <= ID_STATUS_IDLE; i++)
		SetStatusMsgs(i, message);
}

void ProtocolArray::SetStatusMsgs(int status, const TCHAR *message)
{
	DBWriteContactSettingTString(NULL,"SRAway",StatusModeToDbSetting(status,"Msg"),message);

	// Save default also
	if ( !db_get_b(NULL,"SRAway",StatusModeToDbSetting(status,"UsePrev"),0))
		DBWriteContactSettingTString(NULL,"SRAway",StatusModeToDbSetting(status,"Default"),message);

	for ( int i = 0 ; i < buffer_len ; i++ )
		if (buffer[i]->status == status)
			buffer[i]->SetStatusMsg(status, message);
}

void ProtocolArray::GetDefaultNick()
{
	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(0, MODULE_NAME, SETTING_DEFAULT_NICK, &dbv)) {
		lstrcpyn(default_nick, dbv.ptszVal, SIZEOF(default_nick));
		DBFreeVariant(&dbv);
	}
	else default_nick[0] = '\0';
}

void ProtocolArray::GetDefaultAvatar()
{
	DBVARIANT dbv;
	if ( !DBGetContactSettingTString(0, "ContactPhoto", "File", &dbv)) {
		lstrcpyn(default_avatar_file, dbv.ptszVal, SIZEOF(default_avatar_file));
		DBFreeVariant(&dbv);
	}
	else default_avatar_file[0] = '\0';
}

TCHAR* ProtocolArray::GetDefaultStatusMsg()
{
	return GetDefaultStatusMsg(CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
}

TCHAR* ProtocolArray::GetDefaultStatusMsg(int status)
{
	default_status_message[0] = '\0';

	if (ServiceExists(MS_AWAYMSG_GETSTATUSMSG)) {
		if (status == ID_STATUS_CONNECTING)
			status = ID_STATUS_OFFLINE;

		TCHAR *tmp = (TCHAR*) CallService(MS_AWAYMSG_GETSTATUSMSGT, (WPARAM)status, 0);
		if (tmp != NULL) {
			lstrcpyn(default_status_message, tmp, SIZEOF(default_status_message));
			mir_free(tmp);
		}
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

static char *StatusModeToDbSetting(int status,const char *suffix)
{
	char *prefix;
	static char str[64];

	switch(status) {
		case ID_STATUS_AWAY: prefix="Away";	break;
		case ID_STATUS_NA: prefix="Na";	break;
		case ID_STATUS_DND: prefix="Dnd"; break;
		case ID_STATUS_OCCUPIED: prefix="Occupied"; break;
		case ID_STATUS_FREECHAT: prefix="FreeChat"; break;
		case ID_STATUS_ONLINE: prefix="On"; break;
		case ID_STATUS_OFFLINE: prefix="Off"; break;
		case ID_STATUS_INVISIBLE: prefix="Inv"; break;
		case ID_STATUS_ONTHEPHONE: prefix="Otp"; break;
		case ID_STATUS_OUTTOLUNCH: prefix="Otl"; break;
		case ID_STATUS_IDLE: prefix="Idl"; break;
		default: return NULL;
	}
	lstrcpyA(str,prefix); lstrcatA(str,suffix);
	return str;
}
