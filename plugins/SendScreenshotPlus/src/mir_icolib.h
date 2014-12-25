/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef _UINFOEX_ICONS_H_INCLUDED_
#define _UINFOEX_ICONS_H_INCLUDED_ 1

// sections
#define SECT_COMMON				SZ_SENDSS
#define SECT_BUTTONS			SZ_SENDSS"/"LPGEN("Buttons")
#define SECT_OVERLAY			SZ_SENDSS"/"LPGEN("Overlays")

// icons
#define ICO_COMMON_SSWINDOW1	SZ_SENDSS"_plug_SSwindow1"
#define ICO_COMMON_SSWINDOW2	SZ_SENDSS"_plug_SSwindow2"
#define ICO_COMMON_SSMONITOR	SZ_SENDSS"_plug_SSmonitor"
#define ICO_COMMON_SSDEFAULT	SZ_SENDSS"_plug_SSdefault"
#define ICO_COMMON_SSTARGET		SZ_SENDSS"_plug_SSTarget"
#define ICO_COMMON_SSHELP		SZ_SENDSS"_plug_SSHelp"
#define ICO_COMMON_SSFOLDERO	SZ_SENDSS"_plug_SSFolderOpen"
#define ICO_COMMON_ARROWL		SZ_SENDSS"_plug_SSArrowL"
#define ICO_COMMON_ARROWR		SZ_SENDSS"_plug_SSArrowR"
#define ICO_COMMON_SSDESKOFF	SZ_SENDSS"_plug_SSDeskOff"
#define ICO_COMMON_SSDESKON		SZ_SENDSS"_plug_SSDeskOn"
#define ICO_COMMON_SSDELOFF		SZ_SENDSS"_plug_SSDelOff"
#define ICO_COMMON_SSDELON		SZ_SENDSS"_plug_SSDelOn"
#define ICO_DLG_DETAILS			SZ_SENDSS"_dlg_details"
#define ICO_BTN_UPDATE			SZ_SENDSS"_plug_update"
#define ICO_BTN_OK				SZ_SENDSS"_plug_ok"
#define ICO_BTN_CANCEL			SZ_SENDSS"_plug_cancel"
//#define ICO_BTN_APPLY			SZ_SENDSS"_plug_apply"
#define ICO_BTN_EDIT			SZ_SENDSS"_plug_edit"
#define ICO_BTN_EDITON			SZ_SENDSS"_plug_editOn"
#define ICO_BTN_COPY			SZ_SENDSS"_plug_copy"
#define ICO_BTN_BBC				SZ_SENDSS"_plug_bbc"
#define ICO_BTN_BBC2			SZ_SENDSS"_plug_bbclnk"
#define ICO_BTN_DOWNARROW		SZ_SENDSS"_plug_downarrow"

//#define ICO_BTN_OVERLAYOFF		SZ_SENDSS"_plug_SSOverlayOff"
//#define ICO_BTN_OVERLAYON		SZ_SENDSS"_plug_SSOverlayOn"


void	IcoLib_LoadModule();

#endif /* _UINFOEX_ICONS_H_INCLUDED_ */
