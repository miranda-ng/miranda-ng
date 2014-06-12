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

class Protocol : public MZeroedObject
{
	// Attributes ////////////
protected:
	bool valid;
	bool can_set_nick;
	bool can_have_avatar;
	bool can_have_listening_to;
	int PF3;

	void lcopystr(TCHAR *dest, TCHAR *src, size_t maxlen);

public:
	// Name of protocol
	char name[256];
	TCHAR description[256];
	TCHAR nickname[256];
	TCHAR status_name[256];
	TCHAR status_message[1024];
	TCHAR listening_to[1024];
	AVATARCACHEENTRY *ace;
	TCHAR avatar_file[1024];
	HBITMAP avatar_bmp;
	int status;
	int custom_status;

	int avatar_max_width;
	int avatar_max_height;

	bool data_changed;

	// Methods ///////////////

	Protocol(const char *name, const TCHAR *descr);
	~Protocol();

	bool IsValid();

	int GetStatus();		// Copy to cache and return a copy
	void SetStatus(int aStatus);

	bool HasAvatar();
	bool CanGetAvatar();
	void GetAvatar();		// Copy to cache

	bool CanSetAvatar();
	void SetAvatar(const TCHAR *file_name);
	//void SetAvatar(const char *file_name, HBITMAP hBmp);

	bool CanGetNick();
	TCHAR *GetNick();			// Copy to cache and return a copy
	int GetNickMaxLength();
	bool CanSetNick();
	void SetNick(const TCHAR *nick);

	bool CanGetListeningTo();
	bool CanSetListeningTo();
	bool ListeningToEnabled();
	TCHAR *GetListeningTo();	// Copy to cache and return a copy

	bool CanGetStatusMsg();
	bool CanGetStatusMsg(int aStatus);
	TCHAR *GetStatusMsg();	// Copy to cache and return a copy
	void GetStatusMsg(int aStatus, TCHAR *msg, size_t msg_size);
	bool CanSetStatusMsg();
	bool CanSetStatusMsg(int aStatus);
	void SetStatusMsg(const TCHAR *message);
	void SetStatusMsg(int aStatus, const TCHAR *message);
};



class ProtocolArray
{
protected:
	// Attributes ////////////
	Protocol **buffer;
	int buffer_len;

public:
	TCHAR default_nick[256];
	TCHAR default_avatar_file[256];
	TCHAR default_status_message[256];

	// Methods ///////////////

	ProtocolArray(int max_size);
	virtual ~ProtocolArray();

	int GetSize();

	void Add(Protocol *p);
	Protocol *Get(int i);
	Protocol *Get(const char *name);

	void GetAvatars();
	bool CanSetAvatars();
	void SetAvatars(const TCHAR *file);

	void SetNicks(const TCHAR *nick);

	void SetStatus(int aStatus);

	void SetStatusMsgs(const TCHAR *message);
	void SetStatusMsgs(int status, const TCHAR *message);

	void GetStatusMsgs();
	void GetStatuses();
	int  GetGlobalStatus();

	void GetDefaultNick();	// Copy to cache
	void GetDefaultAvatar();	// Copy to cache
	TCHAR *GetDefaultStatusMsg();	// Copy to cache
	TCHAR *GetDefaultStatusMsg(int status);

	bool CanSetListeningTo();
	bool ListeningToEnabled();
};

extern ProtocolArray *protocols;



void InitProtocolData();
void DeInitProtocolData();






#endif // __DATA_H__
