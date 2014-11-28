/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2009 by
 *          dufte <dufte@justmail.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

//Klasse behandelt alle Iconsachen unter Xfire

#include "stdafx.h"

#ifndef _XFIRE_VOICECHAT
#define _XFIRE_VOICECHAT

#include "client.h"
#include "sendgamestatus2packet.h"
#include "Xfire_base.h"

#include <vector>

/* tsr definitionen für teamspeak 2 */
struct TtsrServerInfo
{
	char ServerName[30];
	char WelcomeMessage[256];
	int ServerVMajor;
	int ServerVMinor;
	int ServerVRelease;
	int ServerVBuild;
	char ServerPlatform[28]; //geändert in 28 - dufte
	char ServerIp[30];
	char ServerHost[102];
	int ServerType;
	int ServerMaxUsers;
	int SupportedCodecs;
	int ChannelCount;
	int PlayerCount;
};

typedef int(__stdcall  *LPtsrGetServerInfo)(TtsrServerInfo *tsrServerInfo);

/* klassen definitionen */
struct ts3IPPORT {
	unsigned char ip[4];
	unsigned short port;
};

enum XFIREVOICECHATTYPE {
	XFIREVOICECHAT_NOVOICE,
	XFIREVOICECHAT_TS3 = 35,
	XFIREVOICECHAT_TS2 = 32,
	XFIREVOICECHAT_MUMBLE = 34,
	XFIREVOICECHAT_VENTRILO = 33,
	XFIREVOICECHAT_IPDETECT = 999
};

using namespace xfirelib;

class Xfire_voicechat : public Xfire_base {
private:
	//interner status, des aktiven voicechats
	XFIREVOICECHATTYPE currentvoice;
	//ipportport structur für ts3
	ts3IPPORT* ipport;
	//statuspacket caching
	SendGameStatus2Packet lastpacket;
	//tsremotedll
	HMODULE tsrDLL;
	//tsrGetServerInfo funktion vom tsremotedll
	LPtsrGetServerInfo tsrGetServerInfo;
	//pid für mumble
	DWORD pid;
public:
	Xfire_voicechat();
	~Xfire_voicechat();

	void initVoicechat();
	void resetSendGameStatus2Packet(SendGameStatus2Packet* packet);
	void resetCurrentvoicestatus();
	void writeToDatabase(SendGameStatus2Packet* packet);
	HMODULE loadTSR(char* path = NULL, BOOL nolocaltest = FALSE);
	BOOL checkVoicechat(SendGameStatus2Packet* packet);
	BOOL alreadySend(SendGameStatus2Packet* packet);

	//prüf routinen
	BOOL checkforTS3(SendGameStatus2Packet* packet);
	BOOL checkforTS2(SendGameStatus2Packet* packet);
	BOOL checkforMumble(SendGameStatus2Packet* packet);
};

#endif