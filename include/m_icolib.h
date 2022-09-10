/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_ICOLIB_H__
#define M_ICOLIB_H__ 1

#ifndef M_CORE_H__
#include <m_core.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// WARNING: do not use Translate(TS) for p(t)szSection or p(t)szDescription as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".

#define SIDF_SORTED       0x01    // Icons in section are sorted by name
#define SIDF_UNICODE      0x100   // Section and Description are in UCS-2
#define SIDF_PATH_UNICODE 0x200   // Default File is in UCS-2
#define SIDF_ALL_UNICODE  SIDF_PATH_UNICODE | SIDF_UNICODE

struct SKINICONDESC
{
	MAllStrings section;       // section name used to group icons
	MAllStrings description;   // description for options dialog
	char       *pszName;       // name to refer to icon when playing and in db
	MAllStrings defaultFile;   // default icon file to use
	int         iDefaultIndex; // index of icon in default file
	HICON       hDefaultIcon;  // handle to default icon
	int         cx, cy;        // dimensions of icon (if 0 then standard size icon (big and small options available)
	int         flags;         // combination of SIDF_*
};

#if defined(__cplusplus)
extern "C"
{
#endif

///////////////////////////////////////////////////////////////////////////////
// Adds an icon into the icon library
// returns a handle to the newly added item

MIR_APP_DLL(HANDLE) IcoLib_AddIcon(const SKINICONDESC *sid, HPLUGIN pPlugin);

///////////////////////////////////////////////////////////////////////////////
// Removes an icon from icon library by icon's name or handle

MIR_APP_DLL(void)   IcoLib_RemoveIcon(const char *pszIconName);
MIR_APP_DLL(void)   IcoLib_RemoveIconByHandle(HANDLE hItem);

///////////////////////////////////////////////////////////////////////////////
// Retrieves HICON with the name specified in lParam
// Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib

MIR_APP_DLL(HICON)  IcoLib_GetIcon(const char *pszIconName, bool big = false);
MIR_APP_DLL(HICON)  IcoLib_GetIconByHandle(HANDLE hItem, bool big = false);

///////////////////////////////////////////////////////////////////////////////
// Retrieves an icolib handle by the icon's name specified in lParam

MIR_APP_DLL(HANDLE) IcoLib_GetIconHandle(const char *pszIconName);

///////////////////////////////////////////////////////////////////////////////
// Adds 1 to an icon's ref counter. prevents an icon from being unloaded

MIR_APP_DLL(int)    IcoLib_AddRef(HICON hIcon);

///////////////////////////////////////////////////////////////////////////////
// Retrieved HICON is not needed anymore (releases a reference; thus helps to optimize GDI usage)

MIR_APP_DLL(int)    IcoLib_Release(const char *pszIconName, bool big = false);
MIR_APP_DLL(int)    IcoLib_ReleaseIcon(HICON hIcon, bool big = false);

///////////////////////////////////////////////////////////////////////////////
// Checks whether HICON is managed by IcoLib

MIR_APP_DLL(HANDLE) IcoLib_IsManaged(HICON hIcon);

///////////////////////////////////////////////////////////////////////////////
// Helper to apply an icolib's icon to a button

MIR_APP_DLL(void)   Button_SetIcon_IcoLib(MWindow hDlg, int itemId, HANDLE hIcolib, const char *tooltip = nullptr);
MIR_APP_DLL(void)   Button_SetSkin_IcoLib(MWindow hDlg, int itemId, int skinIconId, const char *tooltip = nullptr);
MIR_APP_DLL(void)   Button_FreeIcon_IcoLib(MWindow hDlg, int itemId);

///////////////////////////////////////////////////////////////////////////////
// Helper to apply an icolib's icon to a window

MIR_APP_DLL(void)   Window_SetIcon_IcoLib(MWindow hWnd, HANDLE hIcolib);
MIR_APP_DLL(void)   Window_SetSkinIcon_IcoLib(MWindow hWnd, int iconId);
MIR_APP_DLL(void)   Window_SetProtoIcon_IcoLib(MWindow hWnd, const char *szProto, int iconId);
MIR_APP_DLL(void)   Window_FreeIcon_IcoLib(MWindow hWnd);

///////////////////////////////////////////////////////////////////////////////
// Helper to add an icolib's icon to a HIMAGELIST

MIR_APP_DLL(int) ImageList_AddSkinIcon(HIMAGELIST hIml, int iconId);
MIR_APP_DLL(int) ImageList_AddProtoIcon(HIMAGELIST hIml, const char *szProto, int iconId);

///////////////////////////////////////////////////////////////////////////////
//  Icons' change notification event

#define ME_SKIN_ICONSCHANGED "Skin/IconsChanged"

#if defined(__cplusplus)
}
#endif

#endif /* M_ICOLIB_H__ */
