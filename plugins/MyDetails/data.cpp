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
#include <algorithm>


static char *StatusModeToDbSetting(int status,const char *suffix);


static bool IsValid(const char *proto)
{
	if (proto == NULL || proto[0] == 0)
		return false;

	int caps = CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0);
	return (caps & PF1_IM) == PF1_IM && strcmp(proto, "MetaContacts") != 0;
}

static bool AccOrderComp(PROTOACCOUNT *p1, PROTOACCOUNT *p2)
{
	return p1->iOrder < p2->iOrder;
}

static void GetAccounts(std::vector<PROTOACCOUNT *> *result)
{
	int count;
	PROTOACCOUNT **protos;
	ProtoEnumAccounts(&count, &protos);
	
	for (int i = 0; i < count; i++)
	{
		if (protos[i]->type != PROTOTYPE_PROTOCOL)
			continue;
		
		if (!IsAccountEnabled(protos[i]))
			continue;
		
		if (!IsValid(protos[i]->szModuleName))
			continue;
		
		result->push_back(protos[i]);
	}
	
	std::sort(result->begin(), result->begin(), AccOrderComp);
}


void GetProtocols(std::vector<Protocol> *result)
{
	std::vector<PROTOACCOUNT *> accs;
	GetAccounts(&accs);

	unsigned int accsSize = accs.size();
	for (unsigned int i = 0; i < accsSize ; ++i)
		result->push_back(Protocol(accs[i]->szModuleName));
}


int GetProtocolIndexByName(const char *moduleName)
{
	std::vector<PROTOACCOUNT *> protos;
	GetAccounts(&protos);

	int protosSize = (int) protos.size();
	for(int i = 0; i < protosSize; ++i)
	{
		if (strcmp(protos[i]->szModuleName, moduleName) == 0)
			return i;
	}

	return -1;
}


int GetNumProtocols()
{
	std::vector<PROTOACCOUNT *> protos;
	GetAccounts(&protos);
	return protos.size();
}


struct ProtoCache
{
	Protocol *proto;
	int index;

	void Free()
	{
		delete proto;
		proto = NULL;
		index = -1;
	}
};

static ProtoCache current = { NULL, -1 };


void SetCurrentProtocol(int index)
{
	current.Free();
	
	int protosSize = GetNumProtocols();
	if (protosSize > 0)
		current.index = (index % protosSize + protosSize) % protosSize;

	DBWriteContactSettingWord(NULL, "MyDetails", "ProtocolNumber", current.index);
}

Protocol * GetCurrentProtocol(bool createIfDontExist)
{
	if (createIfDontExist && current.index >= 0 && current.proto == NULL)
	{
		std::vector<PROTOACCOUNT *> protos;
		GetAccounts(&protos);

		int protosSize = protos.size();
		if (current.index >= protosSize)
		{
			current.index = -1;
			return NULL;
		}

		current.proto = new Protocol(protos[current.index]->szModuleName);
	}
	
	return current.proto;	
}

int GetCurrentProtocolIndex()
{
	return current.index;
}


Protocol GetProtocolByIndex(int index)
{
	std::vector<PROTOACCOUNT *> protos;
	GetAccounts(&protos);
	int protosSize = protos.size();
	
	if (protosSize < 1)
		return Protocol(NULL);
	
	index = (index % protosSize + protosSize) % protosSize;
	return Protocol(protos[index]->szModuleName);
}


Protocol GetProtocolByName(const char *moduleName)
{
	std::vector<PROTOACCOUNT *> protos;
	GetAccounts(&protos);
	
	int protosSize = (int) protos.size();
	for(int i = 0; i < protosSize; ++i)
	{
		if (strcmp(protos[i]->szModuleName, moduleName) == 0)
			return Protocol(protos[i]->szModuleName);
	}
	
	return Protocol(NULL);
}


ProtocolArray *protocols = NULL;


void InitProtocolData()
{
	protocols = new ProtocolArray();
}


void DeInitProtocolData()
{
	current.Free();

	delete protocols;
	protocols = NULL;
}


// Protocol Class ///////////////////////////////////////////////////////////////////////////////////////////


Protocol::Protocol(const char *aName)
{
	if (aName)
		name = aName;

	avatar_bmp = NULL;
	status = 0;
	custom_status = 0;
	locked = false;
	emails = 0;
	
	// Initial value
	UpdateAll();
}

Protocol::~Protocol()
{
}


bool Protocol::IsValid()
{
	return !name.empty();
}


Protocol::operator bool ()
{
	return IsValid();
}


void Protocol::UpdateAll()
{
	status_initialized = false;
	status_message_initialized = false;
	nickname_initialized = false;
	avatar_initialized = false;
	locked_initialized = false;
	emails_initialized = false;
	listening_to_initialized = false;
}


