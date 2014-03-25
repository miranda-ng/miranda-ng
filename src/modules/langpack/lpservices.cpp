/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "..\..\core\commonheaders.h"

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvTranslateString(WPARAM wParam, LPARAM lParam)
{
	if (wParam & LANG_UNICODE)
		return (INT_PTR)TranslateW_LP((const WCHAR*)lParam, wParam);
	return (INT_PTR)TranslateA_LP((const char *)lParam, wParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvTranslateMenu(WPARAM wParam, LPARAM lParam)
{
	TranslateMenu_LP((HMENU)wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvRegisterLP(WPARAM wParam, LPARAM lParam)
{
	PLUGININFOEX* ppi = (PLUGININFOEX*)lParam;
	if (wParam && ppi)
		*(int*)wParam = GetPluginFakeId(ppi->uuid, Langpack_MarkPluginLoaded(ppi));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvGetDefaultCodePage(WPARAM, LPARAM)
{
	return Langpack_GetDefaultCodePage();
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvGetDefaultLocale(WPARAM, LPARAM)
{
	return Langpack_GetDefaultLocale();
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvPcharToTchar(WPARAM wParam, LPARAM lParam)
{
	char *pszStr = (char*)lParam;
	if (pszStr == NULL)
		return NULL;

	int len = (int)strlen(pszStr);
	TCHAR *result = (TCHAR*)alloca((len+1)*sizeof(TCHAR));
	MultiByteToWideChar(Langpack_GetDefaultCodePage(), 0, pszStr, -1, result, len);
	result[len] = 0;
	return (INT_PTR)mir_wstrdup(TranslateW_LP(result, wParam));
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvReloadLangpack(WPARAM wParam, LPARAM lParam)
{
	ReloadLangpack((TCHAR*)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvGetPluginLangpack(WPARAM wParam, LPARAM lParam)
{
	return GetPluginLangByInstance((HINSTANCE)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadLangpackModule(void)
{
	LoadLangPackModule();

	CreateServiceFunction(MS_LANGPACK_TRANSLATESTRING, srvTranslateString);
	CreateServiceFunction(MS_LANGPACK_TRANSLATEMENU, srvTranslateMenu);
	CreateServiceFunction(MS_LANGPACK_GETCODEPAGE, srvGetDefaultCodePage);
	CreateServiceFunction(MS_LANGPACK_GETLOCALE, srvGetDefaultLocale);
	CreateServiceFunction(MS_LANGPACK_PCHARTOTCHAR, srvPcharToTchar);
	CreateServiceFunction(MS_LANGPACK_REGISTER, srvRegisterLP);
	CreateServiceFunction(MS_LANGPACK_RELOAD, srvReloadLangpack);
	CreateServiceFunction(MS_LANGPACK_LOOKUPHANDLE, srvGetPluginLangpack);
	return 0;
}
