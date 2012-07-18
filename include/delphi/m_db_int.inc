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

{$IFNDEF M_DB_INT}
{$DEFINE M_DB_INT}

type
  PMIDatabase = ^TMIDatabase;
  TMIDatabase = interface
    procedure SetCacheSafetyMode(val:bool); stdcall;

    function GetContactCount():long; stdcall;
    function FindFirstContact(const szProto:PAnsiChar = NIL):THANDLE; stdcall;
    function FindNextContact(hContact:THANDLE; const szProto:PAnsiChar = NIL):THANDLE; stdcall;

    function DeleteContact(hContact:THANDLE):long; stdcall;
    function AddContact():THANDLE; stdcall;
    function IsDbContact(hContact:THANDLE):bool; stdcall;

    function GetEventCount(hContact:THANDLE):long; stdcall;
    function AddEvent(hContact:THANDLE; dbe:PDBEVENTINFO):THANDLE; stdcall;
    function DeleteEvent(hContact:THANDLE; hDbEvent:THANDLE):bool; stdcall;
    function GetBlobSize(hDbEvent:THANDLE):long; stdcall;
    function GetEvent(hDbEvent:THANDLE; dbe:PDBEVENTINFO):bool; stdcall;
    function MarkEventRead(hContact:THANDLE; hDbEvent:THANDLE):bool; stdcall;
    function GetEventContact(hDbEvent:THANDLE):THANDLE; stdcall;
    function FindFirstEvent(hContact:THANDLE):THANDLE; stdcall;
    function FindFirstUnreadEvent(hContact:THANDLE):THANDLE; stdcall;
    function FindLastEvent(hContact:THANDLE):THANDLE; stdcall;
    function FindNextEvent(hDbEvent:THANDLE):THANDLE; stdcall;
    function FindPrevEvent(hDbEvent:THANDLE):THANDLE; stdcall;

    function EnumModuleNames(pFunc:TDBMODULEENUMPROC; pParam:pointer):bool; stdcall;

    function GetContactSetting      (hContact:THANDLE; dbcgs:PDBCONTACTGETSETTING):bool; stdcall;
    function GetContactSettingStr   (hContact:THANDLE; dbcgs:PDBCONTACTGETSETTING):bool; stdcall;
    function GetContactSettingStatic(hContact:THANDLE; dbcgs:PDBCONTACTGETSETTING):bool; stdcall;
    function FreeVariant(dbv:PDBVARIANT):bool; stdcall;
    function WriteContactSetting (hContact:THANDLE; dbcws:PDBCONTACTWRITESETTING):bool; stdcall;
    function DeleteContactSetting(hContact:THANDLE; dbcgs:PDBCONTACTGETSETTING):bool; stdcall;
    function EnumContactSettings (hContact:THANDLE; dbces:PDBCONTACTENUMSETTINGS):bool; stdcall;
    function SetSettingResident  (bIsResident:bool; const pszSettingName:PAnsiChar):bool; stdcall;
    function EnumResidentSettings(pFunc:TDBMODULEENUMPROC; pParam:pointer):bool; stdcall;
  end;

type
  PDATABASELINK = ^TDATABASELINK;
  TDATABASELINK = record
    cbSize : int;
    {
      returns what the driver can do given the flag
    }
    getCapability : function (flag:int):int; cdecl;
    {
       buf: pointer to a string buffer
       cch: length of buffer
       shortName: if true, the driver should return a short but descriptive name, e.g. "3.xx profile"
       Affect: The database plugin must return a "friendly name" into buf and not exceed cch bytes,
         e.g. "Database driver for 3.xx profiles"
       Returns: 0 on success, non zero on failure
    }
    getFriendlyName : function (buf:TChar; cch:size_t; shortName:int):int; cdecl;
    {
      profile: pointer to a string which contains full path + name
      Affect: The database plugin should create the profile, the filepath will not exist at
        the time of this call, profile will be C:\..\<name>.dat
      Note: Do not prompt the user in anyway about this operation.
      Note: Do not initialise internal data structures at this point!
      Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_
    }
    makeDatabase : function (profile:TChar; error:Pint):int; cdecl;
    {
      profile: [in] a null terminated string to file path of selected profile
      error: [in/out] pointer to an int to set with error if any
      Affect: Ask the database plugin if it supports the given profile, if it does it will
        return 0, if it doesnt return 1, with the error set in error -- EGROKPRF_  can be valid error
        condition, most common error would be [EGROKPRF_UNKHEADER]
      Note: Just because 1 is returned, doesnt mean the profile is not supported, the profile might be damaged
        etc.
      Returns: 0 on success, non zero on failure
    }
    grokHeader : function (profile:TChar; error:Pint):int; cdecl;
    {
      Affect: Tell the database to create all services/hooks that a 3.xx legecy database might support into link
      Returns: 0 on success, nonzero on failure
    }
    Load : function (profile:TChar):PMIDatabase; cdecl;
    {
      Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
      Returns: 0 on success, nonzero on failure
      Note: Unload() might be called even if Load() was never called, wasLoaded is set to 1 if Load() was ever called.
    }
    Unload : function (wasLoaded:int):int; cdecl;
  end;

{$ENDIF}
