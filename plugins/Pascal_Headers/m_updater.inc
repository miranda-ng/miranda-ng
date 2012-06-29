{$IFNDEF M_UPDATER}
{$DEFINE M_UPDATER}

{
  if you set Update::szUpdateURL to the following value when registering, as
  well as setting your beta site and version data, updater will ignore
  szVersionURL and pbVersionPrefix, and attempt to find the file listing URL's
  from the backend XML data. for this to work, the plugin name in
  pluginInfo.shortName must match the file listing exactly (except for case)
}
const
  UPDATER_AUTOREGISTER = 'UpdaterAUTOREGISTER';

type
  PUpdate_tag = ^TUpdate_tag;
  TUpdate_tag = record
    cbSize          : int;
    szComponentName : PAnsiChar; // component name as it will appear in the UI
                             // (will be translated before displaying)
    szVersionURL    : PAnsiChar; // URL where the current version can be found (NULL to disable)

    pbVersionPrefix : PAnsiChar; // bytes occuring in VersionURL before the version,
                             // used to locate the version information within
                             // the URL data (not that this URL could point at
                             // a binary file - dunno why, but it could :)
    cpbVersionPrefix: int;   // number of bytes pionted to by pbVersionPrefix
    szUpdateURL     : PAnsiChar; // URL where dll/zip is located
    szBetaVersionURL: PAnsiChar; // URL where the beta version can be found (NULL to disable betas)

    pbBetaVersionPrefix:PAnsiChar; // bytes occuring in VersionURL before the
                               // version, used to locate the version
                               // information within the URL data
    cpbBetaVersionPrefix:int;  // number of bytes pointed to by pbVersionPrefix
    szBetaUpdateURL   : PAnsiChar; // URL where dll/zip is located
    pbVersion         : PAnsiChar; // bytes of current version, used for comparison
                               // with those in VersionURL
    cpbVersion        : int;   // number of bytes pionted to by pbVersion
    szBetaChangelogURL:PAnsiChar;  // url for displaying changelog for beta versions
  end;
  TUpdate = TUpdate_tag;
  PUpdate = ^TUpdate;

const
  {$IFNDEF WIN64}
  OLD_UPDATER_SIZE = SizeOf(TUpdate)-SizeOf(PAnsiChar);
  {$ELSE}
  OLD_UPDATER_SIZE = SizeOf(TUpdate);
  {$ENDIF}

const
  {
    register a comonent with the updater
    wparam = 0
    lparam = (LPARAM)&Update
  }
   MS_UPDATE_REGISTER:PAnsiChar = 'Update/Register';

  {
    register the 'easy' way - use this method if you have no beta URL and the
    plugin is on the miranda file listing
    NOTE: the plugin 'short name' in pluginInfo must match the name of the
    plugin on the file listing, exactly (including case) AND the plugin version
    string on the file listing must be the string version of the version in
    pluginInfo (i.e. 0.0.0.1, so no letters, brackets, etc.)

    wParam = (int)fileID - this is the file ID from the file listing
             (i.e. the number at the end of the download link)
    lParam = (PLUGININFO*)&pluginInfo
  }
  MS_UPDATE_REGISTERFL:PAnsiChar = 'Update/RegisterFL';

  {
    this function can be used to 'unregister' components - useful for plugins
    that register non-plugin/langpack components and may need to change those
    components on the fly
    lParam = (AnsiChar *)szComponentName
  }
  MS_UPDATE_UNREGISTER:PAnsiChar = 'Update/Unregister';

  {
    this event is fired when the startup process is complete, but NOT if a
    restart is imminent it is designed for status managment plugins to use as a
    trigger for beggining their own startup process
    wParam = lParam = 0 (unused)
    (added in version 0.1.6.0)
  }
  ME_UPDATE_STARTUPDONE:PAnsiChar = 'Update/StartupDone';

  {
    this service can be used to enable/disable Updater's global status control
    it can be called from the StartupDone event handler
    wParam = (BOOL)enable
    lParam = 0
    (added in version 0.1.6.0)
  }
  MS_UPDATE_ENABLESTATUSCONTROL:PAnsiChar = 'Update/EnableStatusControl';

  {
  An description of usage of the above service and event:
  Say you are a status control plugin that normally sets protocol or global statuses in your ModulesLoaded event handler.
  In order to make yourself 'Updater compatible', you would move the status control code from ModulesLoaded to another function, 
  say DoStartup. Then, in ModulesLoaded you would check for the existence of the MS_UPDATE_ENABLESTATUSCONTROL service.
  If it does not exist, call DoStartup. If it does exist, hook the ME_UPDATE_STARTUPDONE event and call DoStartup from there. You may
  also wish to call MS_UPDATE_ENABLESTATUSCONTROL with wParam == FALSE at this time, to disable Updater's own status control feature.

  this service can be used to determine whether updates are possible for a component with the given name
  wParam = 0
  lParam = (AnsiChar *)szComponentName
  returns TRUE if updates are supported, FALSE otherwise
  }
  MS_UPDATE_ISUPDATESUPPORTED:PAnsiChar = 'Update/IsUpdateSupported';

{
  An description of usage of the above service and event:
  Say you are a status control plugin that normally sets protocol or global
  statuses in your ModulesLoaded event handler.
  In order to make yourself 'updater compatible', you would move the status
  control code from ModulesLoaded to another function, say DoStartup. Then, in
  ModulesLoaded you would check for the existence of the MS_UPDATE_ENABLESTATUSCONTROL
  service. If it does not exist, call DoStartup. If it does exist, hook the
  ME_UPDATE_STARTUPDONE event and call DoStartup from there. You may also wish
  to call MS_UPDATE_ENABLESTATUSCONTROL with wParam == FALSE at this time, to
  disable Updater's own status control feature.
}

{$ENDIF}
