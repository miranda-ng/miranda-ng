#include "AvatarHistory.h"

enum IconIndex
{
	I_HISTORY,
	I_OVERLAY
};

typedef struct
{
	TCHAR* szDescr;
	char* szName;
	int   defIconID;
	BOOL  core;
} IconStruct;

static IconStruct iconList[] =
{
	{ LPGENT("History"),		"core_main_10",	IDI_AVATARHIST,		TRUE  },
	{ LPGENT("Avatar Overlay"),	"avh_overlay",	IDI_AVATAROVERLAY,	FALSE  }
};

extern HANDLE hHooks[];

static HICON LoadIconEx(IconIndex i)
{
	HICON hIcon;

	if (hHooks[4])
		hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)iconList[(int)i].szName);
	else
		hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(iconList[(int)i].defIconID), 
			IMAGE_ICON, 0, 0, 0);

	return hIcon;
}


static void ReleaseIconEx(HICON hIcon)
{
	if (hHooks[4])
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
	else
		DestroyIcon(hIcon);
}

static void IcoLibUpdateMenus()
{
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = createDefaultOverlayedIcon(FALSE);
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenu, ( LPARAM )&mi );
	DestroyIcon(mi.hIcon);
}

int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam)
{
	IcoLibUpdateMenus();
	return 0;
}

void SetupIcoLib()
{
	if (hHooks[4])
	{
		SKINICONDESC sid = {0};
		TCHAR path[MAX_PATH];

   		GetModuleFileName(hInst, path, sizeof(path));

		sid.cbSize = SKINICONDESC_SIZE;
		sid.ptszSection = LPGENT("Avatar History");
		sid.ptszDefaultFile = path;
		sid.flags = SIDF_ALL_TCHAR;

		for (unsigned i = 0; i < MAX_REGS(iconList); i++) 
		{
			if (!iconList[i].core)
			{
				sid.ptszDescription = iconList[i].szDescr;
				sid.pszName = iconList[i].szName;
				sid.iDefaultIndex = -iconList[i].defIconID;
				CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			}
		}
	}
	IcoLibUpdateMenus();
}

static HICON getOverlayedIcon(HICON icon, HICON overlay, BOOL big)
{
	HIMAGELIST il = ImageList_Create(
		GetSystemMetrics(big?SM_CXICON:SM_CXSMICON),
		GetSystemMetrics(big?SM_CYICON:SM_CYSMICON),
		ILC_COLOR32|ILC_MASK, 2, 2);
	ImageList_AddIcon(il, icon);
	ImageList_AddIcon(il, overlay);
	HIMAGELIST newImage = ImageList_Merge(il,0,il,1,0,0);
	ImageList_Destroy(il);
	HICON hIcon = ImageList_GetIcon(newImage, 0, 0);
	ImageList_Destroy(newImage);
	return hIcon; // the result should be destroyed by DestroyIcon()
}


HICON createDefaultOverlayedIcon(BOOL big)
{
	HICON icon0 = LoadIconEx(I_HISTORY);
	HICON icon1 = LoadIconEx(I_OVERLAY);

	HICON resIcon = getOverlayedIcon(icon0, icon1, FALSE);

	ReleaseIconEx(icon0);
	ReleaseIconEx(icon1);

	return resIcon;
}


HICON createProtoOverlayedIcon(HANDLE hContact)
{
	HICON icon1 = LoadIconEx(I_OVERLAY);

	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	HICON icon0 = LoadSkinnedProtoIcon(szProto, ID_STATUS_ONLINE);

	HICON resIcon = getOverlayedIcon(icon0, icon1, FALSE);

	ReleaseIconEx(icon1);
	CallService(MS_SKIN2_RELEASEICON, (WPARAM)icon0, 0);

	return resIcon;
}
