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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aLONG with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.

this file is taken from UserinfoEx plugin and support UserinfoEx icon pack !!!!

*/

#include "global.h"

HICON ghDefIcon = NULL;

//IDI_PLUG_MAIN must be the first icon from Plugin.dll, all other icon must be IDI_PLUG_MAIN+n

struct
{
	LPSTR	pszName;
	LPSTR	pszDesc;
	LPSTR	pszSection;
	WORD	idResource;
	BYTE	size;
}
static icoDesc[] = 
{
	// common
	{ ICO_PLUG_SSWINDOW1,   LPGEN("Screenshot Icon1"), SECT_COMMON,  IDI_PLUG_MAIN,      -1 },
	{ ICO_PLUG_SSWINDOW2,   LPGEN("Screenshot Icon2"), SECT_COMMON,  IDI_PLUG_ICON1,      0 },
	{ ICO_PLUG_SSTARGET,    LPGEN("Target Cursor"),    SECT_COMMON,  IDI_PLUG_ICON2,      1 },
	{ ICO_PLUG_SSMONITOR,   LPGEN("Target Desktop"),   SECT_COMMON,  IDI_PLUG_ICON3,      1 },
	{ ICO_PLUG_SSDEFAULT,   LPGEN("Default"),          SECT_COMMON,  IDI_PLUG_DEFAULT,    0 },

	// overlays
	{ ICO_PLUG_OVERLAYON,   LPGEN("Overlay on"),       SECT_OVERLAY, IDI_PLUG_OVERLAYON,  0 },
	{ ICO_PLUG_OVERLAYOFF,  LPGEN("Overlay off"),      SECT_OVERLAY, IDI_PLUG_OVERLAYOFF, 0 },

	// button icons
	{ ICO_PLUG_SSHELP,      LPGEN("Help"),             SECT_BUTTONS, IDI_PLUG_HELP,       0 },
	{ ICO_PLUG_SSFOLDERO,   LPGEN("Open Folder"),      SECT_BUTTONS, IDI_PLUG_FOLDERO,    0 },
	{ ICO_PLUG_SSDESKOFF,   LPGEN("Description off"),  SECT_BUTTONS, IDI_PLUG_DESKOFF,    0 },
	{ ICO_PLUG_SSDESKON,    LPGEN("Description on"),   SECT_BUTTONS, IDI_PLUG_DESKON,     0 },
	{ ICO_PLUG_SSDELOFF,    LPGEN("Delete off"),       SECT_BUTTONS, IDI_PLUG_DELOFF,     0 },
	{ ICO_PLUG_SSDELON,     LPGEN("Delete on"),        SECT_BUTTONS, IDI_PLUG_DELON,      0 },
	{ ICO_PLUG_ARROWL,      LPGEN("Prev"),             SECT_BUTTONS, IDI_PLUG_ARROWL,     0 },
	{ ICO_PLUG_ARROWR,      LPGEN("Next"),             SECT_BUTTONS, IDI_PLUG_ARROWR,     0 },
	{ ICO_PLUG_UPDATE,      LPGEN("Update"),           SECT_BUTTONS, IDI_PLUG_UPDATE,     0 },
	{ ICO_PLUG_OK,          LPGEN("OK"),               SECT_BUTTONS, IDI_PLUG_OK,         0 },
	{ ICO_PLUG_CANCEL,      LPGEN("Cancel"),           SECT_BUTTONS, IDI_PLUG_CLOSE,      0 },
	{ ICO_PLUG_APPLY,       LPGEN("Apply"),            SECT_BUTTONS, IDI_PLUG_APPLY,      0 },
	{ ICO_PLUG_EDIT,        LPGEN("Edit"),             SECT_BUTTONS, IDI_PLUG_EDIT,       0 },
	{ ICO_PLUG_DOWNARROW,   LPGEN("Down arrow"),       SECT_BUTTONS, IDI_PLUG_DOWNARROW,  0 },
};

/**
 * Returns a icon, identified by a name
 *
 * @param	pszIcon	- name of the icon
 *
 * @return:	HICON if the icon is loaded, NULL otherwise
 **/
HICON IcoLib_GetIcon(LPCSTR pszIcon, bool big)
{
	return (pszIcon) ? Skin_GetIcon(pszIcon, big) : NULL;
}

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

static HANDLE IcoLib_RegisterIconHandleEx(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, LPTSTR szDefaultFile, INT idIcon, INT Size, HICON hDefIcon)
{
	if (!szIconID || !szDescription || !szSection)
		return NULL;

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_ALL_TCHAR;
	sid.pszName = szIconID;
	sid.ptszDescription = mir_a2t(szDescription);
	sid.ptszSection = mir_a2t(szSection);

	switch (Size) {
	// small and big icons
	case -1:
		sid.cx = sid.cy = 0;
		break;

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
	if (sid.ptszDefaultFile && sid.ptszDefaultFile[0])
		sid.iDefaultIndex = idIcon - IDI_PLUG_MAIN;
	else {
		sid.hDefaultIcon = hDefIcon;
		sid.iDefaultIndex = -1;
	}

	HANDLE hIconHandle = Skin_AddIcon(&sid);
	mir_free(sid.ptszDescription);
	mir_free(sid.ptszSection);
	return hIconHandle;
}

/**
* Add default icons to the skin library or load customized icons
*
* @param		none
*
* @return		nothing
**/
VOID IcoLib_LoadModule()
{
	LPTSTR	szPluginFile;
	INT_PTR i;

	szPluginFile = _T("Plugins\\")_T(__FILENAME);

	// load default icon if required
	ghDefIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_PLUG_DEFAULT), IMAGE_ICON, 
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

	for (i = 0; i < SIZEOF(icoDesc); i++) 
		IcoLib_RegisterIconHandleEx(icoDesc[i].pszName, icoDesc[i].pszDesc, icoDesc[i].pszSection, szPluginFile, icoDesc[i].idResource, icoDesc[i].size, ghDefIcon);
}

