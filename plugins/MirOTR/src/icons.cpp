#include "stdafx.h"

IconItem iconList[ICON_MAXID] =
{
	{ LPGEN("OTR"),            "otr",         IDI_OTR        },
	{ LPGEN("OTR private"),    "private",     IDI_PRIVATE    },
	{ LPGEN("OTR unverified"), "unverified",  IDI_UNVERIFIED },
	{ LPGEN("OTR finished"),   "finished",    IDI_FINISHED   },
	{ LPGEN("OTR not secure"), "not_private", IDI_INSECURE   },
	{ LPGEN("Refresh"),        "refresh",     IDI_REFRESH    }
};

void InitIcons()
{
	g_plugin.registerIcon("OTR", iconList, MODULENAME);
}
