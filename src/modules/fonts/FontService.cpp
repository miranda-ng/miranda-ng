/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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
#include "FontService.h"

int code_page = CP_ACP;
HANDLE hFontReloadEvent, hColourReloadEvent;

int OptInit(WPARAM, LPARAM);
int FontsModernOptInit(WPARAM wParam, LPARAM lParam);

INT_PTR RegisterFont(WPARAM wParam, LPARAM lParam);
INT_PTR RegisterFontW(WPARAM wParam, LPARAM lParam);

INT_PTR GetFont(WPARAM wParam, LPARAM lParam);
INT_PTR GetFontW(WPARAM wParam, LPARAM lParam);

INT_PTR RegisterColour(WPARAM wParam, LPARAM lParam);
INT_PTR RegisterColourW(WPARAM wParam, LPARAM lParam);

INT_PTR GetColour(WPARAM wParam, LPARAM lParam);
INT_PTR GetColourW(WPARAM wParam, LPARAM lParam);

INT_PTR RegisterEffect(WPARAM wParam, LPARAM lParam);
INT_PTR RegisterEffectW(WPARAM wParam, LPARAM lParam);

INT_PTR GetEffect(WPARAM wParam, LPARAM lParam);
INT_PTR GetEffectW(WPARAM wParam, LPARAM lParam);

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_MODERNOPT_INITIALIZE, FontsModernOptInit);
	return 0;
}

static int OnPreShutdown(WPARAM, LPARAM)
{
	DestroyHookableEvent(hFontReloadEvent);
	DestroyHookableEvent(hColourReloadEvent);

	font_id_list.destroy();
	colour_id_list.destroy();
	return 0;
}

int LoadFontserviceModule(void)
{
	code_page = Langpack_GetDefaultCodePage();

	CreateServiceFunction("Font/Register", RegisterFont);
	CreateServiceFunction("Font/RegisterW", RegisterFontW);
	CreateServiceFunction(MS_FONT_GET, GetFont);
	CreateServiceFunction(MS_FONT_GETW, GetFontW);

	CreateServiceFunction("Colour/Register", RegisterColour);
	CreateServiceFunction("Colour/RegisterW", RegisterColourW);
	CreateServiceFunction(MS_COLOUR_GET, GetColour);
	CreateServiceFunction(MS_COLOUR_GETW, GetColourW);

	CreateServiceFunction("Effect/Register", RegisterEffect);
	CreateServiceFunction("Effect/RegisterW", RegisterEffectW);
	CreateServiceFunction(MS_EFFECT_GET, GetEffect);
	CreateServiceFunction(MS_EFFECT_GETW, GetEffectW);

	hFontReloadEvent = CreateHookableEvent(ME_FONT_RELOAD);
	hColourReloadEvent = CreateHookableEvent(ME_COLOUR_RELOAD);

	// create generic fonts
	FontIDT fontid = { sizeof(fontid) };
	strncpy(fontid.dbSettingsGroup, "Fonts", sizeof(fontid.dbSettingsGroup));
	_tcsncpy(fontid.group, LPGENT("General"), SIZEOF(fontid.group));

	_tcsncpy(fontid.name, LPGENT("Headers"), SIZEOF(fontid.name));
	fontid.flags = FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_CLASSHEADER;
	strncpy(fontid.prefix, "Header", SIZEOF(fontid.prefix));
	FontRegisterT(&fontid);

	_tcsncpy(fontid.name, LPGENT("Generic text"), SIZEOF(fontid.name));
	fontid.flags = FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_CLASSGENERAL;
	strncpy(fontid.prefix, "Generic", SIZEOF(fontid.prefix));
	FontRegisterT(&fontid);

	_tcsncpy(fontid.name, LPGENT("Small text"), SIZEOF(fontid.name));
	fontid.flags = FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_CLASSSMALL;
	strncpy(fontid.prefix, "Small", SIZEOF(fontid.prefix));
	FontRegisterT(&fontid);

	// do last for silly dyna plugin
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	return 0;
}
