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

int hLangpack = 0;

int lpprintf(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	const int MAX_SIZE = 16192;
	char buffer[MAX_SIZE] = {0};
	int len = mir_vsnprintf(buffer, MAX_SIZE - 1, format, va);
	buffer[MAX_SIZE - 1] = 0;
	va_end(va);
	CharToOemBuff(buffer, buffer, len);
	printf("%s", buffer);

	return len;
}

char *GetProgramName(char *programName, int size)
{
	char name[512];
	GetModuleFileName(GetModuleHandle(NULL), name, sizeof(name));
	char *p = strrchr(name, '\\');
	if (p)
	{
		STRNCPY(programName, p + 1, size);
	}
	else{
		STRNCPY(programName, name, size);
	}

	return programName;
}

void PrintUsage()
{
	char name[128];
	GetProgramName(name, sizeof(name));

	lpprintf(Translate("%s usage:\n"), name);
	lpprintf(Translate("%s <command> [<param> [<param> [...]]].\n"), name);
	lpprintf(Translate("This will tell Miranda to run the specified command. The commands can have zero, one or more parameters. Use '%s help' to get a list of possible commands.\n"), name);
	lpprintf(Translate("No command can have more than %d parameters.\n"), MAX_ARGUMENTS - 1);
}

void ShowVersion()
{
	char name[128];
	char message[1024];
	GetProgramName(name, sizeof(name));
	mir_snprintf(message, sizeof(message), Translate("%s version %s"), name, __VERSION_STRING_DOTS);

	lpprintf("%s\n", message);
}

int main(int argc, char *argv[])
{
	int error = 0;
	if ((argc == 2) && (mir_strcmp(argv[1], "-v") == 0))
	{
		ShowVersion();

		return 0;
	}

	if ((InitClient()) || (ConnectToMiranda()) || (GetKnownCommands()) || (LoadLangPackModule()))
	{
		lpprintf("Could not create connection with Miranda or could not retrieve list of known commands.\n");
		error = MIMRES_NOMIRANDA;
	}
	else{
		if ((argc <= 1) || (argc > MAX_ARGUMENTS))
		{
			PrintUsage();
		}
		else{
			PReply reply = ParseCommand(argv, argc);
			if (reply)
			{
				error = reply->code;
				lpprintf("%s\n", reply->message);
			}
			else{
				lpprintf(Translate("Unknown command '%s'.\n"), argv[1]);
			}

			DestroyKnownCommands();
			DisconnectFromMiranda();
			DestroyClient();
		}
	}

	return error;
}
