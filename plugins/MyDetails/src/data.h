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

	void lcopystr(wchar_t *dest, wchar_t *src, size_t maxlen);

public:
	// Name of protocol
	char name[256];
	wchar_t description[256];
	wchar_t nickname[256];
	wchar_t status_name[256];
	wchar_t status_message[1024];
	wchar_t listening_to[1024];
	AVATARCACHEENTRY *ace;
	wchar_t avatar_file[1024];
	HBITMAP avatar_bmp;
	int status;
	int custom_status;

	int avatar_max_width;
	int avatar_max_height;

	bool data_changed;

	// Methods ///////////////

	Protocol(const char *name, const wchar_t *descr);
	~Protocol();

	bool IsValid();

	int GetStatus();		// Copy to cache and return a copy
	void SetStatus(int aStatus);

	bool HasAvatar();
	bool CanGetAvatar();
	void GetAvatar();		// Copy to cache

	bool CanSetAvatar();
	void SetAvatar(const wchar_t *file_name);
	//void SetAvatar(const char *file_name, HBITMAP hBmp);

	wchar_t *GetNick();			// Copy to cache and return a copy
	int GetNickMaxLength();
	bool CanSetNick();
	void SetNick(const wchar_t *nick);

	bool CanGetListeningTo();
	bool CanSetListeningTo();
	bool ListeningToEnabled();
	wchar_t *GetListeningTo();	// Copy to cache and return a copy

	bool CanGetStatusMsg();
	bool CanGetStatusMsg(int aStatus);
	wchar_t *GetStatusMsg();	// Copy to cache and return a copy
	void GetStatusMsg(int aStatus, wchar_t *msg, size_t msg_size);
	bool CanSetStatusMsg();
	bool CanSetStatusMsg(int aStatus);
	void SetStatusMsg(const wchar_t *message);
	void SetStatusMsg(int aStatus, const wchar_t *message);
};

struct ProtocolArray : public LIST<Protocol>
{
	wchar_t default_nick[256];
	wchar_t default_avatar_file[256];
	wchar_t default_status_message[256];

	// Methods ///////////////

	ProtocolArray();

	Protocol* GetByName(const char *name);

	void GetAvatars();

	void SetNicks(const wchar_t *nick);

	void GetStatusMsgs();
	void GetStatuses();

	static bool CanSetAvatars();
	static void SetAvatars(const wchar_t *file);

	static bool CanSetStatusMsgPerProtocol();
	static void SetStatus(int aStatus);
	void SetStatusMsgs(const wchar_t *message);
	void SetStatusMsgs(int status, const wchar_t *message);

	static int GetGlobalStatus();

	void GetDefaultNick();	// Copy to cache
	void GetDefaultAvatar();	// Copy to cache
	wchar_t *GetDefaultStatusMsg();	// Copy to cache
	wchar_t *GetDefaultStatusMsg(int status);

	bool CanSetListeningTo();
	bool ListeningToEnabled();
};

extern ProtocolArray protocols;

void InitProtocolData();
void DeInitProtocolData();

#endif // __DATA_H__
