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

#include "commonheaders.h"

typedef struct _ICODESC 
{
	LPSTR	pszName;
	LPSTR	pszDesc;
	LPSTR	pszSection;
	WORD	idResource;
	BYTE	size;
} ICODESC;

HICON ghDefIcon = NULL;

static ICODESC icoDesc[] = {
	// common
	{ ICO_COMMON_IM,         LPGEN("IM naming"),             SECT_COMMON,  IDI_MIRANDA,             0 },
	{ ICO_COMMON_PASSWORD,   LPGEN("Password"),              SECT_COMMON,  IDI_PASSWORD,            0 },
	{ ICO_COMMON_FEMALE,     LPGEN("Female"),                SECT_COMMON,  IDI_FEMALE,              0 },
	{ ICO_COMMON_MALE,       LPGEN("Male"),                  SECT_COMMON,  IDI_MALE,                0 },
	{ ICO_COMMON_BIRTHDAY,   LPGEN("Birthday"),              SECT_COMMON,  IDI_BIRTHDAY,            0 },
	{ ICO_COMMON_CLOCK,      LPGEN("Timezone"),              SECT_COMMON,  IDI_CLOCK,               1 },
	{ ICO_COMMON_MARITAL,    LPGEN("Marital status"),        SECT_COMMON,  IDI_MARITAL,             0 },
	{ ICO_COMMON_ADDRESS,    LPGEN("Address"),               SECT_COMMON,  IDI_TREE_ADDRESS,        0 },
	{ ICO_COMMON_ANNIVERSARY,LPGEN("Anniversary"),           SECT_COMMON,  IDI_ANNIVERSARY,         0 },

	//zodiac
	{ ICO_ZOD_AQUARIUS,      LPGEN("Aquarius"),              SECT_COMMON,  IDI_ZOD_AQUARIUS,      128 },
	{ ICO_ZOD_ARIES,         LPGEN("Aries"),                 SECT_COMMON,  IDI_ZOD_ARIES,         128 },
	{ ICO_ZOD_CANCER,        LPGEN("Cancer"),                SECT_COMMON,  IDI_ZOD_CANCER,        128 },
	{ ICO_ZOD_CAPRICORN,     LPGEN("Capricorn"),             SECT_COMMON,  IDI_ZOD_CAPRICORN,     128 },
	{ ICO_ZOD_GEMINI,        LPGEN("Gemini"),                SECT_COMMON,  IDI_ZOD_GEMINI,        128 },
	{ ICO_ZOD_LEO,           LPGEN("Leo"),                   SECT_COMMON,  IDI_ZOD_LEO,           128 },
	{ ICO_ZOD_LIBRA,         LPGEN("Libra"),                 SECT_COMMON,  IDI_ZOD_LIBRA,         128 },
	{ ICO_ZOD_PISCES,        LPGEN("Pisces"),                SECT_COMMON,  IDI_ZOD_PISCES,        128 },
	{ ICO_ZOD_SAGITTARIUS,   LPGEN("Sagittarius"),           SECT_COMMON,  IDI_ZOD_SAGITTARIUS,   128 },
	{ ICO_ZOD_SCORPIO,       LPGEN("Scorpio"),               SECT_COMMON,  IDI_ZOD_SCORPIO,       128 },
	{ ICO_ZOD_TAURUS,        LPGEN("Taurus"),                SECT_COMMON,  IDI_ZOD_TAURUS,        128 },
	{ ICO_ZOD_VIRGO,         LPGEN("Virgo"),                 SECT_COMMON,  IDI_ZOD_VIRGO,         128 },

	// lists
	{ ICO_LST_MODULES,       LPGEN("Export: modules"),       SECT_COMMON,  IDI_LST_MODULES,         0 },
	{ ICO_LST_FOLDER,        LPGEN("Export: folder"),        SECT_COMMON,  IDI_LST_FOLDER,          0 },
	{ ICO_TREE_DEFAULT,      LPGEN("Default"),               SECT_TREE,    IDI_LST_FOLDER,          0 },

	// dialogs
	{ ICO_DLG_DETAILS,       LPGEN("Details info bar"),      SECT_DLG,     IDI_DLG_DETAILS,        48 },
	{ ICO_DLG_PHONE,         LPGEN("Phone info bar"),        SECT_DLG,     IDI_DLG_PHONE,           1 },
	{ ICO_DLG_EMAIL,         LPGEN("E-mail info bar"),       SECT_DLG,     IDI_DLG_EMAIL,           1 },
	{ ICO_DLG_EXPORT,        LPGEN("Export VCard"),          SECT_DLG,     IDI_EXPORT,              1 },
	{ ICO_DLG_IMPORT,        LPGEN("Import VCard"),          SECT_DLG,     IDI_IMPORT,              1 },
	{ ICO_DLG_ANNIVERSARY,   LPGEN("Anniversary info bar"),  SECT_DLG,     IDI_ANNIVERSARY,         1 },

	// button icons
	{ ICO_BTN_UPDATE,        LPGEN("Update"),                SECT_BUTTONS, IDI_BTN_UPDATE,          0 },
	{ ICO_BTN_IMPORT,        LPGEN("Import"),                SECT_BUTTONS, IDI_IMPORT,              0 },
	{ ICO_BTN_EXPORT,        LPGEN("Export"),                SECT_BUTTONS, IDI_EXPORT,              0 },
	{ ICO_BTN_OK,            LPGEN("OK"),                    SECT_BUTTONS, IDI_BTN_OK,              0 },
	{ ICO_BTN_CANCEL,        LPGEN("Cancel"),                SECT_BUTTONS, IDI_BTN_CLOSE,           0 },
	{ ICO_BTN_APPLY,         LPGEN("Apply"),                 SECT_BUTTONS, IDI_BTN_APPLY,           0 },
	{ ICO_BTN_GOTO,          LPGEN("Goto"),                  SECT_BUTTONS, IDI_BTN_GOTO,            0 },
	{ ICO_BTN_PHONE,         LPGEN("Phone"),                 SECT_BUTTONS, IDI_BTN_PHONE,           0 },
	{ ICO_BTN_FAX,           LPGEN("Fax"),                   SECT_BUTTONS, IDI_BTN_FAX,             0 },
	{ ICO_BTN_CELLULAR,      LPGEN("Cellular"),              SECT_BUTTONS, IDI_BTN_CELLULAR,        0 },
	{ ICO_BTN_CUSTOMPHONE,   LPGEN("Custom phone"),          SECT_BUTTONS, IDI_BTN_CUSTOMPHONE,     0 },
	{ ICO_BTN_EMAIL,         LPGEN("E-mail"),                SECT_BUTTONS, IDI_BTN_EMAIL,           0 },
	{ ICO_BTN_DOWNARROW,     LPGEN("Down arrow"),            SECT_BUTTONS, IDI_BTN_DOWNARROW,       0 },
	{ ICO_BTN_ADD,           LPGEN("Add"),                   SECT_BUTTONS, IDI_BTN_ADD,             0 },
	{ ICO_BTN_EDIT,          LPGEN("Edit"),                  SECT_BUTTONS, IDI_BTN_EDIT,            0 },
	{ ICO_BTN_DELETE,        LPGEN("Delete"),                SECT_BUTTONS, IDI_BTN_DELETE,          0 },
	{ ICO_BTN_SEARCH,        LPGEN("Search"),                SECT_BUTTONS, IDI_SEARCH,              0 },
	{ ICO_BTN_EXIMPORT,      LPGEN("Export/import"),         SECT_BUTTONS, IDI_BTN_EXIMPORT,        0 },
	{ ICO_BTN_BDAY_BACKUP,   LPGEN("Backup birthday"),       SECT_BUTTONS, IDI_BTN_BIRTHDAY_BACKUP, 0 },

	//birthday and anniversary
	{ ICO_RMD_DTB0,          LPGEN("Birthday today"),        SECT_REMIND,  IDI_RMD_DTB0,            0 },
	{ ICO_RMD_DTB1,          LPGEN("Birthday tomorrow"),     SECT_REMIND,  IDI_RMD_DTB1,            0 },
	{ ICO_RMD_DTB2,          LPGEN("Birthday in 2 days"),    SECT_REMIND,  IDI_RMD_DTB2,            0 },
	{ ICO_RMD_DTB3,          LPGEN("Birthday in 3 days"),    SECT_REMIND,  IDI_RMD_DTB3,            0 },
	{ ICO_RMD_DTB4,          LPGEN("Birthday in 4 days"),    SECT_REMIND,  IDI_RMD_DTB4,            0 },
	{ ICO_RMD_DTB5,          LPGEN("Birthday in 5 days"),    SECT_REMIND,  IDI_RMD_DTB5,            0 },
	{ ICO_RMD_DTB6,          LPGEN("Birthday in 6 days"),    SECT_REMIND,  IDI_RMD_DTB6,            0 },
	{ ICO_RMD_DTB7,          LPGEN("Birthday in 7 days"),    SECT_REMIND,  IDI_RMD_DTB7,            0 },
	{ ICO_RMD_DTB8,          LPGEN("Birthday in 8 days"),    SECT_REMIND,  IDI_RMD_DTB8,            0 },
	{ ICO_RMD_DTB9,          LPGEN("Birthday in 9 days"),    SECT_REMIND,  IDI_RMD_DTB9,            0 },
	{ ICO_RMD_DTBX,          LPGEN("Birthday later"),        SECT_REMIND,  IDI_RMD_DTBX,            0 },

	{ ICO_RMD_DTA0,          LPGEN("Anniversary today"),     SECT_REMIND,  IDI_RMD_DTA0,            0 },
	{ ICO_RMD_DTA1,          LPGEN("Anniversary tomorrow"),  SECT_REMIND,  IDI_RMD_DTA1,            0 },
	{ ICO_RMD_DTA2,          LPGEN("Anniversary in 2 days"), SECT_REMIND,  IDI_RMD_DTA2,            0 },
	{ ICO_RMD_DTA3,          LPGEN("Anniversary in 3 days"), SECT_REMIND,  IDI_RMD_DTA3,            0 },
	{ ICO_RMD_DTA4,          LPGEN("Anniversary in 4 days"), SECT_REMIND,  IDI_RMD_DTA4,            0 },
	{ ICO_RMD_DTA5,          LPGEN("Anniversary in 5 days"), SECT_REMIND,  IDI_RMD_DTA5,            0 },
	{ ICO_RMD_DTA6,          LPGEN("Anniversary in 6 days"), SECT_REMIND,  IDI_RMD_DTA6,            0 },
	{ ICO_RMD_DTA7,          LPGEN("Anniversary in 7 days"), SECT_REMIND,  IDI_RMD_DTA7,            0 },
	{ ICO_RMD_DTA8,          LPGEN("Anniversary in 8 days"), SECT_REMIND,  IDI_RMD_DTA8,            0 },
	{ ICO_RMD_DTA9,          LPGEN("Anniversary in 9 days"), SECT_REMIND,  IDI_RMD_DTA9,            0 },
	{ ICO_RMD_DTAX,          LPGEN("Anniversary later"),     SECT_REMIND,  IDI_RMD_DTAX,            0 },
};

