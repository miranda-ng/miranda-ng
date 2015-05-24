{
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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

{$IFNDEF M_FOLDERS}
{$DEFINE M_FOLDERS}

const
  FOLDERS_API = 501; //dunno why it's here but it is :)

  PROFILE_PATH     = '%profile_path%';
  CURRENT_PROFILE  = '%current_profile%';
  MIRANDA_PATH     = '%miranda_path%';
  PLUGINS_PATH     = '%miranda_path%\plugins';
  MIRANDA_USERDATA = '%miranda_userdata%';

  PROFILE_PATHW     = '%profile_path%';
  CURRENT_PROFILEW  = '%current_profile%';
  MIRANDA_PATHW     = '%miranda_path%';
  MIRANDA_USERDATAW = '%miranda_userdata%';
  PLUGINS_PATHW     = '%miranda_path%\plugins';

  FOLDER_AVATARS        = PROFILE_PATH+'\'+CURRENT_PROFILE+'\avatars';
  FOLDER_VCARDS         = PROFILE_PATH+'\'+CURRENT_PROFILE+'\vcards';
  FOLDER_LOGS           = PROFILE_PATH+'\'+CURRENT_PROFILE+'\logs';
  FOLDER_RECEIVED_FILES = PROFILE_PATH+'\'+CURRENT_PROFILE+'\received files';
  FOLDER_DOCS           = MIRANDA_PATH+'\'+'docs';
  
  FOLDER_CONFIG  = PLUGINS_PATH+'\config';
  FOLDER_SCRIPTS = MIRANDA_PATH+'\scripts';
  FOLDER_UPDATES = MIRANDA_PATH+'\updates';

  FOLDER_CUSTOMIZE         = MIRANDA_PATH+'\customize';
  FOLDER_CUSTOMIZE_SOUNDS  = FOLDER_CUSTOMIZE+'\sounds';
  FOLDER_CUSTOMIZE_ICONS   = FOLDER_CUSTOMIZE+'\icons';
  FOLDER_CUSTOMIZE_SMILEYS = FOLDER_CUSTOMIZE+'\smileys';
  FOLDER_CUSTOMIZE_SKINS   = FOLDER_CUSTOMIZE+'\skins';
  FOLDER_CUSTOMIZE_THEMES  = FOLDER_CUSTOMIZE+'\themes';

  FOLDERS_NAME_MAX_SIZE = 64;  //maximum name and section size

  FF_UNICODE = $00000001;

type
  TFOLDERSDATA = record
    cbSize    :integer;   // size of struct
    szSection :PAnsiChar; // section name, if it doesn't exist it will be created
                          // otherwise it will just add this entry to it
    szName    :PAnsiChar; // entry name - will be shown in options
    szFormat  :TCHAR;     // default string format. Fallback string in case
                          // there's no entry in the database for this
                          // folder. This should be the initial value for
                          // the path, users will be able to change it later.
    flags     :dword;     // FF_* flags
    // V2.0
    szUserName:TChar;     // for display purposes. if NULL, plugins gets it as the
                          // translated szName String is dup()'d so you can free it
                          // later. If you set the unicode string don't forget to set
                          // the flag accordingly.
  end;

const
{
  Folders/Register/Path service
  wParam - not used, must be 0
  lParam - (LPARAM) (const FOLDERDATA *) - Data structure filled with
  the necessary information.
  Returns a handle to the registered path or 0 on error.
  You need to use this to call the other services.
}
  MS_FOLDERS_REGISTER_PATH:PAnsiChar = 'Folders/Register/Path';

{
  Folders/Get/PathSize service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (int *) - pointer to the variable that receives the size of the path
  string (not including the null character). Depending on the flags set when creating the path
  it will either call mir_strlen() or mir_wstrlen() to get the length of the string.
  Returns the size of the buffer.
}
  MS_FOLDERS_GET_SIZE:PAnsiChar = 'Folders/Get/PathSize';

type
  TFOLDERSGETDATA = record
    cbSize:integer;
    nMaxPathSize:integer; // maximum size of buffer. This represents the number
                          // of characters that can be copied to it (so for
                          // unicode strings you don't send the number of
                          // bytes but the length of the string).
		szPath:TChar;         // pointer to the buffer that receives the path without the last "\\"
		Flags: dword;         // FF_* flags
  end;

const
{
  Folders/Get/Path service
  wParam - (WPARAM) (int) - handle to registered path
  lParam - (LPARAM) (FOLDERSGETDATA *) pointer to a FOLDERSGETDATA that has all the relevant fields filled.
  Should return 0 on success, or nonzero otherwise.
}
  MS_FOLDERS_GET_PATH:PAnsiChar = 'Folders/Get/Path';

{
  Folders/On/Path/Changed
	wParam - (WPARAM) 0
	lParam - (LPARAM) 0
	Triggered when the folders change, you should reget the paths you registered.
}
  ME_FOLDERS_PATH_CHANGED:PAnsiChar = 'Folders/On/Path/Changed';

{$ENDIF}
