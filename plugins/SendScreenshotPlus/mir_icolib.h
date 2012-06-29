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

*/
#ifndef _UINFOEX_ICONS_H_INCLUDED_
#define _UINFOEX_ICONS_H_INCLUDED_ 1

#include "m_icolib.h"

// sections
#define SECT_COMMON				MODNAME
#define SECT_DLG				MODNAME"/Dialogs"
#define SECT_BUTTONS			MODNAME"/Buttons"
#define SECT_OVERLAY			MODNAME"/overlays"
#define SECT_TOOLBAR			"ToolBar" // global toolbar section as used by modern clist

// icons
#define ICO_PLUG_SSWINDOW1		MODNAME"_plug_SSwindow1"
#define ICO_PLUG_SSWINDOW2		MODNAME"_plug_SSwindow2"
#define ICO_PLUG_SSMONITOR		MODNAME"_plug_SSmonitor"
#define ICO_PLUG_SSDEFAULT		MODNAME"_plug_SSdefault"
#define ICO_PLUG_SSTARGET		MODNAME"_plug_SSTarget"
#define ICO_PLUG_SSHELP			MODNAME"_plug_SSHelp"
#define ICO_PLUG_SSFOLDERO		MODNAME"_plug_SSFolderOpen"
#define ICO_PLUG_ARROWL			MODNAME"_plug_SSArrowL"
#define ICO_PLUG_ARROWR			MODNAME"_plug_SSArrowR"
#define ICO_PLUG_SSDESKOFF		MODNAME"_plug_SSDeskOff"
#define ICO_PLUG_SSDESKON		MODNAME"_plug_SSDeskOn"
#define ICO_PLUG_SSDELOFF		MODNAME"_plug_SSDelOff"
#define ICO_PLUG_SSDELON		MODNAME"_plug_SSDelOn"

#define ICO_PLUG_OVERLAYOFF		MODNAME"_plug_SSOverlayOff"
#define ICO_PLUG_OVERLAYON		MODNAME"_plug_SSOverlayOn"

#define ICO_COMMON_IM			MODNAME"_common_im"
#define ICO_COMMON_FEMALE		MODNAME"_common_female"
#define ICO_COMMON_MALE			MODNAME"_common_male"
#define ICO_COMMON_CLOCK		MODNAME"_common_clock"
#define ICO_COMMON_MARITAL		MODNAME"_common_marital"
#define ICO_COMMON_PASSWORD		MODNAME"_common_password"
#define ICO_COMMON_ADDRESS		MODNAME"_common_address"
#define ICO_TREE_DEFAULT		MODNAME"_tree_default"
#define ICO_DLG_DETAILS			MODNAME"_dlg_details"
#define ICO_DLG_PHONE			MODNAME"_dlg_phone"
#define ICO_DLG_EMAIL			MODNAME"_dlg_email"
#define ICO_DLG_EXPORT			MODNAME"_dlg_export"
#define ICO_DLG_IMPORT			MODNAME"_dlg_import"
#define ICO_DLG_SEARCH			MODNAME"_dlg_search"
#define ICO_LST_MODULES			MODNAME"_lst_modules"
#define ICO_LST_FOLDER			MODNAME"_lst_folder"
#define ICO_BTN_UPDATE			MODNAME"_btn_update"
#define ICO_BTN_OK				MODNAME"_btn_ok"
#define ICO_BTN_CANCEL			MODNAME"_btn_cancel"
#define ICO_BTN_APPLY			MODNAME"_btn_apply"
#define ICO_BTN_GOTO			MODNAME"_btn_goto"
#define ICO_BTN_ADD				MODNAME"_btn_add"
#define ICO_BTN_EDIT			MODNAME"_btn_edit"
#define ICO_BTN_DELETE			MODNAME"_btn_delete"
#define ICO_BTN_IMPORT			MODNAME"_btn_import"
#define ICO_BTN_EXPORT			MODNAME"_btn_export"
#define ICO_BTN_NOTES			MODNAME"_btn_notes"
#define ICO_BTN_ABOUT			MODNAME"_btn_about"
#define ICO_BTN_PROFILE			MODNAME"_btn_profile"
#define ICO_BTN_DOWNARROW		MODNAME"_btn_downarrow"
#define ICO_BTN_PHONE			MODNAME"_btn_phone"
#define ICO_BTN_FAX				MODNAME"_btn_fax"
#define ICO_BTN_CELLULAR		MODNAME"_btn_cellular"
#define ICO_BTN_EMAIL			MODNAME"_btn_email"
#define ICO_BTN_SEARCH			MODNAME"_btn_search"
#define ICO_BTN_YES				MODNAME"_btn_yes"
#define ICO_BTN_NO				MODNAME"_btn_no"
#define ICO_BTN_IGNORE			MODNAME"_btn_ignore"

