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
	{ LPGEN("Main"),    "common_main",    IDI_MAIN    },
	{ LPGEN("Default"), "common_default", IDI_DEFAULT }
};

static IconItem icons1[] =
{
	// common
	{ "common_im",           LPGEN("IM naming"),             IDI_MIRANDA                },
	{ "common_female",       LPGEN("Female"),                IDI_FEMALE                 },
	{ "common_male",         LPGEN("Male"),                  IDI_MALE                   },
	{ "common_birthday",     LPGEN("Birthday"),              IDI_BIRTHDAY               },
	{ "common_clock",        LPGEN("Timezone"),              IDI_CLOCK,              32 },
	{ "common_marital",      LPGEN("Marital status"),        IDI_MARITAL                },
	{ "common_address",      LPGEN("Address"),               IDI_TREE_ADDRESS           },
	{ "common_anniversary",  LPGEN("Anniversary"),           IDI_ANNIVERSARY            },

	// zodiac
	{ "zod_aquarius",        LPGEN("Aquarius"),              IDI_ZOD_AQUARIUS,      128 },
	{ "zod_aries",           LPGEN("Aries"),                 IDI_ZOD_ARIES,         128 },
	{ "zod_cancer",          LPGEN("Cancer"),                IDI_ZOD_CANCER,        128 },
	{ "zod_capricorn",       LPGEN("Capricorn"),             IDI_ZOD_CAPRICORN,     128 },
	{ "zod_gemini",          LPGEN("Gemini"),                IDI_ZOD_GEMINI,        128 },
	{ "zod_leo",             LPGEN("Leo"),                   IDI_ZOD_LEO,           128 },
	{ "zod_libra",           LPGEN("Libra"),                 IDI_ZOD_LIBRA,         128 },
	{ "zod_pisces",          LPGEN("Pisces"),                IDI_ZOD_PISCES,        128 },
	{ "zod_sagittarius",     LPGEN("Sagittarius"),           IDI_ZOD_SAGITTARIUS,   128 },
	{ "zod_scorpio",         LPGEN("Scorpio"),               IDI_ZOD_SCORPIO,       128 },
	{ "zod_taurus",          LPGEN("Taurus"),                IDI_ZOD_TAURUS,        128 },
	{ "zod_virgo",           LPGEN("Virgo"),                 IDI_ZOD_VIRGO,         128 },

	// lists
	{ "lst_modules",         LPGEN("Export: modules"),       IDI_LST_MODULES            },
	{ "lst_folder",          LPGEN("Export: folder"),        IDI_LST_FOLDER             },
};

static IconItem icons2[] =
{
	// tree
	{ "tree_default",        LPGEN("Default"),               IDI_LST_FOLDER             },
};

static IconItem icons3[] =
{
	// dialogs
	{ "dlg_details",         LPGEN("Details info bar"),      IDI_DLG_DETAILS,        48 },
	{ "dlg_phone",           LPGEN("Phone info bar"),        IDI_DLG_PHONE,          32 },
	{ "dlg_email",           LPGEN("E-mail info bar"),       IDI_DLG_EMAIL,          32 },
	{ "dlg_export",          LPGEN("Export VCard"),          IDI_EXPORT,             32 },
	{ "dlg_import",          LPGEN("Import VCard"),          IDI_IMPORT,             32 },
	{ "dlg_anniversary",     LPGEN("Anniversary info bar"),  IDI_ANNIVERSARY,        32 },
};

static IconItem icons4[] =
{
	// button icons
	{ "btn_update",          LPGEN("Update"),                IDI_BTN_UPDATE             },
	{ "btn_import",          LPGEN("Import"),                IDI_IMPORT                 },
	{ "btn_export",          LPGEN("Export"),                IDI_EXPORT                 },
	{ "btn_ok",              LPGEN("OK"),                    IDI_BTN_OK                 },
	{ "btn_cancel",          LPGEN("Cancel"),                IDI_BTN_CLOSE              },
	{ "btn_apply",           LPGEN("Apply"),                 IDI_BTN_APPLY              },
	{ "btn_goto",            LPGEN("Goto"),                  IDI_BTN_GOTO               },
	{ "btn_phone",           LPGEN("Phone"),                 IDI_BTN_PHONE              },
	{ "btn_fax",             LPGEN("Fax"),                   IDI_BTN_FAX                },
	{ "btn_cellular",        LPGEN("Cellular"),              IDI_BTN_CELLULAR           },
	{ "btn_customphone",     LPGEN("Custom phone"),          IDI_BTN_CUSTOMPHONE        },
	{ "btn_email",           LPGEN("E-mail"),                IDI_BTN_EMAIL              },
	{ "btn_downarrow",       LPGEN("Down arrow"),            IDI_BTN_DOWNARROW          },
	{ "btn_add",             LPGEN("Add"),                   IDI_BTN_ADD                },
	{ "btn_edit",            LPGEN("Edit"),                  IDI_BTN_EDIT               },
	{ "btn_delete",          LPGEN("Delete"),                IDI_BTN_DELETE             },
	{ "btn_search",          LPGEN("Search"),                IDI_SEARCH                 },
	{ "btn_eximport",        LPGEN("Export/import"),         IDI_BTN_EXIMPORT           },
	{ "btn_bdaybackup",      LPGEN("Backup birthday"),       IDI_BTN_BIRTHDAY_BACKUP    },
};

