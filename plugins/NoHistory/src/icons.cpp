#include "stdafx.h"

static IconItem iconList[] = 
{
	{ LPGEN("Disable"), "remove", IDI_HREMOVE },
	{ LPGEN("Enable"),  "keep",   IDI_HKEEP   },
	{ LPGEN("Clear"),   "clear",  IDI_HCLEAR  },
};

void InitIcons()
{
	g_plugin.registerIcon(LPGEN("No History"), iconList, MODULENAME);
}