int Protocol::Call(const char *service, WPARAM wParam, LPARAM lParam)
{
	return CallProtoService(name.c_str(), service, wParam, lParam);
}


bool Protocol::CanCall(const char *service)
{
	return ProtoServiceExists(name.c_str(), service) != 0;
}


std::string Protocol::GetDBSettingString(const char *key, const char *def)
{
	std::string result = def;

	DBVARIANT dbv;
	if (!DBGetContactSettingTString(0, name.c_str(), key, &dbv))
	{
		if (dbv.ptszVal != NULL && dbv.ptszVal[0] != 0)
			result = dbv.ptszVal;
		
		DBFreeVariant(&dbv);
	}

	return result;
}


const char * Protocol::GetName()
{
	return name.c_str();
}


const char * Protocol::GetDescription()
{
	if (description.empty())
	{
		PROTOACCOUNT *acc = ProtoGetAccount(name.c_str());

		if (acc == NULL || acc->tszAccountName == NULL || acc->tszAccountName[0] == 0)
		{
			char tmp[1024];
			Call(PS_GETNAME, sizeof(tmp), (LPARAM) tmp);
			description = tmp;
		}
		else
		{
			if (mir_is_unicode())
			{
				char *tmp = mir_u2a((const wchar_t *) acc->tszAccountName);
				description = tmp;
				mir_free(tmp);
			}
			else
			{
				description = acc->tszAccountName;
			}
		}
	}

	return description.c_str();
}

void Protocol::UpdateStatus()
{
	status_initialized = true;

	status = Call(PS_GETSTATUS);

	if (status > ID_STATUS_OFFLINE && CanCall(PS_ICQ_GETCUSTOMSTATUS))
	{
		char *name_key = NULL;
		char *message_key = NULL;

		custom_status = Call(PS_ICQ_GETCUSTOMSTATUS, (WPARAM) &name_key, (LPARAM) &message_key);

		// Fix fo jabber, that returns 0xbaadf00d here
		if (custom_status < 0)
			custom_status = 0;

		custom_status_name_key = (name_key ? name_key : "");
		custom_status_message_key = (message_key ? message_key : "");
	}
	else
	{
		custom_status = 0;
		custom_status_name_key = "";
		custom_status_message_key = "";
	}

	if (custom_status == 0)
	{
		status_name = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status, GCMDF_TCHAR);
	}
	else
	{
		status_name = "";

		if (!custom_status_name_key.empty())
			status_name = GetDBSettingString(custom_status_name_key.c_str());

		if (!custom_status_message_key.empty())
		{
			std::string tmp = GetDBSettingString(custom_status_message_key.c_str());
			if (!tmp.empty())
			{
				status_name += ": ";
				status_name += tmp;
			}
		}
	}
	
	if (status_name.empty())
		status_name = TranslateTS("<no status name>");
}

const char * Protocol::GetStatusName()
{
	if (!status_initialized)
		UpdateStatus();

	return status_name.c_str();
}

const char * Protocol::GetCustomStatusNameKey()
{
	if (!status_initialized)
		UpdateStatus();
	
	return custom_status_name_key.c_str();
}

const char * Protocol::GetCustomStatusMessageKey()
{
	if (!status_initialized)
		UpdateStatus();
	
	return custom_status_message_key.c_str();
}

int Protocol::GetStatus()
{
	if (!status_initialized)
		UpdateStatus();
	
	return status;
}

int Protocol::GetCustomStatus()
{
	if (!status_initialized)
		UpdateStatus();
	
	return custom_status;
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
		pse[0]->szName = (char *) name.c_str();

		GetStatusMsg(aStatus, status_msg, sizeof(status_msg));
		pse[0]->szMsg = status_msg;

		CallService(MS_CS_SETSTATUSEX, (WPARAM) &pse, 0);

		for(i = 0; i < pCount; i++)
			mir_free(pse[i]);
		mir_free(pse);
	}
	else
	{
		Call(PS_SETSTATUS, aStatus);

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
	return CanGetStatusMsg(GetStatus());
}

