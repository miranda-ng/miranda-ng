/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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

#define SKINICONDESC_SIZE     sizeof(SKINICONDESC)					  // v0.0.1.0+
#define SKINICONDESC_SIZE_V1  (offsetof(SKINICONDESC, hDefaultIcon))
#define SKINICONDESC_SIZE_V2  (offsetof(SKINICONDESC, cx))			  // v0.0.0.2+
#define SKINICONDESC_SIZE_V3  (offsetof(SKINICONDESC, flags))		  // v0.0.0.3+

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

//
//  Add a icon into options UI
//
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(SKINICONDESC*)sid;
//  returns a handle to the newly added item
//
#define MS_SKIN2_ADDICON "Skin2/Icons/AddIcon"

//
//  Remove a icon from options UI
//
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(char*)pszName
//  WARNING: This will invalidate all HICONs retrieved for specified pszName
//
#define MS_SKIN2_REMOVEICON "Skin2/Icons/RemoveIcon"

//
//  Retrieve HICON with name specified in lParam
//  wParam = (WPARAM)0 - small 1 - big
//  lParam = (LPARAM)(char*)pszName
//  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib
//
#define MS_SKIN2_GETICON "Skin2/Icons/GetIcon"

//
//  Retrieve an icolib handle for icon by name specified in lParam
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(char*)pszName
//
#define MS_SKIN2_GETICONHANDLE "Skin2/Icons/GetIconHandle"

//
//  Retrieve HICON with HANDLE specified in lParam
//  wParam = (WPARAM)0 - small 1 - big
//  lParam = (LPARAM)(HANDLE)hIcoLibIcon
//  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib
//
#define MS_SKIN2_GETICONBYHANDLE "Skin2/Icons/GetIconByHandle"

//
//  Add reference to HICON
//
//  wParam = (WPARAM)HICON
//  lParam = 0 - small 1 - big
//
#define MS_SKIN2_ADDREFICON "Skin2/Icons/AddRef"

//
//  Retrieved HICON is not needed anymore (release reference; this helps optimize GDI usage)
//
//  wParam = (WPARAM)HICON (optional)
//  lParam = (LPARAM)(char*)pszName (optional)  // at least one needs to be specified
//
#define MS_SKIN2_RELEASEICON "Skin2/Icons/ReleaseIcon"
#define MS_SKIN2_RELEASEICONBIG "Skin2/Icons/ReleaseIconBig"

//
//  Check whether HICON is managed by IcoLib
//
//  wParam = (WPARAM)HICON
//  lParam = 0
//
#define MS_SKIN2_ISMANAGEDICON "Skin2/Icons/IsManaged"

//
//  Icons change notification
//
#define ME_SKIN2_ICONSCHANGED "Skin2/IconsChanged"

#endif /* M_ICOLIB_H__ */
