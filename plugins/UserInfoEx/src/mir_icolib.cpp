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
	{ LPGEN("IM naming"),             "common_im",           IDI_MIRANDA                },
	{ LPGEN("Female"),                "common_female",       IDI_FEMALE                 },
	{ LPGEN("Male"),                  "common_male",         IDI_MALE                   },
	{ LPGEN("Birthday"),              "common_birthday",     IDI_BIRTHDAY               },
	{ LPGEN("Timezone"),              "common_clock",        IDI_CLOCK,              32 },
	{ LPGEN("Marital status"),        "common_marital",      IDI_MARITAL                },
	{ LPGEN("Address"),               "common_address",      IDI_TREE_ADDRESS           },
	{ LPGEN("Anniversary"),           "common_anniversary",  IDI_ANNIVERSARY            },

	// zodiac
	{ LPGEN("Aquarius"),              "zod_aquarius",        IDI_ZOD_AQUARIUS,      128 },
	{ LPGEN("Aries"),                 "zod_aries",           IDI_ZOD_ARIES,         128 },
	{ LPGEN("Cancer"),                "zod_cancer",          IDI_ZOD_CANCER,        128 },
	{ LPGEN("Capricorn"),             "zod_capricorn",       IDI_ZOD_CAPRICORN,     128 },
	{ LPGEN("Gemini"),                "zod_gemini",          IDI_ZOD_GEMINI,        128 },
	{ LPGEN("Leo"),                   "zod_leo",             IDI_ZOD_LEO,           128 },
	{ LPGEN("Libra"),                 "zod_libra",           IDI_ZOD_LIBRA,         128 },
	{ LPGEN("Pisces"),                "zod_pisces",          IDI_ZOD_PISCES,        128 },
	{ LPGEN("Sagittarius"),           "zod_sagittarius",     IDI_ZOD_SAGITTARIUS,   128 },
	{ LPGEN("Scorpio"),               "zod_scorpio",         IDI_ZOD_SCORPIO,       128 },
	{ LPGEN("Taurus"),                "zod_taurus",          IDI_ZOD_TAURUS,        128 },
	{ LPGEN("Virgo"),                 "zod_virgo",           IDI_ZOD_VIRGO,         128 },

	// lists
	{ LPGEN("Export: modules"),       "lst_modules",         IDI_LST_MODULES            },
	{ LPGEN("Export: folder"),        "lst_folder",          IDI_LST_FOLDER             },
};

static IconItem icons2[] =
{
	// tree
	{ LPGEN("Default"),               "tree_default",        IDI_LST_FOLDER             },
};

static IconItem icons3[] =
{
	// dialogs
	{ LPGEN("Details info bar"),      "dlg_details",         IDI_DLG_DETAILS,        48 },
	{ LPGEN("Phone info bar"),        "dlg_phone",           IDI_DLG_PHONE,          32 },
	{ LPGEN("E-mail info bar"),       "dlg_email",           IDI_DLG_EMAIL,          32 },
	{ LPGEN("Export VCard"),          "dlg_export",          IDI_EXPORT,             32 },
	{ LPGEN("Import VCard"),          "dlg_import",          IDI_IMPORT,             32 },
	{ LPGEN("Anniversary info bar"),  "dlg_anniversary",     IDI_ANNIVERSARY,        32 },
};

