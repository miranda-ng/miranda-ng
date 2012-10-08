////////////////////////////////////////////////////////////////////////////////
// All code below is exclusively owned by author of Chess4Net - Pavel Perminov
// (packpaul@mail.ru, packpaul1@gmail.com).
// Any changes, modifications, borrowing and adaptation are a subject for
// explicit permition from the owner.

library Chess4Net_MI;
{*******************************
  plugin library for Miranda
********************************}

(*
{$IFDEF FASTMM4}
  FastMM4,
{$ENDIF}
*)

uses
{$IFDEF FASTMM4}
  FastMM4,
{$ENDIF}
  Forms,
  SysUtils,
  Windows,
  BitmapResUnit in '..\BitmapResUnit.pas',
  ChessBoardHeaderUnit in '..\ChessBoardHeaderUnit.pas',
  ChessBoardUnit in '..\ChessBoardUnit.pas' {ChessBoard},
  PosBaseChessBoardLayerUnit in '..\PosBaseChessBoardLayerUnit.pas',
  GameChessBoardUnit in '..\GameChessBoardUnit.pas' {GameChessBoard},
  ConnectingUnit in '..\ConnectingUnit.pas' {ConnectingForm},
  ConnectorUnit in 'ConnectorUnit.pas',
  ContinueUnit in '..\ContinueUnit.pas' {ContinueForm},
  ControlUnit in 'ControlUnit.pas',
  DialogUnit in '..\DialogUnit.pas',
  GameOptionsUnit in '..\GameOptionsUnit.pas' {GameOptionsForm},
  GlobalsLocalUnit in 'GlobalsLocalUnit.pas',
  GlobalsUnit in '..\GlobalsUnit.pas',
  InfoUnit in '..\InfoUnit.pas' {InfoForm},
  LocalizerUnit in '..\LocalizerUnit.pas',
  LookFeelOptionsUnit in '..\LookFeelOptionsUnit.pas' {OptionsForm},
  ManagerUnit in '..\ManagerUnit.pas' {Manager},
  MessageDialogUnit in '..\MessageDialogUnit.pas',
  ModalForm in '..\ModalForm.pas',
  PluginCommonUnit in 'PluginCommonUnit.pas',
  PosBaseUnit in '..\PosBaseUnit.pas',
  PromotionUnit in '..\PromotionUnit.pas' {PromotionForm},
  ChessRulesEngine in '..\ChessRulesEngine.pas',
  ManagerUnit.MI in 'ManagerUnit.MI.pas',
  TransmitGameSelectionUnit in 'TransmitGameSelectionUnit.pas' {TransmitGameSelectionForm},
  ChessClockUnit in '..\ChessClockUnit.pas',
  URLVersionQueryUnit in '..\URLVersionQueryUnit.pas',
  DontShowMessageDlgUnit in '..\DontShowMessageDlgUnit.pas',
  IniSettingsUnit in '..\IniSettingsUnit.pas',
  NonRefInterfacedObjectUnit in '..\NonRefInterfacedObjectUnit.pas';

{$R ..\Chess4Net.res}

begin
  ControlUnit.PLUGIN_NAME := 'Chess4Net';
  PLUGIN_MENU_NAME := '&Chess4Net';

  with _PluginInfo^ do
    begin
      shortName := 'Chess4Net';
      version := MakeMirandaPluginVersion(201,1,0,1); // 2010.0
      description := PLUGIN_PLAYING_OVER;
      author := 'Pavel Perminov';
      authorEmail := 'packpaul@mail.ru';
      copyright := '(c) 2007-2011 No Copyrights';
      homepage := 'http://www.chess4net.ru';
    end;

  guidPlugin := StringToGUID('{BF17C6E3-C52C-4CB8-88ED-E0FC5F5D566A}');
  miidPlugin := StringToGUID('{EBB410F6-E9AA-4F1B-8912-8C41E4EC0F90}'); // interface

  MirandaPluginMenuPosition := $7FFFFFFF; // or < $7FFFFFFF

  gCreatePluginInstance    := CreatePluginInstance;
  gInitializeControls      := InitializeControls;
  gDeinitializeControls    := DeinitializeControls;
  gErrorDuringPluginStart  := ErrorDuringPluginStart;
end.
