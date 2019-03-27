/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

static IconItem common[] =
{
	{ LPGEN("Main"),    ICO_COMMON_MAIN,    IDI_MAIN    },
	{ LPGEN("Default"), ICO_COMMON_DEFAULT, IDI_DEFAULT }
};

static IconItem icons1[] =
{
	// common
	{ ICO_COMMON_IM,         LPGEN("IM naming"),             IDI_MIRANDA                },
	{ ICO_COMMON_PASSWORD,   LPGEN("Password"),              IDI_PASSWORD               },
	{ ICO_COMMON_FEMALE,     LPGEN("Female"),                IDI_FEMALE                 },
	{ ICO_COMMON_MALE,       LPGEN("Male"),                  IDI_MALE                   },
	{ ICO_COMMON_BIRTHDAY,   LPGEN("Birthday"),              IDI_BIRTHDAY               },
	{ ICO_COMMON_CLOCK,      LPGEN("Timezone"),              IDI_CLOCK,              32 },
	{ ICO_COMMON_MARITAL,    LPGEN("Marital status"),        IDI_MARITAL                },
	{ ICO_COMMON_ADDRESS,    LPGEN("Address"),               IDI_TREE_ADDRESS           },
	{ ICO_COMMON_ANNIVERSARY,LPGEN("Anniversary"),           IDI_ANNIVERSARY            },

	// zodiac
	{ ICO_ZOD_AQUARIUS,      LPGEN("Aquarius"),              IDI_ZOD_AQUARIUS,      128 },
	{ ICO_ZOD_ARIES,         LPGEN("Aries"),                 IDI_ZOD_ARIES,         128 },
	{ ICO_ZOD_CANCER,        LPGEN("Cancer"),                IDI_ZOD_CANCER,        128 },
	{ ICO_ZOD_CAPRICORN,     LPGEN("Capricorn"),             IDI_ZOD_CAPRICORN,     128 },
	{ ICO_ZOD_GEMINI,        LPGEN("Gemini"),                IDI_ZOD_GEMINI,        128 },
	{ ICO_ZOD_LEO,           LPGEN("Leo"),                   IDI_ZOD_LEO,           128 },
	{ ICO_ZOD_LIBRA,         LPGEN("Libra"),                 IDI_ZOD_LIBRA,         128 },
	{ ICO_ZOD_PISCES,        LPGEN("Pisces"),                IDI_ZOD_PISCES,        128 },
	{ ICO_ZOD_SAGITTARIUS,   LPGEN("Sagittarius"),           IDI_ZOD_SAGITTARIUS,   128 },
	{ ICO_ZOD_SCORPIO,       LPGEN("Scorpio"),               IDI_ZOD_SCORPIO,       128 },
	{ ICO_ZOD_TAURUS,        LPGEN("Taurus"),                IDI_ZOD_TAURUS,        128 },
	{ ICO_ZOD_VIRGO,         LPGEN("Virgo"),                 IDI_ZOD_VIRGO,         128 },

	// lists
	{ ICO_LST_MODULES,       LPGEN("Export: modules"),       IDI_LST_MODULES            },
	{ ICO_LST_FOLDER,        LPGEN("Export: folder"),        IDI_LST_FOLDER             },
};

static IconItem icons2[] =
{
	// tree
	{ ICO_TREE_DEFAULT,      LPGEN("Default"),               IDI_LST_FOLDER             },
};

static IconItem icons3[] =
{
	// dialogs
	{ ICO_DLG_DETAILS,       LPGEN("Details info bar"),      IDI_DLG_DETAILS,        48 },
	{ ICO_DLG_PHONE,         LPGEN("Phone info bar"),        IDI_DLG_PHONE,          32 },
	{ ICO_DLG_EMAIL,         LPGEN("E-mail info bar"),       IDI_DLG_EMAIL,          32 },
	{ ICO_DLG_EXPORT,        LPGEN("Export VCard"),          IDI_EXPORT,             32 },
	{ ICO_DLG_IMPORT,        LPGEN("Import VCard"),          IDI_IMPORT,             32 },
	{ ICO_DLG_ANNIVERSARY,   LPGEN("Anniversary info bar"),  IDI_ANNIVERSARY,        32 },
};

