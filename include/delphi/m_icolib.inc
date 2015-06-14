{$IFNDEF ICOLIB}
{$DEFINE ICOLIB}

type
// WARNING: do not use Translate(TS) for p(t)szSection or p(t)szDescription as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
  PSKINICONDESC = ^TSKINICONDESC;
  TSKINICONDESC = record
    szSection      :TChar; // [TRANSLATED-BY-CORE] section name used to group icons
    szDescription  :TChar; // [TRANSLATED-BY-CORE] description for options dialog
    pszName        :PAnsiChar; // name to refer to icon when playing and in db
                           // this name is miranda-wide. so use prefixes of
                           // your plugin: "isee_connect", "clist_delete", etc
    szDefaultFile  :TChar; // default icon file to use
    iDefaultIndex  :int;   // index of icon in default file
    // V2
    hDefaultIcon   :HICON; // handle to default icon
    // V3
    cx, cy         :int;   // dimensions of icon (if 0 then standard size icon (big and small options available)
    // new
    flags          :int;
  end;

const
  SIDF_SORTED       = 1;    // Icons in section are sorted by name
  SIDF_UNICODE      = $100; // Section and Description are in UCS-2
  SIDF_PATH_UNICODE = $200; // Default File is in UCS-2
  SIDF_ALL_UNICODE  = SIDF_PATH_UNICODE or SIDF_UNICODE;

{
  Add a icon into options UI
  returns a handle to the newly added item
}

function IcoLib_AddIcon(pIcon:PSKINICONDESC; iLang:int):THANDLE; stdcall;
                 external AppDLL name 'IcoLib_AddIcon';

{
  Remove a icon from options UI
  WARNING: This will invalidate all HICONs retrieved for specified pszName
}

procedure IcoLib_RemoveIcon(iconName:PAnsiChar); stdcall;
                 external AppDLL name 'IcoLib_RemoveIcon';
procedure IcoLib_RemoveIconByHandle(icon:THandle); stdcall;
                 external AppDLL name 'IcoLib_RemoveIconByHandle';

{
  Retrieve HICON with name specified in lParam
  Returned HICON SHOULDN'T be destroyed, it managed by IcoLib

  wParam = 0 - small 1 - big
  lParam = pszName
}

function IcoLib_GetIcon(iconName:PAnsiChar; big:byte) : HICON; stdcall;
                 external AppDLL name 'IcoLib_GetIcon';

{
  Retrieve HICON with HANDLE specified in lParam
  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib
}

function IcoLib_GetIconByHandle(icon:THandle; big:byte) : HICON; stdcall;
                 external AppDLL name 'IcoLib_GetIconByHandle';
{
  Retrieve an icolib handle for icon by name specified in lParam
}

function IcoLib_GetIconHandle(iconName:PAnsiChar):THandle; stdcall;
                 external AppDLL name 'IcoLib_GetIconHandle';

{
  Add reference to HICON

  wParam = (WPARAM)HICON
  lParam = 0 - small 1 - big
}

function IcoLib_AddRef(icon:HICON) : int; stdcall;
                 external AppDLL name 'IcoLib_AddRef';

{
  Retrieved HICON is not needed anymore (this helps optimize GDI usage)
}

function IcoLib_Release(iconName:PAnsiChar; big:byte) : int; stdcall;
                 external AppDLL name 'IcoLib_Release';
function IcoLib_ReleaseIcon(icon:THandle; big:byte) : int; stdcall;
                 external AppDLL name 'IcoLib_ReleaseIcon';

{
  Check whether HICON is managed by IcoLib
}

function IcoLib_IsManaged(icon:HICON) : THandle; stdcall;
                 external AppDLL name 'IcoLib_IsManaged';

{
  Icons change notification
}
const
  ME_SKIN2_ICONSCHANGED:PAnsiChar = 'Skin2/IconsChanged';

{$ENDIF}
