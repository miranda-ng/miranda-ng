#include "stdafx.h"
#include "MraIcons.h"

HANDLE hXStatusAdvancedStatusIcons[MRA_XSTATUS_COUNT+4];

IconItem gdiMenuItems[MAIN_MENU_ITEMS_COUNT] =
{
	{ MRA_GOTO_INBOX_STR,         MRA_GOTO_INBOX,         IDI_INBOX             },
	{ MRA_SHOW_INBOX_STATUS_STR,  MRA_SHOW_INBOX_STATUS,  IDI_MAIL_NOTIFY       },
	{ MRA_EDIT_PROFILE_STR,       MRA_EDIT_PROFILE,       IDI_PROFILE           },
	{ MRA_MY_ALBUM_STR,           MRA_VIEW_ALBUM,         IDI_MRA_PHOTO         },
	{ MRA_MY_BLOGSTATUS_STR,      MRA_REPLY_BLOG_STATUS,  IDI_BLOGSTATUS        },
	{ MRA_MY_VIDEO_STR,           MRA_VIEW_VIDEO,         IDI_MRA_VIDEO         },
	{ MRA_MY_ANSWERS_STR,         MRA_ANSWERS,            IDI_MRA_ANSWERS       },
	{ MRA_MY_WORLD_STR,           MRA_WORLD,              IDI_MRA_WORLD         },
	{ MRA_WEB_SEARCH_STR,         MRA_WEB_SEARCH,         IDI_MRA_WEB_SEARCH    },
	{ MRA_UPD_ALL_USERS_INFO_STR, MRA_UPD_ALL_USERS_INFO, IDI_PROFILE           },
	{ MRA_CHK_USERS_AVATARS_STR,  MRA_CHK_USERS_AVATARS,  IDI_PROFILE           },
	{ MRA_REQ_AUTH_FOR_ALL_STR,   MRA_REQ_AUTH_FOR_ALL,   IDI_AUTHRUGUEST       }
};

IconItem gdiContactMenuItems[CONTACT_MENU_ITEMS_COUNT] =
{
	{ MRA_REQ_AUTH_STR,           MRA_REQ_AUTH,           IDI_AUTHRUGUEST       },
	{ MRA_GRANT_AUTH_STR,         MRA_GRANT_AUTH,         IDI_AUTHGRANT         },
	{ MRA_SEND_EMAIL_STR,         MRA_SEND_EMAIL,         IDI_INBOX             },
	{ MRA_SEND_POSTCARD_STR,      MRA_SEND_POSTCARD,      IDI_MRA_POSTCARD      },
	{ MRA_VIEW_ALBUM_STR,         MRA_VIEW_ALBUM,         IDI_MRA_PHOTO         },
	{ MRA_REPLY_BLOG_STATUS_STR,  MRA_REPLY_BLOG_STATUS,  IDI_BLOGSTATUS        },
	{ MRA_VIEW_VIDEO_STR,         MRA_VIEW_VIDEO,         IDI_MRA_VIDEO         },
	{ MRA_ANSWERS_STR,            MRA_ANSWERS,            IDI_MRA_ANSWERS       },
	{ MRA_WORLD_STR,              MRA_WORLD,              IDI_MRA_WORLD         },
	{ MRA_SENDNUDGE_STR,          PS_SEND_NUDGE,          IDI_MRA_ALARM         }
};

IconItem gdiExtraStatusIconsItems[ADV_ICON_MAX] =
{
   { ADV_ICON_DELETED_STR,		ADV_ICON_DELETED_ID,		IDI_DELETED     },
   { ADV_ICON_NOT_ON_SERVER_STR,	ADV_ICON_NOT_ON_SERVER_ID,	IDI_AUTHGRANT   },
   { ADV_ICON_NOT_AUTHORIZED_STR,	ADV_ICON_NOT_AUTHORIZED_ID,	IDI_AUTHRUGUEST },
   { ADV_ICON_PHONE_STR,		ADV_ICON_PHONE_ID,		IDI_MRA_PHONE   },
   { ADV_ICON_BLOGSTATUS_STR,		ADV_ICON_BLOGSTATUS_ID,		IDI_BLOGSTATUS  }
};

//////////////////////////////////////////////////////////////////////////////////////

HICON IconLibGetIcon(HANDLE hIcon)
{
	return IconLibGetIconEx(hIcon, LR_SHARED);
}

