// Copyright © 2008 sss, chaos.persei
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"

HANDLE ghConnection;
HANDLE ghPacketReciever;
BOOL Connected;
int gbState;
SONGINFO SongInfo = {0};

void Start(void* param)
{
	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	char *tmp = (char*)mir_u2a(gbHost);
	nloc.szHost = tmp;
	nloc.timeout = 5;
	nloc.flags = NLOCF_V2;
	nloc.wPort = gbPort;
	Connected = FALSE;
	ghConnection =  (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)ghNetlibUser, (LPARAM)&nloc);
	if(ghConnection)
		ghPacketReciever = (HANDLE)CallService(MS_NETLIB_CREATEPACKETRECVER,(WPARAM)ghConnection,2048);
}

void ReStart(void *param)
{
	if(ghPacketReciever)
		Netlib_CloseHandle(ghPacketReciever);
	if(ghConnection)
		Netlib_CloseHandle(ghConnection);
	Sleep(500);
	mir_forkthread(&Start, 0);
}

int Parser()
{
	static NETLIBPACKETRECVER nlpr = {0};
	char *ptr;
	char tmp[256];
	int i;
	char *buf;
	static char ver[16];
	nlpr.cbSize = sizeof(nlpr);
	nlpr.dwTimeout = 5;
	if(!ghConnection)
	{
		mir_forkthread(&Start, 0);
	}
	if(ghConnection)
	{	
		int recvResult;
/*		do
		{
			recvResult = CallService(MS_NETLIB_GETMOREPACKETS,(WPARAM)ghPacketReciever, (LPARAM)&nlpr);
			if(recvResult == SOCKET_ERROR)
			{
				ReStart();
				return 1;
			}
		}
		while(recvResult > 0);*/
		if(!Connected)
		{
			char tmp[128];
			char *tmp2 = mir_t2a(gbPassword);
			recvResult = CallService(MS_NETLIB_GETMOREPACKETS,(WPARAM)ghPacketReciever, (LPARAM)&nlpr);
			if(recvResult == SOCKET_ERROR)
			{
				mir_forkthread(&ReStart, 0);
//				ReStart();
				return 1;
			}
			if(strlen(tmp2) > 2)
			{
				strcpy(tmp, "password ");
				strcat(tmp, tmp2);
				strcat(tmp, "\n");
				Netlib_Send(ghConnection, tmp, (int)strlen(tmp), 0);
				recvResult = CallService(MS_NETLIB_GETMOREPACKETS,(WPARAM)ghPacketReciever, (LPARAM)&nlpr);
				if(recvResult == SOCKET_ERROR)
				{
					mir_forkthread(&ReStart, 0);
					return 1;
				}
			}
			mir_free(tmp2);
		}
		Netlib_Send(ghConnection, "status\n", (int)strlen("status\n"), 0);
		recvResult = CallService(MS_NETLIB_GETMOREPACKETS,(WPARAM)ghPacketReciever, (LPARAM)&nlpr);
		if(recvResult == SOCKET_ERROR)
		{
			mir_forkthread(&ReStart, 0);
			return 1;
		}
		Netlib_Send(ghConnection, "currentsong\n", (int)strlen("currentsong\n"), 0);
		recvResult = CallService(MS_NETLIB_GETMOREPACKETS,(WPARAM)ghPacketReciever, (LPARAM)&nlpr);
		if(recvResult == SOCKET_ERROR)
		{
			mir_forkthread(&ReStart, 0);
			return 1;
		}
		nlpr.bytesUsed = nlpr.bytesAvailable;
	}
	buf = (char*)nlpr.buffer;
	if(ptr = strstr(buf, "MPD"))
	{
		Connected = TRUE;
		ptr = &ptr[4];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		strcpy(ver, tmp);
		SongInfo.txtver = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.txtver = (TCHAR*)mir_utf8decodeW(ver);
	if(ptr = strstr(buf, "file:"))
	{
		ptr = &ptr[6];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.mfile = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.mfile = _T("");
	if(ptr = strstr(buf, "Time:"))
	{
		ptr = &ptr[6];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.total = atoi(tmp);
	}
	else if(!SongInfo.total)
		SongInfo.total = 0;
	if(ptr = strstr(buf, "time:"))
	{
		ptr = &ptr[6];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.time = atoi(tmp);
	}
	else if(!SongInfo.time)
		SongInfo.time = 0;
	if(ptr = strstr(buf, "Title:"))
	{
		ptr = &ptr[7];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.title = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.title = _T("Unknown track");
	if(ptr = strstr(buf, "Artist:"))
	{
		ptr = &ptr[8];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.artist = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.artist = _T("Unknown artist");
	if(ptr = strstr(buf, "Genre:"))
	{
		ptr = &ptr[7];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.genre = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.genre = _T("Unknown genre");
	if(ptr = strstr(buf, "Album:"))
	{
		ptr = &ptr[7];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.album = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.album = _T("Unknown album");
	if(ptr = strstr(buf, "Date:"))
	{
		ptr = &ptr[6];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.year = (TCHAR*)mir_utf8decodeW(tmp);
	}
	else
		SongInfo.year = _T("Unknown year");
	if(ptr = strstr(buf, "volume:"))
	{
		ptr = &ptr[8];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.volume = atoi(tmp);
	}
	else if(!SongInfo.volume)
		SongInfo.volume = 0;
	if(ptr = strstr(buf, "audio:"))
	{
		ptr = &ptr[7];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.khz = atoi(tmp);
	}
	else if(!SongInfo.khz)
		SongInfo.khz = 0;
	if(ptr = strstr(buf, "bitrate:"))
	{
		ptr = &ptr[9];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.kbps = atoi(tmp);
	}
	else if(!SongInfo.kbps)
		SongInfo.kbps = 0;

	if(ptr = strstr(buf, "Track:"))
	{
		ptr = &ptr[7];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		SongInfo.track = atoi(tmp);
	}
	else if(!SongInfo.track)
		SongInfo.track = 0;
	if(ptr = strstr(buf, "state:"))
	{
		ptr = &ptr[7];
		for(i = 0; ((ptr[i] != '\n') && (ptr[i] != '\0')); i++)
			tmp[i] = ptr[i];
		tmp[i] = '\0';
		if(strstr(tmp, "play"))
			gbState = WAT_PLS_PLAYING;
		if(strstr(tmp, "pause"))
			gbState = WAT_PLS_PAUSED;
		if(strstr(tmp, "stop"))
			gbState = WAT_PLS_STOPPED;
	}
	else if(!gbState)
		gbState = WAT_PLS_UNKNOWN;
	return 0;
}

void Stop()
{
	if(ghPacketReciever)
		Netlib_CloseHandle(ghPacketReciever);
	if(ghConnection)
		Netlib_CloseHandle(ghConnection);
	if(ghNetlibUser && (ghNetlibUser != INVALID_HANDLE_VALUE))
		CallService(MS_NETLIB_SHUTDOWN,(WPARAM)ghNetlibUser,0);
}

LPINITPROC Init()
{
	mir_forkthread(&Start, 0);
	return 0;
}

LPDEINITPROC DeInit()
{
	Stop();
	return 0;
}

LPCHECKPROC CheckPlayer(HWND wnd, int flags)
{
	if(!ghConnection)
	{
		mir_forkthread(&Start, 0);
		return 0;
	}
	if(Parser())
		return (LPCHECKPROC)WAT_PLS_STOPPED;
	if(Connected)		
		return (LPCHECKPROC)WAT_PLS_PLAYING;
	return 0;
}

LPSTATUSPROC GetStatus()
{
	if(!ghConnection)
	{
		mir_forkthread(&Start, 0);
		return 0;
	}
	if(Parser())
		return (LPSTATUSPROC)-1;
	return (LPSTATUSPROC)(gbState);
}

LPNAMEPROC GetFileName(HWND wnd, int flags)
{
	if(!ghConnection)
	{
		mir_forkthread(&Start, 0);
		return 0;
	}
	return 0;
}

LPINFOPROC GetPlayerInfo(LPSONGINFO info, int flags)
{
	if(!ghConnection)
	{
		mir_forkthread(&Start, 0);
		return 0;
	}
	if(Parser())
		return (LPINFOPROC)-1;
/*	
	
	info->channels = SongInfo.channels;
	info->codec = SongInfo.codec;
	info->comment = SongInfo.comment;
	info->cover = SongInfo.cover;
	info->date = SongInfo.date;
	info->fps = SongInfo.fps;
	info->fsize = SongInfo.fsize;
	
	info->icon = SongInfo.icon;
	info->kbps = SongInfo.kbps;
	info->khz = SongInfo.khz;
	info->lyric = SongInfo.lyric;
	info->mfile = SongInfo.mfile;
	info->player = SongInfo.player;
	info->plyver = SongInfo.plyver;
	info->status = SongInfo.status;
	info->time = SongInfo.time;
	info->title = SongInfo.title;
	info->total = SongInfo.total;
	info->track = SongInfo.track;*/
	info->total = SongInfo.total;
	info->time = SongInfo.time;
	info->mfile = SongInfo.mfile;
	info->txtver = SongInfo.txtver;
	info->title = SongInfo.title; 
	info->artist = SongInfo.artist;
	info->genre = SongInfo.genre;
	info->album = SongInfo.album;
	info->year = SongInfo.year;
	info->kbps = SongInfo.kbps;
	info->track = SongInfo.track;
	info->khz = SongInfo.khz;
	info->volume = SongInfo.volume;
/*	info->url = SongInfo.url; //??
	info->vbr = SongInfo.vbr;
	info->volume = SongInfo.volume;
	*/
	return 0;
}

LPCOMMANDPROC SendCommand(HWND wnd, int command, int value)
{
	switch (command)
	{
	case WAT_CTRL_PREV:
		Netlib_Send(ghConnection, "previous\n", (int)strlen("previous\n"), 0);
		break;
	case WAT_CTRL_PLAY: //add resuming support
		if(gbState != WAT_PLS_PAUSED)
			Netlib_Send(ghConnection, "play\n", (int)strlen("play\n"), 0);
		else
			Netlib_Send(ghConnection, "pause 0\n", (int)strlen("pause 0\n"), 0);
		break;
	case WAT_CTRL_PAUSE:
		Netlib_Send(ghConnection, "pause 1\n", (int)strlen("pause 1\n"), 0);
		break;
	case WAT_CTRL_STOP:
		Netlib_Send(ghConnection, "stop\n", (int)strlen("stop\n"), 0);
		break;
	case WAT_CTRL_NEXT:
		Netlib_Send(ghConnection, "next\n", (int)strlen("next\n"), 0);
		break;
	case WAT_CTRL_VOLDN:
		break;
	case WAT_CTRL_VOLUP:
		break;
	case WAT_CTRL_SEEK:
		break;
	default:
		break;
	}
	return 0;
}

void RegisterPlayer()
{
	if(!bWatrackService)
		return;
	{
		PLAYERCELL player = {0};
		player.Desc = "Music Player Daemon";
		player.Check = (LPCHECKPROC)CheckPlayer;
		player.Init = (LPINITPROC)Init;
		player.DeInit = (LPDEINITPROC)DeInit;
		player.GetStatus = (LPSTATUSPROC)GetStatus;
		player.Command = (LPCOMMANDPROC)SendCommand;
		player.flags = (WAT_OPT_HASURL|WAT_OPT_SINGLEINST|WAT_OPT_PLAYERINFO);
		player.GetName = (LPNAMEPROC)GetFileName;
		player.GetInfo = (LPINFOPROC)GetPlayerInfo;
//		player.Icon = //TODO:implement icon support
		player.Notes = _T("mpd is a nice music player for *nix which have not any gui, just daemon.\nuses very small amount of ram, cpu.");
		player.URL = "http://www.musicpd.org";
		CallService(MS_WAT_PLAYER, (WPARAM)WAT_ACT_REGISTER, (LPARAM)&player);
	}
}