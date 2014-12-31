/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

extern int hLangpack;

///////////////////////////////////////////////////////////////////////////////
// WARNING: do not use Translate(TS) for p(t)szSection or p(t)szDescription as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".

typedef struct {
  size_t cbSize;
  union {
    char *pszSection;         // section name used to group icons
    TCHAR *ptszSection;       // [TRANSLATED-BY-CORE]
    WCHAR *pwszSection;
  };
  union {
    char *pszDescription;     // description for options dialog
    TCHAR *ptszDescription;   // [TRANSLATED-BY-CORE]
    WCHAR *pwszDescription;
  };
  char *pszName;              // name to refer to icon when playing and in db
  union {
    char *pszDefaultFile;     // default icon file to use
    TCHAR *ptszDefaultFile;
    WCHAR *pwszDefaultFile;
  };
  int  iDefaultIndex;         // index of icon in default file
  HICON hDefaultIcon;         // handle to default icon
  int cx,cy;                  // dimensions of icon (if 0 then standard size icon (big and small options available)
  int flags;                  // combination of SIDF_*
} SKINICONDESC;

#define SIDF_SORTED       0x01    // Icons in section are sorted by name
#define SIDF_UNICODE      0x100   // Section and Description are in UCS-2
#define SIDF_PATH_UNICODE 0x200   // Default File is in UCS-2
#define SIDF_ALL_UNICODE  SIDF_PATH_UNICODE | SIDF_UNICODE

#if defined(_UNICODE)
  #define SIDF_TCHAR      SIDF_UNICODE
  #define SIDF_PATH_TCHAR SIDF_PATH_UNICODE
  #define SIDF_ALL_TCHAR  SIDF_ALL_UNICODE
#else
  #define SIDF_TCHAR      0
  #define SIDF_PATH_TCHAR 0
  #define SIDF_ALL_TCHAR  0
#endif

///////////////////////////////////////////////////////////////////////////////
//  Adds an icon into options UI
//
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(SKINICONDESC*)sid;
//  returns a handle to the newly added item

__forceinline HANDLE Skin_AddIcon(SKINICONDESC* si)
{	return (HANDLE)CallService("Skin2/Icons/AddIcon", hLangpack, (LPARAM)si);
}

///////////////////////////////////////////////////////////////////////////////
//  Removes an icon from options UI
//
//  wParam = (WPARAM)(HANDLE)hIcolib (optional)
//  lParam = (LPARAM)(char*)pszName (optional)
//  at least one needs to be specified

#define MS_SKIN2_REMOVEICON "Skin2/Icons/RemoveIcon"

__forceinline void Skin_RemoveIcon(const char* szIconName)
{	CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)szIconName);
}

__forceinline void Skin_RemoveIconHandle(HANDLE hIcolib)
{	CallService(MS_SKIN2_REMOVEICON, (WPARAM)hIcolib, 0);
}

///////////////////////////////////////////////////////////////////////////////
//  Retrieves HICON with the name specified in lParam
//
//  wParam = (WPARAM)0 - small 1 - big
//  lParam = (LPARAM)(char*)pszName
//  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib

#define MS_SKIN2_GETICON "Skin2/Icons/GetIcon"

#ifdef __cplusplus
__forceinline HICON Skin_GetIcon(const char* szIconName, int size=0)
#else
__forceinline HICON Skin_GetIcon(const char* szIconName, int size)
#endif
{	return (HICON)CallService(MS_SKIN2_GETICON, size, (LPARAM)szIconName);
}

///////////////////////////////////////////////////////////////////////////////
//  Retrieves an icolib handle by the icon's name specified in lParam
//
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(char*)pszName

#define MS_SKIN2_GETICONHANDLE "Skin2/Icons/GetIconHandle"

__forceinline HANDLE Skin_GetIconHandle(const char* szIconName)
{	return (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)szIconName);
}

///////////////////////////////////////////////////////////////////////////////
//  Retrieves HICON with HANDLE specified in lParam
//
//  wParam = (WPARAM)0 - small 1 - big
//  lParam = (LPARAM)(HANDLE)hIcoLibIcon
//  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib

#define MS_SKIN2_GETICONBYHANDLE "Skin2/Icons/GetIconByHandle"

#ifdef __cplusplus
__forceinline HICON Skin_GetIconByHandle(HANDLE hIcolibIcon, int size=0)
#else
__forceinline HICON Skin_GetIconByHandle(HANDLE hIcolibIcon, int size)
#endif
{	return (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, size, (LPARAM)hIcolibIcon);
}

///////////////////////////////////////////////////////////////////////////////
//  Adds a reference to HICON
//
//  wParam = (WPARAM)HICON
//  lParam = 0 - small 1 - big

#define MS_SKIN2_ADDREFICON "Skin2/Icons/AddRef"

///////////////////////////////////////////////////////////////////////////////
//  Retrieved HICON is not needed anymore (releases a reference; thus helps to optimize GDI usage)
//
//  wParam = (WPARAM)HICON (optional)
//  lParam = (LPARAM)(char*)pszName (optional)  // at least one needs to be specified

#define MS_SKIN2_RELEASEICON "Skin2/Icons/ReleaseIcon"
#define MS_SKIN2_RELEASEICONBIG "Skin2/Icons/ReleaseIconBig"

__forceinline void Skin_ReleaseIcon(const char* szIconName)
{	CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)szIconName);
}

#ifdef __cplusplus
__forceinline void Skin_ReleaseIcon(const char* szIconName, int big)
{	CallService((big) ? MS_SKIN2_RELEASEICONBIG : MS_SKIN2_RELEASEICON, 0, (LPARAM)szIconName);
}

__forceinline void Skin_ReleaseIcon(HICON hIcon)
#else
__forceinline void Skin_ReleaseIcon2(HICON hIcon)
#endif
{	CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}

///////////////////////////////////////////////////////////////////////////////
//  Checks whether HICON is managed by IcoLib
//
//  wParam = (WPARAM)HICON
//  lParam = 0

#define MS_SKIN2_ISMANAGEDICON "Skin2/Icons/IsManaged"

///////////////////////////////////////////////////////////////////////////////
//  Icons' change notification

#define ME_SKIN2_ICONSCHANGED "Skin2/IconsChanged"

#endif /* M_ICOLIB_H__ */