static IconItem icons4[] =
{
	// button icons
	{ ICO_BTN_UPDATE,        LPGEN("Update"),                IDI_BTN_UPDATE             },
	{ ICO_BTN_IMPORT,        LPGEN("Import"),                IDI_IMPORT                 },
	{ ICO_BTN_EXPORT,        LPGEN("Export"),                IDI_EXPORT                 },
	{ ICO_BTN_OK,            LPGEN("OK"),                    IDI_BTN_OK                 },
	{ ICO_BTN_CANCEL,        LPGEN("Cancel"),                IDI_BTN_CLOSE              },
	{ ICO_BTN_APPLY,         LPGEN("Apply"),                 IDI_BTN_APPLY              },
	{ ICO_BTN_GOTO,          LPGEN("Goto"),                  IDI_BTN_GOTO               },
	{ ICO_BTN_PHONE,         LPGEN("Phone"),                 IDI_BTN_PHONE              },
	{ ICO_BTN_FAX,           LPGEN("Fax"),                   IDI_BTN_FAX                },
	{ ICO_BTN_CELLULAR,      LPGEN("Cellular"),              IDI_BTN_CELLULAR           },
	{ ICO_BTN_CUSTOMPHONE,   LPGEN("Custom phone"),          IDI_BTN_CUSTOMPHONE        },
	{ ICO_BTN_EMAIL,         LPGEN("E-mail"),                IDI_BTN_EMAIL              },
	{ ICO_BTN_DOWNARROW,     LPGEN("Down arrow"),            IDI_BTN_DOWNARROW          },
	{ ICO_BTN_ADD,           LPGEN("Add"),                   IDI_BTN_ADD                },
	{ ICO_BTN_EDIT,          LPGEN("Edit"),                  IDI_BTN_EDIT               },
	{ ICO_BTN_DELETE,        LPGEN("Delete"),                IDI_BTN_DELETE             },
	{ ICO_BTN_SEARCH,        LPGEN("Search"),                IDI_SEARCH                 },
	{ ICO_BTN_EXIMPORT,      LPGEN("Export/import"),         IDI_BTN_EXIMPORT           },
	{ ICO_BTN_BDAY_BACKUP,   LPGEN("Backup birthday"),       IDI_BTN_BIRTHDAY_BACKUP    },
};

static IconItem icons5[] =
{
	// birthday and anniversary
	{ ICO_RMD_DTB0,          LPGEN("Birthday today"),        IDI_RMD_DTB0 },
	{ ICO_RMD_DTB1,          LPGEN("Birthday tomorrow"),     IDI_RMD_DTB1 },
	{ ICO_RMD_DTB2,          LPGEN("Birthday in 2 days"),    IDI_RMD_DTB2 },
	{ ICO_RMD_DTB3,          LPGEN("Birthday in 3 days"),    IDI_RMD_DTB3 },
	{ ICO_RMD_DTB4,          LPGEN("Birthday in 4 days"),    IDI_RMD_DTB4 },
	{ ICO_RMD_DTB5,          LPGEN("Birthday in 5 days"),    IDI_RMD_DTB5 },
	{ ICO_RMD_DTB6,          LPGEN("Birthday in 6 days"),    IDI_RMD_DTB6 },
	{ ICO_RMD_DTB7,          LPGEN("Birthday in 7 days"),    IDI_RMD_DTB7 },
	{ ICO_RMD_DTB8,          LPGEN("Birthday in 8 days"),    IDI_RMD_DTB8 },
	{ ICO_RMD_DTB9,          LPGEN("Birthday in 9 days"),    IDI_RMD_DTB9 },
	{ ICO_RMD_DTBX,          LPGEN("Birthday later"),        IDI_RMD_DTBX },

	{ ICO_RMD_DTA0,          LPGEN("Anniversary today"),     IDI_RMD_DTA0 },
	{ ICO_RMD_DTA1,          LPGEN("Anniversary tomorrow"),  IDI_RMD_DTA1 },
	{ ICO_RMD_DTA2,          LPGEN("Anniversary in 2 days"), IDI_RMD_DTA2 },
	{ ICO_RMD_DTA3,          LPGEN("Anniversary in 3 days"), IDI_RMD_DTA3 },
	{ ICO_RMD_DTA4,          LPGEN("Anniversary in 4 days"), IDI_RMD_DTA4 },
	{ ICO_RMD_DTA5,          LPGEN("Anniversary in 5 days"), IDI_RMD_DTA5 },
	{ ICO_RMD_DTA6,          LPGEN("Anniversary in 6 days"), IDI_RMD_DTA6 },
	{ ICO_RMD_DTA7,          LPGEN("Anniversary in 7 days"), IDI_RMD_DTA7 },
	{ ICO_RMD_DTA8,          LPGEN("Anniversary in 8 days"), IDI_RMD_DTA8 },
	{ ICO_RMD_DTA9,          LPGEN("Anniversary in 9 days"), IDI_RMD_DTA9 },
	{ ICO_RMD_DTAX,          LPGEN("Anniversary later"),     IDI_RMD_DTAX },
};

