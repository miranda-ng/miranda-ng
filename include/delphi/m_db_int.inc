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

///////////////////////////////////////////////////////////////////////////////
// basic database interface
type
  TDBCachedGlobalValue = record
    name : PAnsiChar;
    value:TDBVARIANT;
  end;
type
  PDBCachedContactValue = ^TDBCachedContactValue;
  TDBCachedContactValue = record
    name:PAnsiChar;
    value:TDBVARIANT;
    next: PDBCachedContactValue;
  end;

  PDBCachedContact = ^TDBCachedContact;
  TDBCachedContact = record
    contactID:TMCONTACT;
    dwDriverData:dword;
    szProto:PAnsiChar;
    first  :PDBCachedContactValue;
    last   :PDBCachedContactValue;
    // metacontacts
    nSubs:int;          // == -1 -> not a metacontact
    pSubs:PMCONTACT;
    parentID:TMCONTACT; // == 0 -> not a subcontact
    nDefault:int        // default sub number
  end;

  PMIDatabaseCache = ^MIDatabaseCache;
  MIDatabaseCache = interface
    function AddContactToCache(contactID:TMCONTACT):PDBCachedContact; stdcall;
    function GetCachedContact(contactID:TMCONTACT):PDBCachedContact; stdcall;
    function GetFirstContact():PDBCachedContact; stdcall;
    function GetNextContact(contactID:TMCONTACT):PDBCachedContact; stdcall;
    procedure FreeCachedContact(contactID:TMCONTACT); stdcall;

    function InsertCachedSetting(szName:PAnsiChar; param:int):PAnsiChar; stdcall;
    function GetCachedSetting(szModuleName:PAnsiChar; szSettingName:PAnsiChar; param1:int; param2:int):PAnsiChar; stdcall;
    procedure SetCachedVariant(s:PDBVARIANT; d:PDBVARIANT); stdcall;
    function GetCachedValuePtr(contactID:TMCONTACT; szSetting:PAnsiChar; bAllocate:int):PDBVARIANT; stdcall;
  end;

type
  PMIDatabase = ^TMIDatabase;
  TMIDatabase = record
    m_cache: PMIDatabaseCache;

    SetCacheSafetyMode:procedure(val:bool); stdcall;

    GetContactCount:function():long; stdcall;
    FindFirstContact:function(const szProto:PAnsiChar = NIL):TMCONTACT; stdcall;
    FindNextContact:function(contactID:TMCONTACT; const szProto:PAnsiChar = NIL):TMCONTACT; stdcall;

    DeleteContact:function(contactID:TMCONTACT):long; stdcall;
    AddContact:function():TMCONTACT; stdcall;
    IsDbContact:function(contactID:TMCONTACT):bool; stdcall;

    GetEventCount:function(contactID:TMCONTACT):long; stdcall;
    AddEvent:function(contactID:TMCONTACT; dbe:PDBEVENTINFO):THANDLE; stdcall;
    DeleteEvent:function(contactID:TMCONTACT; hDbEvent:THANDLE):bool; stdcall;
    GetBlobSize:function(hDbEvent:THANDLE):long; stdcall;
    GetEvent:function(hDbEvent:THANDLE; dbe:PDBEVENTINFO):bool; stdcall;
    MarkEventRead:function(contactID:TMCONTACT; hDbEvent:THANDLE):bool; stdcall;
    GetEventContact:function(hDbEvent:THANDLE):TMCONTACT; stdcall;
    FindFirstEvent:function(contactID:TMCONTACT):THANDLE; stdcall;
    FindFirstUnreadEvent:function(contactID:TMCONTACT):THANDLE; stdcall;
    FindLastEvent:function(contactID:TMCONTACT):THANDLE; stdcall;
    FindNextEvent:function(contactID:TMCONTACT;hDbEvent:THANDLE):THANDLE; stdcall;
    FindPrevEvent:function(contactID:TMCONTACT;hDbEvent:THANDLE):THANDLE; stdcall;

    EnumModuleNames:function(pFunc:TDBMODULEENUMPROC; pParam:pointer):bool; stdcall;

    GetContactSetting:function(contactID:TMCONTACT; szModule, szSetting:PAnsiChar; dbv:PDBVARIANT):bool; stdcall;
    GetContactSettingStr:function(contactID:TMCONTACT; szModule, szSetting:PAnsiChar; dbv:PDBVARIANT):bool; stdcall;
    GetContactSettingStatic:function(contactID:TMCONTACT; szModule, szSetting:PAnsiChar; dbv:PDBVARIANT):bool; stdcall;
    FreeVariant:function(dbv:PDBVARIANT):bool; stdcall;
    WriteContactSetting:function(contactID:TMCONTACT; dbcws:PDBCONTACTWRITESETTING):bool; stdcall;
    DeleteContactSetting:function(contactID:TMCONTACT; szModule, szSetting:PAnsiChar):bool; stdcall;
    EnumContactSettings:function(contactID:TMCONTACT; dbces:PDBCONTACTENUMSETTINGS):bool; stdcall;
    SetSettingResident:function(bIsResident:bool; const pszSettingName:PAnsiChar):bool; stdcall;
    EnumResidentSettings:function(pFunc:TDBMODULEENUMPROC; pParam:pointer):bool; stdcall;
    IsSettingEncrypted:function(szModule:PAnsiChar; szSetting:PAnsiChar):bool; stdcall;

    MetaDetouchSub:function(contact:PDBCachedContact; nSub:int):bool; stdcall;
    MetaSetDefault:function(contact:PDBCachedContact):bool; stdcall;
    MetaMergeHistory:function(ccMeta:PDBCachedContact; ccSub:PDBCachedContact):bool; stdcall;
    MetaSplitHistory:function(ccMeta:PDBCachedContact; ccSub:PDBCachedContact):bool; stdcall;
  end;