/**
 * This function finds the default iconpack file and return its path.
 *
 * @param		- none
 *
 * @return		This function returns the relative path to an existing icon pack.
 **/
LPTSTR IcoLib_GetDefaultIconFileName()
{
	static LPTSTR path[] = {
		_T("Icons\\uinfoex_icons.dll"),
		_T("Plugins\\uinfoex_icons.dll"),
		_T("Customize\\Icons\\uinfoex_icons.dll")
	};
	TCHAR absolute[MAX_PATH];

	for (int i = 0; i < SIZEOF(path); i++) {
		PathToAbsoluteT(path[i], absolute);
		if (PathFileExists(absolute))
			return path[i];
	}
	return NULL;
}

/**
 * This function checks the version of an iconpack.
 * If the icon pack's version differs from the desired one, 
 * dialog with a warning is displayed.
 *
 * @param		szIconPack	- This is the path to the icon pack. 
 *							  It can be absolute or relative.
 *
 * @return	nothing
 **/
static void IcoLib_CheckIconPackVersion(LPTSTR szIconPack)
{
	if (db_get_b(NULL, MODNAME, SET_ICONS_CHECKFILEVERSION, TRUE)) {
		if (szIconPack) {
			TCHAR szAbsolutePath[MAX_PATH];
			PathToAbsoluteT(szIconPack, szAbsolutePath);

			HMODULE hIconDll = LoadLibrary(szAbsolutePath);
			if (hIconDll) {
				CHAR szFileVersion[64];

				if (!LoadStringA(hIconDll, IDS_ICOPACKVERSION, szFileVersion, sizeof(szFileVersion))
					|| mir_strcmp(szFileVersion, "__UserInfoEx_IconPack_1.2__"))
					MsgErr(NULL, LPGENT("Warning: Your current IconPack's version differs from the one UserInfoEx is designed for.\nSome icons may not be displayed correctly"));
				FreeLibrary(hIconDll);
			}
		}
		else
			MsgErr(NULL, LPGENT("Warning: No IconPack found in one of the following directories: 'customize\\icons', 'icons' or 'plugins'!"));
	}
}