bool Protocol::CanGetStatusMsg(int aStatus)
{
	return (Call(PS_GETCAPS, PFLAGNUM_1) & PF1_MODEMSGSEND) != 0 
			&& (Call(PS_GETCAPS, (WPARAM)PFLAGNUM_3) & Proto_Status2Flag(aStatus));
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
		lstrcpyn(msg, "", msg_size);
		return;
	}

	bool isCurrentStatus = (aStatus == GetStatus());

	if (isCurrentStatus && CanCall(PS_GETMYAWAYMSG))
	{
		char *tmp = (char *) Call(PS_GETMYAWAYMSG);
		lstrcpyn(msg, tmp == NULL ? "" : tmp, msg_size);
	}
	else if (isCurrentStatus && ServiceExists(MS_SA_ISSARUNNING) && CallService(MS_SA_ISSARUNNING, 0, 0)) 
	{
		char *tmp = (char *) CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) ID_STATUS_CURRENT, (LPARAM) name.c_str());

		if (tmp != NULL)
		{
			lstrcpyn(msg, tmp, msg_size);
			mir_free(tmp);
		}
		else lstrcpyn(msg, "", msg_size);
		
	}
	else if (ServiceExists(MS_NAS_GETSTATE))
	{
		NAS_PROTOINFO pi;

		ZeroMemory(&pi, sizeof(pi));
		pi.cbSize = sizeof(NAS_PROTOINFO);
		pi.szProto = (char *) name.c_str();
		pi.status = (isCurrentStatus ? 0 : aStatus);
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
						lstrcpyn(msg, pi.szMsg, msg_size);
						mir_free(pi.szMsg);
					}
					else lstrcpyn(msg, "", msg_size);
				}
				else lstrcpyn(msg, "", msg_size);
			}
			else // if (pi.szMsg != NULL)
			{
				lstrcpyn(msg, pi.szMsg, msg_size);
				mir_free(pi.szMsg);
			}
		}
		else lstrcpyn(msg, "", msg_size);

		if (ServiceExists(MS_VARS_FORMATSTRING))
		{
			char *tmp = variables_parse(msg, NULL, NULL);
			lstrcpyn(msg, tmp, msg_size);
			variables_free(tmp);
		}
	}
	// TODO: Remove when removing old NAS services support
	else if (ServiceExists("NewAwaySystem/GetState"))
	{
		NAS_PROTOINFO pi, *pii;

		ZeroMemory(&pi, sizeof(pi));
		pi.cbSize = sizeof(NAS_PROTOINFO);
		pi.szProto = (char *) name.c_str();
		pi.status = (isCurrentStatus ? 0 : aStatus);
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
						lstrcpyn(msg, pi.szMsg, msg_size);
						mir_free(pi.szMsg);
					}
					else lstrcpyn(msg, "", msg_size);
				}
				else lstrcpyn(msg, "", msg_size);
			}
			else // if (pi.szMsg != NULL)
			{
				lstrcpyn(msg, pi.szMsg, msg_size);
				mir_free(pi.szMsg);
			}
		}
		else lstrcpyn(msg, "", msg_size);

		if (ServiceExists(MS_VARS_FORMATSTRING))
		{
			char *tmp = variables_parse(msg, NULL, NULL);
			lstrcpyn(msg, tmp, msg_size);
			variables_free(tmp);
		}
	}
	else if (ServiceExists(MS_AWAYMSG_GETSTATUSMSG))
	{
		char *tmp = (char *) CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM)aStatus, 0);

		if (tmp != NULL)
		{
			lstrcpyn(msg, tmp, msg_size);
			mir_free(tmp);
		}
		else lstrcpyn(msg, "", msg_size);
	}
}

void Protocol::UpdateStatusMsg()
{
	status_message_initialized = true;

	TCHAR tmp[1024];
	GetStatusMsg(GetStatus(), tmp, sizeof(tmp));

	status_message = tmp;
}

const char * Protocol::GetStatusMsg()
{
	if (!status_message_initialized)
		UpdateStatusMsg();

	return status_message.c_str();
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
		pi.szProto = (char *) name.c_str();
		pi.szMsg = mir_strdup(message);
		pi.status = aStatus;

		pii = &pi;

		CallService(MS_NAS_SETSTATE, (WPARAM) &pii, 1);
	}
	else
	{
		Call(PS_SETAWAYMSG, (WPARAM) aStatus, (LPARAM) message);
	}
}

bool Protocol::HasAvatar()
{
	if (!avatar_initialized)
		UpdateAvatar();

	return avatar_bmp != NULL;
}

bool Protocol::CanGetAvatar()
{
	int caps = Call(PS_GETCAPS, PFLAGNUM_4);

	if ((caps & PF4_AVATARS) == 0)
		return false;

	if (!ServiceExists(MS_AV_GETMYAVATAR))
		return false;

	return true;
}

void Protocol::UpdateAvatar()
{
	avatar_initialized = true;
	avatar_file = "";
	avatar_bmp = NULL;
	
	// See if can get one
	if (!CanGetAvatar())
		return;

	// Get HBITMAP from cache
	AVATARCACHEENTRY *ace = (avatarCacheEntry *) CallService(MS_AV_GETMYAVATAR, 0, (LPARAM) name.c_str());
	if (ace != NULL)
	{
		avatar_file = ace->szFilename;
		avatar_bmp = ace->hbmPic;
	}
}

const char * Protocol::GetAvatarFile()
{
	if (!avatar_initialized)
		UpdateAvatar();

	return avatar_file.c_str();
}

