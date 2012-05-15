#include "Mra.h"
#include "MraIcons.h"




void AddIcoLibItems(LPWSTR lpwszRootSectionName,LPWSTR lpwszSubSectionName,const GUI_DISPLAY_ITEM *pgdiItems,SIZE_T dwCount,HANDLE *hResult)
{
	char szBuff[MAX_PATH];
	WCHAR wszSection[MAX_PATH],wszPath[MAX_FILEPATH];
	SKINICONDESC sid={0};


	sid.cbSize=SKINICONDESC_SIZE;
	sid.pwszSection=wszSection;
	sid.pwszDefaultFile=wszPath;
	sid.cx=sid.cy=16;
	sid.flags=SIDF_ALL_UNICODE;

	if (lpwszSubSectionName==NULL) lpwszSubSectionName=L"";
	GetModuleFileName(masMraSettings.hInstance,wszPath,SIZEOF(wszPath));
	mir_sntprintf(wszSection,SIZEOF(wszSection),L"%s%s%s",lpwszRootSectionName,PROTOCOL_NAMEW,lpwszSubSectionName);

	for (SIZE_T i=0;i<dwCount;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s_%s",PROTOCOL_NAMEA,pgdiItems[i].lpszName);
		sid.pszName=szBuff;
		sid.pwszDescription=pgdiItems[i].lpwszDescr;
		sid.iDefaultIndex=-pgdiItems[i].defIcon;
		sid.hDefaultIcon=(HICON)LoadImage(masMraSettings.hInstance,MAKEINTRESOURCE(pgdiItems[i].defIcon),IMAGE_ICON,0,0,LR_SHARED);
		if (sid.hDefaultIcon==NULL) sid.hDefaultIcon=(HICON)LoadImage(NULL,MAKEINTRESOURCE(pgdiItems[i].defIcon),IMAGE_ICON,0,0,LR_SHARED);
		hResult[i]=(HANDLE)CallService(MS_SKIN2_ADDICON,0,(LPARAM)&sid);
	}
}

void RemoveIcoLibItems(const GUI_DISPLAY_ITEM *pgdiItems,SIZE_T dwCount)
{
	char szBuff[MAX_PATH];

	for (SIZE_T i=0;i<dwCount;i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s_%s",PROTOCOL_NAMEA,pgdiItems[i].lpszName);
		//***deb until 08 release
		//CallService(MS_SKIN2_REMOVEICON,0,(LPARAM)szBuff);
	}
}


HICON IconLibGetIcon(HANDLE hIcon)
{
return(IconLibGetIconEx(hIcon,LR_SHARED));
}


HICON IconLibGetIconEx(HANDLE hIcon,DWORD dwFlags)
{
	HICON hiIcon=NULL;

	if (hIcon)
	{
		hiIcon=(HICON)CallService(MS_SKIN2_GETICONBYHANDLE,0,(LPARAM)hIcon);
		if ((dwFlags&LR_SHARED)==0)
		{
			hiIcon=CopyIcon(hiIcon);
		}
	}
return(hiIcon);
}


//////////////////////////////////////////////////////////////////////////////////////
void IconsLoad()
{
	AddIcoLibItems(L"Protocols/",L"/MainMenu",gdiMenuItems,SIZEOF(gdiMenuItems),masMraSettings.hMainMenuIcons);
	AddIcoLibItems(L"Protocols/",L"/ContactMenu",gdiContactMenuItems,SIZEOF(gdiContactMenuItems),masMraSettings.hContactMenuIcons);
	// Advanced Status Icons initialization
	AddIcoLibItems(L"Protocols/",L"/Extra status",gdiExtraStatusIconsItems,SIZEOF(gdiExtraStatusIconsItems),masMraSettings.hAdvancedStatusIcons);
}


void IconsUnLoad()
{
	RemoveIcoLibItems(gdiMenuItems,SIZEOF(gdiMenuItems));
	memset(masMraSettings.hMainMenuIcons, 0, sizeof(masMraSettings.hMainMenuIcons));
	
	RemoveIcoLibItems(gdiContactMenuItems,SIZEOF(gdiContactMenuItems));
	memset(masMraSettings.hContactMenuIcons, 0, sizeof(masMraSettings.hContactMenuIcons));
	
	// Advanced Status Icons initialization
	RemoveIcoLibItems(gdiExtraStatusIconsItems,SIZEOF(gdiExtraStatusIconsItems));
	memset(masMraSettings.hAdvancedStatusIcons, 0, sizeof(masMraSettings.hAdvancedStatusIcons));
	memset(masMraSettings.hAdvancedStatusItems,0, sizeof(masMraSettings.hAdvancedStatusItems));
}




void InitXStatusIcons()
{
	int iCurIndex;
	char szBuff[MAX_PATH];
	WCHAR wszSection[MAX_PATH],wszPath[MAX_FILEPATH];
	SKINICONDESC sid={0};

	sid.cbSize=SKINICONDESC_SIZE;
	sid.pwszSection=wszSection;
	sid.pwszDefaultFile=wszPath;
	sid.cx=sid.cy=16;
	sid.flags=SIDF_ALL_UNICODE;

	if (masMraSettings.hDLLXStatusIcons)
	{
		GetModuleFileName(masMraSettings.hDLLXStatusIcons,wszPath,SIZEOF(wszPath));
	}else{
		memset(wszPath, 0, sizeof(wszPath));
	}
	mir_sntprintf(wszSection,SIZEOF(wszSection),L"Status Icons/%s/Custom Status",PROTOCOL_NAMEW);

	masMraSettings.hXStatusAdvancedStatusIcons[0]=NULL;
	for (SIZE_T i=1;i<(MRA_XSTATUS_COUNT+1);i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"%s_xstatus%ld",PROTOCOL_NAMEA,i);
		iCurIndex=(IDI_XSTATUS1-1+i);
		sid.pszName=szBuff;
		sid.pwszDescription=lpcszXStatusNameDef[i];
		sid.iDefaultIndex=-iCurIndex;
		if (masMraSettings.hDLLXStatusIcons)
		{
			sid.hDefaultIcon=(HICON)LoadImage(masMraSettings.hDLLXStatusIcons,MAKEINTRESOURCE(iCurIndex),IMAGE_ICON,0,0,LR_SHARED);
		}else{
			sid.hDefaultIcon=NULL;
		}
		masMraSettings.hXStatusAdvancedStatusIcons[i]=(HANDLE)CallService(MS_SKIN2_ADDICON,0,(LPARAM)&sid);
	}
}

void DestroyXStatusIcons()
{
	char szBuff[MAX_PATH];

	for (SIZE_T i=1;i<(MRA_XSTATUS_COUNT+1);i++)
	{
		mir_snprintf(szBuff,SIZEOF(szBuff),"xstatus%ld",i);
		//***deb until 08 release
		//CallService(MS_SKIN2_REMOVEICON,0,(LPARAM)szBuff);
	}
	memset(masMraSettings.hXStatusAdvancedStatusIcons, 0, sizeof(masMraSettings.hXStatusAdvancedStatusIcons));
	memset(masMraSettings.hXStatusAdvancedStatusItems, 0, sizeof(masMraSettings.hXStatusAdvancedStatusItems));
}


