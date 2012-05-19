#include "common.h"
#include "icons.h"

typedef struct
{
	char* szDescr;
	char* szName;
	int   defIconID;
} IconStruct;

static const IconStruct iconList[] =
{
	{ "Check for Plugin Updates",	"updater_check",      IDI_MAINMENU       },
	{ "Restart",					"updater_restart",    IDI_RESTART        },
	{ "Update and Exit",			"updater_checkexit",  IDI_UPDATEANDEXIT  },
};

HANDLE hIcolibIcon[SIZEOF(iconList)];


HICON LoadIconEx(IconIndex i, bool big)
{
	return (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, big, (LPARAM)hIcolibIcon[(int)i]);
} 

HANDLE GetIconHandle(IconIndex i)
{
	return hIcolibIcon[(int)i];
}

void ReleaseIconEx(HICON hIcon)
{
	CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}


void InitIcons(void) 
{
	SKINICONDESC sid = {0};
	TCHAR path[MAX_PATH];
	int i;
	
	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszSection = MODULE;
	sid.flags = SIDF_PATH_TCHAR;
	sid.ptszDefaultFile = path;
	GetModuleFileName(hInst, path, sizeof(path));

	for (i = 0; i < SIZEOF(iconList); ++i)
	{
		sid.pszDescription = iconList[i].szDescr;
		sid.pszName = iconList[i].szName;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIcolibIcon[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}
