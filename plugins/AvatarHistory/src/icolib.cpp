#include "stdafx.h"

enum IconIndex
{
	I_HISTORY,
	I_OVERLAY
};

static IconItem iconList[] =
{
	{ LPGEN("History"),        "",             IDI_AVATARHIST    },
	{ LPGEN("Avatar Overlay"),	"avh_overlay",  IDI_AVATAROVERLAY }
};

static HICON LoadIconEx(IconIndex i)
{
	return Skin_GetIconByHandle(iconList[(int)i].hIcolib);
}

static void ReleaseIconEx(HICON hIcon)
{
	Skin_ReleaseIcon(hIcon);
}

static void IcoLibUpdateMenus()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIM_ICON;
	mi.hIcon = createDefaultOverlayedIcon(FALSE);
	Menu_ModifyItem(hMenu, &mi);
	DestroyIcon(mi.hIcon);
}

int IcoLibIconsChanged(WPARAM, LPARAM)
{
	IcoLibUpdateMenus();
	return 0;
}

void SetupIcoLib()
{
	iconList[0].hIcolib = LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
	Icon_Register(hInst, LPGEN("Avatar History"), iconList+1, SIZEOF(iconList)-1);
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


HICON createDefaultOverlayedIcon(BOOL)
{
	HICON icon0 = LoadIconEx(I_HISTORY);
	HICON icon1 = LoadIconEx(I_OVERLAY);

	HICON resIcon = getOverlayedIcon(icon0, icon1, FALSE);

	ReleaseIconEx(icon0);
	ReleaseIconEx(icon1);

	return resIcon;
}


HICON createProtoOverlayedIcon(MCONTACT hContact)
{
	HICON icon1 = LoadIconEx(I_OVERLAY);

	char *szProto = GetContactProto(hContact);
	HICON icon0 = LoadSkinnedProtoIcon(szProto, ID_STATUS_ONLINE);

	HICON resIcon = getOverlayedIcon(icon0, icon1, FALSE);

	ReleaseIconEx(icon1);
	Skin_ReleaseIcon(icon0);
	return resIcon;
}
