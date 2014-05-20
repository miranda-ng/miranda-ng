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
*/

#include "headers.h"

static IconItem iconList[] =
{
	//toolbar
	{ LPGEN("Popups are enabled"),          ICO_TB_POPUP_ON,    IDI_POPUP          },
	{ LPGEN("Popups are disabled"),         ICO_TB_POPUP_OFF,   IDI_NOPOPUP        },

	//                               common popup
	{ LPGEN("Popups are enabled"),          ICO_POPUP_ON,       IDI_POPUP          },
	{ LPGEN("Popups are disabled"),         ICO_POPUP_OFF,      IDI_NOPOPUP        },
	{ LPGEN("With \"favorite\" overlay"),   ICO_FAV,            IDI_PU_FAVOURITE   },
	{ LPGEN("With \"fullscreen\" overlay"), ICO_FULLSCREEN,     IDI_PU_FULLSCREEN  },
	{ LPGEN("Popup History"),               ICO_HISTORY,        IDI_HISTORY        },

	//                               option
	{ LPGEN("Refresh skin list"),           ICO_OPT_RELOAD,     IDI_RELOAD         },
	{ LPGEN("Popup placement"),             ICO_OPT_RESIZE,     IDI_RESIZE         },
	{ LPGEN("OK"),                          ICO_OPT_OK,         IDI_ACT_OK         },
	{ LPGEN("Cancel"),                      ICO_OPT_CANCEL,     IDI_ACT_CLOSE      },
	{ LPGEN("Popup group"),                 ICO_OPT_GROUP,      IDI_OPT_GROUP      },
	{ LPGEN("Show default"),                ICO_OPT_DEF,        IDI_ACT_OK         },
	{ LPGEN("Favorite contact"),            ICO_OPT_FAV,        IDI_OPT_FAVORITE   },
	{ LPGEN("Show in fullscreen"),          ICO_OPT_FULLSCREEN, IDI_OPT_FULLSCREEN },
	{ LPGEN("Blocked contact"),             ICO_OPT_BLOCK,      IDI_OPT_BLOCK      },

	//                               action
	{ LPGEN("Quick reply"),                 ICO_ACT_REPLY,      IDI_ACT_REPLY      },
	{ LPGEN("Pin popup"),                   ICO_ACT_PIN,        IDI_ACT_PIN        },
	{ LPGEN("Pinned popup"),                ICO_ACT_PINNED,     IDI_ACT_PINNED     },
	{ LPGEN("Send message"),                ICO_ACT_MESS,       IDI_ACT_MESSAGE    },
	{ LPGEN("User details"),                ICO_ACT_INFO,       IDI_ACT_INFO       },
	{ LPGEN("Contact menu"),                ICO_ACT_MENU,       IDI_ACT_MENU       },
	{ LPGEN("Add contact permanently"),     ICO_ACT_ADD,        IDI_ACT_ADD        },
	{ LPGEN("Dismiss popup"),               ICO_ACT_CLOSE,      IDI_ACT_CLOSE      },
	{ LPGEN("Copy to clipboard"),           ICO_ACT_COPY,       IDI_ACT_COPY       }

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
	Icon_Register(hInst, SECT_POPUP, iconList + 2, 5);
	Icon_Register(hInst, SECT_POPUP SECT_POPUP_OPT, iconList + 7, 9);
	Icon_Register(hInst, SECT_POPUP SECT_POPUP_ACT, iconList + 16, 9);
}
