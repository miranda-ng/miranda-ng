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

#include "commonheaders.h"

HANDLE hModulesLoaded;
HANDLE hOptionsInitialize;
HANDLE hShutdown;
HANDLE hProtoAck;

int bShouldProcessAcks = FALSE;

ACKDATA acks[50] = {0};
int cAcks = sizeof(acks) / sizeof(acks[0]);

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
	for (int i = cAcks - 1; i > 0; i--)
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
	for (int i = 0; i < cAcks; i++)
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