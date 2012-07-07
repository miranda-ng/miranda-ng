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

HANDLE hsmCmdLine = NULL;
HANDLE hmClient = NULL;
HANDLE heServerExec = NULL;
HANDLE heServerDone = NULL;
HANDLE heServerClose = NULL;
HANDLE heServerBufferFull = NULL;

PSharedData sdCmdLine = NULL;


int InitClient()
{
	int res = (CreateSystemEvents() || CreateMutexes() || CreateSharedMem());
	
	return res;
}

int InitServer()
{
	int res = (CreateSystemEvents() || CreateSharedMem());
	
	return res;
}

int DestroyClient()
{
	int res = (DestroySystemEvents() || DestroyMutexes() || DestroySharedMem());
	
	return res;
}

int DestroyServer()
{
	int res = (DestroySystemEvents() || DestroySharedMem());
	
	return res;
}

int CreateSystemEvents()
{
	heServerExec = CreateEvent(NULL, FALSE, FALSE, EVENT_SERVER_EXEC);
	heServerDone = CreateEvent(NULL, FALSE, FALSE, EVENT_SERVER_DONE);
	heServerClose = CreateEvent(NULL, FALSE, FALSE, EVENT_SERVER_CLOSE);
	heServerBufferFull = CreateEvent(NULL, FALSE, FALSE, EVENT_SERVER_BUFFERFULL);
	
	return ((heServerExec == NULL) || (heServerDone == NULL) || (heServerClose == NULL) || (heServerBufferFull == NULL));
}

int CreateMutexes()
{
	hmClient = CreateMutex(NULL, FALSE, MUTEX_CLIENT_NAME);
	
	return (hmClient == NULL);
}

int CreateSharedMem()
{
	hsmCmdLine = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TSharedData), SHAREDMEM_NAME);
	sdCmdLine = (PSharedData) MapViewOfFile(hsmCmdLine, FILE_MAP_WRITE, 0, 0, sizeof(TSharedData));
	
	return ((hsmCmdLine == NULL) || (sdCmdLine == NULL));
}

int DestroySystemEvents()
{
	int res = (!CloseHandle(heServerExec)) || (!CloseHandle(heServerDone)) || (!CloseHandle(heServerClose) || (!CloseHandle(heServerBufferFull)));
	
	return res;
}

int DestroyMutexes()
{
	int res = !CloseHandle(hmClient);
	
	return res;
}

int DestroySharedMem()
{
	int res = (!UnmapViewOfFile(sdCmdLine)) || (!CloseHandle(hsmCmdLine));
	
	return res;
}