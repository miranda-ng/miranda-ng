{

'File Association Manager'-Plugin for
Miranda IM: the free IM client for Microsoft* Windows*

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_ASSOCMGR}
{$DEFINE M_ASSOCMGR}

const

{
 File Association Manager v0.1.0.3
}

{ interface id }
  MIID_ASSOCMGR: TGUID = '{A05B56C0-CF7B-4389-A1E9-F13DB9360EF1}';

{ Add a new file type   v0.1.0.0+
Add a new file type to be registered with Windows.
You probably want to call this event when
ME_SYSTEM_MODULESLOADED is fired.
 wParam : 0
 lParam : (PFILETYPEDESC)ftd
Returns 0 on success, nonzero otherwise.
}
  MS_ASSOCMGR_ADDNEWFILETYPE = 'AssocMgr/AddNewFileType';

type
  PFILETYPEDESC = ^TFILETYPEDESC;
  TFILETYPEDESC = record
    cbSize: integer;        // size of this structure, in bytes

    szDescription: TChar; // description for options dialog and in registry.
                            // please Translate().

    hInstance: HINST;       // instance where the icon resource is located

    nIconResID: uint;       // resource id of an icon to use for the file type.
                            // this icon should contain icons of all sizes and color depths
                            // needed by Windows.
                            // set this to 0 to use the generic 'miranda file' icon
                            // provided by assocmgr.

    pszService: PAnsiChar;  // service to call when a file is opened
                            // this service will be called with lParam set to
                            // the file name being opened including path.
                            // it can be assumed that the provided file name 
                            // is always the long path name.
                            // return zero on suceess, nonzero on error.
                            // Note: set this to nil to pass the file name as
                            // commandline argument to miranda32.exe (db file).

    flags: dword;           // see FTDF_* flags below

    pszFileExt: PAnsiChar;  // file extension, e.g. ".ext"
                            // first character must be a dot, assumed to be all lower case.
                            // may only consist of ascii characters.

    pszMimeType: PAnsiChar; // MIME type of the file, e.g. "application/x-icq"
                            // may only consist of ascii characters.

    szVerbDesc: TChar;      // description for the open verb e.g. "&Install".
                            // set this to nil to use the default description "Open".
                            // include an ampersand (&) character for a mnemonic key.
                            // please Translate().
  end;

const
  FTDF_UNICODE         = $0001; // pszDescription and pszVerbDesc in struct are Unicode.
                                // the specified service is called with Unicode parameters.

  FTDF_DEFAULTDISABLED = $0002; // file type is not registered by default, it needs to be
                                // enabled explicitly on the options page.

  FTDF_BROWSERAUTOOPEN = $0004; // tells the browser to download and open the file directly
                                // without prompt (currently IE and Opera6+) - be careful!
                                // use only in conjunction with pszMimeType set.
                                // this tells Windows that open can be safely invoked for
                                // downloaded files.
                                // Note that this flag may create a security risk,
                                // because downloaded files could contain malicious content.
                                // you need to protect against such an exploit.

  FTDF_ISTEXT          = $0008; // tells Windows that this file can be opened
                                // as a text file using e.g Notepad.
                                // only has an effect on Windows XP and higher.

 FTDF_ISSHORTCUT       = $0010; // file type behaves as shortcut, this means a
                                // small overlay arrow is applied and the extension is never shown


{ Remove a file type   v0.1.0.0+
Remove a file type registered previously using
MS_ASSOCMGR_ADDNEWFILETYPE.
This removes all settings in database and in registry
associated with the file type.
 wParam : 0
 lParam : (PAnsiChar)pszFileExt
Returns 0 on success, nonzero otherwise.
}
  MS_ASSOCMGR_REMOVEFILETYPE = 'AssocMgr/RemoveFileType';

{ Add a new url protocol type   v0.1.0.0+
Add a new url type to be registered with Windows.
You probably want to call this event when
ME_SYSTEM_MODULESLOADED is fired.
 wParam : 0
 lParam : (PURLTYPEDESC)utd
Returns 0 on success, nonzero otherwise.
}
  MS_ASSOCMGR_ADDNEWURLTYPE = 'AssocMgr/AddNewUrlType';

type
  PURLTYPEDESC = ^TURLTYPEDESC;
  TURLTYPEDESC = record
    cbSize: integer;           // size of this structure, in bytes

    pszDescription: TChar;     // description for options dialog and in registry.
                               // please Translate().

    hInstance: HINST;          // instance where the icon resource is located

    nIconResID: uint;          // resource id of an icon to use for the url type.
                               // only a small one (16x16) is needed by Windows,
                               // e.g. proto icon as used in Miranda.
                               // set this to 0 to use the default miranda icon.

    pszService: PAnsiChar;     // service to call when a url is opened (can't be nil)
                               // this service will be called with lParam set to
                               // the url being opened including the prefix.
                               // the provided string has already been urldecoded.
                               // return zero on suceess, nonzero on error.

    flags: dword;              // see UTDF_* flags below

    pszProtoPrefix: PAnsiChar; // protocol prefix, e.g. "http:"
                               // last character must be a colon, assumed to be all lower case.
                               // may only consist of ascii characters.
  end;

const
  UTDF_UNICODE         = $0001; // pszDescription in struct is Unicode.
                                // the specified service is called with Unicode parameters.

  UTDF_DEFAULTDISABLED = $0002; // url type is not registered by default, it needs to be
                                // enabled explicitly on the options page.

{ Remove an url protocol type   v0.1.0.0+
Remove an url registered previously using
MS_ASSOCMGR_ADDNEWURLTYPE.
This removes all settings in database and in registry
associated with the url type.
 wParam : 0
 lParam : (PAnsiChar)pszProtoPrefix
Returns 0 on success, nonzero otherwise.
}
  MS_ASSOCMGR_REMOVEURLTYPE = 'AssocMgr/RemoveUrlType';

{$ENDIF}
