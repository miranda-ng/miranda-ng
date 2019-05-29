#include "stdafx.h"

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"),   "main", IDI_TOX  },
	{ LPGEN("Action icon"),     "main", IDI_ME   },
	{ LPGEN("Correction icon"), "edit",	IDI_EDIT },
	{ LPGEN("Copy ID"),         "copy", IDI_COPY },
};

void CToxProto::InitIcons()
{
	g_plugin.registerIcon(LPGEN("Protocols") "/" MODULE, iconList, MODULE);
}