static IconItem icons4[] =
{
	// button icons
	{ LPGEN("Update"),                "btn_update",          IDI_BTN_UPDATE             },
	{ LPGEN("Import"),                "btn_import",          IDI_IMPORT                 },
	{ LPGEN("Export"),                "btn_export",          IDI_EXPORT                 },
	{ LPGEN("OK"),                    "btn_ok",              IDI_BTN_OK                 },
	{ LPGEN("Cancel"),                "btn_cancel",          IDI_BTN_CLOSE              },
	{ LPGEN("Apply"),                 "btn_apply",           IDI_BTN_APPLY              },
	{ LPGEN("Goto"),                  "btn_goto",            IDI_BTN_GOTO               },
	{ LPGEN("Phone"),                 "btn_phone",           IDI_BTN_PHONE              },
	{ LPGEN("Fax"),                   "btn_fax",             IDI_BTN_FAX                },
	{ LPGEN("Cellular"),              "btn_cellular",        IDI_BTN_CELLULAR           },
	{ LPGEN("Custom phone"),          "btn_customphone",     IDI_BTN_CUSTOMPHONE        },
	{ LPGEN("E-mail"),                "btn_email",           IDI_BTN_EMAIL              },
	{ LPGEN("Down arrow"),            "btn_downarrow",       IDI_BTN_DOWNARROW          },
	{ LPGEN("Add"),                   "btn_add",             IDI_BTN_ADD                },
	{ LPGEN("Edit"),                  "btn_edit",            IDI_BTN_EDIT               },
	{ LPGEN("Delete"),                "btn_delete",          IDI_BTN_DELETE             },
	{ LPGEN("Search"),                "btn_search",          IDI_SEARCH                 },
	{ LPGEN("Export/import"),         "btn_eximport",        IDI_BTN_EXIMPORT           },
	{ LPGEN("Backup birthday"),       "btn_bdaybackup",      IDI_BTN_BIRTHDAY_BACKUP    },
};

static IconItem icons5[] =
{
	// birthday and anniversary
	{ LPGEN("Birthday today"),        "rmd_dtb0",            IDI_RMD_DTB0 },
	{ LPGEN("Birthday tomorrow"),     "rmd_dtb1",            IDI_RMD_DTB1 },
	{ LPGEN("Birthday in 2 days"),    "rmd_dtb2",            IDI_RMD_DTB2 },
	{ LPGEN("Birthday in 3 days"),    "rmd_dtb3",            IDI_RMD_DTB3 },
	{ LPGEN("Birthday in 4 days"),    "rmd_dtb4",            IDI_RMD_DTB4 },
	{ LPGEN("Birthday in 5 days"),    "rmd_dtb5",            IDI_RMD_DTB5 },
	{ LPGEN("Birthday in 6 days"),    "rmd_dtb6",            IDI_RMD_DTB6 },
	{ LPGEN("Birthday in 7 days"),    "rmd_dtb7",            IDI_RMD_DTB7 },
	{ LPGEN("Birthday in 8 days"),    "rmd_dtb8",            IDI_RMD_DTB8 },
	{ LPGEN("Birthday in 9 days"),    "rmd_dtb9",            IDI_RMD_DTB9 },
	{ LPGEN("Birthday later"),        "rmd_dtbx",            IDI_RMD_DTBX },

	{ LPGEN("Anniversary today"),     "rmd_dta0",            IDI_RMD_DTA0 },
	{ LPGEN("Anniversary tomorrow"),  "rmd_dta1",            IDI_RMD_DTA1 },
	{ LPGEN("Anniversary in 2 days"), "rmd_dta2",            IDI_RMD_DTA2 },
	{ LPGEN("Anniversary in 3 days"), "rmd_dta3",            IDI_RMD_DTA3 },
	{ LPGEN("Anniversary in 4 days"), "rmd_dta4",            IDI_RMD_DTA4 },
	{ LPGEN("Anniversary in 5 days"), "rmd_dta5",            IDI_RMD_DTA5 },
	{ LPGEN("Anniversary in 6 days"), "rmd_dta6",            IDI_RMD_DTA6 },
	{ LPGEN("Anniversary in 7 days"), "rmd_dta7",            IDI_RMD_DTA7 },
	{ LPGEN("Anniversary in 8 days"), "rmd_dta8",            IDI_RMD_DTA8 },
	{ LPGEN("Anniversary in 9 days"), "rmd_dta9",            IDI_RMD_DTA9 },
	{ LPGEN("Anniversary later"),     "rmd_dtax",            IDI_RMD_DTAX },
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
	if (g_plugin.bCheckVersion) {
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

void IcoLib_SetCtrlIcons(HWND hDlg, const ICONCTRL *pCtrl, uint8_t numCtrls)
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
