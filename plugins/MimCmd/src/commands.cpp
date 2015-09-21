/*
CmdLine plugin for Miranda IM

Copyright � 2007 Cristian Libotean

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

#include "stdafx.h"

LISTCOMMANDS ListCommands = NULL;

PCommand knownCommands = NULL;
int cKnownCommands = 0;

HMODULE hCmdLineDLL = NULL;

char *GetMirandaFolder(char *mimFolder, int size)
{
	strncpy_s(mimFolder, size, sdCmdLine->mimFolder, _TRUNCATE);
	return mimFolder;
}

int ConnectToMiranda()
{
	TCHAR tszPath[MAX_PATH];
	GetModuleFileName(NULL, tszPath, _countof(tszPath));
	TCHAR *p = _tcsrchr(tszPath, '\\');
	if (p) p[1] = 0;

	_tcsncat_s(tszPath, _T("libs"), _TRUNCATE);
	DWORD cbPath = (DWORD)_tcslen(tszPath);

	DWORD cbSize = GetEnvironmentVariable(_T("PATH"), NULL, 0);
	TCHAR *ptszVal = new TCHAR[cbSize + MAX_PATH + 2];
	_tcscpy(ptszVal, tszPath);
	_tcscat(ptszVal, _T(";"));
	GetEnvironmentVariable(_T("PATH"), ptszVal + cbPath + 1, cbSize);
	SetEnvironmentVariable(_T("PATH"), ptszVal);
	delete[] ptszVal;

	char pluginPath[1024];
	GetMirandaFolder(pluginPath, sizeof(pluginPath));
	mir_strcat(pluginPath, "\\plugins\\cmdline.dll");

	ListCommands = NULL;

	hCmdLineDLL = LoadLibraryA(pluginPath);

	int failure = 1;
	if (hCmdLineDLL)
		ListCommands = (LISTCOMMANDS) GetProcAddress(hCmdLineDLL, "ListCommands");

	if (ListCommands)
		failure = 0;

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
	strncpy_s(lower, command, _TRUNCATE);
	_strlwr(lower);

	for (i = 0; i < cKnownCommands; i++)
		if (mir_strcmp(knownCommands[i].command, lower) == 0)
			return &knownCommands[i];

	//allow more parameters to trigger the help command - /h -h /? --help
	if ((mir_strcmp(lower, "/h") == 0) || (mir_strcmp(lower, "-h") == 0) || (mir_strcmp(lower, "/?") == 0) || (mir_strcmp(lower, "--help") == 0))
		for (i = 0; i < cKnownCommands; i++)
			if (knownCommands[i].ID == MIMCMD_HELP)
				return &knownCommands[i];

	return NULL;
}

void HandleHelpCommand(PCommand, char *argv[], int argc, PReply reply)
{
	CMStringA szReply;

	if (argc >= 3) {
		PCommand command = GetCommand(argv[2]);

		if (command) {
			reply->code = MIMRES_SUCCESS;
			szReply.Append(Translate(command->help));
		}
		else {
			reply->code = MIMRES_NOTFOUND;
			szReply.AppendFormat(Translate("No help for '%s'."), argv[2]);
		}
	}
	else {
		reply->code = MIMRES_SUCCESS;
		szReply.Append(Translate("Available commands: "));

		for (int i = 0; i < cKnownCommands - 1; i++) {
			szReply.Append(knownCommands[i].command);
			szReply.Append(", ");
		}
		szReply.Append(knownCommands[cKnownCommands-1].command);
		szReply.AppendChar('.');
	}
	strncpy_s(reply->message, szReply, _TRUNCATE);
}

PReply ParseCommand(char *argv[], int argc)
{
	PCommand command = GetCommand(argv[1]);
	if (!command)
		return NULL;

	PReply reply = &sdCmdLine->reply;
	if (command->ID == MIMCMD_HELP)
		HandleHelpCommand(command, argv, argc, reply);
	else
		ProcessConsoleCommand(command, argv, argc, reply);

	return reply;
}

void FillSharedDataStruct(PCommand command, char *arguments[], int count)
{
	for (int i = 0; i < count; i++)
		strncpy_s(sdCmdLine->arguments[i], ARGUMENT_SIZE, arguments[i], _TRUNCATE);

	sdCmdLine->cArguments = count;
	sdCmdLine->command = *command;
	*sdCmdLine->reply.message = 0;
	sdCmdLine->reply.code =-1;
}

void ProcessConsoleCommand(PCommand command, char *arguments[], int count, PReply reply)
{
	const HANDLE events[] = { heServerDone, heServerClose, heServerBufferFull };
	const int cEvents = sizeof(events) / sizeof(events[0]);

	if (WaitForSingleObject(hmClient, INFINITE) == WAIT_OBJECT_0) {//got the mutex, we're the only one who can talk to miranda now
		FillSharedDataStruct(command, arguments, count);
		SetEvent(heServerExec); //tell Miranda to process the request

		int done = FALSE;
		while (!done) {
			// wait until server either finished processing or miranda was closed
			switch (WaitForMultipleObjects(cEvents, events, FALSE, INFINITE)) {
			case WAIT_OBJECT_0: //done event
				done = TRUE;
				break; //nothing to do

			case WAIT_OBJECT_0 + 1: //close event
			default:
				mir_strcpy(sdCmdLine->reply.message, Translate("Miranda has been closed or an error has occurred while waiting for the result, could not process request."));
				done = TRUE;
				break;

			case WAIT_OBJECT_0 + 2: //buffer full event
				lpprintf("%s", reply->message);
				break;
			}
		}

		reply->code = sdCmdLine->reply.code;
		strncpy_s(reply->message, sdCmdLine->reply.message, _TRUNCATE);

		ReleaseMutex(hmClient); //let other possible clients talk to the server
	}
	else {
		reply->code = -1;
		*reply->message = 0;
	}
}
