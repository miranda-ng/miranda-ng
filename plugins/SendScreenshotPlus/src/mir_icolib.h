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
#define SECT_COMMON				MODNAME
#define SECT_BUTTONS			MODNAME"/"LPGEN("Buttons")
#define SECT_OVERLAY			MODNAME"/"LPGEN("Overlays")

// icons
#define ICO_COMMON_SSWINDOW1	MODNAME"_plug_SSwindow1"
#define ICO_COMMON_SSWINDOW2	MODNAME"_plug_SSwindow2"
#define ICO_COMMON_SSMONITOR	MODNAME"_plug_SSmonitor"
#define ICO_COMMON_SSDEFAULT	MODNAME"_plug_SSdefault"
#define ICO_COMMON_SSTARGET		MODNAME"_plug_SSTarget"
#define ICO_COMMON_SSHELP		MODNAME"_plug_SSHelp"
#define ICO_COMMON_SSFOLDERO	MODNAME"_plug_SSFolderOpen"
#define ICO_COMMON_ARROWL		MODNAME"_plug_SSArrowL"
#define ICO_COMMON_ARROWR		MODNAME"_plug_SSArrowR"
#define ICO_COMMON_SSDESKOFF	MODNAME"_plug_SSDeskOff"
#define ICO_COMMON_SSDESKON		MODNAME"_plug_SSDeskOn"
#define ICO_COMMON_SSDELOFF		MODNAME"_plug_SSDelOff"
#define ICO_COMMON_SSDELON		MODNAME"_plug_SSDelOn"
#define ICO_DLG_DETAILS			MODNAME"_dlg_details"
#define ICO_BTN_UPDATE			MODNAME"_plug_update"
#define ICO_BTN_OK				MODNAME"_plug_ok"
#define ICO_BTN_CANCEL			MODNAME"_plug_cancel"
//#define ICO_BTN_APPLY			MODNAME"_plug_apply"
#define ICO_BTN_EDIT			MODNAME"_plug_edit"
#define ICO_BTN_EDITON			MODNAME"_plug_editOn"
#define ICO_BTN_COPY			MODNAME"_plug_copy"
#define ICO_BTN_BBC				MODNAME"_plug_bbc"
#define ICO_BTN_BBC2			MODNAME"_plug_bbclnk"
#define ICO_BTN_DOWNARROW		MODNAME"_plug_downarrow"

//#define ICO_BTN_OVERLAYOFF		MODNAME"_plug_SSOverlayOff"
//#define ICO_BTN_OVERLAYON		MODNAME"_plug_SSOverlayOn"


void	IcoLib_LoadModule();

#endif /* _UINFOEX_ICONS_H_INCLUDED_ */
