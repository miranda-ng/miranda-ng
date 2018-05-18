#include "stdafx.h"

static IconItem iconList1[] =
{
	// toolbar
	{ LPGEN("Popups are enabled"),          ICO_TB_POPUP_ON,    IDI_POPUP          },
	{ LPGEN("Popups are disabled"),         ICO_TB_POPUP_OFF,   IDI_NOPOPUP        }
};

static IconItem iconList2[] =
{
	// common
	{ LPGEN("Popups are enabled"),          ICO_POPUP_ON,       IDI_POPUP          },
	{ LPGEN("Popups are disabled"),         ICO_POPUP_OFF,      IDI_NOPOPUP        },
	{ LPGEN("Popup History"),               ICO_HISTORY,        IDI_HISTORY        },
};

void InitIcons()
{
	g_plugin.registerIcon(SECT_TOLBAR, iconList1);
	g_plugin.registerIcon(SECT_POPUP,  iconList2);
}
