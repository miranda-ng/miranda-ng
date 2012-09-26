unit PluginCommonUnit;

interface

uses
  ConnectorUnit,
  ControlUnit;

function CreatePluginInstance(Connector: TConnector): IMirandaPlugin;
procedure InitializeControls;
procedure DeinitializeControls;
procedure ErrorDuringPluginStart;

implementation

uses
  Windows, Forms, Dialogs, Graphics, SysUtils, Controls
  // plugin units
  , GlobalsLocalUnit, ManagerUnit.MI, ModalForm;

function CreatePluginInstance(Connector: TConnector): IMirandaPlugin;
begin
  Result := TManagerMIFactory.Create(Connector, ErrorDuringPluginStart);
end;


procedure InitializeControls;
begin
  MirandaPluginPath := MirandaPluginsPath + 'Chess4Net\';
  Chess4NetPath := MirandaPluginPath;
  if (not DirectoryExists(Chess4NetPath)) then
    CreateDir(Chess4NetPath);
  Chess4NetIcon := TIcon.Create;
  Chess4NetIcon.Handle := LoadIcon(hInstance, 'MAINICON');
  pluginIcon := Chess4NetIcon;

  MirandaPluginIcon := Chess4NetIcon;

  InitConnectorGlobals(MSG_INVITATION, PROMPT_HEAD, MSG_DATA_SEPARATOR);
end;


procedure DeinitializeControls;
begin
  StopAllPlugins;
  FreeAndNil(Chess4NetIcon);
end;


procedure ErrorDuringPluginStart;
begin
  TDialogs.ShowMessage('ERROR: Cannot start Chess4Net!');
end;

end.
