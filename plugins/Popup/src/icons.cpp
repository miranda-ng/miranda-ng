/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/icons.cpp $
Revision       : $Revision: 1610 $
Last change on : $Date: 2010-06-23 00:55:13 +0300 (Ð¡Ñ€, 23 Ð¸ÑŽÐ½ 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

struct
{
	LPSTR	pszName;
	LPSTR	ptszDesc;
	LPSTR	ptszSection;
	WORD	idResource;
	LPSTR	pszIcon;
	int	size;
}
static icoDesc[] = 
{
	//toolbar
	{ ICO_TB_POPUP_ON,    "Popups are enabled",          SECT_TOLBAR,               IDI_POPUP,          NULL,  0 },
	{ ICO_TB_POPUP_OFF,   "Popups are disabled",         SECT_TOLBAR,               IDI_NOPOPUP,        NULL,  0 },

	//common popup
	{ ICO_POPUP_ON,       "Popups are enabled",          SECT_POPUP,                IDI_POPUP,          NULL,  0 },
	{ ICO_POPUP_OFF,      "Popups are disabled",         SECT_POPUP,                IDI_NOPOPUP,        NULL,  0 },
	{ ICO_FAV,            "With \"favourite\" overlay",  SECT_POPUP,                IDI_PU_FAVOURITE,   NULL,  0 },
	{ ICO_FULLSCREEN,     "With \"fullscreen\" overlay", SECT_POPUP,                IDI_PU_FULLSCREEN,  NULL,  0 },
	{ ICO_HISTORY,        "Popup History",               SECT_POPUP,                IDI_HISTORY,        NULL, -1 },

	//option
	{ ICO_OPT_RELOAD,     "Refresh skin list",           SECT_POPUP SECT_POPUP_OPT, IDI_RELOAD,         NULL,  0 },
	{ ICO_OPT_RESIZE,     "Popup Placement",             SECT_POPUP SECT_POPUP_OPT, IDI_RESIZE,         NULL,  0 },
	{ ICO_OPT_OK,         "OK",                          SECT_POPUP SECT_POPUP_OPT, IDI_ACT_OK,         NULL,  0 },
	{ ICO_OPT_CANCEL,     "Cancel",                      SECT_POPUP SECT_POPUP_OPT, IDI_ACT_CLOSE,      NULL,  0 },
	{ ICO_OPT_GROUP,      "Popup Group",                 SECT_POPUP SECT_POPUP_OPT, IDI_OPT_GROUP,      NULL,  0 },
	{ ICO_OPT_DEF,        "Show default",                SECT_POPUP SECT_POPUP_OPT, IDI_ACT_OK,         NULL,  0 },
	{ ICO_OPT_FAV,        "Favorite Contact",            SECT_POPUP SECT_POPUP_OPT, IDI_OPT_FAVORITE,   NULL,  0 },
	{ ICO_OPT_FULLSCREEN, "Show in Fullscreen",          SECT_POPUP SECT_POPUP_OPT, IDI_OPT_FULLSCREEN, NULL,  0 },
	{ ICO_OPT_BLOCK,      "Blocked Contact",             SECT_POPUP SECT_POPUP_OPT, IDI_OPT_BLOCK,      NULL,  0 },

	//action
	{ ICO_ACT_REPLY,      "Quick Reply",                 SECT_POPUP SECT_POPUP_ACT, IDI_ACT_REPLY,      NULL, -1 },
	{ ICO_ACT_PIN,        "Pin Popup",                   SECT_POPUP SECT_POPUP_ACT, IDI_ACT_PIN,        NULL, -1 },
	{ ICO_ACT_PINNED,     "Pinned Popup",                SECT_POPUP SECT_POPUP_ACT, IDI_ACT_PINNED,     NULL, -1 },
	{ ICO_ACT_MESS,       "Send Message",                SECT_POPUP SECT_POPUP_ACT, IDI_ACT_MESSAGE,    NULL, -1 },
	{ ICO_ACT_INFO,       "User Details",                SECT_POPUP SECT_POPUP_ACT, IDI_ACT_INFO,       NULL, -1 },
	{ ICO_ACT_MENU,       "Contact Menu",                SECT_POPUP SECT_POPUP_ACT, IDI_ACT_MENU,       NULL, -1 },
	{ ICO_ACT_ADD,        "Add Contact Permanently",     SECT_POPUP SECT_POPUP_ACT, IDI_ACT_ADD,        NULL, -1 },
	{ ICO_ACT_CLOSE,      "Dismiss Popup",               SECT_POPUP SECT_POPUP_ACT, IDI_ACT_CLOSE,      NULL, -1 },
	{ ICO_ACT_COPY,       "Copy to clipboard",           SECT_POPUP SECT_POPUP_ACT, IDI_ACT_COPY,       NULL, -1 }

};

/**
 * Returns a icon, identified by a name
 * @param	pszIcon		- name of the icon
 * @param	big			- bool big icon (default = false)
 * @return:	HICON if the icon is loaded, NULL otherwise
 **/
HICON IcoLib_GetIcon(LPCSTR pszIcon, bool big)
{
	return (pszIcon) ? Skin_GetIcon(pszIcon, big) : NULL;
}

void InitIcons()
{
	TCHAR selfDLL[MAX_PATH];
	GetModuleFileName(hInst, selfDLL, SIZEOF(selfDLL));

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_PATH_TCHAR;
	sid.ptszDefaultFile = selfDLL;

	for(int i=0; i < SIZEOF(icoDesc); i++) {
		sid.pszName = icoDesc[i].pszName;
		sid.pszDescription = icoDesc[i].ptszDesc;		// [TRANSLATED-BY-CORE]
		sid.pszSection = icoDesc[i].ptszSection;		//must be always untranslatet !!!!!
		sid.iDefaultIndex = -icoDesc[i].idResource;

		switch (icoDesc[i].size){
		// small and big icons
		case -1:
			sid.cx = sid.cy = 0;
			break;

		// small icons (16x16)
		case 0:
			sid.cx = sid.cy = 16;
			break;

		// normal icons (32x32)
		case 1:
			sid.cx = sid.cy = 32;
			break;

		// custom icon size
		default:
			sid.cx = sid.cy = icoDesc[i].size;
			break;
		}
		Skin_AddIcon(&sid);
	}
}
