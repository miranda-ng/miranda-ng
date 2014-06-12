/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-07 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* DB/SetPassword				v0.5.1.3+
This service is used to set, change or clear the password used for encrypting the profile.
It opens the password change dialog.
wParam=0
lParam=0
*/
#define MS_DB_SETPASSWORD		"DB/SetPassword"

/* DB/Backup					v0.5.1.3+
This service will make a backup of your current profile. Backups are named
<ProfileName> xx.bak where xx is the number of backups. The larger the number, the
older the backup.
wParam=0
lParam=0
*/
#define MS_DB_BACKUP			"DB/Backup"

/* DB/Backup					v0.5.1.3+
This service is the trigger action service and does the same as the service above.
Only difference is wParam carries flags from trigger plugin.
wParam=flags
lParam=0
*/
#define MS_DB_BACKUPTRIGGER		"DB/BackupTriggerAct"

/* DB/GetProfilePath( W )			v0.5.1.5+
Gets the path of the profile currently being used by the database module. This
path does not include the last '\'. It is appended with the profile's name if
ProfileSubDir=yes is set in the mirandaboot.ini.
  wParam=( WPARAM )( int )cbName
  lParam=( LPARAM )( char* )pszName
pszName is a pointer to the buffer that receives the path of the profile
cbName is the size in bytes of the pszName buffer
Returns 0 on success or nonzero otherwise
*/
#define MS_DB_GETPROFILEPATHW  "DB/GetProfilePathW"


/* DB/GetProfilePathBasic( W )	v0.5.1.5+
Gets the path of the profile currently being used by the database module. This
path does not include the last '\'. This is never appended with the profile's name.
  wParam=( WPARAM )( int )cbName
  lParam=( LPARAM )( char* )pszName
pszName is a pointer to the buffer that receives the path of the profile
cbName is the size in bytes of the pszName buffer
Returns 0 on success or nonzero otherwise
*/
#define MS_DB_GETPROFILEPATH_BASIC	"DB/GetProfilePathBasic"
#define	MS_DB_GETPROFILEPATH_BASICW	"DB/GetProfilePathBasicW"

/* Utils/PathToAbsolute( W )		v0.5.1.5+
This service routine expands all environment variables of a path string.
It supports:
%MIRANDAPATH%	- expands to the installation folder of miranda,
%PROFILEPATH%	- expands to the folder the current profile is stored in,
%PROFILENAME%	- expands to the name of the current profile,
and all windows like environment variables such as:
%USERPROFILE%, %WINDIR%, ...
It returns the length of the absolute path in characters on success or
0 if any of the environment variables was not translated.
wParam=relative source path ( must be smaller than MAX_PATH )
lParam=absolute destination path ( must be larger or equal to MAX_PATH )
*/
#define MIRANDAPATH		"%MIRANDAPATH%"
#define MIRANDAPATHW	L"%MIRANDAPATH%"
#define PROFILEPATH		"%PROFILEPATH%"
#define PROFILEPATHW	L"%PROFILEPATH%"
#define PROFILENAME		"%PROFILENAME%"
#define PROFILENAMEW	L"%PROFILENAME%"

#ifndef MS_UTILS_PATHTOABSOLUTE
 #define MS_UTILS_PATHTOABSOLUTE		"Utils/PathToAbsolute"
#endif
#define MS_UTILS_PATHTOABSOLUTEW		"Utils/PathToAbsoluteW"

/* Utils/PathToRelative( W )		v0.5.1.5+
This service parses the given absolute path and translates it to a string
containing as much environment variables as possible.
The return value is the length of the relative path in characters.
wParam=absolute source path ( must be smaller than MAX_PATH )
lParam=relative destination path ( must be larger or equal to MAX_PATH )
*/
#ifndef MS_UTILS_PATHTORELATIVE
 #define MS_UTILS_PATHTORELATIVE		"Utils/PathToRelative"
#endif
#define MS_UTILS_PATHTORELATIVEW		"Utils/PathToRelativeW"

/* Unicode/Multibyte wrapping via TCHAR
*/
#ifdef _UNICODE
 #define MS_UTILS_PATHTOABSOLUTET	MS_UTILS_PATHTOABSOLUTEW
 #define MS_UTILS_PATHTORELATIVET	MS_UTILS_PATHTORELATIVEW
#else
 #define MS_UTILS_PATHTOABSOLUTET	MS_UTILS_PATHTOABSOLUTE
 #define MS_UTILS_PATHTORELATIVET	MS_UTILS_PATHTORELATIVE
#endif