/////////////////////////////////////////////////////////////////////////////////////////
// This function finds the default iconpack file and return its path.

wchar_t *IcoLib_GetDefaultIconFileName()
{
	static wchar_t *path[] = {
		L"Icons\\uinfoex_icons.dll",
		L"Plugins\\uinfoex_icons.dll",
		L"Customize\\Icons\\uinfoex_icons.dll"
	};
	wchar_t absolute[MAX_PATH];

	for (auto &it : path) {
		PathToAbsoluteW(it, absolute);
		if (PathFileExists(absolute))
			return it;
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function checks the version of an iconpack.
// If the icon pack's version differs from the desired one, 
// dialog with a warning is displayed.

static HINSTANCE IcoLib_CheckIconPackVersion(wchar_t *szIconPack)
{
	if (g_plugin.getByte(SET_ICONS_CHECKFILEVERSION, TRUE)) {
		if (szIconPack) {
			wchar_t szAbsolutePath[MAX_PATH];
			PathToAbsoluteW(szIconPack, szAbsolutePath);

			HMODULE hIconDll = LoadLibrary(szAbsolutePath);
			if (hIconDll) {
				CHAR szFileVersion[64];

				if (LoadStringA(hIconDll, IDS_ICOPACKVERSION, szFileVersion, sizeof(szFileVersion)))
					if (!mir_strcmp(szFileVersion, "__UserInfoEx_IconPack_1.2__"))
						return hIconDll;
				
				MsgErr(nullptr, LPGENW("Warning: Your current IconPack's version differs from the one UserInfoEx is designed for.\nSome icons may not be displayed correctly"));
			}
		}
		else MsgErr(nullptr, LPGENW("Warning: No IconPack found in one of the following directories: 'customize\\icons', 'icons' or 'plugins'!"));
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Set the icon of each control in the list

void IcoLib_SetCtrlIcons(HWND hDlg, const ICONCTRL *pCtrl, BYTE numCtrls)
{
	for (int i = 0; i < numCtrls; i++) {
		HICON	hIcon = IcoLib_GetIcon(pCtrl[i].pszIcon);
		if (!pCtrl[i].idCtrl) {
			SendMessage(hDlg, pCtrl[i].Message, ICON_BIG, (LPARAM)hIcon);
			continue;
		}
		
		HWND hCtrl = GetDlgItem(hDlg, pCtrl[i].idCtrl);
		switch (pCtrl[i].Message) {
		case STM_SETICON:
		case STM_SETIMAGE:
			ShowWindow(hCtrl, hIcon ? SW_SHOW : SW_HIDE);
			__fallthrough;

		case BM_SETIMAGE:
			SendMessage(hCtrl, pCtrl[i].Message, IMAGE_ICON, (LPARAM)hIcon);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Add default icons to the skin library or load customized icons

void IcoLib_LoadModule()
{
	g_plugin.registerIcon(SECT_COMMON, common);

	wchar_t *szDefaultFile = IcoLib_GetDefaultIconFileName();
	HINSTANCE hIconPack = IcoLib_CheckIconPackVersion(szDefaultFile);
	if (hIconPack) {
		Icon_Register(hIconPack, SECT_COMMON,  icons1, _countof(icons1), 0, &g_plugin);
		Icon_Register(hIconPack, SECT_TREE,    icons2, _countof(icons2), 0, &g_plugin);
		Icon_Register(hIconPack, SECT_DLG,     icons3, _countof(icons3), 0, &g_plugin);
		Icon_Register(hIconPack, SECT_BUTTONS, icons4, _countof(icons4), 0, &g_plugin);
		Icon_Register(hIconPack, SECT_REMIND,  icons5, _countof(icons5), 0, &g_plugin);
		FreeLibrary(hIconPack);
	}
}
