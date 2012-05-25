/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

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

#include "services.h"

HANDLE hsMenuCommand;
HANDLE hsGetInfo;
HANDLE hsServiceMode;

int bServiceMode = 0; //true only if plugin is running in service mode

int InitServices()
{
	hsMenuCommand = CreateServiceFunction(MS_VERSIONINFO_MENU_COMMAND, PluginMenuCommand);
	hsGetInfo = CreateServiceFunction(MS_VERSIONINFO_GETINFO, GetInfoService);
	hsServiceMode = CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceModeService);
	return 0;
}

int DestroyServices()
{
	DestroyServiceFunction(hsMenuCommand);
	DestroyServiceFunction(hsGetInfo);
	DestroyServiceFunction(hsServiceMode);
	return 0;
}

INT_PTR PluginMenuCommand(WPARAM wParam, LPARAM lParam)
{
	int debugTo = DBGetContactSettingByte(NULL, ModuleName, "DebugTo", TO_DIALOGBOX);
	DoDebugTo(debugTo);
	if (verbose) PUShowMessage("I have printed the information.", SM_NOTIFY);
/*	char *data;
	CallService(MS_VERSIONINFO_GETINFO, 1, (LPARAM) &data); */
	return 0;
}

INT_PTR GetInfoService(WPARAM wParam, LPARAM lParam)
{
	int result = 1; //failure
	if (lParam != NULL)
		{
			CVersionInfo myInfo;
			myInfo.Initialize();
			std::string VI = myInfo.GetInformationsAsString(wParam);
			char **retData = (char **) lParam;
			(*retData) = (char *) MirandaMalloc(VI.size() + 1);
			if (retData)
				{
					strcpy(*retData, VI.c_str());
					result = 0; //success
				}
		}
	return result;
}

INT_PTR ServiceModeService(WPARAM wParam, LPARAM lParam)
{
	bServiceMode = 1;
	DoDebugTo(TO_ASK);
	
	return 0;
}
