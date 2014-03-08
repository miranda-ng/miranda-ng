{
Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
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
}


{$IFNDEF M_PLUGINUPDATER}
{$DEFINE M_PLUGINUPDATER}

type
  PPUHashItem = ^TPUHashItem;
  TPUHashItem = record
    fileName :PWideChar;             // file name
    fileCrc  :dword;                 // zip's crc
    hash     :array [0..32] of char; // file's md5 hash
    selected :byte;
  end;

const
{
  Parses hashes.zip file at the specified URL and fills the base url buffer.
  later a plugin can use it to address files. %platform% macro is allowed,
  it expands to 32 or 64 depending on a Miranda's platform. For example,
    http://miranda-ng.org/distr/stable/x%platform%

  wParam = (PWideChar)ptszUrl = url of hashes.zip
  lParam = (array [1..260] of PWideChar)ptszBaseUrl = decoded base url
  return = (THANDLE)array of hash items or NULL on error
}
  MS_PU_PARSEHASHES = 'PluginUpdater/ParseHashes';

{
  Frees the hashes handle
  wParam = 0 (unused)
  lParam = (THANDLE) handle, returned from MS_PU_PARSEHASHES
  return = always returs 0
}
  MS_PU_FREEHASHES = 'PluginUpdater/FreeHashes';

{
  Gets number of hashes
  wParam = 0 (unused)
  lParam = (THANDLE) handle, returned from MS_PU_PARSEHASHES
  return = (int)number of hashes in an object
}
  MS_PU_GETHASHCOUNT = 'PluginUpdater/GetHashCount';

{
  Gets concrete hash by index
  wParam = (int)zero-based index (0..MS_PU_GETHASHCOUNT-1)
  lParam = (THANDLE) handle, returned from MS_PU_PARSEHASHES
  return = (PU_HASH_ITEM*)pointer to a hash structure
}
  MS_PU_GETNTHHASH = 'PluginUpdater/GetNthCount';

{$ENDIF}
