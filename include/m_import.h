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

#ifndef M_IMPORT_H__
#define M_IMPORT_H__ 1

// launches the import wizard with given file name & options
// always returns 0 

// Custom import options
#define IOPT_ADDUNKNOWN    0x00000001
#define IOPT_MSGSENT       0x00000002
#define IOPT_MSGRECV       0x00000004
#define IOPT_AUTHREQ       0x00000020
#define IOPT_ADDED         0x00000040
#define IOPT_FILESENT      0x00000080
#define IOPT_FILERECV      0x00000100
#define IOPT_OTHERSENT     0x00000200
#define IOPT_OTHERRECV     0x00000400
#define IOPT_HISTORY 		0x000007FE

#define IOPT_SYSTEM        0x00000800
#define IOPT_CONTACTS      0x00001000
#define IOPT_GROUPS        0x00002000
#define IOPT_SYS_SETTINGS  0x00004000
#define IOPT_COMPLETE      0x00007FFE

#define IOPT_CHECKDUPS     0x00010000

struct MImportOptions
{
	const wchar_t *pwszFileName;
	uint32_t dwFlags; // IOPT_* flags combination
};

#define MS_IMPORT_RUN "Import/Run"

#endif // M_IMPORT_H__
