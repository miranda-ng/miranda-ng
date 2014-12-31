/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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


#ifndef M_XSTATUS_H__
#define M_XSTATUS_H__ 1

/* Custom Status info
 */

#define CSSF_MASK_STATUS    0x0001  // status member valid for set/get
#define CSSF_MASK_NAME      0x0002  // pszName member valid for set/get
#define CSSF_MASK_MESSAGE   0x0004  // pszMessage member valid for set/get
#define CSSF_DISABLE_MENU   0x0020  // disable default custom status menu, wParam = bEnable
#define CSSF_DISABLE_UI     0x0040  // disable default custom status UI, wParam = bEnable
#define CSSF_DEFAULT_NAME   0x0080  // only with CSSF_MASK_NAME and get API to get default custom status name (wParam = status)
#define CSSF_STATUSES_COUNT 0x0100  // returns number of custom statuses in wParam, only get API
#define CSSF_STR_SIZES      0x0200  // returns sizes of custom status name & message (wParam & lParam members) in chars
#define CSSF_UNICODE        0x1000  // strings are in UCS-2

#if defined(_UNICODE)
  #define CSSF_TCHAR  CSSF_UNICODE
#else
  #define CSSF_TCHAR  0
#endif

typedef struct {
  int cbSize;         // size of the structure
  int flags;          // combination of CSSF_*
  int *status;        // custom status id
  union {
	  char *pszName;    // buffer for custom status name
	  TCHAR *ptszName;
	  WCHAR *pwszName;
  };
  union {
	  char *pszMessage; // buffer for custom status message
	  TCHAR *ptszMessage;
	  WCHAR *pwszMessage;
  };
  WPARAM *wParam;     // extra params, see flags
  LPARAM *lParam;
} CUSTOM_STATUS;

// Retrieves custom status details for specified hContact
//wParam = (HANDLE)hContact
//lParam = (CUSTOM_STATUS*)pData  // receives details (members must be prepared)
//return = 0 (for success)
#define PS_GETCUSTOMSTATUSEX "/GetXStatusEx"

// Sets owner current custom status
//wParam = 0                          // reserved
//lParam = (CUSTOM_STATUS*)pData  // contains what to set and new values
//return = 0 (for success)
#define PS_SETCUSTOMSTATUSEX "/SetXStatusEx"

#define LR_BIGICON 0x40

// Retrieves specified custom status icon
//wParam = (int)N  // custom status id (1-32), 0 = my current custom status
//lParam = flags   // use LR_SHARED for shared HICON, LR_BIGICON for 32x32 icon
//return = HICON   // custom status icon (use DestroyIcon to release resources if not LR_SHARED)
#define PS_GETCUSTOMSTATUSICON "/GetXStatusIcon"

// Called from contact list in order to get index of custom status icon in list
// wParam = hContact
// lParam = 0
// return = (int)index of extra contact icon shifted <<16 (the low word will be normal status icon, the high will be xStatus Icon
#define PS_GETADVANCEDSTATUSICON "/GetAdvancedStatusIcon"

#endif // M_XSTATUS_H__
