#include "stdafx.h"
#include "icons.h"
TCHAR g_dllpath[1024];

struct
{
	const TCHAR* stzDescr;
	const char* szName;
	int   defIconID;
}
static iconList[] = {
	{	_T(LANG_ICON_OTR),			ICON_OTR,			IDI_OTR			},
	{	_T(LANG_ICON_PRIVATE),		ICON_PRIVATE,		IDI_PRIVATE		},
	{	_T(LANG_ICON_UNVERIFIED),	ICON_UNVERIFIED,	IDI_UNVERIFIED	},
	{	_T(LANG_ICON_FINISHED),		ICON_FINISHED,		IDI_FINISHED	},
	{	_T(LANG_ICON_NOT_PRIVATE),	ICON_NOT_PRIVATE,	IDI_INSECURE	}
	
};

HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons() {
	TCHAR szFile[MAX_PATH+500];

	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.ptszSection = _T(MODULENAME);
	sid.flags = SIDF_ALL_TCHAR;

	for ( int i = 0; i < SIZEOF(iconList); i++ ) {
		sid.pszName = (char*)iconList[i].szName;
		sid.ptszDescription = (TCHAR*)iconList[i].stzDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}

}

void DeinitIcons() {
	for ( int i = 0; i < SIZEOF(iconList); i++ ) {
		(HANDLE)CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)iconList[i].szName);
		hIconLibItem[i] = NULL;
	}
}

HICON LoadIcon(const char* name, int big) {
	return ( HICON )CallService( MS_SKIN2_GETICON, big, (LPARAM)name );
}

HANDLE GetIconHandle(const char* name) {
	return ( HANDLE )CallService( MS_SKIN2_GETICONHANDLE, 0, (LPARAM)name );
}

void ReleaseIcon(const char* name, int big) {
	CallService( big ? MS_SKIN2_RELEASEICONBIG : MS_SKIN2_RELEASEICON, 0, (LPARAM)name );
}
void ReleaseIcon(HICON handle, int big) {
	CallService( big ? MS_SKIN2_RELEASEICONBIG : MS_SKIN2_RELEASEICON, (WPARAM)handle, 0 );
}
