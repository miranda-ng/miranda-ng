{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project, 
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

{$IFNDEF M_DB3XSA}
{$DEFINE M_DB3XSA}

const
{ DB/SetPassword				v0.5.1.3+
  This service is used to set, change or clear the password used for encrypting the profile.
  It opens the password change dialog.
  wParam=0
  lParam=0
}
  MS_DB_SETPASSWORD = 'DB/SetPassword';

{ DB/Backup					v0.5.1.3+
  This service will make a backup of your current profile. Backups are named
  <ProfileName> xx.bak where xx is the number of backups. The larger the number, the
  older the backup.
  wParam=0
  lParam=0
}
  MS_DB_BACKUP = 'DB/Backup';

{ DB/Backup					v0.5.1.3+
  This service is the trigger action service and does the same as the service above.
  Only difference is wParam carries flags from trigger plugin.
  wParam=flags
  lParam=0
}
  MS_DB_BACKUPTRIGGER = 'DB/BackupTriggerAct';

{ DB/GetProfilePath(W)			v0.5.1.5+
  Gets the path of the profile currently being used by the database module. This
  path does not include the last '\'. It is appended with the profile's name if 
  ProfileSubDir=yes is set in the mirandaboot.ini.
    wParam=(WPARAM)(int)cbName
    lParam=(LPARAM)(AnsiChar*)pszName
  pszName is a pointer to the buffer that receives the path of the profile
  cbName is the size in bytes of the pszName buffer
  Returns 0 on success or nonzero otherwise
}
  MS_DB_GETPROFILEPATHW = 'DB/GetProfilePathW';


{ DB/GetProfilePathBasic(W)	v0.5.1.5+
  Gets the path of the profile currently being used by the database module. This
  path does not include the last '\'. This is never appended with the profile's name.
    wParam=(WPARAM)(int)cbName
    lParam=(LPARAM)(AnsiChar*)pszName
  pszName is a pointer to the buffer that receives the path of the profile
  cbName is the size in bytes of the pszName buffer
  Returns 0 on success or nonzero otherwise
}
  MS_DB_GETPROFILEPATH_BASIC  = 'DB/GetProfilePathBasic';
  MS_DB_GETPROFILEPATH_BASICW = 'DB/GetProfilePathBasicW';

  MIRANDAPATH   = '%MIRANDAPATH%';
  MIRANDAPATHW  = '%MIRANDAPATH%';
  PROFILEPATH   = '%PROFILEPATH%';
  PROFILEPATHW  = '%PROFILEPATH%';
  PROFILENAME   = '%PROFILENAME%';
  PROFILENAMEW  = '%PROFILENAME%';

{$ENDIF}