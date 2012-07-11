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
	PROTOCOLDESCRIPTOR **protos;
	int i, count;

	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);

	protocols = new ProtocolArray(count);

	for (i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;

		if (protos[i]->szName == NULL || protos[i]->szName[0] == '\0')
			continue;

		// Found a protocol
		Protocol *p = new Protocol(protos[i]->szName);

		if (p->IsValid())
		{
			protocols->Add(p);
		}
		else
		{
			delete p;
		}
	}
}


void DeInitProtocolData()
{
	delete protocols;
}


// Protocol Class ///////////////////////////////////////////////////////////////////////////////////////////


Protocol::Protocol(const char *aName)
{
	lstrcpyn(name, aName, MAX_REGS(name));

	description[0] = _T('\0');
	nickname[0] = _T('\0');
	status_message[0] = _T('\0');
	listening_to[0] = _T('\0');
	ace = NULL;
	avatar_file[0] = _T('\0');
	avatar_bmp = NULL;
	custom_status = 0;
	data_changed = true;

	// Load services

	int caps;

	caps = CallProtoService(name, PS_GETCAPS, PFLAGNUM_1, 0);
	valid = (caps & PF1_IM) == PF1_IM && strcmp(aName, "MetaContacts");

	if (!valid)
		return;

	can_have_listening_to = (ProtoServiceExists(name, PS_SET_LISTENINGTO) != 0);

	caps = CallProtoService(name, PS_GETCAPS, PFLAGNUM_4, 0);
	can_have_avatar = (caps & PF4_AVATARS) != 0;

	PF3 = CallProtoService(name, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0);

	avatar_max_width = 0;
	avatar_max_height = 0;
	if (ProtoServiceExists(name, PS_GETMYAVATARMAXSIZE))
	{
		CallProtoService(name, PS_GETMYAVATARMAXSIZE, (WPARAM) &avatar_max_width, (LPARAM) &avatar_max_height);
	}

	CallProtoService(name, PS_GETNAME, sizeof(description),(LPARAM) description);

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

void Protocol::lcopystr(TCHAR *dest, TCHAR *src, int maxlen)
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
	status = CallProtoService(name, PS_GETSTATUS, 0, 0);

	if (old_status != status)
		data_changed = true;

	if (/*status > ID_STATUS_OFFLINE &&*/ ProtoServiceExists(name, PS_ICQ_GETCUSTOMSTATUS))
	{
		custom_status = CallProtoService(name, PS_ICQ_GETCUSTOMSTATUS, (WPARAM) &custom_status_name, 
																		(LPARAM) &custom_status_message);
	}
	else
	{
		custom_status = 0;
	}

	if (custom_status == 0)
	{
		TCHAR *tmp = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, GCMDF_TCHAR);
		lcopystr(status_name, tmp, MAX_REGS(status_name));
	}
	else
	{
		DBVARIANT dbv;
		TCHAR tmp[256] = "\0";

		if (custom_status_name != NULL && custom_status_name[0] != '\0' 
				&& !DBGetContactSettingTString(0, name, custom_status_name, &dbv))
		{
			if (dbv.ptszVal != NULL && dbv.ptszVal[0] != _T('\0'))
				lstrcpyn(tmp, dbv.ptszVal, MAX_REGS(tmp));
			else
				lstrcpyn(tmp, TranslateTS("<no status name>"), MAX_REGS(tmp));

			DBFreeVariant(&dbv);
		}
		else
		{
			lstrcpyn(tmp, TranslateTS("<no status name>"), MAX_REGS(tmp));
		}

		if (custom_status_message != NULL && custom_status_message[0] != '\0' 
				&& !DBGetContactSettingTString(0, name, custom_status_message, &dbv))
		{
			if (dbv.ptszVal != NULL && dbv.ptszVal[0] != '\0')
			{
				int len = lstrlen(tmp);

				if (len < MAX_REGS(tmp))
					lstrcpyn(&tmp[len], _T(": "), MAX_REGS(tmp) - len);

				len += 2;

				if (len < MAX_REGS(tmp))
					lstrcpyn(&tmp[len], dbv.ptszVal, MAX_REGS(tmp) - len);
			}

			DBFreeVariant(&dbv);
		}

		lcopystr(status_name, tmp, MAX_REGS(status_name));
	}

	return status;
}