HBITMAP Protocol::GetAvatarImage()
{
	if (!avatar_initialized)
		UpdateAvatar();
	
	return avatar_bmp;
}


bool Protocol::CanGetNick()
{
	return ServiceExists(MS_CONTACT_GETCONTACTINFO) != FALSE;
}

int Protocol::GetNickMaxLength()
{
	if (CanCall(PS_GETMYNICKNAMEMAXLENGTH))
	{
		int ret = Call(PS_GETMYNICKNAMEMAXLENGTH);
		if (ret <= 0)
			ret = MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE;
		return ret;
	}
	else
		return MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE;
}

void Protocol::UpdateNick()
{
	nickname_initialized = true;
	nickname = "";

	// See if can get one
	if (!CanGetNick())
		return;

	// Get it
	CONTACTINFO ci;
    ZeroMemory(&ci, sizeof(ci));
    ci.cbSize = sizeof(ci);
    ci.hContact = NULL;
    ci.szProto = (char *) name.c_str();
    ci.dwFlag = CNF_DISPLAY;

#ifdef UNICODE
	ci.dwFlag |= CNF_UNICODE;
#endif

    if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) 
	{
        // CNF_DISPLAY always returns a string type
        nickname = ci.pszVal;
		mir_free(ci.pszVal);
    }
}

const char * Protocol::GetNick()
{
	if (!nickname_initialized)
		UpdateNick();

	return nickname.c_str();
}


bool Protocol::CanSetNick()
{
	return CanCall(PS_SETMYNICKNAME) != 0;
}


void Protocol::SetNick(const char *nick)
{
	// See if can get one
	if (!CanSetNick())
		return;

	if (nick == NULL)
		return;

	// Get it
	Call(PS_SETMYNICKNAME, NULL, (LPARAM) nick);
}


bool Protocol::CanSetAvatar()
{
	return ServiceExists(MS_AV_SETMYAVATAR) != FALSE && ServiceExists(MS_AV_CANSETMYAVATAR) != FALSE && 
			CallService(MS_AV_CANSETMYAVATAR, (WPARAM) name.c_str(), 0);
}

void Protocol::SetAvatar(const char *file_name)
{
	if (!CanSetAvatar())
		return;

	CallService(MS_AV_SETMYAVATAR, (WPARAM) name.c_str(), (LPARAM) file_name);
}

bool Protocol::CanGetListeningTo()
{
	return CanCall(PS_SET_LISTENINGTO) != 0;
}

bool Protocol::CanSetListeningTo()
{
	return CanGetListeningTo() && ServiceExists(MS_LISTENINGTO_ENABLE);
}

bool Protocol::ListeningToEnabled()
{
	return CanSetListeningTo() && CallService(MS_LISTENINGTO_ENABLED, (WPARAM) name.c_str(), 0) != 0;
}

void Protocol::UpdateListeningTo()
{
	listening_to_initialized = true;
	listening_to = "";

	if (!CanGetListeningTo())
		return;

	listening_to = GetDBSettingString("ListeningTo");
}

const char * Protocol::GetListeningTo()
{
	if (!listening_to_initialized)
		UpdateListeningTo();

	return listening_to.c_str();
}

void Protocol::UpdateLocked()
{
	locked_initialized = true;

	locked = (DBGetContactSettingByte(NULL, name.c_str(), "LockMainStatus", 0) != 0);
}

bool Protocol::IsLocked()
{
	if (!locked_initialized)
		UpdateLocked();

	return locked;
}

bool Protocol::CanGetEmailCount()
{
	return CanCall(PS_GETUNREADEMAILCOUNT) != 0 
		&& GetStatus() > ID_STATUS_OFFLINE;
}

void Protocol::UpdateEmailCount()
{
	emails_initialized = true;
	
	if (!CanGetEmailCount())
		emails = 0;
	else
		emails = max(0, Call(PS_GETUNREADEMAILCOUNT));
}

int Protocol::GetEmailCount()
{
	if (!emails_initialized)
		UpdateEmailCount();

	return emails;
}


// ProtocolDataArray Class /////////////////////////////////////////////////////////////////////////////


ProtocolArray::ProtocolArray()
{
	GetDefaultNick();
	GetDefaultAvatar();
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

	std::vector<Protocol> protos;
	GetProtocols(&protos);

	unsigned int protosSize = protos.size();
	for (int i = 0; i < protosSize; ++i)
		protos[i].SetNick(default_nick);
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
	
	std::vector<Protocol> protos;
	GetProtocols(&protos);
	
	unsigned int protosSize = protos.size();
	for (int i = 0; i < protosSize; ++i)
	{
		if (protos[i].GetStatus() == status)
			protos[i].SetStatusMsg(status, message);
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


