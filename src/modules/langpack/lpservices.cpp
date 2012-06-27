/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

MIR_CORE_DLL(int) LangPackMarkPluginLoaded(PLUGININFOEX* pInfo);

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
	*(int*)wParam = LangPackMarkPluginLoaded((PLUGININFOEX*)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvGetDefaultCodePage(WPARAM, LPARAM)
{
	return LangPackGetDefaultCodePage();
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvGetDefaultLocale(WPARAM, LPARAM)
{
	return LangPackGetDefaultLocale();
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR srvPcharToTchar(WPARAM wParam, LPARAM lParam)
{
	char* pszStr = (char*)lParam;
	if (pszStr == NULL)
		return NULL;

	int len = (int)strlen(pszStr);
	TCHAR* result = (TCHAR*)alloca((len+1)*sizeof(TCHAR));
	MultiByteToWideChar(LangPackGetDefaultCodePage(), 0, pszStr, -1, result, len);
	result[len] = 0;
	return (INT_PTR)mir_wstrdup( TranslateW_LP(result, wParam));
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR srvReloadLangpack(WPARAM wParam, LPARAM lParam)
{	
	ReloadLangpack(( TCHAR* )lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) LoadLangPackModule(void);

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
	return 0;
}