void Protocol::SetStatus(int aStatus)
{
	char status_msg[256];

	if (ServiceExists(MS_CS_SETSTATUSEX))
	{
		// :'(

		// BEGIN From commomstatus.cpp (KeepStatus)
		int i, count, pCount;
		PROTOCOLDESCRIPTOR **protos;

		pCount = 0;
		CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&count,(LPARAM)&protos);
		for(i=0;i<count;i++) {
			if(protos[i]->type!=PROTOTYPE_PROTOCOL || CallProtoService(protos[i]->szName,PS_GETCAPS,PFLAGNUM_2,0)==0) continue;
			pCount += 1;
		}
		// END From commomstatus.cpp (KeepStatus)


		PROTOCOLSETTINGEX **pse = (PROTOCOLSETTINGEX **) mir_alloc0(pCount * sizeof(PROTOCOLSETTINGEX *));

		for(i = 0; i < pCount; i++)
		{
			pse[i] = (PROTOCOLSETTINGEX *) mir_alloc0(sizeof(PROTOCOLSETTINGEX));
			pse[i]->szName = "";
		}

		pse[0]->cbSize = sizeof(PROTOCOLSETTINGEX);
		pse[0]->status = aStatus;
		pse[0]->szName = name;

		GetStatusMsg(aStatus, status_msg, sizeof(status_msg));
		pse[0]->szMsg = status_msg;

		CallService(MS_CS_SETSTATUSEX, (WPARAM) &pse, 0);

		for(i = 0; i < pCount; i++)
			mir_free(pse[i]);
		mir_free(pse);
	}
	else
	{
		CallProtoService(name, PS_SETSTATUS, aStatus, 0);

		if (CanSetStatusMsg(aStatus))
		{
			char status_msg[MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE];
			GetStatusMsg(aStatus, status_msg, sizeof(status_msg));
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
	return CanSetStatusMsg(GetStatus()) // <- Simple away handled by this one
			|| ServiceExists(MS_NAS_INVOKESTATUSWINDOW);

}

bool Protocol::CanSetStatusMsg(int aStatus)
{
	return CanGetStatusMsg(aStatus);
}

void Protocol::GetStatusMsg(int aStatus, char *msg, size_t msg_size)
{
	if (!CanGetStatusMsg())
	{
		lcopystr(msg, "", msg_size);
		return;
	}

	if (aStatus == status && ProtoServiceExists(name, PS_GETMYAWAYMSG) )
	{
		char *tmp = (char *) CallProtoService(name, PS_GETMYAWAYMSG, 0, 0);
		lcopystr(msg, tmp == NULL ? "" : tmp, msg_size);
	} 
	else if (ServiceExists(MS_NAS_GETSTATE))
	{
		NAS_PROTOINFO pi;

		ZeroMemory(&pi, sizeof(pi));
		pi.cbSize = sizeof(NAS_PROTOINFO);
		pi.szProto = name;
		pi.status = aStatus == status ? 0 : aStatus;
		pi.szMsg = NULL;

		if (CallService(MS_NAS_GETSTATE, (WPARAM) &pi, 1) == 0)
		{
			if (pi.szMsg == NULL)
			{
				pi.szProto = NULL;

				if (CallService(MS_NAS_GETSTATE, (WPARAM) &pi, 1) == 0)
				{
					if (pi.szMsg != NULL)
					{
						lcopystr(msg, pi.szMsg, msg_size);
						mir_free(pi.szMsg);
					}
					else lcopystr(msg, "", msg_size);
				}
				else lcopystr(msg, "", msg_size);
			}
			else // if (pi.szMsg != NULL)
			{
				lcopystr(msg, pi.szMsg, msg_size);
				mir_free(pi.szMsg);
			}
		}
		else lcopystr(msg, "", msg_size);

		if (ServiceExists(MS_VARS_FORMATSTRING))
		{
			char *tmp = variables_parse(msg, NULL, NULL);
			lcopystr(msg, tmp, msg_size);
			variables_free(tmp);
		}
	}
	// TODO: Remove when removing old NAS services support
	else if (ServiceExists("NewAwaySystem/GetState"))
	{
		NAS_PROTOINFO pi, *pii;

		ZeroMemory(&pi, sizeof(pi));
		pi.cbSize = sizeof(NAS_PROTOINFO);
		pi.szProto = name;
		pi.status = aStatus == status ? 0 : aStatus;
		pi.szMsg = NULL;

		pii = &pi;

		if (CallService("NewAwaySystem/GetState", (WPARAM) &pii, 1) == 0)
		{
			if (pi.szMsg == NULL)
			{
				pi.szProto = NULL;

				if (CallService("NewAwaySystem/GetState", (WPARAM) &pii, 1) == 0)
				{
					if (pi.szMsg != NULL)
					{
						lcopystr(msg, pi.szMsg, msg_size);
						mir_free(pi.szMsg);
					}
					else lcopystr(msg, "", msg_size);
				}
				else lcopystr(msg, "", msg_size);
			}
			else // if (pi.szMsg != NULL)
			{
				lcopystr(msg, pi.szMsg, msg_size);
				mir_free(pi.szMsg);
			}
		}
		else lcopystr(msg, "", msg_size);

		if (ServiceExists(MS_VARS_FORMATSTRING))
		{
			char *tmp = variables_parse(msg, NULL, NULL);
			lcopystr(msg, tmp, msg_size);
			variables_free(tmp);
		}
	}
	else if (ServiceExists(MS_AWAYMSG_GETSTATUSMSG))
	{
		char *tmp = (char *) CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM)aStatus, 0);

		if (tmp != NULL)
		{
			lcopystr(msg, tmp, msg_size);
			mir_free(tmp);
		}
		else lcopystr(msg, "", msg_size);
	}
}

