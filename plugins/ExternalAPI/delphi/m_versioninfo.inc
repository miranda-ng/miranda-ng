{
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

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

{$IFNDEF M_VERSIONINFO}
{$DEFINE M_VERSIONINFO}

const
{
  Brings up the versioninfo post as configured in the options
  wParam - not used
  lParam - not used
}
  MS_VERSIONINFO_MENU_COMMAND = 'VersionInfo/MenuCommand';

{
  Returns a string containing the versioninfo post
  wParam - (bool) suppress forum style formatting. If true the post won't have forum
           style formatting even if the option is checked in miranda's options.
  lParam - (^PAnsiChar) Pointer to a string that receives the info. Memory is allocated
           using miranda's version of malloc() and you need to use miranda's version
           of free() on it.
  Returns 0 on success.

how to use:
//...
  data:PAnsiChar;
  if GetInfoService(TRUE,LPARAM(@data))=0 then
    //success
}
  MS_VERSIONINFO_GETINFO = 'Versioninfo/GetInfo';

{$ENDIF}
