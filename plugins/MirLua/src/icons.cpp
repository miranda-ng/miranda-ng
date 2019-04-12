#include "stdafx.h"

static IconItem Icons[] =
{
	{ LPGEN("Script"),  "script",  IDI_SCRIPT  },
	{ LPGEN("Loaded"),  "loaded",  IDI_LOADED  },
	{ LPGEN("Failed"),  "failed",  IDI_FAILED  },
	{ LPGEN("Open"),    "open",    IDI_OPEN    },
	{ LPGEN("Reload"),  "reload",  IDI_RELOAD  },
	{ LPGEN("Compile"), "compile", IDI_COMPILE },
};

void LoadIcons()
{
	g_plugin.registerIcon(MODULENAME, Icons, MODULENAME);
}
