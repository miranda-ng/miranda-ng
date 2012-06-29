library testdll;

uses

    m_globaldefs, m_api, Windows;

    {$include m_helpers.inc}

    function MirandaPluginInfo(mirandaVersion: DWORD): PPLUGININFO; cdecl;
    begin
        Result := @PLUGININFO;
        PLUGININFO.cbSize := sizeof(TPLUGININFO);
        PLUGININFO.shortName := 'Plugin Template';
        PLUGININFO.version := PLUGIN_MAKE_VERSION(0,0,0,1);
        PLUGININFO.description := 'The long description of your plugin, to go in the plugin options dialog';
        PLUGININFO.author := 'J. Random Hacker';
        PLUGININFO.authorEmail := 'noreply@sourceforge.net';
        PLUGININFO.copyright := '(c) 2003 J. Random Hacker';
        PLUGININFO.homepage := 'http://miranda-icq.sourceforge.net/';
        PLUGININFO.isTransient := 0;
        PLUGININFO.replacesDefaultModule := 0;
    end;

    function PluginMenuCommand(wParam: WPARAM; lParam: LPARAM): Integer; cdecl;
    begin
        Result := 0;
        // this is called by Miranda, thus has to use the cdecl calling convention
        // all services and hooks need this.
        MessageBox(0, 'Just groovy, baby!', 'Plugin-o-rama', MB_OK);
    end;

    function Load(link: PPLUGINLINK): int; cdecl;
    var
        mi: TCListMenuItem;
    begin
        // this line is VERY VERY important, if it's not present, expect crashes.
        PLUGINLINK := Pointer(link);
        pluginLink^.CreateServiceFunction('TestPlug/MenuCommand', @PluginMenuCommand);
        FillChar(mi, sizeof(mi), 0);
        mi.cbSize := sizeof(mi);
        mi.position := $7FFFFFFF;
        mi.flags := 0;
        mi.hIcon := LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
        mi.pszName := '&Test Plugin...';
        mi.pszService := 'TestPlug/MenuCommand';
        pluginLink^.CallService(MS_CLIST_ADDMAINMENUITEM, 0, lParam(@mi));
        Result := 0;
    end;

    function Unload: int; cdecl;
    begin
        Result := 0;
    end;

    exports

        MirandaPluginInfo, Load, Unload;

begin
end.
