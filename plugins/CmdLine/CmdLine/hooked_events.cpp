/*
CmdLine plugin for Miranda IM

Copyright © 2007 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "hooked_events.h"
#include "mimcmd_ipc.h"
#include "services.h"

HANDLE hModulesLoaded;
HANDLE hOptionsInitialize;
HANDLE hShutdown;
HANDLE hProtoAck;

int bShouldProcessAcks = FALSE;

ACKDATA acks[50] = {0};
int cAcks = sizeof(acks) / sizeof(acks[0]);

#define HOST "http://eblis.tla.ro/projects"

#if defined(WIN64) || defined(_WIN64)
#define CMDLINE_VERSION_URL HOST "/miranda/CmdLine/updater/x64/CmdLine.html"
#define CMDLINE_UPDATE_URL HOST "/miranda/CmdLine/updater/x64/CmdLine.zip"
#else
#define CMDLINE_VERSION_URL HOST "/miranda/CmdLine/updater/CmdLine.html"
#define CMDLINE_UPDATE_URL HOST "/miranda/CmdLine/updater/CmdLine.zip"
#endif
#define CMDLINE_VERSION_PREFIX "CmdLine version "

int HookEvents()
{
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	hProtoAck = HookEvent(ME_PROTO_ACK, OnProtoAck);
//	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	
	return 0;
}

int UnhookEvents()
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hShutdown);
	UnhookEvent(OnProtoAck);
//	UnhookEvent(hOptionsInitialize);
	
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	char buffer[1024];
	Update update = {0};
	update.cbSize = sizeof(Update);
	update.szComponentName = __PLUGIN_DISPLAY_NAME;
	update.pbVersion = (BYTE *) CreateVersionString(VERSION, buffer);
	update.cpbVersion = (int) strlen((char *) update.pbVersion);
	update.szUpdateURL = UPDATER_AUTOREGISTER;
	update.szBetaVersionURL = CMDLINE_VERSION_URL;
	update.szBetaUpdateURL = CMDLINE_UPDATE_URL;
	update.pbBetaVersionPrefix = (BYTE *) CMDLINE_VERSION_PREFIX;
	update.cpbBetaVersionPrefix = (int) strlen(CMDLINE_VERSION_PREFIX);
	CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);
	
	StartServer();
	
	return 0;
}

int OnShutdown(WPARAM wParam, LPARAM lParam)
{
	SetEvent(heServerClose); //tell the listening server to stop
	
	return 0;
}

int QueueAck(ACKDATA *ack)
{
	int i;
	for (i = cAcks - 1; i > 0; i--)
	{
		acks[i] = acks[i - 1];
	}
	
	acks[0] = *ack;
	
	return 0;
}

int ClearAckQueue()
{
	memset(acks, 0, cAcks * sizeof(ACKDATA));
	
	return 0;
}

ACKDATA *GetAck(HANDLE hProcess)
{
	int i;
	for (i = 0; i < cAcks; i++)
	{
		if (acks[i].hProcess == hProcess)
		{
			return &acks[i];
		}
	}
	
	return NULL;
}

int OnProtoAck(WPARAM wParam, LPARAM lParam)
{
	if (bShouldProcessAcks)
	{
		ACKDATA *ack = (ACKDATA *) lParam;
		if ((ack) && (ack->type == ACKTYPE_MESSAGE)) //if it's a message ack
		{
			QueueAck(ack);
		}
	}
	
	return 0;
}