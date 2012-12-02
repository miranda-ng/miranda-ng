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

static IconItem iconList[] = 
{
	//toolbar
	{ ICO_TB_POPUP_ON,    "Popups are enabled",          IDI_POPUP          },
	{ ICO_TB_POPUP_OFF,   "Popups are disabled",         IDI_NOPOPUP        },

	//common popup
	{ ICO_POPUP_ON,       "Popups are enabled",          IDI_POPUP          },
	{ ICO_POPUP_OFF,      "Popups are disabled",         IDI_NOPOPUP        },
	{ ICO_FAV,            "With \"favourite\" overlay",  IDI_PU_FAVOURITE   },
	{ ICO_FULLSCREEN,     "With \"fullscreen\" overlay", IDI_PU_FULLSCREEN  },
	{ ICO_HISTORY,        "Popup History",               IDI_HISTORY        },

	//option
	{ ICO_OPT_RELOAD,     "Refresh skin list",           IDI_RELOAD         },
	{ ICO_OPT_RESIZE,     "Popup Placement",             IDI_RESIZE         },
	{ ICO_OPT_OK,         "OK",                          IDI_ACT_OK         },
	{ ICO_OPT_CANCEL,     "Cancel",                      IDI_ACT_CLOSE      },
	{ ICO_OPT_GROUP,      "Popup Group",                 IDI_OPT_GROUP      },
	{ ICO_OPT_DEF,        "Show default",                IDI_ACT_OK         },
	{ ICO_OPT_FAV,        "Favorite Contact",            IDI_OPT_FAVORITE   },
	{ ICO_OPT_FULLSCREEN, "Show in Fullscreen",          IDI_OPT_FULLSCREEN },
	{ ICO_OPT_BLOCK,      "Blocked Contact",             IDI_OPT_BLOCK      },

	//action
	{ ICO_ACT_REPLY,      "Quick Reply",                 IDI_ACT_REPLY      },
	{ ICO_ACT_PIN,        "Pin Popup",                   IDI_ACT_PIN        },
	{ ICO_ACT_PINNED,     "Pinned Popup",                IDI_ACT_PINNED     },
	{ ICO_ACT_MESS,       "Send Message",                IDI_ACT_MESSAGE    },
	{ ICO_ACT_INFO,       "User Details",                IDI_ACT_INFO       },
	{ ICO_ACT_MENU,       "Contact Menu",                IDI_ACT_MENU       },
	{ ICO_ACT_ADD,        "Add Contact Permanently",     IDI_ACT_ADD        },
	{ ICO_ACT_CLOSE,      "Dismiss Popup",               IDI_ACT_CLOSE      },
	{ ICO_ACT_COPY,       "Copy to clipboard",           IDI_ACT_COPY       }

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
	Icon_Register(hInst, SECT_TOLBAR, iconList, 2);
	Icon_Register(hInst, SECT_POPUP,  iconList+2, 5);
	Icon_Register(hInst, SECT_POPUP SECT_POPUP_OPT,  iconList+7, 9);
	Icon_Register(hInst, SECT_POPUP SECT_POPUP_ACT,  iconList+16, 9);
}