/**
 * Set the icon of each control in the list
 *
 * @param	hDlg		- handle to the dialog control, that owns the controls
 * @param	pCtrl		- list to all controls and its icon names
 * @param	numCtrls	- number of elements in the pCtrl list
 *
 * @return	nothing
 **/
void IcoLib_SetCtrlIcons(HWND hDlg, const ICONCTRL *pCtrl, BYTE numCtrls)
{
	for (int i = 0; i < numCtrls; i++) {
		HICON	hIcon = Skin_GetIcon(pCtrl[i].pszIcon);
		if (pCtrl[i].idCtrl) {
			HWND hCtrl = GetDlgItem(hDlg, pCtrl[i].idCtrl);
			switch (pCtrl[i].Message) {
			case STM_SETICON:
			case STM_SETIMAGE:
				ShowWindow(hCtrl, hIcon ? SW_SHOW : SW_HIDE);

			case BM_SETIMAGE:
				SendMessage(hCtrl, pCtrl[i].Message, IMAGE_ICON, (LPARAM)hIcon);
			}
		}
		else
			SendMessage(hDlg, pCtrl[i].Message, ICON_BIG, (LPARAM)hIcon);
	}
}

/**
 * This function manually registers a single icon from the default icon library.
 *
 * @param		szIconID		- This is the uniquely identifying string for an icon. 
 *								  This string is the setting name in the database and should 
 *								  only use ASCII characters.
 * @param		szDescription	- This is the description displayed in the options dialog.
 * @param		szSection		- This is the subsection, where the icon is organized in the options dialog.
 * @param		szDefaultFile	- This is the validated path to the default icon file.
 * @param		idIcon			- This is the ResourceID of the icon in the default file.
 * @param		Size			- This is the desired size of the icon to load.
 *								  0:	default size for small icons (16x16)
 *								  1:	default size for normal icons (32x32)
 * @param		hDefIcon		- This is the default icon to use if the default icon
 *								  file does not exist and no custom icon is set up in the config.
 *
 * @return	This function returns the HANDLE of the icon item.
 **/
