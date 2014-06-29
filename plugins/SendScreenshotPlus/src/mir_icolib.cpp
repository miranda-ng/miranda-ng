/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

this file is taken from UserinfoEx plugin and support UserinfoEx icon pack !!!!

*/

#include "global.h"

typedef struct _ICODESC 
{
	LPSTR	pszName;
	LPSTR	pszDesc;
	LPSTR	pszSection;
	WORD	idResource;
	BYTE	size;
} ICODESC;

HICON ghDefIcon = NULL;

//IDI_MAIN must be the first icon from Plugin.dll, all other icon must be IDI_MAIN+n

static ICODESC icoDesc[] = 
{
	// common
	{ ICO_COMMON_SSWINDOW1,	LPGEN("Screenshot Icon1"),	SECT_COMMON,	IDI_MAIN,			1 },
	{ ICO_COMMON_SSWINDOW2,	LPGEN("Screenshot Icon2"),	SECT_COMMON,	IDI_MAIN,			0 },
	{ ICO_COMMON_SSTARGET,	LPGEN("Target Cursor"),		SECT_COMMON,	IDI_TARGET,			1 },
	{ ICO_COMMON_SSMONITOR,	LPGEN("Target Desktop"),	SECT_COMMON,	IDI_MONITOR,		1 },
	//{ ICO_COMMON_SSDEFAULT,	LPGEN("Default"),			SECT_COMMON,	IDI_DEFAULT,		0 },
	// overlays
//	{ ICO_BTN_OVERLAYON,	LPGEN("Overlay on"),		SECT_OVERLAY,	IDI_OVERLAYON,		0 },
//	{ ICO_BTN_OVERLAYOFF,	LPGEN("Overlay off"),		SECT_OVERLAY,	IDI_OVERLAYOFF,		0 },
	// button icons
	{ ICO_COMMON_SSHELP,	LPGEN("Help"),				SECT_BUTTONS,	IDI_HELP,			0 },
	{ ICO_COMMON_SSFOLDERO,	LPGEN("Open Folder"),		SECT_BUTTONS,	IDI_FOLDERO,		0 },
	{ ICO_COMMON_SSDESKOFF,	LPGEN("Description off"),	SECT_BUTTONS,	IDI_DESKOFF,		0 },
	{ ICO_COMMON_SSDESKON,	LPGEN("Description on"),	SECT_BUTTONS,	IDI_DESKON,			0 },
	{ ICO_COMMON_SSDELOFF,	LPGEN("Delete off"),		SECT_BUTTONS,	IDI_DELOFF,			0 },
	{ ICO_COMMON_SSDELON,	LPGEN("Delete on"),			SECT_BUTTONS,	IDI_DELON,			0 },
	{ ICO_COMMON_ARROWL,	LPGEN("Prev"),				SECT_BUTTONS,	IDI_ARROWL,			0 },
	{ ICO_COMMON_ARROWR,	LPGEN("Next"),				SECT_BUTTONS,	IDI_ARROWR,			0 },
	{ ICO_BTN_UPDATE,		LPGEN("Update"),			SECT_BUTTONS,	IDI_UPDATE,			0 },
	{ ICO_BTN_OK,			LPGEN("OK"),				SECT_BUTTONS,	IDI_OK,				0 },
	{ ICO_BTN_CANCEL,		LPGEN("Cancel"),			SECT_BUTTONS,	IDI_CLOSE,			0 },
//	{ ICO_BTN_APPLY,		LPGEN("Apply"),				SECT_BUTTONS,	IDI_APPLY,			0 },
	{ ICO_BTN_EDIT,			LPGEN("Edit"),				SECT_BUTTONS,	IDI_EDIT,			0 },
	{ ICO_BTN_EDITON,		LPGEN("Edit on"),			SECT_BUTTONS,	IDI_EDITON,			0 },
	{ ICO_BTN_COPY,			LPGEN("Copy"),				SECT_BUTTONS,	IDI_COPY,			0 },
	{ ICO_BTN_BBC,			LPGEN("BBC"),				SECT_BUTTONS,	IDI_BBC,			0 },
	{ ICO_BTN_BBC2,			LPGEN("BBC link"),			SECT_BUTTONS,	IDI_BBC2,			0 },
	{ ICO_BTN_DOWNARROW,	LPGEN("Down arrow"),		SECT_BUTTONS,	IDI_DOWNARROW,		0 },
};

/**
 * This function manually registers a single icon from the default icon library.
 *
 * @param		szIconID		- This is the uniquely identifying string for an icon. 
 *								  This string is the setting name in the database and should 
 *								  only use ASCII characters.
 * @param		szDescription	- This is the description displayed in the options dialog.
 * @param		szSection		- This is the subsection, where the icon is organized in the options dialog.
 * @param		szDefaultFile	- This is the validated path to the default icon file.
 * @param		idIcon			- This is the ResourceID of the icon in the default file.
 * @param		Size			- This is the desired size of the icon to load.
 *								  0:	default size for small icons (16x16)
 *								  1:	default size for normal icons (32x32)
 * @param		hDefIcon		- This is the default icon to use if the default icon
 *								  file does not exist and no custom icon is set up in the config.
 *
 * @return	This function returns the HANDLE of the icon item.
 **/
static HANDLE IcoLib_RegisterIconHandleEx(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, LPTSTR szDefaultFile, int idIcon, int Size)
{
	HANDLE hIconHandle = NULL;

	if (szIconID && szDescription && szSection) {
		SKINICONDESC sid = { sizeof(sid) };
		sid.flags = SIDF_ALL_TCHAR;
		sid.pszName = szIconID;
		sid.ptszDescription = mir_a2t(szDescription);
		sid.ptszSection = mir_a2t(szSection);

		if (sid.ptszDescription && sid.ptszSection) {
			switch (Size) {
			// small icons (16x16)
			case 0:
				sid.cx = GetSystemMetrics(SM_CXSMICON);
				sid.cy = GetSystemMetrics(SM_CYSMICON);
				break;

			// normal icons (32x32)
			case 1:
				sid.cx = GetSystemMetrics(SM_CXICON);
				sid.cy = GetSystemMetrics(SM_CYICON);
				break;

			// custom icon size
			default:
				sid.cx = sid.cy = Size;
				break;
			}

			sid.ptszDefaultFile = szDefaultFile;
			sid.iDefaultIndex = -idIcon;
			hIconHandle = Skin_AddIcon(&sid);
		}
		MIR_FREE(sid.ptszDescription);
		MIR_FREE(sid.ptszSection);
	}
	return hIconHandle;
}

/**
 * Add default icons to the skin library or load customized icons
 *
 * @param		none
 *
 * @return		nothing
 **/

//static IconItem main = { LPGEN("Main"), ICO_COMMON_MAIN, IDI_MAIN };

void IcoLib_LoadModule()
{
	LPTSTR szDefaultFile = _T("Plugins\\")_T(__FILENAME);

	for (int i = 0; i < SIZEOF(icoDesc); i++)
		IcoLib_RegisterIconHandleEx(
			icoDesc[i].pszName, icoDesc[i].pszDesc, icoDesc[i].pszSection, 
			szDefaultFile, icoDesc[i].idResource, icoDesc[i].size);
}
