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


#ifndef __DATA_H__
# define __DATA_H__


//#include "protocol_config.h"

class Protocol
{
	// Attributes ////////////
	std::string name;
	
	std::string description;

	bool avatar_initialized;
	std::string avatar_file;
	HBITMAP avatar_bmp;

	bool status_message_initialized;
	std::string status_message;

	bool nickname_initialized;
	std::string nickname;

	bool locked_initialized;
	bool locked;

	bool emails_initialized;
	int emails;

	bool listening_to_initialized;
	std::string listening_to;

	bool status_initialized;
	std::string status_name;
	std::string custom_status_name_key;
	std::string custom_status_message_key;
	int status;
	int custom_status;

public:	
	Protocol(const char *name);
	~Protocol();

	bool IsValid();
	operator bool ();

	void UpdateAll();

	const char * GetName();
	const char * GetDescription();

	const char * GetStatusName();
	const char * GetCustomStatusNameKey();
	const char * GetCustomStatusMessageKey();
	int GetStatus();
	int GetCustomStatus();
	void SetStatus(int aStatus);

	bool HasAvatar();
	bool CanGetAvatar();
	const char * GetAvatarFile();
	HBITMAP GetAvatarImage();

	bool CanSetAvatar();
	void SetAvatar(const TCHAR *file_name);

	bool CanGetNick();
	const char * GetNick();
	int GetNickMaxLength();
	bool CanSetNick();
	void SetNick(const TCHAR *nick);

	bool CanGetListeningTo();
	bool CanSetListeningTo();
	bool ListeningToEnabled();
	const char * GetListeningTo();

	bool CanGetStatusMsg();
	bool CanGetStatusMsg(int aStatus);
	void GetStatusMsg(int aStatus, TCHAR *msg, size_t msg_size);
	const char * GetStatusMsg();

	bool CanSetStatusMsg();
	bool CanSetStatusMsg(int aStatus);
	void SetStatusMsg(const TCHAR *message);
	void SetStatusMsg(int aStatus, const TCHAR *message);

	bool IsLocked();

	bool CanGetEmailCount();
	int GetEmailCount();

	int Call(const char *service, WPARAM wParam = 0, LPARAM lParam = 0);
	bool CanCall(const char *service);

	std::string GetDBSettingString(const char *key, const char *def = "");

private:
	void UpdateStatus();
	void UpdateAvatar();
	void UpdateNick();
	void UpdateListeningTo();
	void UpdateStatusMsg();
	void UpdateLocked();
	void UpdateEmailCount();
};



class ProtocolArray
{
public:
	TCHAR default_nick[256];
	TCHAR default_avatar_file[256];
	TCHAR default_status_message[256];

	// Methods ///////////////

	ProtocolArray();

	bool CanSetAvatars();
	void SetAvatars(const TCHAR *file);

	void SetNicks(const TCHAR *nick);

	void SetStatus(int aStatus);

	void SetStatusMsgs(const TCHAR *message);
	void SetStatusMsgs(int status, const TCHAR *message);

	int GetGlobalStatus();

	void GetDefaultNick();	// Copy to cache
	void GetDefaultAvatar();	// Copy to cache
	TCHAR * GetDefaultStatusMsg();	// Copy to cache
	TCHAR * GetDefaultStatusMsg(int status);

	bool CanSetListeningTo();
	bool ListeningToEnabled();
};

extern ProtocolArray *protocols;


void SetCurrentProtocol(int index);
Protocol * GetCurrentProtocol(bool createIfDontExist = true);
int GetCurrentProtocolIndex();

void GetProtocols(std::vector<Protocol> *result);
int GetProtocolIndexByName(const char *moduleName);
int GetNumProtocols();
Protocol GetProtocolByIndex(int index);
Protocol GetProtocolByName(const char *proto);

void InitProtocolData();
void DeInitProtocolData();






#endif // __DATA_H__
