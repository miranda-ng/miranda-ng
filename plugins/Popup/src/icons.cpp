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

typedef struct _ICODESC 
{
	LPSTR	pszName;
	LPTSTR	ptszDesc;
	LPTSTR	ptszSection;
	BOOL	bfromIconPack;
	WORD	idResource;
	LPSTR	pszIcon;
	int	size;
} ICODESC;

static ICODESC icoDesc[] = 
{
	//toolbar
	{ ICO_TB_POPUP_ON,		_T("Popups are enabled"),			_T(SECT_TOLBAR),					0,	IDI_POPUP,			NULL,			0	},
	{ ICO_TB_POPUP_OFF,		_T("Popups are disabled"),			_T(SECT_TOLBAR),					0,	IDI_NOPOPUP,		NULL,			0	},
	//common popup
	{ ICO_POPUP_ON,			_T("Popups are enabled"),			_T(SECT_POPUP),						0,	IDI_POPUP,			NULL,			0	},
	{ ICO_POPUP_OFF,		_T("Popups are disabled"),			_T(SECT_POPUP),						0,	IDI_NOPOPUP,		NULL,			0	},
	{ ICO_FAV,				_T("With \"favourite\" overlay"),	_T(SECT_POPUP),						0,	IDI_PU_FAVOURITE,	NULL,			0	},
	{ ICO_FULLSCREEN,		_T("With \"fullscreen\" overlay"),	_T(SECT_POPUP),						0,	IDI_PU_FULLSCREEN,	NULL,			0	},
	{ ICO_HISTORY,			_T("Popup History"),				_T(SECT_POPUP),						0,	IDI_HISTORY,		NULL,			-1	},
	//misc (register is done inside notification service)
	//{ ICO_MISC_NOTIFY,	_T("Notification"),					_T(SECT_POPUP) _T(SECT_POPUP_MISC),	0,	IDI_MB_INFO,		NULL,			0	},
	//{ ICO_MISC_WARNING,	_T("Warning"),						_T(SECT_POPUP) _T(SECT_POPUP_MISC),	0,	IDI_MB_WARN,		NULL,			0	},
	//{ ICO_MISC_ERROR,		_T("Error"),						_T(SECT_POPUP) _T(SECT_POPUP_MISC),	0,	IDI_MB_STOP,		NULL,			0	},
	//option
	{ ICO_OPT_RELOAD,		_T("Refresh skin list"),			_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_RELOAD,			NULL,			0	},
	{ ICO_OPT_RESIZE,		_T("Popup Placement"),				_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_RESIZE,			NULL,			0	},
	{ ICO_OPT_OK,			_T("OK"),							_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_ACT_OK,			NULL,			0	},
	{ ICO_OPT_CANCEL,		_T("Cancel"),						_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_ACT_CLOSE,		NULL,			0	},
	{ ICO_OPT_GROUP,		_T("Popup Group"),					_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_OPT_GROUP,		NULL,			0	},
	{ ICO_OPT_DEF,			_T("Show default"),					_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_ACT_OK,			NULL,			0	},
	{ ICO_OPT_FAV,			_T("Favorite Contact"),				_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_OPT_FAVORITE,	NULL,			0	},
	{ ICO_OPT_FULLSCREEN,	_T("Show in Fullscreen"),			_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_OPT_FULLSCREEN,	NULL,			0	},
	{ ICO_OPT_BLOCK,		_T("Blocked Contact"),				_T(SECT_POPUP) _T(SECT_POPUP_OPT),	0,	IDI_OPT_BLOCK,		NULL,			0	},
	//action
	{ ICO_ACT_REPLY,		_T("Quick Reply"),					_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_REPLY,		NULL,			-1	},
	{ ICO_ACT_PIN,			_T("Pin Popup"),					_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_PIN,		NULL,			-1	},
	{ ICO_ACT_PINNED,		_T("Pinned Popup"),					_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_PINNED,		NULL,			-1	},
	{ ICO_ACT_MESS,			_T("Send Message"),					_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_MESSAGE,	NULL,			-1	},
	{ ICO_ACT_INFO,			_T("User Details"),					_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_INFO,		NULL,			-1	},
	{ ICO_ACT_MENU,			_T("Contact Menu"),					_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_MENU,		NULL,			-1	},
	{ ICO_ACT_ADD,			_T("Add Contact Permanently"),		_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_ADD,		NULL,			-1	},
	{ ICO_ACT_CLOSE,		_T("Dismiss Popup"),				_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_CLOSE,		NULL,			-1	},
	{ ICO_ACT_COPY,			_T("Copy to clipboard"),			_T(SECT_POPUP) _T(SECT_POPUP_ACT),	0,	IDI_ACT_COPY,		NULL,			-1	}

};

/**
 * Returns a icon, identified by a name
 * @param	pszIcon		- name of the icon
 * @param	big			- bool big icon (default = false)
 * @return:	HICON if the icon is loaded, NULL otherwise
 **/
HICON IcoLib_GetIcon(LPCSTR pszIcon, bool big)
{
	return (pszIcon) ? (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)big, (LPARAM) pszIcon) : NULL;
}

void InitIcons()
{
	SKINICONDESC sid;
	ZeroMemory(&sid, sizeof(sid));
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;
	TCHAR selfDLL[1024];
	GetModuleFileName(hInst, selfDLL, 1024);

	for(int i = 0; i < SIZEOF(icoDesc); i++){
		sid.pszName = icoDesc[i].pszName;
		sid.ptszDescription = icoDesc[i].ptszDesc;		// [TRANSLATED-BY-CORE]
		sid.ptszSection = icoDesc[i].ptszSection;		//must be always untranslatet !!!!!

		if(icoDesc[i].idResource==0){
			//use icon from icon lib
			sid.hDefaultIcon = (HICON)CallService(MS_SKIN2_GETICON,0 , (LPARAM)icoDesc[i].pszIcon);
			sid.ptszDefaultFile = NULL;
			sid.iDefaultIndex = 0;
		}else{
			//load and register from popup.dll
			sid.hDefaultIcon = 0;
			sid.ptszDefaultFile = selfDLL;
			sid.iDefaultIndex = -icoDesc[i].idResource;
		}

		switch (icoDesc[i].size){
		// small and big icons
		case -1:{
				sid.cx = sid.cy = 0;
				break;
			}
		// small icons (16x16)
		case 0:{
				sid.cx = sid.cy = 16;
				break;
			}
		// normal icons (32x32)
		case 1:{
				sid.cx = sid.cy = 32;
				break;
			}
		// custom icon size
		default:{
				sid.cx = sid.cy = icoDesc[i].size;
				break;
			}
		}
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}