///////////////////////////////////////////////////////////////////////////////
// basic database checker interface

const
  STATUS_MESSAGE = 0;
  STATUS_WARNING = 1;
  STATUS_ERROR   = 2;
  STATUS_FATAL   = 3;
  STATUS_SUCCESS = 4;

type
  pDBCHeckCallback = ^tDBCHeckCallback;
  tDBCHeckCallback = record
    cbSize:int;
    spaceProcessed,
    spaceUsed: dword;
    hOutFile:THANDLE;
    bCheckOnly,
    bBackup,
    bAggressive,
    bEraseHistory,
    bMarkRead,
    bConvertUtf:int;
    pfnAddLogMessage: procedure(_type:int; const szFormat:PWideChar{;...}); cdecl;
  end;

type
  PMIDatabaseChecker = ^MIDatabaseChecker;
  MIDatabaseChecker = interface
	  function Start(callback:PDBCHeckCallback):bool;stdcall;
    function CheckDb(phase:int; firstTime:int):bool; stdcall;
    procedure Destroy(); stdcall;
  end;

///////////////////////////////////////////////////////////////////////////////
// Each database plugin should register itself using this structure


{
 Codes for DATABASELINK functions
}
const
// grokHeader() error codes
  EGROKPRF_NOERROR   = 0;
  EGROKPRF_CANTREAD  = 1; // can't open the profile for reading
  EGROKPRF_UNKHEADER = 2; // header not supported, not a supported profile
  EGROKPRF_VERNEWER  = 3; // header correct, version in profile newer than reader/writer
  EGROKPRF_DAMAGED   = 4; // header/version fine, other internal data missing, damaged.
  EGROKPRF_OBSOLETE  = 5; // obsolete database version detected, requiring conversion 

// makeDatabase() error codes
  EMKPRF_CREATEFAILED = 1; // for some reason CreateFile() didnt like something

type
  PDATABASELINK = ^TDATABASELINK;
  TDATABASELINK = record
    cbSize : int;
    szShortName:PAnsiChar;  // uniqie short database name
    szFullName:TChar;  // in English, auto-translated by the core
    {
      profile: pointer to a string which contains full path + name
      Affect: The database plugin should create the profile, the filepath will not exist at
        the time of this call, profile will be C:\..\<name>.dat
      Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_
    }
    makeDatabase : function (const profile:TChar):int; cdecl;
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
    grokHeader : function (const profile:TChar):int; cdecl;
    {
      Affect: Tell the database to create all services/hooks that a 3.xx legacy database might support into link,
        which is a DATABASELINK structure
      Returns: 0 on success, nonzero on failure
    }
    Load : function (const profile:TChar; bReadOnly:bool):PMIDatabase; cdecl;
    {
      Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
      Returns: 0 on success, nonzero on failure
      Note: Unload() might be called even if Load() was never called, wasLoaded is set to 1 if Load() was ever called.
    }
    Unload : function (db:PMIDatabase):int; cdecl;
    {
      Returns a pointer to the database checker or NULL if a database doesn't support checking
      When you don't need this object aanymore,  call its Destroy() method
    }
    CheckDB : function (const profile:PWideChar; error:pint):PMIDatabaseChecker;cdecl;
  end;

///////////////////////////////////////////////////////////////////////////////
// cache access functions

function db_get_contact(hContact:TMCONTACT):PDBCachedContact; stdcall;
                 external CoreDLL name 'db_get_contact';

///////////////////////////////////////////////////////////////////////////////
// Database list's services

const
{
  MS_DB_REGISTER_PLUGIN : registers a database plugin
  wParam : 0 (unused)
  lParam : DATABASELINK* = database link description
}
  MS_DB_REGISTER_PLUGIN:PAnsiChar = 'DB/RegisterPlugin';

{
  MS_DB_FIND_PLUGIN : looks for a database plugin suitable to open this file
  wParam : 0 (unused)
  lParam : const TCHAR* = name of the database file
  returns DATABASELINK* of the required plugin or NULL on error
}
  MS_DB_FIND_PLUGIN:PAnsiChar = 'DB/FindPlugin';

{
  MS_DB_GET_CURRENT : returns the database pointer for the current profile
  wParam : 0 (unused)
  lParam : 0 (unused)
  returns MIDatabase* of the current profile or NULL on error
}
  MS_DB_GET_CURRENT:PAnsiChar = 'DB/GetCurrentDb';

{
  MS_DB_INIT_INSTANCE : initializes a database instance
  wParam : 0 (unused)
  lParam : MIDatabase* = pointer to a database instance
  returns 0
}
  MS_DB_INIT_INSTANCE:PAnsiChar = 'DB/InitDbInstance';


{
  MS_DB_DESTROY_INSTANCE : destroys a database instance
  wParam : 0 (unused)
  lParam : MIDatabase* = pointer to a database instance
  returns 0
}
  MS_DB_DESTROY_INSTANCE:PAnsiChar = 'DB/DestroyDbInstance';

{$ENDIF}