char * Protocol::GetStatusMsg()
{
	GetStatusMsg(status, status_message, sizeof(status_message));
	return status_message;
}

void Protocol::SetStatusMsg(const char *message)
{
	SetStatusMsg(GetStatus(), message);
}

void Protocol::SetStatusMsg(int aStatus, const char *message)
{
	if (!CanSetStatusMsg(aStatus))
		return;

	if (ServiceExists(MS_NAS_SETSTATE))
	{
		NAS_PROTOINFO pi = {0}, *pii;

		pi.cbSize = sizeof(pi);
		pi.szProto = name;
		pi.szMsg = mir_strdup(message);
		pi.status = aStatus;

		pii = &pi;

		CallService(MS_NAS_SETSTATE, (WPARAM) &pii, 1);
	}
	else
	{
		CallProtoService(name, PS_SETAWAYMSG, (WPARAM)aStatus, (LPARAM)message);
	}
}

bool Protocol::HasAvatar()
{
	GetAvatar();

	return avatar_bmp != NULL;
}

bool Protocol::CanGetAvatar()
{
	if (!can_have_avatar)
		return false;

	if (!ServiceExists(MS_AV_GETMYAVATAR))
		return false;

	return true;
}

void Protocol::GetAvatar()
{
	// See if can get one
	if (!CanGetAvatar())
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

char * Protocol::GetNick()
{
	// See if can get one
	if (!CanGetNick())
		return NULL;

	// Get it
	CONTACTINFO ci;
    ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = sizeof(ci);
    ci.hContact = NULL;
    ci.szProto = name;
    ci.dwFlag = CNF_DISPLAY;

#ifdef UNICODE
	ci.dwFlag |= CNF_UNICODE;
#endif

    if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) 
	{
        // CNF_DISPLAY always returns a string type
        lcopystr(nickname, ci.pszVal, MAX_REGS(nickname));
		mir_free(ci.pszVal);
    }
    else
        lcopystr(nickname, "", MAX_REGS(nickname));

	return nickname;
}


bool Protocol::CanSetNick()
{
	return can_set_nick;
}


void Protocol::SetNick(const char *nick)
{
	// See if can get one
	if (!CanSetNick())
		return;

	if (nick == NULL)
		return;

	// Get it
	CallProtoService(name, PS_SETMYNICKNAME, NULL, (LPARAM)nick);
}


bool Protocol::CanSetAvatar()
{
	return ServiceExists(MS_AV_SETMYAVATAR) != FALSE && ServiceExists(MS_AV_CANSETMYAVATAR) != FALSE && 
			CallService(MS_AV_CANSETMYAVATAR, (WPARAM) name, 0);
}