static HANDLE IcoLib_RegisterIconHandleEx(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, LPTSTR szDefaultFile, int idIcon, int Size, HICON hDefIcon)
{
	HANDLE hIconHandle = NULL;

	if (szIconID && szDescription && szSection) {
		SKINICONDESC sid = { sizeof(sid) };
		sid.flags = SIDF_ALL_TCHAR;
		sid.pszName = szIconID;
		sid.ptszDescription = mir_a2t(szDescription);
		sid.ptszSection = mir_a2t(szSection);

		if (sid.ptszDescription && sid.ptszSection) {
			switch (Size) {
			// small icons (16x16)
			case 0:
				sid.cx = GetSystemMetrics(SM_CXSMICON);
				sid.cy = GetSystemMetrics(SM_CYSMICON);
				break;

			// normal icons (32x32)
			case 1:
				sid.cx = GetSystemMetrics(SM_CXICON);
				sid.cy = GetSystemMetrics(SM_CYICON);
				break;

			// custom icon size
			default:
				sid.cx = sid.cy = Size;
				break;
			}

			sid.ptszDefaultFile = szDefaultFile;
			if (sid.ptszDefaultFile && sid.ptszDefaultFile[0])
				sid.iDefaultIndex = -idIcon;
			else {
				sid.hDefaultIcon = hDefIcon;
				sid.iDefaultIndex = -1;
			}
			hIconHandle = Skin_AddIcon(&sid);
		}
		MIR_FREE(sid.ptszDescription);
		MIR_FREE(sid.ptszSection);
	}
	return hIconHandle;
}

