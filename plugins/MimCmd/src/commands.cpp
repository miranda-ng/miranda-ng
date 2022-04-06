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

#include "stdafx.h"

LISTCOMMANDS ListCommands = nullptr;

PCommand knownCommands = nullptr;
int cKnownCommands = 0;

HMODULE hCmdLineDLL = nullptr;

wchar_t* GetMirandaFolder(wchar_t *mimFolder, int size)
{
	wcsncpy_s(mimFolder, size, sdCmdLine->mimFolder, _TRUNCATE);
	return mimFolder;
}

int ConnectToMiranda()
{
	wchar_t tszPath[MAX_PATH];
	GetModuleFileName(nullptr, tszPath, _countof(tszPath));
	wchar_t *p = wcsrchr(tszPath, '\\');
	if (p) p[1] = 0;

	wcsncat_s(tszPath, L"libs", _TRUNCATE);
	uint32_t cbPath = (uint32_t)wcslen(tszPath);

	uint32_t cbSize = GetEnvironmentVariable(L"PATH", nullptr, 0);
	wchar_t *ptszVal = new wchar_t[cbSize + MAX_PATH + 2];
	wcscpy(ptszVal, tszPath);
	wcscat(ptszVal, L";");
	GetEnvironmentVariable(L"PATH", ptszVal + cbPath + 1, cbSize);
	SetEnvironmentVariable(L"PATH", ptszVal);
	delete[] ptszVal;

	wchar_t pluginPath[1024] = {0};
	GetMirandaFolder(pluginPath, _countof(pluginPath));
	if(pluginPath[0])
		mir_wstrcat(pluginPath, L"\\plugins\\cmdline.dll");
	else
		mir_wstrcat(pluginPath, L"plugins\\cmdline.dll");

	ListCommands = nullptr;

	hCmdLineDLL = LoadLibraryW(pluginPath);

	int failure = 1;
	if (hCmdLineDLL)
		ListCommands = (LISTCOMMANDS)GetProcAddress(hCmdLineDLL, "ListCommands");

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
	return (knownCommands == nullptr);
}

int DestroyKnownCommands()
{
	return 0;
}

PCommand GetCommand(wchar_t *command)
{
	int i;
	wchar_t lower[512];
	wcsncpy_s(lower, command, _TRUNCATE);
	_wcslwr(lower);

	for (i = 0; i < cKnownCommands; i++)
		if (mir_wstrcmp(knownCommands[i].command, lower) == 0)
			return &knownCommands[i];

	//allow more parameters to trigger the help command - /h -h /? --help
	if ((mir_wstrcmp(lower, L"/h") == 0) || (mir_wstrcmp(lower, L"-h") == 0) || (mir_wstrcmp(lower, L"/?") == 0) || (mir_wstrcmp(lower, L"--help") == 0))
		for (i = 0; i < cKnownCommands; i++)
			if (knownCommands[i].ID == MIMCMD_HELP)
				return &knownCommands[i];

	return nullptr;
}

void HandleHelpCommand(PCommand, wchar_t *argv[], int argc, PReply reply)
{
	CMStringW szReply;

	if (argc >= 3) {
		PCommand command = GetCommand(argv[2]);

		if (command) {
			reply->code = MIMRES_SUCCESS;
			szReply.Append(TranslateW_LP(command->help));
		}
		else {
			reply->code = MIMRES_NOTFOUND;
			szReply.AppendFormat(TranslateW_LP(L"No help for '%s'."), argv[2]);
		}
	}
	else {
		reply->code = MIMRES_SUCCESS;
		szReply.Append(TranslateW_LP(L"Available commands: "));

		for (int i = 0; i < cKnownCommands - 1; i++) {
			szReply.Append(knownCommands[i].command);
			szReply.Append(L", ");
		}
		szReply.Append(knownCommands[cKnownCommands-1].command);
		szReply.AppendChar('.');
	}
	wcsncpy_s(reply->message, szReply, _TRUNCATE);
}

PReply ParseCommand(wchar_t *argv[], int argc)
{
	PCommand command = GetCommand(argv[1]);
	if (!command)
		return nullptr;

	PReply reply = &sdCmdLine->reply;
	if (command->ID == MIMCMD_HELP)
		HandleHelpCommand(command, argv, argc, reply);
	else
		ProcessConsoleCommand(command, argv, argc, reply);

	return reply;
}

void FillSharedDataStruct(PCommand command, wchar_t *arguments[], int count)
{
	for (int i = 0; i < count; i++)
		wcsncpy_s(sdCmdLine->arguments[i], ARGUMENT_SIZE, arguments[i], _TRUNCATE);

	sdCmdLine->cArguments = count;
	sdCmdLine->command = *command;
	*sdCmdLine->reply.message = 0;
	sdCmdLine->reply.code =-1;
}

void ProcessConsoleCommand(PCommand command, wchar_t *arguments[], int count, PReply reply)
{
	const HANDLE events[] = { heServerDone, heServerClose, heServerBufferFull };

	if (WaitForSingleObject(hmClient, INFINITE) == WAIT_OBJECT_0) {//got the mutex, we're the only one who can talk to miranda now
		FillSharedDataStruct(command, arguments, count);
		SetEvent(heServerExec); //tell Miranda to process the request

		int done = FALSE;
		while (!done) {
			// wait until server either finished processing or miranda was closed
			switch (WaitForMultipleObjects(_countof(events), events, FALSE, INFINITE)) {
			case WAIT_OBJECT_0: //done event
				done = TRUE;
				break; //nothing to do

			case WAIT_OBJECT_0 + 1: //close event
			default:
				mir_wstrcpy(sdCmdLine->reply.message, TranslateW_LP(L"Miranda has been closed or an error has occurred while waiting for the result, could not process request."));
				done = TRUE;
				break;

			case WAIT_OBJECT_0 + 2: //buffer full event
				wprintf(L"%s", reply->message);
				break;
			}
		}

		reply->code = sdCmdLine->reply.code;
		wcsncpy_s(reply->message, sdCmdLine->reply.message, _TRUNCATE);

		ReleaseMutex(hmClient); //let other possible clients talk to the server
	}
	else {
		reply->code = -1;
		*reply->message = 0;
	}
}
