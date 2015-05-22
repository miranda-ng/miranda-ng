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


#ifndef M_PLUGINUPDATER_H__
#define M_PLUGINUPDATER_H__ 1

typedef struct {
	TCHAR *m_fileName;
	DWORD  m_zipCrc;
	char   m_hash[32+1];
	BYTE   m_selected;
}
	PU_HASH_ITEM;

//Parses hashes.zip file at the specified URL and fills the base url buffer.
//later a plugin can use it to address files. %platform% macro is allowed,
//it expands to 32 or 64 depending on a Miranda's platform. For example,
//  http://miranda-ng.org/distr/stable/x%platform%
//
//wParam = (TCHAR*)ptszUrl = url of hashes.zip
//lParam = (TCHAR[260])ptszBaseUrl = decoded base url
//return = (HANDLE)array of hash items or NULL on error

#define MS_PU_PARSEHASHES  "PluginUpdater/ParseHashes"

//Frees the hashes handle
//wParam = 0 (unused)
//lParam = (HANDLE) handle, returned from MS_PU_PARSEHASHES
//return = always returs 0

#define MS_PU_FREEHASHES  "PluginUpdater/FreeHashes"

//Gets number of hashes
//wParam = 0 (unused)
//lParam = (HANDLE) handle, returned from MS_PU_PARSEHASHES
//return = (int)number of hashes in an object

#define MS_PU_GETHASHCOUNT  "PluginUpdater/GetHashCount"

//Gets concrete hash by index
//wParam = (int)zero-based index (0..MS_PU_GETHASHCOUNT-1)
//lParam = (HANDLE) handle, returned from MS_PU_PARSEHASHES
//return = (PU_HASH_ITEM*)pointer to a hash structure

#define MS_PU_GETNTHHASH  "PluginUpdater/GetNthCount"


#define MS_PU_SHOWLIST "PluginUpdater/ShowList"
#define MS_PU_CHECKUPDATES "PluginUpdater/CheckUpdates"
#endif // M_PLUGINUPDATER_H__
