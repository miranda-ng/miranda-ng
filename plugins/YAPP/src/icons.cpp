#include "stdafx.h"

static IconItem iconList[] =
{
	// toolbar
	{ LPGEN("Popups are enabled"),          ICO_TB_POPUP_ON,    IDI_POPUP          },
	{ LPGEN("Popups are disabled"),         ICO_TB_POPUP_OFF,   IDI_NOPOPUP        },

	// common
	{ LPGEN("Popups are enabled"),          ICO_POPUP_ON,       IDI_POPUP          },
	{ LPGEN("Popups are disabled"),         ICO_POPUP_OFF,      IDI_NOPOPUP        },
	{ LPGEN("Popup History"),               ICO_HISTORY,        IDI_HISTORY        },
};

void InitIcons()
{
	Icon_Register(hInst, SECT_TOLBAR, iconList, 2);
	Icon_Register(hInst, SECT_POPUP,  iconList+2, 3);
}