/**
 * This function manually registers a single icon from the default icon library.
 *
 * @param		szIconID		- This is the uniquely identifying string for an icon. 
 *								  This string is the setting name in the database and should 
 *								  only use ASCII characters.
 * @param		szDescription	- This is the description displayed in the options dialog.
 * @param		szSection		- This is the subsection, where the icon is organized in the options dialog.
 * @param		idIcon			- This is the ResourceID of the icon in the default file
 * @param		Size			- This is the desired size of the icon to load.
 *								  0:	default size for small icons (16x16)
 *								  1:	default size for normal icons (32x32)
 *
 * @return	This function returns the HANDLE of the icon item.
 **/
HANDLE IcoLib_RegisterIconHandle(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, int idIcon, int Size)
{
	return IcoLib_RegisterIconHandleEx(szIconID, szDescription, szSection, IcoLib_GetDefaultIconFileName(), idIcon, Size, ghDefIcon);
}

/**
 * This function manually registers a single icon from the default icon library.
 *
 * @param		szIconID		- This is the uniquely identifying string for an icon. 
 *								  This string is the setting name in the database and should 
 *								  only use ASCII characters.
 * @param		szDescription	- This is the description displayed in the options dialog.
 * @param		szSection		- This is the subsection, where the icon is organized in the options dialog.
 * @param		idIcon			- This is the ResourceID of the icon in the default file
 * @param		Size			- This is the desired size of the icon to load.
 *								  0:	default size for small icons (16x16)
 *								  1:	default size for normal icons (32x32)
 *
 * @return	 This function returns the HICON of the icon itself.
 **/
HICON IcoLib_RegisterIcon(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, int idIcon, int Size)
{
	return Skin_GetIconByHandle(IcoLib_RegisterIconHandle(szIconID, szDescription, szSection, idIcon, Size));
}

/**
 * Add default icons to the skin library or load customized icons
 *
 * @param		none
 *
 * @return		nothing
 **/
static IconItem main = { LPGEN("Main"), ICO_COMMON_MAIN, IDI_MAIN };

void IcoLib_LoadModule()
{
	Icon_Register(ghInst, SECT_COMMON, &main, 1);

	LPTSTR szDefaultFile = IcoLib_GetDefaultIconFileName();
	IcoLib_CheckIconPackVersion(szDefaultFile);

	// load default icon if required
	ghDefIcon = (HICON)LoadImage(ghInst, MAKEINTRESOURCE(IDI_DEFAULT), IMAGE_ICON,  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

	for (int i = 0; i < SIZEOF(icoDesc); i++)
		IcoLib_RegisterIconHandleEx(
			icoDesc[i].pszName, icoDesc[i].pszDesc, icoDesc[i].pszSection, 
			szDefaultFile, icoDesc[i].idResource, icoDesc[i].size, ghDefIcon);
}
