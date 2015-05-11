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

int bWaitForUnload = 0;

int InitServices()
{
	return 0;
}

int DestroyServices()
{
	return 0;
}

extern "C" __declspec(dllexport) void ProcessConsoleCommand(PCommand command, TArgument *arguments, int count, PReply reply)
{
	HandleCommand(command, arguments, count, reply);
}

void __cdecl ServerWorkerThread(void *data)
{
	int done = FALSE;
	const HANDLE events[] = {heServerExec, heServerClose};
	const int cEvents = sizeof(events) / sizeof(events[0]);
	
	while (!done)
	{
		switch (WaitForMultipleObjects(cEvents, events, FALSE, INFINITE))
		{
			case WAIT_OBJECT_0: //need to process something
			{
				ProcessConsoleCommand(&sdCmdLine->command, sdCmdLine->arguments, sdCmdLine->cArguments, &sdCmdLine->reply);
				SetEvent(heServerDone); //notify the client we've finished
			
				break;
			}
			
			case WAIT_OBJECT_0 + 1: //server is closing
			{
				done = TRUE; //stop the thread
				
				break;
			}
		}
	}
}

int StartServer()
{
	int failure = 1;
	if (sdCmdLine)
	{
		if (sdCmdLine->instances == 0)
		{
			HANDLE server = mir_forkthread(ServerWorkerThread, 0);
			if (server)
			{
				char path[MIMFOLDER_SIZE];
				GetModuleFileName(GetModuleHandle(NULL), path, sizeof(path));
				char *p = strrchr(path, '\\');
				if (p) { *p = 0; }
				STRNCPY(sdCmdLine->mimFolder, path, MIMFOLDER_SIZE);
				sdCmdLine->instances++;
			
				failure = 0;
			}
			else{
				PUShowMessage(Translate("Could not create CommandLine listening server!"), SM_WARNING);
			}
		}
		else{
			MessageBox(NULL, Translate("You can only run one instance of CmdLine plugin."), Translate("Error"), MB_ICONERROR | MB_OK);
		}
	}
	
	return failure;
}

extern "C" __declspec(dllexport) void ListCommands(PCommand * commands, int *count)
{
	*commands = mimCommands;
	*count = cMimCommands;
}