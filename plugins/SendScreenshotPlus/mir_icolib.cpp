/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aLONG with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.

this file is taken from UserinfoEx plugin and support UserinfoEx icon pack !!!!

*/

#include "global.h"


typedef struct _ICODESC 
{
	LPSTR	pszName;
	LPSTR	pszDesc;
	LPSTR	pszSection;
	BOOL	bfromIconPack;
	WORD	idResource;
	BYTE	size;
} ICODESC;

HICON ghDefIcon = NULL;

//IDI_PLUG_MAIN must be the first icon from Plugin.dll, all other icon must be IDI_PLUG_MAIN+n
static ICODESC icoDesc[] = 
{
	// common
	{ ICO_PLUG_SSWINDOW1,	"Screenshot Icon1",			SECT_COMMON,	0,	IDI_PLUG_MAIN,		-1	},
	{ ICO_PLUG_SSWINDOW2,	"Screenshot Icon2",			SECT_COMMON,	0,	IDI_PLUG_ICON1,		0	},
	{ ICO_PLUG_SSTARGET,	"Target Cursor",			SECT_COMMON,	0,	IDI_PLUG_ICON2,		1	},
	{ ICO_PLUG_SSMONITOR,	"Target Desktop",			SECT_COMMON,	0,	IDI_PLUG_ICON3,		1	},
	{ ICO_PLUG_SSDEFAULT,	"Default",					SECT_COMMON,	0,	IDI_PLUG_DEFAULT,	0	},

	// overlays
	{ ICO_PLUG_OVERLAYON,	"overlay on",				SECT_OVERLAY,	0,	IDI_PLUG_OVERLAYON,	0	},
	{ ICO_PLUG_OVERLAYOFF,	"overlay off",				SECT_OVERLAY,	0,	IDI_PLUG_OVERLAYOFF,0	},

	// dialogs
//	{ ICO_DLG_DETAILS,		"Details Infobar",			SECT_DLG,		1,	IDI_DLG_DETAILS,		48	},
//	{ ICO_DLG_PHONE,		"Phone Infobar",			SECT_DLG,		1,	IDI_DLG_PHONE,			1	},
//	{ ICO_DLG_EMAIL,		"E-Mail Infobar",			SECT_DLG,		1,	IDI_DLG_EMAIL,			1	},

	// button icons
	{ ICO_PLUG_SSHELP,		"Help",						SECT_BUTTONS,	0,	IDI_PLUG_HELP,		0	},
	{ ICO_PLUG_SSFOLDERO,	"Open Folder",				SECT_BUTTONS,	0,	IDI_PLUG_FOLDERO,	0	},
	{ ICO_PLUG_SSDESKOFF,	"description off",			SECT_BUTTONS,	0,	IDI_PLUG_DESKOFF,		0	},
	{ ICO_PLUG_SSDESKON,	"description on",			SECT_BUTTONS,	0,	IDI_PLUG_DESKON,		0	},
	{ ICO_PLUG_SSDELOFF,	"delete off",				SECT_BUTTONS,	0,	IDI_PLUG_DELOFF,		0	},
	{ ICO_PLUG_SSDELON,		"delete on",				SECT_BUTTONS,	0,	IDI_PLUG_DELON,			0	},
	{ ICO_PLUG_ARROWL,		"Prev",						SECT_BUTTONS,	0,	IDI_PLUG_ARROWL,		0	},
	{ ICO_PLUG_ARROWR,		"Next",						SECT_BUTTONS,	0,	IDI_PLUG_ARROWR,		0	},

	{ ICO_BTN_UPDATE,		"Update",					SECT_BUTTONS,	1,	IDI_BTN_UPDATE,			0	},
	{ ICO_BTN_OK,			"Ok",						SECT_BUTTONS,	1,	IDI_BTN_OK,				0	},
	{ ICO_BTN_CANCEL,		"Cancel",					SECT_BUTTONS,	1,	IDI_BTN_CLOSE,			0	},
	{ ICO_BTN_APPLY,		"Apply",					SECT_BUTTONS,	1,	IDI_BTN_APPLY,			0	},
//	{ ICO_BTN_GOTO,			"Goto",						SECT_BUTTONS,	1,	IDI_BTN_GOTO,			0	},
//	{ ICO_BTN_EMAIL,		"e-mail",					SECT_BUTTONS,	1,	IDI_BTN_EMAIL,			0	},
//	{ ICO_BTN_DOWNARROW,	"Down arrow",				SECT_BUTTONS,	1,	IDI_BTN_DOWNARROW,		0	},
//	{ ICO_BTN_ADD,			"Add",						SECT_BUTTONS,	1,	IDI_BTN_ADD,			0	},
	{ ICO_BTN_EDIT,			"Edit",						SECT_BUTTONS,	1,	IDI_BTN_EDIT,			0	},
//	{ ICO_BTN_DELETE,		"Delete",					SECT_BUTTONS,	1,	IDI_BTN_DELETE,			0	},
//	{ ICO_BTN_SEARCH,		"Search",					SECT_BUTTONS,	1,	IDI_SEARCH,				0	},
//	{ ICO_BTN_YES,			"Yes",						SECT_BUTTONS,	1,	IDI_BTN_YES,			0	},
//	{ ICO_BTN_NO,			"No",						SECT_BUTTONS,	1,	IDI_BTN_NO,				0	},
//	{ ICO_BTN_IGNORE,		"Ignore",					SECT_BUTTONS,	1,	IDI_BTN_IGNORE,			0	},

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
	static LPTSTR	path[] = {
		_T("Icons\\uinfoex_icons.dll"),
		_T("Plugins\\uinfoex_icons.dll"),
		_T("Customize\\Icons\\uinfoex_icons.dll")
	};
	TCHAR absolute[MAX_PATH];

	for (INT i = 0; i < SIZEOF(path); i++)
	{
		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)path[i], (LPARAM)absolute);
		if (PathFileExists(absolute))
		{
			return path[i];
		}
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
static VOID IcoLib_CheckIconPackVersion(LPTSTR szIconPack)
{
	//if (DB::Setting::GetByte(SET_ICONS_CHECKFILEVERSION, TRUE))
	if (DBGetContactSettingByte(NULL,MODNAME,SET_ICONS_CHECKFILEVERSION, TRUE))
	{
		if (szIconPack)
		{
			TCHAR	szAbsolutePath[MAX_PATH];
			HMODULE	hIconDll;

			CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)szIconPack, (LPARAM)szAbsolutePath);

			hIconDll = LoadLibrary(szAbsolutePath);
			if (hIconDll) 
			{
				CHAR szFileVersion[64];

				if (!LoadStringA(hIconDll, IDS_ICOPACKVERSION, szFileVersion, sizeof(szFileVersion)) ||
						mir_strcmp(szFileVersion, "__UserInfoEx_IconPack_1.2__"))
				{
					MsgErr(NULL, LPGENT("Warning: Your current IconPack's version differs from the one UserInfoEx is designed for.\nSome icons may not be displayed correctly"));
				}
				FreeLibrary(hIconDll);
			}
		}
		else
		{
			MsgErr(NULL, LPGENT("Warning: No IconPack found in one of the following directories: 'customize\\icons', 'icons' or 'plugins'!"));
		}
	}
}