HICON IconLibGetIconEx(HANDLE hIcon, DWORD dwFlags)
{
	HICON hiIcon = NULL;
	if (hIcon) {
		hiIcon = Skin_GetIconByHandle(hIcon);
		if ((dwFlags & LR_SHARED) == 0)
			hiIcon = CopyIcon(hiIcon);
	}
	return hiIcon;
}

//////////////////////////////////////////////////////////////////////////////////////

void IconsLoad()
{
	g_hMainIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MRA), IMAGE_ICON, 0, 0, LR_SHARED);

	Icon_Register(g_hInstance, LPGEN("Protocols") "/" LPGEN("MRA") "/" LPGEN("Main Menu"), gdiMenuItems, MAIN_MENU_ITEMS_COUNT, "MRA_");
	Icon_Register(g_hInstance, LPGEN("Protocols") "/" LPGEN("MRA") "/" LPGEN("Contact Menu"), gdiContactMenuItems, CONTACT_MENU_ITEMS_COUNT, "MRA_");
	Icon_Register(g_hInstance, LPGEN("Protocols") "/" LPGEN("MRA") "/" LPGEN("Extra status"), gdiExtraStatusIconsItems, ADV_ICON_MAX, "MRA_");
}

void InitXStatusIcons()
{
	// load libs
	TCHAR szBuff[MAX_FILEPATH];
	if (GetModuleFileName(NULL, szBuff, SIZEOF(szBuff))) {
		LPWSTR lpwszFileName;
		g_dwMirWorkDirPathLen = GetFullPathName(szBuff, MAX_FILEPATH, g_szMirWorkDirPath, &lpwszFileName);
		if (g_dwMirWorkDirPathLen) {
			g_dwMirWorkDirPathLen -= mir_wstrlen(lpwszFileName);
			g_szMirWorkDirPath[g_dwMirWorkDirPathLen] = 0;

			// load xstatus icons lib
			DWORD dwBuffLen;
			DWORD dwErrorCode = FindFile(g_szMirWorkDirPath, (DWORD)g_dwMirWorkDirPathLen, _T("xstatus_MRA.dll"), -1, szBuff, SIZEOF(szBuff), &dwBuffLen);
			if (dwErrorCode == NO_ERROR) {
				g_hDLLXStatusIcons = LoadLibraryEx(szBuff, NULL, 0);
				if (g_hDLLXStatusIcons) {
					dwBuffLen = LoadString(g_hDLLXStatusIcons, IDS_IDENTIFY, szBuff, MAX_FILEPATH);
					if (dwBuffLen == 0 || _tcsnicmp(_T("# Custom Status Icons #"), szBuff, 23)) {
						FreeLibrary(g_hDLLXStatusIcons);
						g_hDLLXStatusIcons = NULL;
					}
				}
			}
		}
	}

	GetModuleFileName((g_hDLLXStatusIcons != NULL) ? g_hDLLXStatusIcons : g_hInstance, szBuff, SIZEOF(szBuff));

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszSection = LPGENT("Protocols")_T("/")LPGENT("MRA")_T("/")LPGENT("Custom Status");
	sid.ptszDefaultFile = szBuff;
	sid.cx = sid.cy = 16;
	sid.flags = SIDF_ALL_TCHAR;

	hXStatusAdvancedStatusIcons[0] = NULL;
	for (DWORD i = 1; i < MRA_XSTATUS_COUNT+1; i++) {
		char szBuff[MAX_PATH];
		mir_snprintf(szBuff, SIZEOF(szBuff), "mra_xstatus%ld", i);
		sid.pszName = szBuff;

		int iCurIndex = i+IDI_XSTATUS1-1;
		sid.ptszDescription = (TCHAR*)lpcszXStatusNameDef[i];
		sid.iDefaultIndex = -iCurIndex;

		hXStatusAdvancedStatusIcons[i] = Skin_AddIcon(&sid);
	}
}

void DestroyXStatusIcons()
{
	char szBuff[MAX_PATH];

	for (DWORD i = 1; i < MRA_XSTATUS_COUNT+1; i++) {
		mir_snprintf(szBuff, SIZEOF(szBuff), "mra_xstatus%ld", i);
		Skin_RemoveIcon(szBuff);
	}

	memset(hXStatusAdvancedStatusIcons, 0, sizeof(hXStatusAdvancedStatusIcons));
}
