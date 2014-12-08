{
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
}

{$IFNDEF M_FLAGS}
{$DEFINE M_FLAGS}

const
{* Load a country flag icon from the skin library.   v0.1.0.0+
The retrieved icon should be released using MS_SKIN2_RELEASEICON after use.
The country numbers can be retrieved using MS_UTILS_GETCOUNTRYLIST.
Another way to get the country numbers are the CTRY_* constants in winnls.h of WinAPI.
To retrieve the country number from a locale, call GetLocaleInfo().
with LOCALE_ICOUNTRY.
 wParam=countryNumber
 lParam=(BOOL)fReturnHandle (nonzero to to retrieve the icolib handle instead of the icon)
Returns a icon handle (HICON) on success, NULL on error.
*}
  MS_FLAGS_LOADFLAGICON:PAnsiChar = 'Flags/LoadFlagIcon';

  CTRY_UNSPECIFIED = 0;
  CTRY_OTHER       = 9999;
  CTRY_UNKNOWN     = $FFFF;

{* Create a merged country flag icon.   v0.1.0.0+
The retrieved icon should be released using DestroyIcon() after use.
 wParam=countryNumberUpper
 lParam=countryNumberLower
Returns a icon handle (HICON) on success, NULL on error.
*}
  MS_FLAGS_CREATEMERGEDFLAGICON:PAnsiChar = 'Flags/CreateMergedFlagIcon';

{* Get a corresponding country given an (external) IP address.   v0.1.0.0+
The retrieved number can be converted to a normal country name
using MS_UTILS_GETCOUNTRYBYNUMBER.
 wParam=dwExternalIP (same format as used in Netlib)
 lParam=0
Returns a country number on success,
or 0xFFFF on failure (MS_UTILS_GETCOUNTRYBYNUMBER returns "Unknown" for this).
*}
  MS_FLAGS_IPTOCOUNTRY:PAnsiChar = 'Flags/IpToCountry';

{* Detect the origin country of a contact.   v0.1.0.0+
This uses the contacts's IP first, and falls back on using
CNF_COUNTRY and CNF_COCOUNTRY of contact details.
To get the contact's IP it relies on the db setting
"RealIP" in the proto module.
 wParam=(WPARAM)(HANDLE)hContact
 lParam=0
Returns a country number on success,
or 0xFFFF on failure (MS_UTILS_GETCOUNTRYBYNUMBER returns "Unknown" for this).
*}
  MS_FLAGS_DETECTCONTACTORIGINCOUNTRY:PAnsiChar = 'Flags/DetectContactOriginCountry';
  MS_FLAGS_GETCONTACTORIGINCOUNTRY   :PAnsiChar = 'Flags/GetContactOriginCountry';//for beta version 0.1.1.0

  SETTING_SHOWSTATUSICONFLAG_DEFAULT = 1;
  SETTING_SHOWEXTRAIMGFLAG_DEFAULT   = 1;
  SETTING_USEUNKNOWNFLAG_DEFAULT     = 1;
  SETTING_USEIPTOCOUNTRY_DEFAULT     = 1;

{$ENDIF}
