{ ################################################################################ }
{ #                                                                              # }
{ #  Обновление и установка набора программ IM-History - HistoryToDBUpdater v1.0 # }
{ #                                                                              # }
{ #  License: GPLv3                                                              # }
{ #                                                                              # }
{ #  Author: Grigorev Michael (icq: 161867489, email: sleuthhound@gmail.com)     # }
{ #                                                                              # }
{ ################################################################################ }

program HistoryToDBUpdater;

uses
  madExcept,
  madLinkDisAsm,
  madListHardware,
  madListProcesses,
  madListModules,
  Forms,
  Main in 'Main.pas' {MainForm},
  Global in 'Global.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.Title := 'HistoryToDBUpdater';
  Application.CreateForm(TMainForm, MainForm);
  if MainForm.RunAppDone then
    Application.Run;
end.
