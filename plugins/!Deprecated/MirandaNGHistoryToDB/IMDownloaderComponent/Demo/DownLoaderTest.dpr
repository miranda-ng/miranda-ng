program DownLoaderTest;

uses
  Forms,
  DownLoaderTestUnit in 'DownLoaderTestUnit.pas' {MainForm};

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.Title := 'IM Downloader Demo';
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.
