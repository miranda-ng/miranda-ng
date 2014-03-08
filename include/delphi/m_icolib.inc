{$IFNDEF ICOLIB}
{$DEFINE ICOLIB}

type
// WARNING: do not use Translate(TS) for p(t)szSection or p(t)szDescription as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
  PSKINICONDESC = ^TSKINICONDESC;
  TSKINICONDESC = record
    cbSize         :int;
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

const
{
  Add a icon into options UI

  wParam = 0
  lParam = PSKINICONDESC
  returns a handle to the newly added item
}
  MS_SKIN2_ADDICON:PAnsiChar = 'Skin2/Icons/AddIcon';

{
  Remove a icon from options UI

  wParam = (THANDLE)icon handle
  lParam = (PChar)icon name, one of them must be specified
  WARNING: This will invalidate all HICONs retrieved for specified pszName
}
  MS_SKIN2_REMOVEICON:PAnsiChar = 'Skin2/Icons/RemoveIcon';
{
  Retrieve HICON with name specified in lParam
  Returned HICON SHOULDN'T be destroyed, it managed by IcoLib

  wParam = 0 - small 1 - big
  lParam = pszName
}
  MS_SKIN2_GETICON :PAnsiChar = 'Skin2/Icons/GetIcon';

{
  Retrieve an icolib handle for icon by name specified in lParam

  wParam = 0
  lParam = (LPARAM)(char*)pszName
}
  MS_SKIN2_GETICONHANDLE:PAnsiChar = 'Skin2/Icons/GetIconHandle';

{
  Retrieve HICON with HANDLE specified in lParam
  Returned HICON SHOULDN'T be destroyed, it is managed by IcoLib

  wParam = (WPARAM)0 - small 1 - big
  lParam = (HANDLE)hIcoLibIcon
}
  MS_SKIN2_GETICONBYHANDLE:PAnsiChar = 'Skin2/Icons/GetIconByHandle';

{
  Add reference to HICON

  wParam = (WPARAM)HICON
  lParam = 0 - small 1 - big
}
  MS_SKIN2_ADDREFICON:PAnsiChar = 'Skin2/Icons/AddRef';

{
  Retrieved HICON is not needed anymore (this helps optimize GDI usage)

  wParam = HICON (optional)
  lParam = pszName (optional)  // at least one needs to be specified
}
  MS_SKIN2_RELEASEICON   :PAnsiChar = 'Skin2/Icons/ReleaseIcon';
  MS_SKIN2_RELEASEICONBIG:PAnsiChar = 'Skin2/Icons/ReleaseIconBig';

{
  Check whether HICON is managed by IcoLib

  wParam = (WPARAM)HICON
  lParam = 0
}
  MS_SKIN2_ISMANAGEDICON:PAnsiChar = 'Skin2/Icons/IsManaged';

{
  Icons change notification
}
  ME_SKIN2_ICONSCHANGED:PAnsiChar = 'Skin2/IconsChanged';

{$ENDIF}
