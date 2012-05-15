/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_FLAGS_H__
#define M_FLAGS_H__

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
 #pragma once
#endif

/*
 Country Flags Plugin v0.1.0.3
*/

/* interface id */
#if !defined(MIID_FLAGS)
 #define MIID_FLAGS  {0x88a3b66e,0xc438,0x4381,{0xbc,0x17,0x71,0xd9,0x9d,0x22,0x5f,0x9c}}
#endif

/* Load a country flag icon from the skin library.   v0.1.0.0+
The retrieved icon should be released using MS_SKIN2_RELEASEICON after use.
The country numbers can be retrieved using MS_UTILS_GETCOUNTRYLIST.
Another way to get the country numbers are the CTRY_* constants in winnls.h of WinAPI.
To retrieve the country number from a locale, call GetLocaleInfo().
with LOCALE_ICOUNTRY.
 wParam=countryNumber
 lParam=(BOOL)fReturnHandle (nonzero to to retrieve the icolib handle instead of the icon)
Returns a icon handle (HICON) on success, NULL on error.
*/
#define MS_FLAGS_LOADFLAGICON  "Flags/LoadFlagIcon"

#if !defined(FLAGS_NOHELPERFUNCTIONS)
__inline static HICON LoadFlagIcon(int countryNumber) {
	if(!ServiceExists(MS_FLAGS_LOADFLAGICON)) return NULL;
	return (HICON)CallService(MS_FLAGS_LOADFLAGICON,countryNumber,0);
}
__inline static HANDLE LoadFlagIconHandle(int countryNumber) {
	if(!ServiceExists(MS_FLAGS_LOADFLAGICON)) return NULL;
	return (HICON)CallService(MS_FLAGS_LOADFLAGICON,countryNumber,1);
}
#endif

#define CTRY_UNSPECIFIED  0
#define CTRY_OTHER        9999
#define CTRY_UNKNOWN      0xFFFF

/* Create a merged country flag icon.   v0.1.0.0+
The retrieved icon should be released using DestroyIcon() after use.
 wParam=countryNumberUpper
 lParam=countryNumberLower
Returns a icon handle (HICON) on success, NULL on error.
*/
#define MS_FLAGS_CREATEMERGEDFLAGICON  "Flags/CreateMergedFlagIcon"

/* Get a corresponding country given an (external) IP address.   v0.1.0.0+
The retrieved number can be converted to a normal country name
using MS_UTILS_GETCOUNTRYBYNUMBER.
 wParam=dwExternalIP (same format as used in Netlib)
 lParam=0
Returns a country number on success,
or 0xFFFF on failure (MS_UTILS_GETCOUNTRYBYNUMBER returns "Unknown" for this).
*/
#define MS_FLAGS_IPTOCOUNTRY  "Flags/IpToCountry"

/* Detect the origin country of a contact.   v0.1.0.0+
This uses the contacts's IP first, and falls back on using
CNF_COUNTRY and CNF_COCOUNTRY of contact details.
To get the contact's IP it relies on the db setting
"RealIP" in the proto module.
 wParam=(WPARAM)(HANDLE)hContact
 lParam=0
Returns a country number on success,
or 0xFFFF on failure (MS_UTILS_GETCOUNTRYBYNUMBER returns "Unknown" for this).
*/
#define MS_FLAGS_DETECTCONTACTORIGINCOUNTRY "Flags/DetectContactOriginCountry"
#define MS_FLAGS_GETCONTACTORIGINCOUNTRY "Flags/GetContactOriginCountry"	//for beta version 0.1.1.0

#if !defined(FLAGS_NOSETTINGS) && defined(EXTRA_ICON_ADV2)
#define SETTING_SHOWSTATUSICONFLAG_DEFAULT    1
#define SETTING_SHOWEXTRAIMGFLAG_DEFAULT      1
#define SETTING_EXTRAIMGFLAGCOLUMN_DEFAULT    EXTRA_ICON_ADV2
#define SETTING_USEUNKNOWNFLAG_DEFAULT        1
#define SETTING_USEIPTOCOUNTRY_DEFAULT        1
#endif

#endif // M_FLAGS_H
