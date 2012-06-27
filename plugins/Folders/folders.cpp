/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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
#include "hooked_events.h"
#include "services.h"
#include "events.h"

#include "m_folders.h"

#define MS_FOLDERS_TEST_PLUGIN "Folders/Test/Plugin"

char ModuleName[] = "Folders";
HINSTANCE hInstance;
int hLangpack;

CFoldersList &lstRegisteredFolders = CFoldersList(10); //the list



PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	VERSION,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
  {0x2f129563, 0x2c7d, 0x4a9a, {0xb9, 0x48, 0x97, 0xdf, 0xcc, 0x0a, 0xfd, 0xd7}} //{2f129563-2c7d-4a9a-b948-97dfcc0afdd7}
}; //not used

#ifdef _DEBUG

typedef struct{
	int cbSize;																	//size of struct
	int nUniqueID;															//unique id for this path. This ID should be unique for your plugin.
	char szName[FOLDERS_NAME_MAX_SIZE];					//name to show in options
} FOLDERSDATA_OLD;

INT_PTR TestPlugin(WPARAM wParam, LPARAM lParam)
{
	char temp[MAX_PATH];
	const int MAX = 20;
	int i;
	HANDLE handles[MAX][2];
/*	FOLDERSDATA caca;
	caca.cbSize = sizeof(caca);
	caca.szFormat = FOLDER_LOGS;*/
	char *section;
	for (i = 0; i < MAX; i++)
		{
			switch (i % 4)
				{
					case 0:
						section = "Section 1";
						break;
					case 1:
						section = "Section 2";
						break;
					case 2:
						section = "Yet another section";
						break;
					case 3:
						section = "Section no 4";
						break;
					default:
						section = "Uhh ohh";
						break;
				}
			//strcpy(caca.szSection, section);
			_itoa(i, temp, 10);
			//strcpy(caca.szName, temp);
			
			handles[i][0] = FoldersRegisterCustomPathT(section, temp, FOLDER_LOGS); //CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &caca);
		}
	//caca.szFormatW = L"%profile_path%\\%current_profile%\\Ø";
	//caca.flags = FF_UNICODE;
	FoldersRegisterCustomPath("Unicode stuff", "non unicode", "   %profile_path%\\%current_profile%\\Ø\\\\\\");
	for (i = 0; i < MAX; i++)
		{
			//strcpy(caca.szSection, "Unicode stuff");
			sprintf(temp, "Value %d", i);
			//strcpy(caca.szName, temp);
			handles[i][1] = FoldersRegisterCustomPathW("Unicode stuff", temp, L"%profile_path%\\%current_profile%\\\u1FA6"); //CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM) &caca);
		}
/*		
	FOLDERSAPPENDDATA data;
	data.hRegisteredPath = handles[0];
	data.szAppendData = "just an appended string";
	char *cacat;
	CallService(MS_FOLDERS_GET_PATH_ALLOC_APPEND, (WPARAM) &data, (LPARAM) &cacat);
	Log("Append function returned : %s", cacat); */
	/*FOLDERSGETDATA data = {0};
	data.cbSize = sizeof(data);
	data.nMaxPathSize = sizeof(temp);
	data.szPath = temp; */
	for (i = 0; i < MAX; i++)
		{
			//CallService(MS_FOLDERS_GET_PATH, handles[i][0], (LPARAM) &data);
			FoldersGetCustomPath((HANDLE) handles[i][0], temp, sizeof(temp), "<not found>");
			Log("Path function[%d] returned : %s", i, temp);
		}
	wchar_t buffer[MAX_PATH];
	//data.szPathW = buffer;
	for (i = 0; i < MAX; i++)
		{
			//CallService(MS_FOLDERS_GET_PATH, handles[i][0], (LPARAM) &data);
			FoldersGetCustomPathW((HANDLE) handles[i][1], buffer, MAX_PATH, L"<not found>");
			Log("Unicode path function[%d] returned: %S", i, buffer);
		}
//	GetPathService(CONFIGURATION_PATH, (LPARAM) temp);
//	GetPathAllocService(AVATARS_PATH, (LPARAM) &caca);
	
	
	for (i = 0; i < MAX; i++)
	{
		FoldersGetCustomPathEx((HANDLE) handles[i][0], temp, sizeof(temp), "<not found>", "test");
		Log("Path function Ex (test) [%d] returned : %s", i, temp);
		FoldersGetCustomPathEx((HANDLE) handles[i][0], temp, sizeof(temp), "<not found>", "");
		Log("Path function Ex ()     [%d] returned : %s", i, temp);
		FoldersGetCustomPathEx((HANDLE) handles[i][0], temp, sizeof(temp), "<not found>", NULL);
		Log("Path function Ex (NULL) [%d] returned : %s", i, temp);
	}
	
	for (i = 0; i < MAX; i++)
	{
		FoldersGetCustomPathExW((HANDLE) handles[i][1], buffer, MAX_PATH, L"<not found>", L"test");
		Log("Unicode path function Ex (test) [%d] returned : %S", i, buffer);
		FoldersGetCustomPathExW((HANDLE) handles[i][1], buffer, MAX_PATH, L"<not found>", L"");
		Log("Unicode path function Ex ()     [%d] returned : %S", i, buffer);
		FoldersGetCustomPathExW((HANDLE) handles[i][1], buffer, MAX_PATH, L"<not found>", NULL);
		Log("Unicode path function Ex (NULL) [%d] returned : %S", i, buffer);
	}

	return 0;
}

HANDLE hTestPlugin;
#endif

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_FOLDERS, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(void)
{
#if _MSC_VER >= 1300
	Log("%s", "Entering function " __FUNCTION__);
#endif

	mir_getLP(&pluginInfo);
	
	Log("%s", "Creating service functions ...");
	InitServices();
	InitEvents();
	
#ifdef _DEBUG	
	hTestPlugin = CreateServiceFunction(MS_FOLDERS_TEST_PLUGIN, TestPlugin);
	CLISTMENUITEM mi = {0};
	
	mi.cbSize=sizeof(mi);
	mi.position=300050000;
	mi.flags=0;
	mi.hIcon=0;
	mi.pszName=Translate("Test folders");
	mi.pszService=MS_FOLDERS_TEST_PLUGIN;
	Menu_AddMainMenuItem(&mi);
#endif
#if _MSC_VER >= 1300
	Log("%s", "Hooking events ...");	
#endif
	HookEvents();
#if _MSC_VER >= 1300
	Log("%s", "Leaving function " __FUNCTION__);
#endif
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
#if _MSC_VER >= 1300
	Log("%s", "Entering function " __FUNCTION__);
	Log("%s", "Unhooking events ...");
	
	Log("%s", "Destroying service functions ...");
#endif
//	DestroyServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY);
	DestroyServices();
	DestroyEvents();
	UnhookEvents();
#ifdef _DEBUG	
	DestroyServiceFunction(hTestPlugin);
#endif

#if _MSC_VER >= 1300
	Log("%s", "Leaving function " __FUNCTION__);
#endif
	return 0;
}

bool WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH)
		{
			DisableThreadLibraryCalls(hinstDLL);
		}
	return TRUE;
}