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

#ifndef M_MIMCMD_IPC_H
#define M_MIMCMD_IPC_H

#include "mimcmd_data.h"

#define SHAREDMEM_NAME          "MirandaCmdLine|SharedMem"
#define MUTEX_CLIENT_NAME       "MirandaCmdLine|Mutex|Client"
#define EVENT_SERVER_EXEC       "MirandaCmdLine|Event|Exec"
#define EVENT_SERVER_DONE       "MirandaCmdLine|Event|Done"
#define EVENT_SERVER_CLOSE      "MirandaCmdLine|Event|Close"
#define EVENT_SERVER_BUFFERFULL "MirandaCmdLine|Buffer|Full"

extern HANDLE hsmCmdLine;
extern HANDLE hmClient;
extern HANDLE heServerExec;
extern HANDLE heServerDone;
extern HANDLE heServerClose;
extern HANDLE heServerBufferFull;

extern PSharedData sdCmdLine;

int InitClient();
int InitServer();

int DestroyClient();
int DestroyServer();

int CreateSystemEvents();
int CreateMutexes();
int CreateSharedMem();

int DestroySystemEvents();
int DestroyMutexes();
int DestroySharedMem();

#endif