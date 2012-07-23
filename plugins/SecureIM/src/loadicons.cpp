#include "commonheaders.h"


HINSTANCE LoadIconsPack(const char* szIconsPack)
{
	HINSTANCE hNewIconInst = NULL;
	WORD i;

	hNewIconInst = LoadLibrary(szIconsPack);

	if (hNewIconInst != NULL)
	{
		for(i=ID_FIRSTICON; i<=ID_LASTICON; i++)
			if (LoadIcon(hNewIconInst, MAKEINTRESOURCE(i)) == NULL)
			{
				FreeLibrary(hNewIconInst);
				hNewIconInst = NULL;
				break;
			}
	}
	return hNewIconInst;
}



int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;
	for (int i=0; icons[i].key; i++) {
		hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)icons[i].name);
		if(icons[i].tbl == TBL_IEC)
			g_hIEC[icons[i].idx]=hIcon;
		else
		if(icons[i].tbl == TBL_ICO)
			g_hICO[icons[i].idx]=hIcon;
		else
		if(icons[i].tbl == TBL_POP)
			g_hPOP[icons[i].idx]=hIcon;
	}

	return 0;
}


void InitIcons(void)
{
	HINSTANCE hNewIconInst = NULL;

	if ( g_hFolders ) {
		LPSTR pathname = (LPSTR) alloca(MAX_PATH);
		FoldersGetCustomPathEx(g_hFolders, pathname, MAX_PATH, "icons\\", "secureim_icons.dll");
		if (hNewIconInst == NULL)
			hNewIconInst = LoadIconsPack(pathname);
	}

	if (hNewIconInst == NULL)
		hNewIconInst = LoadIconsPack("icons\\secureim_icons.dll");

	if (hNewIconInst == NULL)
		hNewIconInst = LoadIconsPack("plugins\\secureim_icons.dll");

	if (hNewIconInst == NULL)
		g_hIconInst = g_hInst;
	else
		g_hIconInst = hNewIconInst;


	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof(sid);
	sid.pszSection = "SecureIM";

	HICON hIcon;
	for (int i=0; icons[i].key; i++) {
		sid.pszSection = icons[i].section;
		sid.pszName = icons[i].name;
		sid.pszDescription = icons[i].text;
		sid.pszDefaultFile = "secureim_icons.dll";
		sid.iDefaultIndex = icons[i].key;
		sid.hDefaultIcon = (HICON)LoadImage(g_hIconInst, MAKEINTRESOURCE(icons[i].key), IMAGE_ICON, 16, 16, LR_SHARED);
		Skin_AddIcon(&sid);
		hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)icons[i].name);

		if(icons[i].tbl == TBL_IEC)
			g_hIEC[icons[i].idx]=hIcon;
		else
		if(icons[i].tbl == TBL_ICO)
			g_hICO[icons[i].idx]=hIcon;
		else
		if(icons[i].tbl == TBL_POP)
			g_hPOP[icons[i].idx]=hIcon;
	}

	AddHookFunction(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}

// EOF
