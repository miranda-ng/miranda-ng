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

// sections
#define SECT_COMMON				MODNAME
#define SECT_BUTTONS			MODNAME"/"LPGEN("Buttons")
#define SECT_OVERLAY			MODNAME"/"LPGEN("overlays")

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
#define ICO_PLUG_UPDATE			MODNAME"_plug_update"
#define ICO_PLUG_OK				MODNAME"_plug_ok"
#define ICO_PLUG_CANCEL			MODNAME"_plug_cancel"
#define ICO_PLUG_APPLY			MODNAME"_plug_apply"
#define ICO_PLUG_EDIT			MODNAME"_plug_edit"
#define ICO_PLUG_DOWNARROW		MODNAME"_plug_downarrow"

#define ICO_PLUG_OVERLAYOFF		MODNAME"_plug_SSOverlayOff"
#define ICO_PLUG_OVERLAYON		MODNAME"_plug_SSOverlayOn"

#define ICO_DLG_DETAILS			MODNAME"_dlg_details"

/*typedef struct TIconCtrl 
{
	LPCSTR	pszIcon;
	UINT	Message;
	WORD	idCtrl;
} ICONCTRL, *LPICONCTRL;*/

HICON	IcoLib_GetIcon(LPCSTR pszIcon, bool big = false);

VOID	IcoLib_LoadModule();

#endif /* _UINFOEX_ICONS_H_INCLUDED_ */
