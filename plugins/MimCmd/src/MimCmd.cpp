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
#include "..\..\build\appstub\appstub.cpp"

wchar_t* GetProgramName(wchar_t *programName, size_t size)
{
	wchar_t name[512];
	GetModuleFileNameW(GetModuleHandleW(nullptr), name, _countof(name));
	wchar_t *p = wcsrchr(name, '\\');
	if (p)
		wcsncpy_s(programName, size, p + 1, _TRUNCATE);
	else
		wcsncpy_s(programName, size, name, _TRUNCATE);

	return programName;
}

void PrintUsage()
{
	wchar_t name[128];
	GetProgramName(name, _countof(name));

	wprintf(TranslateW_LP(L"%s usage:\n"), name);
	wprintf(TranslateW_LP(L"%s <command> [<param> [<param> [...]]].\n"), name);
	wprintf(TranslateW_LP(L"This will tell Miranda to run the specified command. The commands can have zero, one or more parameters. Use '%s help' to get a list of possible commands.\n"), name);
	wprintf(TranslateW_LP(L"No command can have more than %d parameters.\n"), MAX_ARGUMENTS - 1);
}

void ShowVersion()
{
	wchar_t name[128];
	GetProgramName(name, _countof(name));
	wprintf(TranslateW_LP(L"%s version %s"), name, __VERSION_STRING_DOTS);
}

int wmain(int argc, wchar_t *argv[])
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	if (argc == 2 && !wcscmp(argv[1], L"-v")) {
		ShowVersion();
		return 0;
	}

	if ((InitClient()) || (ConnectToMiranda()) || (GetKnownCommands()) || (LoadLangPackModule())) {
		wprintf(L"Could not create connection with Miranda or could not retrieve list of known commands.\n");
		return MIMRES_NOMIRANDA;
	}

	if (argc <= 1 || argc > MAX_ARGUMENTS) {
		PrintUsage();
		return 0;
	}

	int error;
	PReply reply = ParseCommand(argv, argc);
	if (reply) {
		error = reply->code;
		wprintf(L"%s\n", reply->message);
	}
	else {
		wprintf(TranslateW_LP(L"Unknown command '%s'.\n"), argv[1]);
		error = 0;
	}

	DestroyKnownCommands();
	DisconnectFromMiranda();
	DestroyClient();
	return error;
}