/**
 * Returns a icon, identified by a name
 *
 * @param	pszIcon	- name of the icon
 *
 * @return:	HICON if the icon is loaded, NULL otherwise
 **/
HICON IcoLib_GetIcon(LPCSTR pszIcon, bool big)
{
	return (pszIcon) ? (HICON)CallService(MS_SKIN2_GETICON, (WPARAM)big, (LPARAM) pszIcon) : NULL;
}

/**
 * Returns a icon, identified by a name
 *
 * @param	hIconItem	- this is the pointer to an IconItem structure in icolib.
 *
 * @return:	HICON if the icon is loaded, NULL otherwise
 **/
HICON IcoLib_GetIconByHandle(HANDLE hIconItem, bool big)
{
	return (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, (WPARAM)big, (LPARAM) hIconItem);
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
VOID IcoLib_SetCtrlIcons(HWND hDlg, const ICONCTRL* pCtrl, BYTE numCtrls)
{
	HICON	hIcon;
	BYTE	i;
	HWND	hCtrl;

	for (i = 0; i < numCtrls; i++) 
	{
		hIcon = IcoLib_GetIcon(pCtrl[i].pszIcon);
		if (pCtrl[i].idCtrl)	
		{
			hCtrl = GetDlgItem(hDlg, pCtrl[i].idCtrl);
			switch (pCtrl[i].Message) 
			{
				case STM_SETICON:
				case STM_SETIMAGE:
					{
						ShowWindow(hCtrl, hIcon ? SW_SHOW : SW_HIDE);
					}
				case BM_SETIMAGE:
					{
						SendMessage(hCtrl, pCtrl[i].Message, IMAGE_ICON, (LPARAM) hIcon);
					}
			}
		}
		else
		{
			SendMessage(hDlg, pCtrl[i].Message, ICON_BIG, (LPARAM) hIcon);
		}
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
static HANDLE IcoLib_RegisterIconHandleEx(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, LPTSTR szDefaultFile, INT idIcon, INT Size, HICON hDefIcon)
{
	HANDLE hIconHandle = NULL;

	if (szIconID && szDescription && szSection)
	{
		SKINICONDESC sid;

		ZeroMemory(&sid, sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.flags = SIDF_ALL_TCHAR;
		sid.pszName = szIconID;
		sid.ptszDescription = mir_a2t(szDescription);
		sid.ptszSection = mir_a2t(szSection);

		if (sid.ptszDescription && sid.ptszSection)
		{
			switch (Size)
			{
				// small and big icons
				case -1:
					{
						sid.cx = sid.cy = 0;
						break;
					}
				// small icons (16x16)
				case 0:
					{
						sid.cx = GetSystemMetrics(SM_CXSMICON);
						sid.cy = GetSystemMetrics(SM_CYSMICON);
						break;
					}
				// normal icons (32x32)
				case 1:
					{
						sid.cx = GetSystemMetrics(SM_CXICON);
						sid.cy = GetSystemMetrics(SM_CYICON);
						break;
					}
				// custom icon size
				default:
					{
						sid.cx = sid.cy = Size;
						break;
					}
			}

			sid.ptszDefaultFile = szDefaultFile;
			if (sid.ptszDefaultFile && sid.ptszDefaultFile[0])
			{
				if(idIcon < IDI_FIRST_ICON || idIcon > IDI_LASTICON) {
					// Icon from Plugin.dll
					sid.iDefaultIndex = idIcon - IDI_PLUG_MAIN;
				}
				else{
					//UserinfoEx Icon pack
					sid.iDefaultIndex = ICONINDEX(idIcon);
				}
			}
			else
			{
				sid.hDefaultIcon = hDefIcon;
				sid.iDefaultIndex = -1;
			}
			hIconHandle = Skin_AddIcon(&sid);
		}
		mir_freeAndNil(sid.ptszDescription);
		mir_freeAndNil(sid.ptszSection);
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
HANDLE IcoLib_RegisterIconHandle(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, INT idIcon, INT Size)
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
HICON IcoLib_RegisterIcon(LPSTR szIconID, LPSTR szDescription, LPSTR szSection, INT idIcon, INT Size)
{
	return IcoLib_GetIconByHandle(IcoLib_RegisterIconHandle(szIconID, szDescription, szSection, idIcon, Size));
}

/**
 * Add default icons to the skin library or load customized icons
 *
 * @param		none
 *
 * @return		nothing
 **/
VOID IcoLib_LoadModule()
{
	LPTSTR	szDefaultFile;
	LPTSTR	szPluginFile;
	INT_PTR i;

	// search for default icon file
	szDefaultFile = IcoLib_GetDefaultIconFileName();
	IcoLib_CheckIconPackVersion(szDefaultFile);

	szPluginFile = _T("Plugins\\")_T(__FILENAME);

	// load default icon if required
	ghDefIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_PLUG_DEFAULT), IMAGE_ICON, 
							 GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

	for (i = 0; i < SIZEOF(icoDesc); i++) 
	{	
		IcoLib_RegisterIconHandleEx(
			icoDesc[i].pszName, icoDesc[i].pszDesc, icoDesc[i].pszSection, 
			icoDesc[i].bfromIconPack ? szDefaultFile : szPluginFile, icoDesc[i].idResource, icoDesc[i].size, ghDefIcon);
	}
}

