/*
Bonsai plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

static CRITICAL_SECTION csServices;

static DWORD OPTIONS_ADDPAGE_HASH = NameHashFunction("Opt/AddPage");

CALLSERVICEFUNCTION realCallServiceFunction = NULL;

int InitServices()
{
	InitializeCriticalSection(&csServices);
	
	HookRealServices();
	
	return 0;
}

int DestroyServices()
{
	UnhookRealServices();
		
	DeleteCriticalSection(&csServices);
	
	return 0;
}

void HookRealServices()
{
	Log("Hooking real CallService(). Replacing 0x%p with 0x%p", pluginLink->CallService, BonsaiCallServiceFunction);
	realCallServiceFunction = pluginLink->CallService;
	pluginLink->CallService = BonsaiCallServiceFunction;
}

void UnhookRealServices()
{
	Log("Unhooking real CallService(). Replacing 0x%p with 0x%p", pluginLink->CallService, realCallServiceFunction);
	EnterCriticalSection(&csServices);
	
	pluginLink->CallService = realCallServiceFunction;
	
	LeaveCriticalSection(&csServices);
}

INT_PTR BonsaiCallServiceFunction(const char *name, WPARAM wParam, LPARAM lParam)
{
	int res;
	int run = 1;
	int fix = 0;
	const int MAX_SIZE = 256;
	WCHAR group[MAX_SIZE];
	WCHAR title[MAX_SIZE];
	WCHAR tabTitle[MAX_SIZE];
	
	BYTE temp[200] = {0};
	OPTIONSDIALOGPAGE *options = (OPTIONSDIALOGPAGE *) lParam;
	
	if (NameHashFunction(name) == OPTIONS_ADDPAGE_HASH)
	{
		memmove(temp, options, options->cbSize);
		options = (OPTIONSDIALOGPAGE *) temp;
		//EnterCriticalSection(&csServices);
		if ((options->cbSize > 60) && !(options->flags & ODPF_UNICODE) && (options->pszTab) && (!bUseANSIStrings))
		{
			MultiByteToWideChar(CP_ACP, 0, options->pszTab, -1, tabTitle, MAX_SIZE);
			options->pszTab = (char *) tabTitle;
		}

		FixPluginOptions(name, options, group, MAX_SIZE, title, MAX_SIZE);
		if (!options->pszTitle)
		{
			run = 0;
		}
	}
	res = (run) ? realCallServiceFunction(name, wParam, (LPARAM) options) : CALLSERVICE_NOTFOUND;
	
	return res;
}

int FixPluginOptions(const char *serviceName, void *options, WCHAR *group, int cGroup, WCHAR *title, int cTitle)
{
	MEMORY_BASIC_INFORMATION mbi;
	HINSTANCE hInstance;
	const int MODULE_MAX_SIZE = 256;
	char mod[MODULE_MAX_SIZE];
	WCHAR plugin[MODULE_MAX_SIZE];
	mod[0] = '\0';
	if(VirtualQuery(serviceName, &mbi, sizeof(mbi) ))
	{
		hInstance = (HINSTANCE)(mbi.AllocationBase);
		char mod_path[MAX_PATH];
		GetModuleFileName(hInstance, mod_path, MAX_PATH);
		// chop path
		char *p = strrchr(mod_path, '.');
		if (p)
		{
			p[0] = '\0';
		}
		
		p = strrchr(mod_path, '\\');
		if(p)
		{
			p++;
			strncpy(mod, p, sizeof(mod));
		}
	}
	
	if (strlen(mod) > 0)
	{
		if (bUseANSIStrings)
		{
			_strlwr(mod);
		}
		MultiByteToWideChar(CP_ACP, 0, mod, -1, plugin, MODULE_MAX_SIZE);
		const int MAX_SIZE = 256;
		WCHAR temp1[MAX_SIZE];
		WCHAR temp2[MAX_SIZE];
		WCHAR *optGroup = temp1;
		WCHAR *optTitle = temp2;
		OPTIONSDIALOGPAGE *odp = (OPTIONSDIALOGPAGE *) options;
		temp1[0] = temp2[0] = L'\0';
		
		if ((odp->flags & ODPF_UNICODE) && (odp->cbSize > 40))
		{
			
			optGroup = (WCHAR *) odp->pszGroup;
			optTitle = (WCHAR *) odp->pszTitle;
			
			Log("[UNICODE] %-15S - Initial options location %S->%S (unicode)", plugin, optGroup, optTitle);
		}
		else{
			Log("          %-15S - Initial options location %s->%s", plugin, odp->pszGroup, odp->pszTitle);
			MultiByteToWideChar(CP_ACP, 0, odp->pszGroup, -1, temp1, MAX_SIZE);
			MultiByteToWideChar(CP_ACP, 0, odp->pszTitle, -1, temp2, MAX_SIZE);
			
			if (odp->cbSize > 40) //some old plugins don't have flags variable in structure
			{
				odp->flags |= ODPF_UNICODE;
			}
		}
		
		int index = lstPluginOptions.Add(plugin, optGroup, optTitle);
		odp->pszGroup = (char *) GetPluginGroup(lstPluginOptions[index], group, cGroup);
		odp->pszTitle = (char *) GetPluginTitle(lstPluginOptions[index], title, cTitle);
		Log("[UNICODE] %-15S - New options location %S->%S", plugin, odp->pszGroup, odp->pszTitle);
		
		if ((bUseANSIStrings) || (odp->cbSize <= 40)) //if plugins doesn't have flags variable in structure then they only support ansi strings
		{
			Log("          Plugin is either very old or we're running on Windows 98, converting back to ansi strings ...");
			WCHAR temp[MAX_SIZE];
			
			if (odp->pszGroup)
			{
				wcsncpy(temp, (WCHAR *) odp->pszGroup, MAX_SIZE);
				WideCharToMultiByte(CP_ACP, 0, temp, -1, odp->pszGroup, cGroup, NULL, NULL);
			}
			
			if (odp->pszTitle)
			{
				wcsncpy(temp, (WCHAR *) odp->pszTitle, MAX_SIZE);
				WideCharToMultiByte(CP_ACP, 0, temp, -1, odp->pszTitle, cTitle, NULL, NULL);
			}
			
			if (odp->cbSize > 40)
			{
				Log("          Windows98 - Resetting ODPF_UNICODE flag");
				odp->flags &= ~ODPF_UNICODE;
			}
		}
	}

	return 0;
}