#define ICO_RMD_DTB0			MODNAME"_rmd_dtb0"
#define ICO_RMD_DTB1			MODNAME"_rmd_dtb1"
#define ICO_RMD_DTB2			MODNAME"_rmd_dtb2"
#define ICO_RMD_DTB3			MODNAME"_rmd_dtb3"
#define ICO_RMD_DTB4			MODNAME"_rmd_dtb4"
#define ICO_RMD_DTB5			MODNAME"_rmd_dtb5"
#define ICO_RMD_DTB6			MODNAME"_rmd_dtb6"
#define ICO_RMD_DTB7			MODNAME"_rmd_dtb7"
#define ICO_RMD_DTB8			MODNAME"_rmd_dtb8"
#define ICO_RMD_DTB9			MODNAME"_rmd_dtb9"
#define ICO_RMD_DTBX			MODNAME"_rmd_dtbx"

#define ICO_RMD_DTA0			MODNAME"_rmd_dta0"
#define ICO_RMD_DTA1			MODNAME"_rmd_dta1"
#define ICO_RMD_DTA2			MODNAME"_rmd_dta2"
#define ICO_RMD_DTA3			MODNAME"_rmd_dta3"
#define ICO_RMD_DTA4			MODNAME"_rmd_dta4"
#define ICO_RMD_DTA5			MODNAME"_rmd_dta5"
#define ICO_RMD_DTA6			MODNAME"_rmd_dta6"
#define ICO_RMD_DTA7			MODNAME"_rmd_dta7"
#define ICO_RMD_DTA8			MODNAME"_rmd_dta8"
#define ICO_RMD_DTA9			MODNAME"_rmd_dta9"
#define ICO_RMD_DTAX			MODNAME"_rmd_dtax"

#define SET_ICONS_CHECKFILEVERSION		"CheckIconPackVersion"
#define SET_ICONS_BUTTONS				"ButtonIcons"

#define ICONINDEX(id)	max((min((id), IDI_LASTICON)) - IDI_FIRST_ICON, 0)

typedef struct TIconCtrl 
{
	LPCSTR	pszIcon;
	UINT	Message;
	WORD	idCtrl;
} ICONCTRL, *LPICONCTRL;

LPTSTR	IcoLib_GetDefaultIconFileName();
VOID	IcoLib_SetCtrlIcons(HWND hDlg, const ICONCTRL* pCtrl, BYTE numCtrls);

HANDLE	IcoLib_RegisterIconHandle(LPSTR szName, LPSTR szDescription, LPSTR szSection, INT idIcon, INT Size);
HICON	IcoLib_RegisterIcon(LPSTR szName, LPSTR szDescription, LPSTR szSection, INT idIcon, INT Size);
HICON	IcoLib_GetIcon(LPCSTR pszIcon, bool big = false);
HICON	IcoLib_GetIconByHandle(HANDLE hIconItem, bool big = false);

VOID	IcoLib_LoadModule();

#endif /* _UINFOEX_ICONS_H_INCLUDED_ */
