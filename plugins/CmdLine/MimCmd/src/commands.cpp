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

#include "common.h"

inline char *STRNCPY(char *output, const char *input, size_t size)
{
	char *res = strncpy(output, input, size);
	output[size - 1] = 0;
	
	return res;
}

LISTCOMMANDS ListCommands = NULL;

PCommand knownCommands = NULL;
int cKnownCommands = 0;

HMODULE hCmdLineDLL = NULL;

char *GetMirandaFolder(char *mimFolder, int size)
{
	STRNCPY(mimFolder, sdCmdLine->mimFolder, size);
	mimFolder[size - 1] = 0;
	
	return mimFolder;
}


int ConnectToMiranda()
{
	char pluginPath[1024];
	GetMirandaFolder(pluginPath, sizeof(pluginPath));
	mir_strcat(pluginPath, "\\plugins\\cmdline.dll");

	ListCommands = NULL;

	hCmdLineDLL = LoadLibrary(pluginPath);
	
	int failure = 1;
	if (hCmdLineDLL)
	{
		ListCommands = (LISTCOMMANDS) GetProcAddress(hCmdLineDLL, "ListCommands");
	}
	
	if (ListCommands)
	{
		failure = 0;
	}
	
	return failure;
}

int DisconnectFromMiranda()
{
	return FreeLibrary(hCmdLineDLL);
}

int GetKnownCommands()
{
	ListCommands(&knownCommands, &cKnownCommands);
	
	return (knownCommands == NULL);
}

int DestroyKnownCommands()
{

	
	return 0;
}

PCommand GetCommand(char *command)
{
	int i;
	char lower[512];
	STRNCPY(lower, command, sizeof(lower));
	_strlwr(lower);
	
	for (i = 0; i < cKnownCommands; i++)
	{
		if (mir_strcmp(knownCommands[i].command, lower) == 0)
		{
			return &knownCommands[i];
		}
	}
	
	//allow more parameters to trigger the help command - /h -h /? --help
	if ((mir_strcmp(lower, "/h") == 0) || (mir_strcmp(lower, "-h") == 0) || (mir_strcmp(lower, "/?") == 0) || (mir_strcmp(lower, "--help") == 0))
	{
		for (i = 0; i < cKnownCommands; i++)
		{
			if (knownCommands[i].ID == MIMCMD_HELP)
			{
				return &knownCommands[i];
			}
		}
	}
	
	return NULL;
}

void HandleHelpCommand(PCommand helpCommand, char *argv[], int argc, PReply reply)
{
	const int size = REPLY_SIZE;
	if (argc >= 3)
	{
		PCommand command = GetCommand(argv[2]);
		
		if (command)
		{
			reply->code = MIMRES_SUCCESS;
			STRNCPY(reply->message, Translate(command->help), size);
		}
		else{
			reply->code = MIMRES_NOTFOUND;
			mir_snprintf(reply->message, size, Translate("No help for '%s'."), argv[2]);
			reply->message[size -1 ] = 0;
		}
	}
	else{
		reply->code = MIMRES_SUCCESS;
		STRNCPY(reply->message, Translate("Available commands: "), size);
		
		for (int i = 0; i < cKnownCommands - 1; i++)
		{
			strncat(reply->message, knownCommands[i].command, SIZEOF(reply->message) - mir_strlen(reply->message));
			strncat(reply->message, ", ", SIZEOF(reply->message) - mir_strlen(reply->message));
		}
		strncat(reply->message, knownCommands[cKnownCommands - 1].command, SIZEOF(reply->message) - mir_strlen(reply->message));
		strncat(reply->message, ".", SIZEOF(reply->message) - mir_strlen(reply->message));
	}
}

PReply ParseCommand(char *argv[], int argc)
{
	PCommand command = GetCommand(argv[1]);
	if (command)
	{
		PReply reply = &sdCmdLine->reply;
		if (command->ID == MIMCMD_HELP)
		{
			HandleHelpCommand(command, argv, argc, reply);
		}
		else{
			ProcessConsoleCommand(command, argv, argc, reply);
		}
		
		return reply;
	}
	else{
		return NULL;
	}
}

void FillSharedDataStruct(PCommand command, char *arguments[], int count)
{
	for (int i = 0; i < count; i++)
	{
		STRNCPY(sdCmdLine->arguments[i], arguments[i], ARGUMENT_SIZE);
	}
	
	sdCmdLine->cArguments = count;
	sdCmdLine->command = *command;
	*sdCmdLine->reply.message = 0;
	sdCmdLine->reply.code =-1;
}

void ProcessConsoleCommand(PCommand command, char *arguments[], int count, PReply reply)
{
	const HANDLE events[] = {heServerDone, heServerClose, heServerBufferFull};
	const int cEvents = sizeof(events) / sizeof(events[0]);
	
	if (WaitForSingleObject(hmClient, INFINITE) == WAIT_OBJECT_0)
	{//got the mutex, we're the only one who can talk to miranda now
		FillSharedDataStruct(command, arguments, count);
		SetEvent(heServerExec); //tell Miranda to process the request
		
		int done = FALSE;
		while (!done)
		{
			switch (WaitForMultipleObjects(cEvents, events, FALSE, INFINITE)) //wait until server either finished processing or miranda was closed
			{
				case WAIT_OBJECT_0: //done event
				{
					done = TRUE;
					
					break; //nothing to do
				}
			
				case WAIT_OBJECT_0 + 1: //close event
				default:
				{
					mir_strcpy(sdCmdLine->reply.message, Translate("Miranda has been closed or an error has occurred while waiting for the result, could not process request."));
					done = TRUE;
					
					break;
				}
				
				case WAIT_OBJECT_0 + 2: //buffer full event
				{
					lpprintf("%s", reply->message);
					
					break;
				}
			}
		}
		
		reply->code = sdCmdLine->reply.code;
		STRNCPY(reply->message, sdCmdLine->reply.message, REPLY_SIZE);
		
		ReleaseMutex(hmClient); //let other possible clients talk to the server
	}
	else{
		int err = GetLastError();
		reply->code = -1;
		*reply->message = 0;
	}
	
}