void Protocol::SetAvatar(const char *file_name)
{
	if (!CanSetAvatar())
		return;

	CallService(MS_AV_SETMYAVATAR, (WPARAM) name, (LPARAM) file_name);
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
	if (!CanGetListeningTo())
	{
		lcopystr(listening_to, "", MAX_REGS(listening_to));
		return listening_to;
	}

	DBVARIANT dbv = {0};
	if (DBGetContactSettingTString(NULL, name, "ListeningTo", &dbv))
	{
		lcopystr(listening_to, "", MAX_REGS(listening_to));
		return listening_to;
	}

	lcopystr(listening_to, dbv.pszVal, MAX_REGS(listening_to));

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
	if (buffer != NULL)
	{
		for ( int i = 0 ; i < buffer_len ; i++ )
		{
			delete buffer[i];
		}

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
	return ServiceExists(MS_ERSATZ_ENABLED) || 
		ServiceExists(MS_NAS_INVOKESTATUSWINDOW) || ServiceExists(MS_SA_CHANGESTATUSMSG);
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
	{
		buffer[i]->GetStatusMsg();
	}
}

void ProtocolArray::GetStatuses()
{
	for ( int i = 0 ; i < buffer_len ; i++ )
	{
		buffer[i]->GetStatus();
	}
}

int ProtocolArray::GetGlobalStatus()
{
	int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	if (status == ID_STATUS_CONNECTING)
	{
		status = ID_STATUS_OFFLINE;
	}

	return status;
}

bool ProtocolArray::CanSetAvatars()
{
	return ServiceExists(MS_AV_SETMYAVATAR) != FALSE;
}

void ProtocolArray::SetAvatars(const char *file_name)
{
	if (!CanSetAvatars())
		return;

	CallService(MS_AV_SETMYAVATAR, NULL, (WPARAM) file_name);
}


void ProtocolArray::SetNicks(const char *nick)
{
	if (nick == NULL || nick[0] == '\0')
		return;

	lstrcpyn(default_nick, nick, sizeof(default_nick));

	DBWriteContactSettingString(0, MODULE_NAME, SETTING_DEFAULT_NICK, nick);
	
	for ( int i = 0 ; i < buffer_len ; i++ )
	{
		buffer[i]->SetNick(default_nick);
	}
}


void ProtocolArray::SetStatus(int aStatus)
{
	CallService(MS_CLIST_SETSTATUSMODE, aStatus, 0);
}

void ProtocolArray::SetStatusMsgs(const char *message)
{
	for (int i = ID_STATUS_OFFLINE ; i <= ID_STATUS_IDLE; i++)
	{
		SetStatusMsgs(i, message);
	}
}

void ProtocolArray::SetStatusMsgs(int status, const char *message)
{
	DBWriteContactSettingString(NULL,"SRAway",StatusModeToDbSetting(status,"Msg"),message);
	if (!DBGetContactSettingByte(NULL,"SRAway",StatusModeToDbSetting(status,"UsePrev"),0))
	{
		// Save default also
		DBWriteContactSettingString(NULL,"SRAway",StatusModeToDbSetting(status,"Default"),message);
	}

	for ( int i = 0 ; i < buffer_len ; i++ )
	{
		if (buffer[i]->status == status)
			buffer[i]->SetStatusMsg(status, message);
	}
}


void ProtocolArray::GetDefaultNick()
{
	DBVARIANT dbv;

	if (!DBGetContactSettingTString(0, MODULE_NAME, SETTING_DEFAULT_NICK, &dbv))
	{
		lstrcpyn(default_nick, dbv.pszVal, sizeof(default_nick));
		DBFreeVariant(&dbv);
	}
	else
	{
		default_nick[0] = '\0';
	}
}

void ProtocolArray::GetDefaultAvatar()
{
	DBVARIANT dbv;

	if (!DBGetContactSettingTString(0, "ContactPhoto", "File", &dbv))
	{
		lstrcpyn(default_avatar_file, dbv.pszVal, sizeof(default_avatar_file));
		DBFreeVariant(&dbv);
	}
	else
	{
		default_avatar_file[0] = '\0';
	}
}

char * ProtocolArray::GetDefaultStatusMsg()
{
	return GetDefaultStatusMsg(CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
}

char * ProtocolArray::GetDefaultStatusMsg(int status)
{
	default_status_message[0] = '\0';

	if (ServiceExists(MS_AWAYMSG_GETSTATUSMSG))
	{
		if (status == ID_STATUS_CONNECTING)
		{
			status = ID_STATUS_OFFLINE;
		}

		char *tmp = (char *) CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM)status, 0);

		if (tmp != NULL)
		{
			lstrcpyn(default_status_message, tmp, sizeof(default_status_message));
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