static IconItem icons5[] =
{
	// birthday and anniversary
	{ "rmd_dtb0",            LPGEN("Birthday today"),        IDI_RMD_DTB0 },
	{ "rmd_dtb1",            LPGEN("Birthday tomorrow"),     IDI_RMD_DTB1 },
	{ "rmd_dtb2",            LPGEN("Birthday in 2 days"),    IDI_RMD_DTB2 },
	{ "rmd_dtb3",            LPGEN("Birthday in 3 days"),    IDI_RMD_DTB3 },
	{ "rmd_dtb4",            LPGEN("Birthday in 4 days"),    IDI_RMD_DTB4 },
	{ "rmd_dtb5",            LPGEN("Birthday in 5 days"),    IDI_RMD_DTB5 },
	{ "rmd_dtb6",            LPGEN("Birthday in 6 days"),    IDI_RMD_DTB6 },
	{ "rmd_dtb7",            LPGEN("Birthday in 7 days"),    IDI_RMD_DTB7 },
	{ "rmd_dtb8",            LPGEN("Birthday in 8 days"),    IDI_RMD_DTB8 },
	{ "rmd_dtb9",            LPGEN("Birthday in 9 days"),    IDI_RMD_DTB9 },
	{ "rmd_dtbx",            LPGEN("Birthday later"),        IDI_RMD_DTBX },

	{ "rmd_dta0",            LPGEN("Anniversary today"),     IDI_RMD_DTA0 },
	{ "rmd_dta1",            LPGEN("Anniversary tomorrow"),  IDI_RMD_DTA1 },
	{ "rmd_dta2",            LPGEN("Anniversary in 2 days"), IDI_RMD_DTA2 },
	{ "rmd_dta3",            LPGEN("Anniversary in 3 days"), IDI_RMD_DTA3 },
	{ "rmd_dta4",            LPGEN("Anniversary in 4 days"), IDI_RMD_DTA4 },
	{ "rmd_dta5",            LPGEN("Anniversary in 5 days"), IDI_RMD_DTA5 },
	{ "rmd_dta6",            LPGEN("Anniversary in 6 days"), IDI_RMD_DTA6 },
	{ "rmd_dta7",            LPGEN("Anniversary in 7 days"), IDI_RMD_DTA7 },
	{ "rmd_dta8",            LPGEN("Anniversary in 8 days"), IDI_RMD_DTA8 },
	{ "rmd_dta9",            LPGEN("Anniversary in 9 days"), IDI_RMD_DTA9 },
	{ "rmd_dtax",            LPGEN("Anniversary later"),     IDI_RMD_DTAX },
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
		HICON	hIcon = g_plugin.getIcon(pCtrl[i].iIcon);
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
	g_plugin.registerIcon(SECT_COMMON, common, MODULENAME);

	wchar_t *szDefaultFile = IcoLib_GetDefaultIconFileName();
	HINSTANCE hIconPack = IcoLib_CheckIconPackVersion(szDefaultFile);
	if (hIconPack) {
		Icon_Register(hIconPack, SECT_COMMON,  icons1, _countof(icons1), MODULENAME, &g_plugin);
		Icon_Register(hIconPack, SECT_TREE,    icons2, _countof(icons2), MODULENAME, &g_plugin);
		Icon_Register(hIconPack, SECT_DLG,     icons3, _countof(icons3), MODULENAME, &g_plugin);
		Icon_Register(hIconPack, SECT_BUTTONS, icons4, _countof(icons4), MODULENAME, &g_plugin);
		Icon_Register(hIconPack, SECT_REMIND,  icons5, _countof(icons5), MODULENAME, &g_plugin);
		FreeLibrary(hIconPack);
	}
}
